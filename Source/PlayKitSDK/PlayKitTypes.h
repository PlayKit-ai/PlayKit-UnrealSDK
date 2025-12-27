// Copyright PlayKit. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PlayKitTypes.generated.h"

//========== Chat Types ==========//

/**
 * Chat message for conversations
 */
USTRUCT(BlueprintType)
struct PLAYKITSDK_API FPlayKitChatMessage
{
	GENERATED_BODY()

	/** Message role: "system", "user", "assistant", or "tool" */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PlayKit")
	FString Role;

	/** Message content */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PlayKit")
	FString Content;

	/** Tool call ID (for tool responses) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PlayKit")
	FString ToolCallId;

	FPlayKitChatMessage() {}
	FPlayKitChatMessage(const FString& InRole, const FString& InContent)
		: Role(InRole), Content(InContent) {}
};

/**
 * Tool call from the AI
 */
USTRUCT(BlueprintType)
struct PLAYKITSDK_API FPlayKitToolCall
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	FString Id;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	FString Type;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	FString FunctionName;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	FString FunctionArguments;
};

/**
 * Chat completion response
 */
USTRUCT(BlueprintType)
struct PLAYKITSDK_API FPlayKitChatResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	FString Content;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	FString FinishReason;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	TArray<FPlayKitToolCall> ToolCalls;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	int32 PromptTokens = 0;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	int32 CompletionTokens = 0;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	int32 TotalTokens = 0;
};

/**
 * Chat configuration
 */
USTRUCT(BlueprintType)
struct PLAYKITSDK_API FPlayKitChatConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PlayKit")
	TArray<FPlayKitChatMessage> Messages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PlayKit", meta=(ClampMin="0.0", ClampMax="2.0"))
	float Temperature = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PlayKit")
	int32 MaxTokens = 0; // 0 = no limit
};

//========== Image Types ==========//

/**
 * Generated image result
 */
USTRUCT(BlueprintType)
struct PLAYKITSDK_API FPlayKitGeneratedImage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	bool bSuccess = false;

	/** Base64 encoded image data */
	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	FString ImageBase64;

	/** Original prompt used for generation */
	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	FString OriginalPrompt;

	/** Revised prompt (if modified by AI) */
	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	FString RevisedPrompt;

	/** When the image was generated */
	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	FDateTime GeneratedAt;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	FString ErrorMessage;
};

/**
 * Image generation options
 */
USTRUCT(BlueprintType)
struct PLAYKITSDK_API FPlayKitImageOptions
{
	GENERATED_BODY()

	/** Image size (e.g., "1024x1024", "1792x1024") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PlayKit")
	FString Size = TEXT("1024x1024");

	/** Number of images to generate (1-10) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PlayKit", meta=(ClampMin="1", ClampMax="10"))
	int32 Count = 1;

	/** Optional seed for reproducible results */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PlayKit")
	int32 Seed = -1; // -1 = no seed
};

//========== Transcription Types ==========//

/**
 * Transcription segment with timestamps
 */
USTRUCT(BlueprintType)
struct PLAYKITSDK_API FPlayKitTranscriptionSegment
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	float Start = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	float End = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	FString Text;
};

/**
 * Transcription result
 */
USTRUCT(BlueprintType)
struct PLAYKITSDK_API FPlayKitTranscriptionResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	FString Text;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	FString Language;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	float DurationSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	TArray<FPlayKitTranscriptionSegment> Segments;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	FString ErrorMessage;
};

//========== Player Types ==========//

/**
 * Player information
 */
USTRUCT(BlueprintType)
struct PLAYKITSDK_API FPlayKitPlayerInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	FString UserId;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	float Credits = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	FString Nickname;
};

/**
 * Daily credits refresh result
 */
USTRUCT(BlueprintType)
struct PLAYKITSDK_API FPlayKitDailyCreditsResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	bool bRefreshed = false;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	float BalanceBefore = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	float BalanceAfter = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="PlayKit")
	float AmountAdded = 0.0f;
};

//========== Common Delegates ==========//

// Chat delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChatResponse, FPlayKitChatResponse, Response);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChatStreamChunk, const FString&, Chunk);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChatStreamComplete, const FString&, FullContent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChatError, const FString&, ErrorCode, const FString&, ErrorMessage);

// Image delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnImageGenerated, FPlayKitGeneratedImage, Image);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnImagesGenerated, const TArray<FPlayKitGeneratedImage>&, Images);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnImageError, const FString&, ErrorCode, const FString&, ErrorMessage);

// Transcription delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTranscriptionComplete, FPlayKitTranscriptionResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTranscriptionError, const FString&, ErrorCode, const FString&, ErrorMessage);

// Player delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerInfoUpdated, FPlayKitPlayerInfo, PlayerInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerTokenReceived, const FString&, PlayerToken);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDailyCreditsRefreshed, FPlayKitDailyCreditsResult, Result);
