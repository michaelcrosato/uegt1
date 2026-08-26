#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/ConfigCacheIni.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUEGT1ProjectConfigurationTest,
	"UEGT1.Smoke.ProjectConfiguration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUEGT1ProjectConfigurationTest::RunTest(const FString& Parameters)
{
	FString DefaultRHI;
	const bool bFoundRHI = GConfig->GetString(
		TEXT("/Script/WindowsTargetPlatform.WindowsTargetSettings"),
		TEXT("DefaultGraphicsRHI"),
		DefaultRHI,
		GEngineIni);

	TestTrue(TEXT("The Windows default RHI is configured"), bFoundRHI);
	TestEqual(TEXT("DirectX 12 is the Windows default RHI"), DefaultRHI, FString(TEXT("DefaultGraphicsRHI_DX12")));
	return !HasAnyErrors();
}

#endif

