#pragma once
#include <Shared.h>

class Trampoline
{
private:
	static inline bool (*SetGameModeInternal)(UWorld*, FURL&);
	static inline void (*TickFlushInternal)(UNetDriver*, float);
public:
	static void Initialize()
	{
		MH_Initialize();

		MH_CreateHook(reinterpret_cast<PVOID>(ModuleBase + 0x2709580), TickFlush, reinterpret_cast<PVOID*>(&TickFlushInternal));

		auto SetGameModeAddress = reinterpret_cast<PVOID>(ModuleBase + 0x29EA530);

		if (SetGameModeAddress)
		{
			Memory::PatchCall(reinterpret_cast<PVOID>(ModuleBase + 0x299050A), SetGameModeAddress);
			MH_CreateHook(SetGameModeAddress, SetGameMode, reinterpret_cast<PVOID*>(&SetGameModeInternal));
		}

		MH_EnableHook(MH_ALL_HOOKS);
	}
private:
	static inline void TickFlush(UNetDriver* NetDriver, float DeltaSeconds)
	{
		if (NetDriver->ClientConnections.Num() > 0)
		{
			if (auto ReplicationDriver = NetDriver->ReplicationDriver)
				reinterpret_cast<void(*)(UReplicationDriver*)>(NetDriver->ReplicationDriver->VTable[0x56])(NetDriver->ReplicationDriver);

			auto StartAircraftKey = FKey();
			StartAircraftKey.KeyName = UKismetStringLibrary::Conv_StringToName(L"F3");
			if (auto LocalPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
			{
				if (LocalPlayerController->WasInputKeyJustPressed(StartAircraftKey))
				{ 
					UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), FString(L"startaircraft"), LocalPlayerController);
				}
			}
		}

		TickFlushInternal(NetDriver, DeltaSeconds);
	}

	static inline auto BroadcastNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure FailureType, const FString& Error = FString(TEXT("")))
	{
		return reinterpret_cast<void(*)(UEngine * Engine, UWorld * World, UNetDriver * NetDriver, ENetworkFailure FailureType, const FString & Error)>(GEngine->VTable[0x48])(GEngine, World, NetDriver, FailureType, Error);
	}

	static inline bool Listen(UWorld* World, FURL& InURL)
	{
		if (World->NetDriver)
		{
			BroadcastNetworkFailure(World, World->NetDriver, ENetworkFailure::NetDriverAlreadyExists);
			return false;
		}

		auto SetWorld = reinterpret_cast<void(*)(UNetDriver * NetDriver, UWorld * World)>(ModuleBase + 0x27080E0);
		auto FindCollectionByType = reinterpret_cast<FLevelCollection * (*)(UWorld * World, ELevelCollectionType LevelCollectionType)>(ModuleBase + 0x29DB200);

		auto GameNetDriver = UKismetStringLibrary::Conv_StringToName(FString(L"GameNetDriver"));
		if (reinterpret_cast<bool(*)(UEngine * Engine, UWorld * World, FName NetDriverName, FName NetDriverDefinition)>(ModuleBase + 0x2975CB0)(GEngine, World, GameNetDriver, GameNetDriver))
		{
			World->NetDriver = reinterpret_cast<UNetDriver * (*)(UEngine * Engine, UWorld*, FName NetDriverName)>(ModuleBase + 0x2980C70)(GEngine, World, GameNetDriver);
			SetWorld(World->NetDriver, World);

			FLevelCollection* SourceCollection = FindCollectionByType(World, ELevelCollectionType::DynamicSourceLevels);
			if (SourceCollection)
			{
				SourceCollection->NetDriver = World->NetDriver;
			}

			FLevelCollection* StaticCollection = FindCollectionByType(World, ELevelCollectionType::StaticLevels);
			if (StaticCollection)
			{
				StaticCollection->NetDriver = World->NetDriver;
			}
		}

		if (World->NetDriver == nullptr)
		{
			BroadcastNetworkFailure(World, NULL, ENetworkFailure::NetDriverCreateFailure);
			return false;
		}

		const bool bReuseAddressAndPort = false;

		FString Error;
		if (!reinterpret_cast<bool(*)(UNetDriver * NetDriver, void* InNotify, FURL & LocalURL, bool bReuseAddressAndPort, FString & Error)>(World->NetDriver->VTable[0x4A])(World->NetDriver, reinterpret_cast<UWorld*>(reinterpret_cast<uintptr_t>(World) + sizeof(UObject)), InURL, bReuseAddressAndPort, Error))
		{
			BroadcastNetworkFailure(World, World->NetDriver, ENetworkFailure::NetDriverListenFailure, Error);
			LOG(LogWorld, Log, "Failed to listen: %s", Error.ToString());
			reinterpret_cast<void(*)(UEngine * Engine, UWorld * World, FName NetDriverName)>(ModuleBase + 0x2979D30)(GEngine, World, GameNetDriver);
			World->NetDriver = nullptr;

			FLevelCollection* SourceCollection = FindCollectionByType(World, ELevelCollectionType::DynamicSourceLevels);
			if (SourceCollection) 
			{
				SourceCollection->NetDriver = nullptr;
			}

			FLevelCollection* StaticCollection = FindCollectionByType(World, ELevelCollectionType::StaticLevels);
			if (StaticCollection)
			{
				StaticCollection->NetDriver = nullptr;
			}

			return false;
		}

		if ((World->NetDriver->MaxInternetClientRate < World->NetDriver->MaxClientRate) && (World->NetDriver->MaxInternetClientRate > 2500))
		{
			World->NetDriver->MaxClientRate = World->NetDriver->MaxInternetClientRate;
		}

		return true;
	}

	static inline bool SetGameMode(UWorld* World, FURL& InURL)
	{
		if (!World->AuthorityGameMode)
		{
			return SetGameModeInternal(World, InURL);
		}
		else
		{
			return Listen(World, InURL);
		}

		return false;
	}
};
