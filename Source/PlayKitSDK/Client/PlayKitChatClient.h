// Copyright PlayKit. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Interfaces/IHttpRequest.h"
#include "PlayKitTypes.h"
#include "PlayKitChatClient.generated.h"

/**
 * PlayKit Chat Client
 * Provides AI text generation and chat functionality.
 *
 * Features:
 * - Text generation (non-streaming)
 * - Streaming text generation
 * - Structured output generation
 * - Tool calling support
 *
 * Usage:
 * UPlayKitChatClient* ChatClient = UPlayKitBlueprintLibrary::CreateChatClient();
 * ChatClient->OnChatResponse.AddDynamic(this, &AMyActor::HandleChatResponse);
 * ChatClient->TextGeneration(Config);
 */
UCLASS(BlueprintType)
class PLAYKITSDK_API UPlayKitChatClient : public UObject
{
	GENERATED_BODY()

public:
	UPlayKitChatClient();

	/** Initialize the client with a model name */
	void Initialize(const FString& InModelName);

	//========== Events ==========//

	/** Fired when chat response is received (non-streaming) */
	UPROPERTY(BlueprintAssignable, Category="PlayKit|Chat")
	FOnChatResponse OnChatResponse;

	/** Fired for each chunk in streaming mode */
	UPROPERTY(BlueprintAssignable, Category="PlayKit|Chat")
	FOnChatStreamChunk OnStreamChunk;

	/** Fired when streaming completes with full content */
	UPROPERTY(BlueprintAssignable, Category="PlayKit|Chat")
	FOnChatStreamComplete OnStreamComplete;

	/** Fired on error */
	UPROPERTY(BlueprintAssignable, Category="PlayKit|Chat")
	FOnChatError OnError;

	//========== Properties ==========//

	/** Get the model name this client is using */
	UFUNCTION(BlueprintPure, Category="PlayKit|Chat")
	FString GetModelName() const { return ModelName; }

	/** Check if a request is currently in progress */
	UFUNCTION(BlueprintPure, Category="PlayKit|Chat")
	bool IsProcessing() const { return bIsProcessing; }

	//========== Text Generation ==========//

	/**
	 * Generate text from a simple prompt.
	 * @param Prompt The user's message
	 * @param Temperature Temperature for response (0.0-2.0)
	 */
	UFUNCTION(BlueprintCallable, Category="PlayKit|Chat", meta=(DisplayName="Text Generation (Simple)"))
	void TextGenerationSimple(const FString& Prompt, float Temperature = 0.7f);

	/**
	 * Generate text with full configuration.
	 * @param Config Chat configuration with messages and settings
	 */
	UFUNCTION(BlueprintCallable, Category="PlayKit|Chat", meta=(DisplayName="Text Generation"))
	void TextGeneration(const FPlayKitChatConfig& Config);

	/**
	 * Generate text with streaming response.
	 * @param Prompt The user's message
	 * @param Temperature Temperature for response (0.0-2.0)
	 */
	UFUNCTION(BlueprintCallable, Category="PlayKit|Chat", meta=(DisplayName="Text Chat Stream (Simple)"))
	void TextChatStreamSimple(const FString& Prompt, float Temperature = 0.7f);

	/**
	 * Generate text with streaming response and full configuration.
	 * @param Config Chat configuration with messages and settings
	 */
	UFUNCTION(BlueprintCallable, Category="PlayKit|Chat", meta=(DisplayName="Text Chat Stream"))
	void TextChatStream(const FPlayKitChatConfig& Config);

	//========== Structured Output ==========//

	/**
	 * Generate a structured JSON object based on a schema.
	 * The response will be a valid JSON object matching your schema.
	 * @param Prompt The generation prompt
	 * @param SchemaJson JSON schema defining the output structure
	 * @param SystemMessage Optional system message
	 * @param Temperature Temperature for response
	 */
	UFUNCTION(BlueprintCallable, Category="PlayKit|Chat|Structured", meta=(DisplayName="Generate Structured"))
	void GenerateStructured(
		const FString& Prompt,
		const FString& SchemaJson,
		const FString& SystemMessage = TEXT(""),
		float Temperature = 0.7f);

	/** Delegate for structured output response */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStructuredResponse, bool, bSuccess, const FString&, JsonResult);

	UPROPERTY(BlueprintAssignable, Category="PlayKit|Chat|Structured")
	FOnStructuredResponse OnStructuredResponse;

	//========== Cancel ==========//

	/** Cancel any in-progress request */
	UFUNCTION(BlueprintCallable, Category="PlayKit|Chat")
	void CancelRequest();

private:
	void SendChatRequest(const FPlayKitChatConfig& Config, bool bStream);
	void HandleChatResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void HandleStreamProgress(FHttpRequestPtr Request, uint64 BytesSent, uint64 BytesReceived);
	void HandleStreamComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void HandleStructuredResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	FString BuildRequestUrl() const;
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> CreateAuthenticatedRequest(const FString& Url);
	FPlayKitChatResponse ParseChatResponse(const FString& ResponseContent);
	void BroadcastError(const FString& ErrorCode, const FString& ErrorMessage);

private:
	FString ModelName;
	bool bIsProcessing = false;

	// Streaming state
	FString StreamBuffer;
	FString AccumulatedContent;
	int32 LastProcessedOffset = 0;

	// Current request
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> CurrentRequest;
};
