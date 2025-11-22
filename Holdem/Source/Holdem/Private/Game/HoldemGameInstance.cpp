// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/HoldemGameInstance.h"

#include <string>

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

void UHoldemGameInstance::Init()
{
	Super::Init();

	// 현재 사용하는 서브시스템을 가져오자
	IOnlineSubsystem* subsys = Online::GetSubsystem(GetWorld());
	if (subsys)
	{
		// 서브시스템의 인터페이스를 가져오자.
		sessionInterface = subsys->GetSessionInterface();
		// 세션 생성 성공시 호출되는 함수 등록
		sessionInterface->OnCreateSessionCompleteDelegates.AddUObject(
			this, &UHoldemGameInstance::OnCreateSessionComplete);
		// 세션 조회 성공시 호출되는 함수 등록
		sessionInterface->OnFindSessionsCompleteDelegates.AddUObject(
			this, &UHoldemGameInstance::OnFindSessionComplete);
		// 세션 참여 성공시 호출되는 함수 등록
		sessionInterface->OnJoinSessionCompleteDelegates.AddUObject(
			this, &UHoldemGameInstance::OnJoinSessionComplete);
		// 세션 파괴 성공시 호출되는 함수 등록                                                    
		sessionInterface->OnDestroySessionCompleteDelegates.AddUObject(
			this, &UHoldemGameInstance::OnDestroySessionComplete);
	}
}

void UHoldemGameInstance::CreateMySession(FString displayName)
{
	// 세션을 만들기 위한 옵션 담을 변수
	FOnlineSessionSettings sessionSettings;
	// 현재 사용중인 서브시스템 이름 가져오자.
	FName subsysName = Online::GetSubsystem(GetWorld())->GetSubsystemName();
	// 만약에 서브시스템이 이름이 NULL 이면 Lan 을 이용하게 설정
	sessionSettings.bIsLANMatch = subsysName.IsEqual(FName(TEXT("NULL")));
	UE_LOG(LogTemp, Warning, TEXT("서브시스템 : %s"), *subsysName.ToString());

	// Steam 에선 필수 (bUseLobbiesIfAvailable, bUsesPresence)
	// Lobby 사용 여부 설정
	sessionSettings.bUseLobbiesIfAvailable = true;
	// 친구 상태를 확인할 수 있는 여부
	sessionSettings.bUsesPresence = true;

	// 세션 검색 허용 여부
	sessionSettings.bShouldAdvertise = true;
	// 세션 최대 참여 인원 설정 (임시로 6명)
	sessionSettings.NumPublicConnections = 6;
	// 커스텀 정보
	// displayName 을 Base64 로 변환
	displayName = StringBase64Encode(displayName);
	sessionSettings.Set(FName("DP_NAME"), displayName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	//sessionSettings.Set(FName("MAP_IDX"), 1, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// sessionSettings 이용해서 세션 생성
	FUniqueNetIdPtr netId =
		GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();
	sessionInterface->CreateSession(*netId, FName(displayName), sessionSettings);
	//sessionInterface->CreateSession(0, FName(displayName), sessionSettings);

	currSessionName = FName(displayName);
}

void UHoldemGameInstance::OnCreateSessionComplete(FName sessionName,
	bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] 세션 생성 성공"), *sessionName.ToString());
		// 게임 맵 으로 이동
		GetWorld()->ServerTravel(TEXT("/Game/Maps/GameMap?listen"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] 세션 생성 실패"), *sessionName.ToString());
	}
}

void UHoldemGameInstance::FindOtherSession()
{
	UE_LOG(LogTemp, Warning, TEXT("세션 조회 시작"));
	// sessionSearch 만들자.
	sessionSearch = MakeShared<FOnlineSessionSearch>();
	// 현재 사용중인 서브시스템 이름 가져오자.
	FName subsysName = Online::GetSubsystem(GetWorld())->GetSubsystemName();
	// 만약에 서브시스템이 이름이 NULL 이면 Lan 을 이용하게 설정
	sessionSearch->bIsLanQuery = subsysName.IsEqual(FName(TEXT("NULL")));
	// 어떤 옵션을 기준으로 검색
	sessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	//sessionSearch->QuerySettings.Set(FName("DP_NAME"), FName("Wanted"), EOnlineComparisonOp::Equals);
	// 검색 갯수
	sessionSearch->MaxSearchResults = 100;
	// 위 설정을 가지고 세션 검색
	sessionInterface->FindSessions(0, sessionSearch.ToSharedRef());
}

