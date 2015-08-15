#include "stdafx.h"
#include "Multiclip.h"
#include "MainFrame.h"

HINSTANCE g_instance;

static bool InitializeApplication(int /*showCommand*/) {
	// Initialize the common controls.
	INITCOMMONCONTROLSEX iccex;
	iccex.dwSize = sizeof(iccex);
	iccex.dwICC = ICC_STANDARD_CLASSES | ICC_LINK_CLASS;
	if(!InitCommonControlsEx(&iccex))
		return false;

	return true;
}

int APIENTRY _tWinMain(HINSTANCE instance, HINSTANCE /*previousInstance*/, LPTSTR /*commandLine*/, int showCommand) {
	g_instance = instance;

	// Initialize the application.
	if(!InitializeApplication(showCommand))
		return 1;

	// Show the application window.
	MainFrame::ShowModal();

	return 0;
}
