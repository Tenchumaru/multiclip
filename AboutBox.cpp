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
	case WM_COMMAND:
		return HANDLE_WM_COMMAND(dialog, wParam, lParam, OnCommand), TRUE;
	case WM_INITDIALOG:
		return HANDLE_WM_INITDIALOG(dialog, wParam, lParam, OnInitDialog);
	}
	return FALSE;
}

void AboutBox::ShowModal(HWND parent) {
	DialogBox(g_instance, MAKEINTRESOURCE(IDD_ABOUTBOX), parent, HandleMessage);
}
