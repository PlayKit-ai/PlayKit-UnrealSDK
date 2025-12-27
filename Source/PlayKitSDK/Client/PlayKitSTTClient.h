// Copyright PlayKit. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Interfaces/IHttpRequest.h"
#include "PlayKitTypes.h"
#include "PlayKitSTTClient.generated.h"

/**
 * PlayKit Speech-to-Text Client
 * Provides audio transcription functionality.
 *
 * Features:
 * - Transcribe audio files
 * - Transcribe audio data from memory
 * - Multiple language support
 * - Timestamp segments
 *
 * Usage:
 * UPlayKitSTTClient* STTClient = UPlayKitBlueprintLibrary::CreateSTTClient("whisper-1");
 * STTClient->OnTranscriptionComplete.AddDynamic(this, &AMyActor::HandleTranscription);
 * STTClient->TranscribeFile("/path/to/audio.wav");
 */
UCLASS(BlueprintType)
class PLAYKITSDK_API UPlayKitSTTClient : public UObject
{
	GENERATED_BODY()

public:
	UPlayKitSTTClient();

	/** Initialize the client with a model name */
	void Initialize(const FString& InModelName);

	//========== Events ==========//

	/** Fired when transcription completes */
	UPROPERTY(BlueprintAssignable, Category="PlayKit|STT")
	FOnTranscriptionComplete OnTranscriptionComplete;

	/** Fired on error */
	UPROPERTY(BlueprintAssignable, Category="PlayKit|STT")
	FOnTranscriptionError OnError;

	//========== Properties ==========//

	/** Get the model name this client is using */
	UFUNCTION(BlueprintPure, Category="PlayKit|STT")
	FString GetModelName() const { return ModelName; }

	/** Check if a request is currently in progress */
	UFUNCTION(BlueprintPure, Category="PlayKit|STT")
	bool IsProcessing() const { return bIsProcessing; }

	//========== Transcription ==========//

	/**
	 * Transcribe an audio file.
	 * @param FilePath Path to the audio file
	 * @param Language Optional language hint (e.g., "en", "zh", "ja")
	 */
	UFUNCTION(BlueprintCallable, Category="PlayKit|STT", meta=(DisplayName="Transcribe File"))
	void TranscribeFile(const FString& FilePath, const FString& Language = TEXT(""));

	/**
	 * Transcribe audio data from memory.
	 * @param AudioData Raw audio data (WAV, MP3, etc.)
	 * @param FileName Filename hint for format detection
	 * @param Language Optional language hint
	 */
	UFUNCTION(BlueprintCallable, Category="PlayKit|STT", meta=(DisplayName="Transcribe Audio Data"))
	void TranscribeAudioData(const TArray<uint8>& AudioData, const FString& FileName = TEXT("audio.wav"), const FString& Language = TEXT(""));

	/** Cancel any in-progress request */
	UFUNCTION(BlueprintCallable, Category="PlayKit|STT")
	void CancelRequest();

private:
	void SendTranscriptionRequest(const TArray<uint8>& AudioData, const FString& FileName, const FString& Language);
	void HandleTranscriptionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> CreateAuthenticatedRequest(const FString& Url);
	void BroadcastError(const FString& ErrorCode, const FString& ErrorMessage);

private:
	FString ModelName;
	bool bIsProcessing = false;

	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> CurrentRequest;
};
