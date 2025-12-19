// Copyright Agentland. All Rights Reserved.


#include "Chat.h"

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"


// Sets default values for this component's properties
UChat::UChat()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UChat::BeginPlay()
{
	Super::BeginPlay();
	// ...
	
}

void UChat::OnPlayKitResponding(const FHttpRequestPtr& _Request)
{
	const FHttpResponsePtr response = _Request->GetResponse();

	//不存在就说明请求失败
	if(!response.IsValid())
	{
		FString logText = "[PlayKitChat]:\nRequest failed";
		UE_LOG(LogTemp, Log, TEXT("%s"), *logText);
		return ;
	}

	//获取返回的字符串格式的数据
	FString responseString = response->GetContentAsString();
	
	//如果不是stream模式，直接解析数据
	if( !PlayKitChatStream )
	{
		ParsePlayKitResponse(responseString);
		return ;
	}
	
	//如果是streaming模式，一条条解析
	TArray<FString> dataBlocks;
	
	//按照回车分割字符串
	responseString.ParseIntoArray(dataBlocks, TEXT("\n"), true);
	UE_LOG(LogTemp, Log, TEXT("%s"), *responseString);
}

void UChat::OnPlayKitResponded()
{
	auto s =  LastCompletedResponse.content;
	
	UE_LOG(LogTemp, Log, TEXT("[PlayKitChatLastCompletedResponse]:\nResponse: %s"), *s);

	//广播【响应后】委托
	OnPlayKitChatResponded.Broadcast(LastCompletedResponse);
}

void UChat::ParsePlayKitResponse(const FString& _Response)
{
	TSharedPtr<FJsonObject> JsonObject;
	if ( !StringToJsonObject(_Response, JsonObject, true) )
	{
		return;
	}
	
	FPlayKitChatResponse PlayKitChatResponse;
	
	PlayKitChatResponse.id = JsonObject->GetStringField(TEXT("id"));
	PlayKitChatResponse.object = JsonObject->GetStringField(TEXT("object"));
	PlayKitChatResponse.created = JsonObject->GetStringField(TEXT("created"));
	PlayKitChatResponse.model = JsonObject->GetStringField(TEXT("model"));
	
	TSharedPtr<FJsonObject> ReplyObject = JsonObject->GetArrayField(TEXT("choices"))[0]->AsObject()->GetObjectField(TEXT("message"));
	PlayKitChatResponse.role = ReplyObject->GetStringField(TEXT("role"));
	PlayKitChatResponse.content = ReplyObject->GetStringField(TEXT("content"));
	PlayKitChatResponse.finsih_reason = ReplyObject->GetStringField(TEXT("finsih_reason"));
	this->AddContext(FChatContext(PlayKitChatResponse.role , PlayKitChatResponse.content));
	
	TSharedPtr<FJsonObject> UsageObject = JsonObject->GetObjectField(TEXT("usage"));
	PlayKitChatResponse.prompt_tokens = UsageObject->GetNumberField(TEXT("prompt_tokens"));
	PlayKitChatResponse.completion_tokens = UsageObject->GetNumberField(TEXT("completion_tokens"));
	PlayKitChatResponse.total_tokens = UsageObject->GetNumberField(TEXT("total_tokens"));
	
	this->LastCompletedResponse = PlayKitChatResponse;
	
	//广播【响应时】委托
	OnPlayKitChatResponding.Broadcast(PlayKitChatResponse);
}


// Called every frame
void UChat::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UChat::InitPrompt(const FString& Prompt,const FString& _Game_ID)
{
	this->Game_ID = _Game_ID;
	this->SystemPrompt = Prompt;
	this->ChatURL = FString::Format(TEXT("https://playkit.agentlandlab.com/ai/{0}/v1/chat"), {this->Game_ID});
	
	
	UE_LOG(LogTemp, Display, TEXT("SystemPrompt: %s"), *this->SystemPrompt);
	
	
	if (!this->AllContext.IsEmpty())
	{
		this->AllContext[0].content = this->SystemPrompt;
	}
	else
	{
		this->AllContext.Add(FChatContext(TEXT("system") , this->SystemPrompt));
	}
	
}

