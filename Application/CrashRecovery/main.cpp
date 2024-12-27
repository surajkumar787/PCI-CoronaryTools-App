// Copyright Koninklijke Philips Electronics N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "AppLauncherClient.h"
#include "Settings.h"
#include "Log.h"
#include "PathUtil.h"
#include "Translation.h"
#include "TranslationHelper.h"

#include <windows.h>

static const std::wstring configFolder = L"Configuration";

using ::CommonPlatform::AppLauncher::Applications;

static LRESULT __stdcall LocalizeButtons(int nCode, WPARAM wParam, LPARAM /*lParam*/)
{
	if (nCode == HCBT_ACTIVATE) {
		SetWindowText(GetDlgItem(reinterpret_cast<HWND>(wParam), IDRETRY), Pci::Core::Localization::Translation::IDS_Retry.c_str());
		SetWindowText(GetDlgItem(reinterpret_cast<HWND>(wParam), IDCANCEL), Pci::Core::Localization::Translation::IDS_Cancel.c_str());
	}
	return 0;
}

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int)
{
	auto &log = CommonPlatform::Logger(CommonPlatform::LoggerType::Platform);

	auto configFolderFullPath = CommonPlatform::Utilities::AbsolutePathFromExeDir(configFolder);
	CommonPlatform::Settings workstation_ini(configFolderFullPath + L"Workstation.ini");
	std::wstring language = workstation_ini.get(L"General", L"UiLanguage", L"en");
	Pci::Core::Localization::Translation::setLanguage(Pci::Core::Localization::getLanguageId(language, true));

	CommonPlatform::AppLauncher::AppLauncherClient client(Applications::CrashRecovery, true);

	HANDLE reportDone = CreateEvent(NULL, FALSE, FALSE, NULL);

	client.EventAppExited = [&reportDone](Applications app, int /* exitCode */)
	{
		if (app == Applications::ProblemReportHidden)
		{
			// Signal ProblemReport finished;
			SetEvent(reportDone);
		}
	};

	if (client.WaitForConnection(1000))
	{
		client.Start(Applications::ProblemReportHidden);
	}
	else
	{
		log.DeveloperWarning(L"Failed to generate problem report: could not connect to LoaderService");
	}

	// Wait until ProblemReport has finished
	if (WaitForSingleObject(reportDone, INFINITE) == WAIT_OBJECT_0)
	{
		log.DeveloperInfo(L"Problem report generated");
	}
	else
	{
		log.DeveloperWarning(L"Failed to generate problem report. Error=%u", GetLastError());
	}
	CloseHandle(reportDone);

	const wchar_t * title = Pci::Core::Localization::Translation::IDS_CoronaryToolsHasStoppedWorking.c_str();
	const wchar_t * text = Pci::Core::Localization::Translation::IDS_CoronaryToolsHasStoppedWorkingFull.c_str();

	auto hook = SetWindowsHookEx(WH_CBT, LocalizeButtons, hInstance, GetCurrentThreadId());
	auto res = MessageBox(nullptr, text, title, MB_ICONWARNING | MB_RETRYCANCEL | MB_SYSTEMMODAL | MB_SETFOREGROUND);
	UnhookWindowsHookEx(hook);

	if (res == IDRETRY)
	{
		client.RebootSystem();
	}
	else if (res == IDCANCEL)
	{
		client.Start(Applications::FieldService);
	}

	return 0;
}