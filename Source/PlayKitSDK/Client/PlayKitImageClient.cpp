// Copyright PlayKit. All Rights Reserved.

#include "PlayKitImageClient.h"
#include "PlayKitSettings.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Misc/Base64.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"

UPlayKitImageClient::UPlayKitImageClient()
{
}

void UPlayKitImageClient::Initialize(const FString& InModelName)
{
	ModelName = InModelName;
	UE_LOG(LogTemp, Log, TEXT("[PlayKit] ImageClient initialized with model: %s"), *ModelName);
}

TSharedRef<IHttpRequest, ESPMode::ThreadSafe> UPlayKitImageClient::CreateAuthenticatedRequest(const FString& Url)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	UPlayKitSettings* Settings = UPlayKitSettings::Get();
	if (Settings)
	{
		FString Token = Settings->HasDeveloperToken() && !Settings->bIgnoreDeveloperToken
			? Settings->GetDeveloperToken()
			: Settings->GetPlayerToken();
		if (!Token.IsEmpty())
		{
			Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Token));
		}
	}

	return Request;
}

void UPlayKitImageClient::GenerateImage(const FString& Prompt, const FString& Size)
{
	FPlayKitImageOptions Options;
	Options.Size = Size;
	Options.Count = 1;
	Options.Seed = -1;
	SendImageRequest(Prompt, Options);
}

void UPlayKitImageClient::GenerateImageWithSeed(const FString& Prompt, const FString& Size, int32 Seed)
{
	FPlayKitImageOptions Options;
	Options.Size = Size;
	Options.Count = 1;
	Options.Seed = Seed;
	SendImageRequest(Prompt, Options);
}

void UPlayKitImageClient::GenerateImages(const FString& Prompt, const FPlayKitImageOptions& Options)
{
	SendImageRequest(Prompt, Options);
}

void UPlayKitImageClient::SendImageRequest(const FString& Prompt, const FPlayKitImageOptions& Options)
{
	if (bIsProcessing)
	{
		BroadcastError(TEXT("REQUEST_IN_PROGRESS"), TEXT("A request is already in progress"));
		return;
	}

	if (Prompt.IsEmpty())
	{
		BroadcastError(TEXT("INVALID_PROMPT"), TEXT("Prompt cannot be empty"));
		return;
	}

	UPlayKitSettings* Settings = UPlayKitSettings::Get();
	if (!Settings)
	{
		BroadcastError(TEXT("CONFIG_ERROR"), TEXT("Settings not found"));
		return;
	}

	FString Url = FString::Printf(TEXT("%s/ai/%s/v1/image"), *Settings->GetBaseUrl(), *Settings->GameId);

	bIsProcessing = true;
	LastPrompt = Prompt;

	// Build request body
	TSharedPtr<FJsonObject> RequestBody = MakeShared<FJsonObject>();
	RequestBody->SetStringField(TEXT("model"), ModelName);
	RequestBody->SetStringField(TEXT("prompt"), Prompt);
	RequestBody->SetNumberField(TEXT("n"), FMath::Clamp(Options.Count, 1, 10));
	RequestBody->SetStringField(TEXT("size"), Options.Size);
	RequestBody->SetStringField(TEXT("response_format"), TEXT("b64_json"));

	if (Options.Seed >= 0)
	{
		RequestBody->SetNumberField(TEXT("seed"), Options.Seed);
	}

	// Serialize
	FString RequestBodyStr;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBodyStr);
	FJsonSerializer::Serialize(RequestBody.ToSharedRef(), Writer);

	CurrentRequest = CreateAuthenticatedRequest(Url);
	CurrentRequest->SetContentAsString(RequestBodyStr);
	CurrentRequest->OnProcessRequestComplete().BindUObject(this, &UPlayKitImageClient::HandleImageResponse);

	UE_LOG(LogTemp, Log, TEXT("[PlayKit] Sending image request to: %s"), *Url);
	CurrentRequest->ProcessRequest();
}

