// Copyright PlayKit. All Rights Reserved.

#include "PlayKitBlueprintLibrary.h"
#include "PlayKitSettings.h"
#include "Client/PlayKitChatClient.h"
#include "Client/PlayKitImageClient.h"
#include "Client/PlayKitSTTClient.h"
#include "Client/PlayKitPlayerClient.h"
#include "NPC/PlayKitNPCClient.h"

#define PLAYKIT_VERSION TEXT("0.2.0")

bool UPlayKitBlueprintLibrary::IsReady()
{
	UPlayKitSettings* Settings = UPlayKitSettings::Get();
	if (!Settings)
	{
		return false;
	}

	// Check if we have a valid Game ID and authentication
	return !Settings->GameId.IsEmpty() && (Settings->HasDeveloperToken() || !Settings->GetPlayerToken().IsEmpty());
}

FString UPlayKitBlueprintLibrary::GetVersion()
{
	return PLAYKIT_VERSION;
}

UPlayKitChatClient* UPlayKitBlueprintLibrary::CreateChatClient(const FString& ModelName)
{
	UPlayKitSettings* Settings = UPlayKitSettings::Get();
	if (!Settings)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayKit] Settings not found. Please configure PlayKit in Project Settings."));
		return nullptr;
	}

	if (Settings->GameId.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayKit] Game ID not configured. Please set it in Project Settings > PlayKit SDK."));
		return nullptr;
	}

	UPlayKitChatClient* Client = NewObject<UPlayKitChatClient>(GetTransientPackage(), NAME_None, RF_Transient);
	FString Model = ModelName.IsEmpty() ? Settings->DefaultChatModel : ModelName;
	Client->Initialize(Model);
	return Client;
}

UPlayKitImageClient* UPlayKitBlueprintLibrary::CreateImageClient(const FString& ModelName)
{
	UPlayKitSettings* Settings = UPlayKitSettings::Get();
	if (!Settings)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayKit] Settings not found. Please configure PlayKit in Project Settings."));
		return nullptr;
	}

	if (Settings->GameId.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayKit] Game ID not configured. Please set it in Project Settings > PlayKit SDK."));
		return nullptr;
	}

	UPlayKitImageClient* Client = NewObject<UPlayKitImageClient>(GetTransientPackage(), NAME_None, RF_Transient);
	FString Model = ModelName.IsEmpty() ? Settings->DefaultImageModel : ModelName;
	Client->Initialize(Model);
	return Client;
}

UPlayKitSTTClient* UPlayKitBlueprintLibrary::CreateSTTClient(const FString& ModelName)
{
	UPlayKitSettings* Settings = UPlayKitSettings::Get();
	if (!Settings)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayKit] Settings not found. Please configure PlayKit in Project Settings."));
		return nullptr;
	}

	if (Settings->GameId.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayKit] Game ID not configured. Please set it in Project Settings > PlayKit SDK."));
		return nullptr;
	}

	UPlayKitSTTClient* Client = NewObject<UPlayKitSTTClient>(GetTransientPackage(), NAME_None, RF_Transient);
	Client->Initialize(ModelName);
	return Client;
}

UPlayKitPlayerClient* UPlayKitBlueprintLibrary::GetPlayerClient()
{
	// Return singleton instance
	return UPlayKitPlayerClient::Get();
}

void UPlayKitBlueprintLibrary::SetupNPC(UPlayKitNPCClient* NPCClient, const FString& ModelName)
{
	if (!NPCClient)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayKit] NPCClient is null"));
		return;
	}

	UPlayKitSettings* Settings = UPlayKitSettings::Get();
	if (!Settings)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayKit] Settings not found. Please configure PlayKit in Project Settings."));
		return;
	}

	FString Model = ModelName.IsEmpty() ? Settings->DefaultChatModel : ModelName;
	NPCClient->Setup(Model);
}

FString UPlayKitBlueprintLibrary::GetAuthToken()
{
	UPlayKitSettings* Settings = UPlayKitSettings::Get();
	if (!Settings)
	{
		return TEXT("");
	}

	// Prefer developer token, fallback to player token
	if (Settings->HasDeveloperToken() && !Settings->bIgnoreDeveloperToken)
	{
		return Settings->GetDeveloperToken();
	}

	return Settings->GetPlayerToken();
}

bool UPlayKitBlueprintLibrary::IsAuthenticated()
{
	return !GetAuthToken().IsEmpty();
}

FString UPlayKitBlueprintLibrary::GetGameId()
{
	UPlayKitSettings* Settings = UPlayKitSettings::Get();
	return Settings ? Settings->GameId : TEXT("");
}

FString UPlayKitBlueprintLibrary::GetBaseUrl()
{
	UPlayKitSettings* Settings = UPlayKitSettings::Get();
	return Settings ? Settings->GetBaseUrl() : TEXT("https://playkit.ai");
}