void UHoldemGameInstance::OnFindSessionComplete(bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("세션 조회 끝"));
	if (bWasSuccessful)
	{
		// 검색된 세션 결과들
		auto results = sessionSearch->SearchResults;
		for (int32 i = 0; i < results.Num(); i++)
		{
			// 방 제목 이름 담을 변수
			FString displayName;
			results[i].Session.SessionSettings.Get(FName(TEXT("DP_NAME")), displayName);
			// displayName 을 UTF8 string 으로 변환
			displayName = StringBase64Decode(displayName);

			// 플레이어 수
			int32 MaxPlayers = results[i].Session.SessionSettings.NumPublicConnections;
			// NumOpenPublicConnections가 스팀에서만 정상 출력된다는 말이 있음
			// [https://jeongsupark.tistory.com/50]
			int32 OpenSlots = results[i].Session.NumOpenPublicConnections;
			int32 CurrentPlayers = MaxPlayers - OpenSlots;

			UE_LOG(LogTemp, Warning, TEXT("세션 %d - MaxPlayers: %d, OpenSlots: %d"),
					 i, MaxPlayers, OpenSlots);
			
			UE_LOG(LogTemp, Warning, TEXT("세션 - %i, 이름 : %s"), i, *displayName);
			// onFindComplete 에 들어있는 함수 실행
			onFindComplete.ExecuteIfBound(i, displayName, CurrentPlayers, MaxPlayers);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("세션 조회 실패"));
	}

	onFindComplete.ExecuteIfBound(-1, FString(), 0, 0);
}

void UHoldemGameInstance::JoinOtherSession(int32 sessionIdx)
{
	// 검색된 세션 결과들
	auto results = sessionSearch->SearchResults;
	// 5.5 이후 부터 바꼈다...
	results[sessionIdx].Session.SessionSettings.bUseLobbiesIfAvailable = true;
	results[sessionIdx].Session.SessionSettings.bUsesPresence = true;

	// 세션 이름 가져오자.
	FString displayName;
	results[sessionIdx].Session.SessionSettings.Get(FName(TEXT("DP_NAME")), displayName);

	// 세션 참여
	sessionInterface->JoinSession(0, FName(displayName), results[sessionIdx]);
}

void UHoldemGameInstance::OnJoinSessionComplete(FName sessionName,
	EOnJoinSessionCompleteResult::Type result)
{
	// 만약 참여 성공 했다면
	if (result == EOnJoinSessionCompleteResult::Success)
	{
		// 서버가 만들어 놓은 세션 URL 을 얻어오자.k
		FString url;
		sessionInterface->GetResolvedConnectString(sessionName, url);
		UE_LOG(LogTemp, Warning, TEXT("URL : %s"), *url);
		// 서버가 있는 맵으로 이동 (최초 1번)
		APlayerController* pc = GetWorld()->GetFirstPlayerController();
		pc->ClientTravel(url, TRAVEL_Absolute);
	}
}

FString UHoldemGameInstance::StringBase64Encode(FString str)
{
	// str 을 std::string 로 변환
	std::string utf8String = TCHAR_TO_UTF8(*str);
	// utf8String 을 uint8 의 Array 변환
	TArray<uint8> arrayData = TArray<uint8>((uint8*)utf8String.c_str(), utf8String.length());

	return FBase64::Encode(arrayData);
}

FString UHoldemGameInstance::StringBase64Decode(FString str)
{
	TArray<uint8> arrayData;
	FBase64::Decode(str, arrayData);
	std::string utf8String((char*)arrayData.GetData(), arrayData.Num());
	return UTF8_TO_TCHAR(utf8String.c_str());
}

void UHoldemGameInstance::SetSelectCharacter(FString userName, int32 characterIdx)
{
	selectCharacter.Add(userName, characterIdx);
}

int32 UHoldemGameInstance::GetSelectCharacter(FString userName)
{
	int32* value = selectCharacter.Find(userName);
	if (value == nullptr) return -1;

	return *value;
}

void UHoldemGameInstance::DestroyMySession()
{
	if (sessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] 세션 파괴 시작"), *currSessionName.ToString());
		sessionInterface->DestroySession(currSessionName);
	}
}

void UHoldemGameInstance::OnDestroySessionComplete(FName sessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] 세션 파괴 성공"), *sessionName.ToString());
		// 메인 메뉴로 이동                                                                                                                                     
		GetWorld()->GetFirstPlayerController()->ClientTravel("/Game/Maps/MainMenu", ETravelType::TRAVEL_Absolute);
	}
	else                                                                                                                                                            
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] 세션 파괴 실패"), *sessionName.ToString());
	}
}

void UHoldemGameInstance::LeaveSession()
{
	if (sessionInterface.IsValid())
	{
		FNamedOnlineSession* Session = sessionInterface->GetNamedSession(currSessionName);
		if (Session)
		{
			// 문제 있음 - 세션 파괴 안되고 바로 메인으로 나가버림
			// 다시 해당 세션 조인이 안됨
			UE_LOG(LogTemp, Warning, TEXT("클라이언트 세션 나가기"));
			sessionInterface->DestroySession(currSessionName);
		}
	}

	// 메인 메뉴로 이동
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
			UE_LOG(LogTemp, Warning, TEXT("메인 메뉴로 이동"));
			GetWorld()->GetFirstPlayerController()->ClientTravel("/Game/Maps/MainMenu", ETravelType::TRAVEL_Absolute);
	}, 0.5f, false);
}
