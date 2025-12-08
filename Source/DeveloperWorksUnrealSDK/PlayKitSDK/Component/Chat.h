// Copyright Agentland. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/IHttpRequest.h"
#include "Chat.generated.h"


//PlayKit返回的数据结构
USTRUCT(BlueprintType)
struct FPlayKitChatResponse
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString object;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString created;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString model;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString role;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString content;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString finsih_reason;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 prompt_tokens;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 completion_tokens;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 total_tokens;
};

//对话上下文结构
USTRUCT(BlueprintType)
struct FChatContext
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString role;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString content;
};

//委托,在[playkit返回后]
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayKitChatRespondedDelegate, FPlayKitChatResponse, Response);

//委托,在[playkit返回时]
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayKitChatRespondingDelegate, FPlayKitChatResponse, Response);

//委托,在[向playkit发送数据后]
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPlayKitChatTalkToDelegate, FString, UserName , FString, ChatName ,FString, Message);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DEVELOPERWORKSUNREALSDK_API UChat : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UChat();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	//收到PlayKit响应时的回调
	void OnPlayKitResponding(const FHttpRequestPtr& _Request);
	
	//收到PlayKit响应后的回调
	void OnPlayKitResponded();
	
	// 解析PlayKit返回的数据
	void ParsePlayKitResponse(const FString& _Response);

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
public:
	UFUNCTION(BlueprintCallable)
	void InitPrompt(const FString& Prompt, const FString& Type, const FString& Function);
	
	UFUNCTION(BlueprintCallable)
	void ClearContext(){ this->AllContext.Empty(); }; 
	
	UFUNCTION(BlueprintCallable)
	void ChatToAI(
		const FString& AuthToken,
		const FString& Message,
		bool IsMeet,
		const FString& Model = "deepseek-chat",
		double temperature = 0.7,
		bool stream = false);
	
public://===================== 委托 ========================//

	//------------------------------------------
	// 委托：PlayKit回应时触发
	//
	// 参数：
	//	- Response：此次响应新增的内容,若是blocking模式则为完整内容
	//------------------------------------------
	UPROPERTY(BlueprintAssignable, Category = "PlayKitAPI|委托")
	FPlayKitChatRespondingDelegate OnPlayKitChatResponding;

	//------------------------------------------
	// 委托：PlayKit回应结束后触发
	//
	// 参数：
	//	- Response：此次响应的完整内容
	//------------------------------------------
	UPROPERTY(BlueprintAssignable, Category = "PlayKitAPI|委托")
	FPlayKitChatRespondedDelegate OnPlayKitChatResponded;
	
	
	//-----------------------------------------------------
	// 委托：向 PlayKit 发送数据后触发
	//
	// 参数：
	//	- UserName：与PlayKit对话的用户名称
	//	- ChatName：PlayKit的名字
	//	- Message：要发送的消息
	//-----------------------------------------------------
	UPROPERTY(BlueprintAssignable, Category = "PlayKitAPI|委托")
	FPlayKitChatTalkToDelegate OnPlayKitChatTalkTo;
	
private:
	FString SystemPrompt;
	TArray<FChatContext> AllContext;
	FString Game_ID = "8230bfea-509c-40d9-b961-f3361ee6fdde";
	FString ChatURL = FString::Format(TEXT("https://playkit.agentlandlab.com/ai/{0}/v1/chat"), {Game_ID});
	
	FPlayKitChatResponse LastCompletedResponse;
	bool PlayKitChatStream;
	
	//当前的Http请求
	TSharedPtr<IHttpRequest,ESPMode::ThreadSafe> CurrentHttpRequest;
	
	void AddContext(const FChatContext& Context)
	{
		this->AllContext.Add(Context);
	};
	
	static  FString JsonObjectToString(const TSharedPtr<FJsonObject>& JsonObject, bool bPrettyPrint = false);
	
	static bool StringToJsonObject(const FString& JsonString, TSharedPtr<FJsonObject>& OutJsonObject, bool bLogErrors = true);
};
