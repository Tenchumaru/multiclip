#include "stdafx.h"
#include "Multiclip.h"
#include "AboutBox.h"

static void OnCommand(HWND dialog, int commandId, HWND /*control*/, UINT /*notificationCode*/) {
	switch(commandId) {
	case IDOK:
	case IDCANCEL:
		EndDialog(dialog, commandId);
		break;
	}
}

static BOOL OnInitDialog(HWND /*dialog*/, HWND /*focusWindow*/, LPARAM /*lParam*/) {
	// Allow the system to set the focus.
	return TRUE;
}

static INT_PTR CALLBACK HandleMessage(HWND dialog, UINT messageId, WPARAM wParam, LPARAM lParam) {
	switch(messageId) {
		HANDLE_MSG(dialog, WM_COMMAND, OnCommand), TRUE;
		HANDLE_MSG(dialog, WM_INITDIALOG, OnInitDialog);
	}
	return FALSE;
}

void AboutBox::ShowModal(HWND parent) {
	DialogBox(g_instance, MAKEINTRESOURCE(IDD_ABOUTBOX), parent, HandleMessage);
}
