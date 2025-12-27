// Copyright PlayKit. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Interfaces/IHttpRequest.h"
#include "Engine/Texture2D.h"
#include "PlayKitTypes.h"
#include "PlayKitImageClient.generated.h"

/**
 * PlayKit Image Client
 * Provides AI image generation functionality.
 *
 * Features:
 * - Single image generation
 * - Batch image generation
 * - Various size options
 * - Base64 to Texture2D conversion
 *
 * Usage:
 * UPlayKitImageClient* ImageClient = UPlayKitBlueprintLibrary::CreateImageClient();
 * ImageClient->OnImageGenerated.AddDynamic(this, &AMyActor::HandleImage);
 * ImageClient->GenerateImage("A beautiful sunset over mountains");
 */
UCLASS(BlueprintType)
class PLAYKITSDK_API UPlayKitImageClient : public UObject
{
	GENERATED_BODY()

public:
	UPlayKitImageClient();

	/** Initialize the client with a model name */
	void Initialize(const FString& InModelName);

	//========== Events ==========//

	/** Fired when a single image is generated */
	UPROPERTY(BlueprintAssignable, Category="PlayKit|Image")
	FOnImageGenerated OnImageGenerated;

	/** Fired when multiple images are generated */
	UPROPERTY(BlueprintAssignable, Category="PlayKit|Image")
	FOnImagesGenerated OnImagesGenerated;

	/** Fired on error */
	UPROPERTY(BlueprintAssignable, Category="PlayKit|Image")
	FOnImageError OnError;

	//========== Properties ==========//

	/** Get the model name this client is using */
	UFUNCTION(BlueprintPure, Category="PlayKit|Image")
	FString GetModelName() const { return ModelName; }

	/** Check if a request is currently in progress */
	UFUNCTION(BlueprintPure, Category="PlayKit|Image")
	bool IsProcessing() const { return bIsProcessing; }

	//========== Image Generation ==========//

	/**
	 * Generate a single image from a text prompt.
	 * @param Prompt Text description of the desired image
	 * @param Size Image size (e.g., "1024x1024")
	 */
	UFUNCTION(BlueprintCallable, Category="PlayKit|Image", meta=(DisplayName="Generate Image"))
	void GenerateImage(const FString& Prompt, const FString& Size = TEXT("1024x1024"));

	/**
	 * Generate a single image with seed for reproducibility.
	 * @param Prompt Text description of the desired image
	 * @param Size Image size
	 * @param Seed Seed for reproducible results (-1 for random)
	 */
	UFUNCTION(BlueprintCallable, Category="PlayKit|Image", meta=(DisplayName="Generate Image With Seed"))
	void GenerateImageWithSeed(const FString& Prompt, const FString& Size, int32 Seed);

	/**
	 * Generate multiple images from a text prompt.
	 * @param Prompt Text description of the desired images
	 * @param Options Generation options
	 */
	UFUNCTION(BlueprintCallable, Category="PlayKit|Image", meta=(DisplayName="Generate Images"))
	void GenerateImages(const FString& Prompt, const FPlayKitImageOptions& Options);

	//========== Utility ==========//

	/**
	 * Convert Base64 encoded image data to a Texture2D.
	 * @param Base64Data Base64 encoded image data
	 * @return The created texture, or nullptr on failure
	 */
	UFUNCTION(BlueprintCallable, Category="PlayKit|Image|Utility", meta=(DisplayName="Base64 to Texture2D"))
	static UTexture2D* Base64ToTexture2D(const FString& Base64Data);

	/** Cancel any in-progress request */
	UFUNCTION(BlueprintCallable, Category="PlayKit|Image")
	void CancelRequest();

private:
	void SendImageRequest(const FString& Prompt, const FPlayKitImageOptions& Options);
	void HandleImageResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> CreateAuthenticatedRequest(const FString& Url);
	void BroadcastError(const FString& ErrorCode, const FString& ErrorMessage);

private:
	FString ModelName;
	bool bIsProcessing = false;
	FString LastPrompt;

	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> CurrentRequest;
};