void UChat::ChatToAI(
		const FString& AuthToken,const FString& Message,const FString& Model,double temperature ,bool stream)
{
	////////////////////////////////// 设置输入和加到上下文 //////////////////////////////////
	
	TSharedPtr<FJsonObject> ContentObject = MakeShareable(new FJsonObject);
	ContentObject->SetStringField(TEXT("message"), Message);
	
	FChatContext CurrentContext;
	CurrentContext.role = "user";
	CurrentContext.content = JsonObjectToString(ContentObject);
	UE_LOG(LogTemp, Warning, TEXT("Content:\n%s"), *CurrentContext.content);
	this->AddContext(CurrentContext);
	
	OnPlayKitChatTalkTo.Broadcast("UserName","ChatName", CurrentContext.content);
	
	////////////////////////////////// 设置请求的内容 ////////////////////////////////
	
	CurrentHttpRequest = FHttpModule::Get().CreateRequest();
	
	CurrentHttpRequest->SetURL(ChatURL);
	CurrentHttpRequest->SetVerb(TEXT("POST"));
	
	//设置请求头
	FString authorization = "Bearer " + AuthToken;
	CurrentHttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	CurrentHttpRequest->SetHeader(TEXT("Authorization"), authorization);
	
	//设置请求体
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	
	//设置模型
	JsonObject->SetStringField(TEXT("model"), Model);
	
	//添加上下文和提示词
	TArray<TSharedPtr<FJsonValue>> ContextObject;
	for (auto context : this->AllContext)
	{
		TSharedPtr<FJsonObject> MessageObject = MakeShareable(new FJsonObject());
		MessageObject->SetStringField("role", context.role);
		MessageObject->SetStringField("content", context.content);
		ContextObject.Add(MakeShareable(new FJsonValueObject(MessageObject)));
	}
	JsonObject->SetArrayField(TEXT("messages"), ContextObject);
	UE_LOG(LogTemp, Warning, TEXT("PromptAndContext：%s"), *JsonObjectToString(JsonObject, true));
	
	//设置其它大模型属性
	JsonObject->SetNumberField("temperature", temperature);
	JsonObject->SetBoolField("stream", stream);
	this->PlayKitChatStream = stream;
	
	UE_LOG(LogTemp, Warning, TEXT("RequestBody:\n%s"), *JsonObjectToString(JsonObject, true));
	
	CurrentHttpRequest->SetContentAsString(JsonObjectToString(JsonObject));
	
	
	////////////////////////////////// 绑定Dify【响应时】的回调 ////////////////////////////////
	
	CurrentHttpRequest->OnRequestProgress64().BindLambda(
	[WeakThis = TWeakObjectPtr<UChat>(this)]
		(const FHttpRequestPtr& _Request, uint64 _BytesSent, uint64 _BytesReceived)
	{
		if(!_Request.IsValid())
		{
			return;
		}
		UE_LOG(LogTemp, Log, TEXT("BytesSent: %llu, BytesReceived: %llu"), _BytesSent, _BytesReceived);

		if(WeakThis.IsValid())
			WeakThis->OnPlayKitResponding(_Request);
	});
	
	////////////////////////////////// 绑定Dify【响应后】的回调 ////////////////////////////////

	CurrentHttpRequest->OnProcessRequestComplete().BindLambda(
		[WeakThis = TWeakObjectPtr<UChat>(this)]
		(FHttpRequestPtr _Request, FHttpResponsePtr _Response, bool bWasSuccessful)
	{
		if(!_Response.IsValid() || !_Request.IsValid())
		{
			//不存在就说明请求失败
			FString logText = "[PlayKitChatError]:\nRequest failed";
			UE_LOG(LogTemp, Log, TEXT("%s"), *logText);
			if(WeakThis.IsValid())
				WeakThis->OnPlayKitResponded();
			return;
		}
			
			
		const int responseCode = _Response->GetResponseCode();
		// 只有代码为200才是正常响应
		if(responseCode != 200) 
		{
			FString logText = "[PlayKitChatError]:\nCode:" + FString::FromInt(responseCode);
			logText+= "\n" + _Response->GetContentAsString();
			//输出报错
			UE_LOG(LogTemp, Error, TEXT("%s"), *logText);
		}

		if(WeakThis.IsValid())
			WeakThis->OnPlayKitResponded();
	});
	
	////////////////////////////////// 发送请求 ////////////////////////////////

	CurrentHttpRequest->ProcessRequest();
	
}

FString UChat::JsonObjectToString(const TSharedPtr<FJsonObject>& JsonObject, bool bPrettyPrint)
{
	if (!JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("JsonObjectToString: Invalid JSON object"));
		return FString();
	}

	FString OutputString;
    
	if (bPrettyPrint)
	{
		// 使用缩进格式化输出
		TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer = 
			TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&OutputString);
		FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	}
	else
	{
		// 紧凑输出
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
		FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	}
    
	return OutputString;
}

bool UChat::StringToJsonObject(const FString& JsonString, TSharedPtr<FJsonObject>& OutJsonObject, bool bLogErrors)
{
	if (JsonString.IsEmpty())
	{
		if (bLogErrors)
		{
			UE_LOG(LogTemp, Warning, TEXT("StringToJsonObject: Input string is empty"));
		}
		return false;
	}

	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    
	if (FJsonSerializer::Deserialize(Reader, OutJsonObject) && OutJsonObject.IsValid())
	{
		return true;
	}
	else
	{
		if (bLogErrors)
		{
			UE_LOG(LogTemp, Error, TEXT("StringToJsonObject: Failed to parse JSON string: %s"), *JsonString);
		}
		return false;
	}
}

