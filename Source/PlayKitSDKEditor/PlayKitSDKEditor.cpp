// Copyright PlayKit. All Rights Reserved.

#include "PlayKitSDKEditor.h"
#include "PlayKitSettings.h"
#include "PlayKitSettingsWindow.h"
#include "ToolMenus.h"
#include "ISettingsModule.h"

#define LOCTEXT_NAMESPACE "FPlayKitSDKEditorModule"

void FPlayKitSDKEditorModule::StartupModule()
{
	// Register settings
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings(
			"Project",
			"Plugins",
			"PlayKitSDK",
			LOCTEXT("PlayKitSettingsName", "PlayKit SDK"),
			LOCTEXT("PlayKitSettingsDescription", "Configure PlayKit SDK settings"),
			GetMutableDefault<UPlayKitSettings>()
		);
	}

	// Register menus after ToolMenus is ready
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(
		this, &FPlayKitSDKEditorModule::RegisterMenus));

	UE_LOG(LogTemp, Log, TEXT("[PlayKitSDKEditor] Module started"));
}

void FPlayKitSDKEditorModule::ShutdownModule()
{
	// Unregister settings
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "PlayKitSDK");
	}

	// Unregister menus
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	UE_LOG(LogTemp, Log, TEXT("[PlayKitSDKEditor] Module shutdown"));
}

void FPlayKitSDKEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	// Add PlayKit menu to the main menu bar
	UToolMenu* MainMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu");
	FToolMenuSection& Section = MainMenu->AddSection("PlayKitSection", LOCTEXT("PlayKitMenu", "PlayKit SDK"));

	UToolMenu* PlayKitMenu = UToolMenus::Get()->RegisterMenu("LevelEditor.MainMenu.PlayKitSDK");

	Section.AddSubMenu(
		"PlayKitSDK",
		LOCTEXT("PlayKitMenuLabel", "PlayKit SDK"),
		LOCTEXT("PlayKitMenuTooltip", "PlayKit SDK tools and settings"),
		FNewToolMenuDelegate::CreateLambda([](UToolMenu* Menu)
		{
			FToolMenuSection& MenuSection = Menu->AddSection("PlayKitSection");

			// Settings
			MenuSection.AddMenuEntry(
				"Settings",
				LOCTEXT("SettingsLabel", "Settings"),
				LOCTEXT("SettingsTooltip", "Open PlayKit SDK settings window"),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Settings"),
				FUIAction(FExecuteAction::CreateStatic(&FPlayKitSettingsWindow::Open))
			);

			MenuSection.AddSeparator("Separator1");

			// Documentation
			MenuSection.AddMenuEntry(
				"Documentation",
				LOCTEXT("DocsLabel", "Documentation"),
				LOCTEXT("DocsTooltip", "Open PlayKit documentation"),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Documentation"),
				FUIAction(FExecuteAction::CreateLambda([]()
				{
					FPlatformProcess::LaunchURL(TEXT("https://docs.playkit.ai"), nullptr, nullptr);
				}))
			);

			// Website
			MenuSection.AddMenuEntry(
				"Website",
				LOCTEXT("WebsiteLabel", "PlayKit Website"),
				LOCTEXT("WebsiteTooltip", "Visit PlayKit website"),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.OpenInBrowser"),
				FUIAction(FExecuteAction::CreateLambda([]()
				{
					FPlatformProcess::LaunchURL(TEXT("https://playkit.ai"), nullptr, nullptr);
				}))
			);
		})
	);
}

void FPlayKitSDKEditorModule::OpenSettingsWindow()
{
	FPlayKitSettingsWindow::Open();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPlayKitSDKEditorModule, PlayKitSDKEditor)
