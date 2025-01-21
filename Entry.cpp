#include <thread>
#include <Native.h>
#include <Trampoline.h>

void ProcessAttach()
{
	AllocConsole();

	FILE* pFile;
	freopen_s(&pFile, "CONOUT$", "w", stdout);

	SetConsoleTitle(TEXT("Celestial"));

	auto& LoggerInstance = Logger::Get();
	LoggerInstance->SetLogFile("Celestial.log");

	GEngine = UEngine::GetEngine();
	ModuleBase = reinterpret_cast<uintptr_t>(GetModuleHandle(NULL));

	Memory::PatchRET(reinterpret_cast<void*>(ModuleBase + 0xc90da0));
	Memory::PatchRET(reinterpret_cast<void*>(ModuleBase + 0x2268960));
	Memory::PatchRET(reinterpret_cast<void*>(ModuleBase + 0xedebb0));
	Memory::PatchRET(reinterpret_cast<void*>(ModuleBase + 0x25c98b0));

	*reinterpret_cast<bool*>(ModuleBase + 0x545c9db) = false;

	Native::Initialize();
	Trampoline::Initialize();
	DefaultObject<UKismetSystemLibrary>()->ExecuteConsoleCommand(GetWorld(), FString(L"open Athena_Terrain"), nullptr);
}

BOOL APIENTRY DllMain(HMODULE, DWORD dwReason, LPVOID)
{
	if (dwReason == DLL_PROCESS_ATTACH) std::thread(ProcessAttach).detach();
	return TRUE;
}