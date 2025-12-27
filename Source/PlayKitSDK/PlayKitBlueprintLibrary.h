// Copyright PlayKit. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PlayKitTypes.h"
#include "PlayKitBlueprintLibrary.generated.h"

class UPlayKitChatClient;
class UPlayKitImageClient;
class UPlayKitSTTClient;
class UPlayKitPlayerClient;
class UPlayKitNPCClient;

/**
 * PlayKit Blueprint Function Library
 * Provides static functions to access PlayKit AI services from Blueprints.
 *
 * Usage:
 * 1. Ensure your game is configured in Project Settings > Plugins > PlayKit SDK
 * 2. Use CreateChatClient/CreateImageClient to get AI clients
 * 3. Call methods on the returned clients
 */
UCLASS()
class PLAYKITSDK_API UPlayKitBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	//========== SDK State ==========//

	/** Check if the SDK is properly configured and ready to use */
	UFUNCTION(BlueprintPure, Category="PlayKit|SDK", meta=(DisplayName="Is PlayKit Ready"))
	static bool IsReady();

	/** Get the current SDK version */
	UFUNCTION(BlueprintPure, Category="PlayKit|SDK", meta=(DisplayName="Get SDK Version"))
	static FString GetVersion();

	//========== Client Factory ==========//

	/**
	 * Create a Chat Client for text generation and conversations.
	 * @param ModelName Optional model override. If empty, uses default from settings.
	 * @return A new Chat Client instance
	 */
	UFUNCTION(BlueprintCallable, Category="PlayKit|Factory", meta=(DisplayName="Create Chat Client"))
	static UPlayKitChatClient* CreateChatClient(const FString& ModelName = TEXT(""));

	/**
	 * Create an Image Client for AI image generation.
	 * @param ModelName Optional model override. If empty, uses default from settings.
	 * @return A new Image Client instance
	 */
	UFUNCTION(BlueprintCallable, Category="PlayKit|Factory", meta=(DisplayName="Create Image Client"))
	static UPlayKitImageClient* CreateImageClient(const FString& ModelName = TEXT(""));

	/**
	 * Create a Speech-to-Text Client for audio transcription.
	 * @param ModelName Model to use (e.g., "whisper-1")
	 * @return A new STT Client instance
	 */
	UFUNCTION(BlueprintCallable, Category="PlayKit|Factory", meta=(DisplayName="Create STT Client"))
	static UPlayKitSTTClient* CreateSTTClient(const FString& ModelName = TEXT("whisper-1"));

	/**
	 * Get the Player Client for user info and credits management.
	 * @return The Player Client singleton
	 */
	UFUNCTION(BlueprintCallable, Category="PlayKit|Factory", meta=(DisplayName="Get Player Client"))
	static UPlayKitPlayerClient* GetPlayerClient();

	//========== NPC Setup ==========//

	/**
	 * Initialize an NPC Client component with the SDK.
	 * Call this after adding a PlayKitNPCClient component to your actor.
	 * @param NPCClient The NPC Client component to initialize
	 * @param ModelName Optional model override. If empty, uses default from settings.
	 */
	UFUNCTION(BlueprintCallable, Category="PlayKit|NPC", meta=(DisplayName="Setup NPC"))
	static void SetupNPC(UPlayKitNPCClient* NPCClient, const FString& ModelName = TEXT(""));

	//========== Utility ==========//

	/** Get the current authentication token (developer or player token) */
	UFUNCTION(BlueprintPure, Category="PlayKit|Auth", meta=(DisplayName="Get Auth Token"))
	static FString GetAuthToken();

	/** Check if user is authenticated */
	UFUNCTION(BlueprintPure, Category="PlayKit|Auth", meta=(DisplayName="Is Authenticated"))
	static bool IsAuthenticated();

	/** Get the configured Game ID */
	UFUNCTION(BlueprintPure, Category="PlayKit|Settings", meta=(DisplayName="Get Game ID"))
	static FString GetGameId();

	/** Get the API base URL */
	UFUNCTION(BlueprintPure, Category="PlayKit|Settings", meta=(DisplayName="Get Base URL"))
	static FString GetBaseUrl();
};