void UPlayKitImageClient::HandleImageResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	bIsProcessing = false;
	CurrentRequest.Reset();

	if (!bWasSuccessful || !Response.IsValid())
	{
		BroadcastError(TEXT("NETWORK_ERROR"), TEXT("Network request failed"));
		return;
	}

	int32 ResponseCode = Response->GetResponseCode();
	FString ResponseContent = Response->GetContentAsString();

	if (ResponseCode < 200 || ResponseCode >= 300)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayKit] Image error %d: %s"), ResponseCode, *ResponseContent);
		BroadcastError(FString::FromInt(ResponseCode), ResponseContent);
		return;
	}

	// Parse response
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		BroadcastError(TEXT("PARSE_ERROR"), TEXT("Failed to parse response"));
		return;
	}

	TArray<FPlayKitGeneratedImage> Results;
	int64 Created = 0;
	JsonObject->TryGetNumberField(TEXT("created"), Created);

	const TArray<TSharedPtr<FJsonValue>>* DataArray;
	if (JsonObject->TryGetArrayField(TEXT("data"), DataArray))
	{
		for (const TSharedPtr<FJsonValue>& DataValue : *DataArray)
		{
			TSharedPtr<FJsonObject> DataObj = DataValue->AsObject();
			if (DataObj)
			{
				FPlayKitGeneratedImage Image;
				Image.bSuccess = true;
				Image.OriginalPrompt = LastPrompt;
				Image.GeneratedAt = FDateTime::FromUnixTimestamp(Created);

				DataObj->TryGetStringField(TEXT("b64_json"), Image.ImageBase64);
				DataObj->TryGetStringField(TEXT("revised_prompt"), Image.RevisedPrompt);

				Results.Add(Image);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[PlayKit] Generated %d images"), Results.Num());

	// Broadcast results
	if (Results.Num() == 1)
	{
		OnImageGenerated.Broadcast(Results[0]);
	}
	OnImagesGenerated.Broadcast(Results);
}

UTexture2D* UPlayKitImageClient::Base64ToTexture2D(const FString& Base64Data)
{
	if (Base64Data.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayKit] Base64 data is empty"));
		return nullptr;
	}

	TArray<uint8> DecodedData;
	if (!FBase64::Decode(Base64Data, DecodedData))
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayKit] Failed to decode base64 data"));
		return nullptr;
	}

	// Use ImageWrapper to load the image
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	if (!ImageWrapper.IsValid())
	{
		ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
	}

	if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(DecodedData.GetData(), DecodedData.Num()))
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayKit] Failed to decompress image data"));
		return nullptr;
	}

	TArray<uint8> RawData;
	if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData))
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayKit] Failed to get raw image data"));
		return nullptr;
	}

	int32 Width = ImageWrapper->GetWidth();
	int32 Height = ImageWrapper->GetHeight();

	UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
	if (!Texture)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayKit] Failed to create texture"));
		return nullptr;
	}

	void* TextureData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, RawData.GetData(), RawData.Num());
	Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
	Texture->UpdateResource();

	return Texture;
}

void UPlayKitImageClient::CancelRequest()
{
	if (CurrentRequest.IsValid())
	{
		CurrentRequest->CancelRequest();
		CurrentRequest.Reset();
	}
	bIsProcessing = false;
}

void UPlayKitImageClient::BroadcastError(const FString& ErrorCode, const FString& ErrorMessage)
{
	UE_LOG(LogTemp, Error, TEXT("[PlayKit] Image error [%s]: %s"), *ErrorCode, *ErrorMessage);
	OnError.Broadcast(ErrorCode, ErrorMessage);

	// Also broadcast a failed image result
	FPlayKitGeneratedImage FailedImage;
	FailedImage.bSuccess = false;
	FailedImage.ErrorMessage = ErrorMessage;
	OnImageGenerated.Broadcast(FailedImage);
}
