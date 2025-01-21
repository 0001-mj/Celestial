#include <Shared.h>

class Native
{
private:
	static inline bool (*ReadyToStartMatchInternal)(AGameMode*);
	static inline void (*HandleStartingNewPlayerInternal)(AGameModeBase*, APlayerController*);
	static inline void (*ServerAcknowledgePossessionInternal)(AFortPlayerController*);
public:
	static void Initialize()
	{
		auto FortGameModeAthenaDefault = AFortGameModeAthena::StaticClass()->DefaultObject;
		Memory::SwapVirtualFunction(FortGameModeAthenaDefault, 0xFB, ReadyToStartMatch, reinterpret_cast<PVOID*>(&ReadyToStartMatchInternal));
		Memory::SwapVirtualFunction(FortGameModeAthenaDefault, 0xC8, HandleStartingNewPlayer, reinterpret_cast<PVOID*>(&HandleStartingNewPlayerInternal));
		Memory::SwapVirtualFunction(FortGameModeAthenaDefault, 0xC2, SpawnDefaultPawnFor);

		auto PlayerControllerAthenaDefault = AFortPlayerControllerAthena::StaticClass()->DefaultObject;
		Memory::SwapVirtualFunction(PlayerControllerAthenaDefault, 0x105, ServerAcknowledgePossession, reinterpret_cast<PVOID*>(&ServerAcknowledgePossessionInternal));
	}
private:
	static inline bool ReadyToStartMatch(AGameMode* GameMode)
	{
		static bool bReadyToStartMatch = false;

		if (!bReadyToStartMatch)
		{
			bReadyToStartMatch = true;

			auto FortGameStateAthena = Cast<AFortGameStateAthena>(GameMode->GameState);
			FortGameStateAthena->WarmupCountdownEndTime = FLT_MAX;
			FortGameStateAthena->GamePhase = EAthenaGamePhase::Warmup;
			FortGameStateAthena->OnRep_GamePhase(EAthenaGamePhase::None);

			auto CurrentPlaylistInfo = FortGameStateAthena->CurrentPlaylistInfo;
			if (auto Playlist = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_DefaultSolo.Playlist_DefaultSolo"))
			{
				CurrentPlaylistInfo.BasePlaylist = Playlist;
				CurrentPlaylistInfo.OverridePlaylist = Playlist;
				CurrentPlaylistInfo.PlaylistReplicationKey++;
			}
			CurrentPlaylistInfo.MarkArrayDirty();
			FortGameStateAthena->OnRep_CurrentPlaylistInfo();

			Cast<AFortGameMode>(GameMode)->bWorldIsReady = true;
		}

		return ReadyToStartMatchInternal(GameMode);
	}

	static inline void HandleStartingNewPlayer(AGameModeBase* GameModeBase, APlayerController* NewPlayer)
	{
		Cast<AFortPlayerController>(NewPlayer)->bHasInitiallySpawned = true;
		return HandleStartingNewPlayerInternal(GameModeBase, NewPlayer);
	}

	static inline APawn* SpawnDefaultPawnFor(AGameModeBase* GameModeBase, APlayerController* NewPlayer, AActor* StartSpot)
	{
		return GameModeBase->SpawnDefaultPawnAtTransform(NewPlayer, StartSpot->GetTransform());
	}

	static inline void ServerAcknowledgePossession(AFortPlayerController* FortPlayerController)
	{
		reinterpret_cast<void(*)(AFortPlayerState * FortPlayerState, AFortPawn * FortPawn)>(ModuleBase + 0x1266e80)(Cast<AFortPlayerState>(FortPlayerController->PlayerState), FortPlayerController->MyFortPawn);
		ServerAcknowledgePossessionInternal(FortPlayerController);
	}
};