#include "stdafx.h"
#include "Multiclip.h"
#include "MainFrame.h"
#include "AboutBox.h"

namespace {
	struct LAYOUT {
		UINT id;
		enum { Stay, Move, Size, Spring, Stretch } xMode, yMode;
		RECT originalRect, currentRect;
	};

	LAYOUT* s_layout;
	LAYOUT* s_pDialogLayout;

	HWND m_edit[5];
	bool m_bLock[5];
	HBITMAP m_bitmap;
	HWND m_hWndCBChain;
	HICON m_hIcon;
	int m_nIndex;
	int m_nLocked;
	bool m_bNoCopy = true;
}

static void InitializeLayout(HWND dialog, LAYOUT layout[]) {
	int i;
	for(i = 0; layout[i].id != 0; ++i) {
		HWND control = ::GetDlgItem(dialog, layout[i].id);
		::GetWindowRect(control, &layout[i].originalRect);
		::MapWindowRect(NULL, dialog, &layout[i].originalRect);
		layout[i].currentRect = layout[i].originalRect;
	}
	s_pDialogLayout = (s_layout = layout) + i;
	::GetClientRect(dialog, &s_pDialogLayout->originalRect);
	s_pDialogLayout->currentRect = s_pDialogLayout->originalRect;
}

static void UpdateLayout(HWND dialog, int cx, int cy) {
	LONG xDiff = cx - s_pDialogLayout->currentRect.right;
	LONG yDiff = cy - s_pDialogLayout->currentRect.bottom;
	s_pDialogLayout->currentRect.right = cx;
	s_pDialogLayout->currentRect.bottom = cy;
	LONG originalDialogWidth = s_pDialogLayout->originalRect.right;
	LONG originalDialogHeight = s_pDialogLayout->originalRect.bottom;
	for(LAYOUT* p = s_layout; p != s_pDialogLayout; ++p) {
		LONG center, size;
		HWND control = ::GetDlgItem(dialog, p->id);
		RECT& rect = p->currentRect;
		switch(p->xMode) {
		case LAYOUT::Move:
			::OffsetRect(&rect, xDiff, 0);
			break;
		case LAYOUT::Size:
			rect.right += xDiff;
			break;
		case LAYOUT::Spring:
			center = (p->originalRect.left + p->originalRect.right) / 2;
			center = MulDiv(center, cx, originalDialogWidth);
			size = p->originalRect.right - p->originalRect.left;
			rect.left = center - size / 2;
			rect.right = rect.left + size;
			break;
		case LAYOUT::Stretch:
			rect.left = MulDiv(p->originalRect.left, cx, originalDialogWidth);
			rect.right = MulDiv(p->originalRect.right, cx, originalDialogWidth);
			break;
		}
		switch(p->yMode) {
		case LAYOUT::Move:
			::OffsetRect(&rect, 0, yDiff);
			break;
		case LAYOUT::Size:
			rect.bottom += yDiff;
			break;
		case LAYOUT::Spring:
			center = (p->originalRect.top + p->originalRect.bottom) / 2;
			center = MulDiv(center, cy, originalDialogHeight);
			size = p->originalRect.bottom - p->originalRect.top;
			rect.top = center - size / 2;
			rect.bottom = rect.top + size;
			break;
		case LAYOUT::Stretch:
			rect.top = MulDiv(p->originalRect.top, cy, originalDialogHeight);
			rect.bottom = MulDiv(p->originalRect.bottom, cy, originalDialogHeight);
			break;
		}
		::MoveWindow(control, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
	}
}

static void OnAlwaysOnTop(HWND dialog) {
	bool isAlwaysOnTop = Button_GetCheck(GetDlgItem(dialog, IDC_ALWAYS_ON_TOP)) == BST_CHECKED;
	SetWindowPos(dialog, isAlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
		SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOSIZE);
}

static void OnChangeCbChain(HWND /*dialog*/, HWND hWndRemove, HWND hWndAfter) {
	if(hWndRemove == m_hWndCBChain) {
		m_hWndCBChain = hWndAfter;
	} else if(m_hWndCBChain != NULL) {
		::SendMessage(m_hWndCBChain, WM_CHANGECBCHAIN, (WPARAM)hWndRemove, (LPARAM)hWndAfter);
	}
}

static void OnClearAll() {
	for(int i = 0; i < 5; i++) {
		if(!m_bLock[i]) {
			Edit_SetSel(m_edit[i], 0, -1);
			Edit_ReplaceSel(m_edit[i], _T(""));
		}
	}
}

static void OnCopy(UINT nID) {
	static_assert(IDC_BUTTON4 - IDC_BUTTON0 == _countof(m_edit) - 1, "invalid button ID range");
	ASSERT(nID >= IDC_BUTTON0 && nID <= IDC_BUTTON4);
	int i = nID - IDC_BUTTON0;
	Edit_SetSel(m_edit[i], 0, -1);
	m_bNoCopy = true;
	SendMessage(m_edit[i], WM_COPY, 0, 0);
}

static void OnLock(UINT nID) {
	static_assert(IDC_CHECK4 - IDC_CHECK0 == _countof(m_bLock) - 1, "invalid checkbox ID range");
	ASSERT(nID >= IDC_CHECK0 && nID <= IDC_CHECK4);
	int i = nID - IDC_CHECK0;
	m_bLock[i] = !m_bLock[i];
	if(m_bLock[i]) {
		++m_nLocked;
	} else {
		--m_nLocked;
	}
}

static bool OnSysCommand(HWND dialog, UINT commandId) {
	if((commandId & 0xfff0) == IDM_ABOUT) {
		AboutBox::ShowModal(dialog);
		return true;
	}
	return false;
}

static void OnCommand(HWND dialog, int commandId, HWND /*control*/, UINT notificationCode) {
	switch(commandId) {
	case ID_HELP_ABOUT:
		AboutBox::ShowModal(dialog);
		break;
	case IDOK:
		break;
	case IDCANCEL:
		::ChangeClipboardChain(dialog, m_hWndCBChain);
		ASSERT(GetLastError() == 0);
		EndDialog(dialog, commandId);
		break;
	case IDC_ALWAYS_ON_TOP:
		if(notificationCode == BN_CLICKED) {
			OnAlwaysOnTop(dialog);
		}
		break;
	case IDC_BUTTON0:
	case IDC_BUTTON1:
	case IDC_BUTTON2:
	case IDC_BUTTON3:
	case IDC_BUTTON4:
		if(notificationCode == BN_CLICKED) {
			OnCopy(commandId);
		}
		break;
	case IDC_CHECK0:
	case IDC_CHECK1:
	case IDC_CHECK2:
	case IDC_CHECK3:
	case IDC_CHECK4:
		if(notificationCode == BN_CLICKED) {
			OnLock(commandId);
		}
		break;
	case IDC_CLEAR_ALL:
		if(notificationCode == BN_CLICKED) {
			OnClearAll();
		}
		break;
	}
}

static void OnDrawClipboard(HWND /*dialog*/) {
	static_assert(_countof(m_bLock) == _countof(m_edit), "count of checkboxes and edits do not match");
	if(!m_bNoCopy && m_nLocked < 5 && IsClipboardFormatAvailable(CF_TEXT)) {
		while(m_bLock[m_nIndex]) {
			m_nIndex = (m_nIndex + 1) % 5;
		}
		Edit_SetSel(m_edit[m_nIndex], 0, -1);
		SendMessage(m_edit[m_nIndex], WM_PASTE, 0, 0);
		m_nIndex = (m_nIndex + 1) % 5;
	}
	if(m_hWndCBChain != NULL) {
		::SendMessage(m_hWndCBChain, WM_DRAWCLIPBOARD, 0, 0);
	}
	m_bNoCopy = false;
}

static BOOL OnInitDialog(HWND dialog, HWND /*focusWindow*/, LPARAM /*lParam*/) {
	// Initialize the layout logic with the current state of the dialog and
	// the controls involved in the dynamic layout.
	static LAYOUT layout[] = {
		{ IDC_EDIT0, LAYOUT::Size, LAYOUT::Stretch },
		{ IDC_EDIT1, LAYOUT::Size, LAYOUT::Stretch },
		{ IDC_EDIT2, LAYOUT::Size, LAYOUT::Stretch },
		{ IDC_EDIT3, LAYOUT::Size, LAYOUT::Stretch },
		{ IDC_EDIT4, LAYOUT::Size, LAYOUT::Stretch },
		{ IDC_CHECK0, LAYOUT::Move, LAYOUT::Spring },
		{ IDC_CHECK1, LAYOUT::Move, LAYOUT::Spring },
		{ IDC_CHECK2, LAYOUT::Move, LAYOUT::Spring },
		{ IDC_CHECK3, LAYOUT::Move, LAYOUT::Spring },
		{ IDC_CHECK4, LAYOUT::Move, LAYOUT::Spring },
		{ IDC_BUTTON0, LAYOUT::Stay, LAYOUT::Spring },
		{ IDC_BUTTON1, LAYOUT::Stay, LAYOUT::Spring },
		{ IDC_BUTTON2, LAYOUT::Stay, LAYOUT::Spring },
		{ IDC_BUTTON3, LAYOUT::Stay, LAYOUT::Spring },
		{ IDC_BUTTON4, LAYOUT::Stay, LAYOUT::Spring },
		{ IDC_CLEAR_ALL, LAYOUT::Spring, LAYOUT::Move },
		{ IDC_ALWAYS_ON_TOP, LAYOUT::Spring, LAYOUT::Move },
		{ IDC_BUFFERS_LABEL, LAYOUT::Spring },
		{ IDC_LOCK_LABEL, LAYOUT::Move },
		{} // null terminator
	};
	InitializeLayout(dialog, layout);

	// Set the icon to use in the ALT+TAB dialog box.
	HANDLE icon = LoadImage(g_instance, MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 32, 32, LR_SHARED);
	SendMessage(dialog, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(icon));

	// Set the icon to use in the title bar.
	icon = static_cast<HICON>(LoadImage(g_instance, MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 16, 16, LR_SHARED));
	SendMessage(dialog, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(icon));

	// Set the button images and get the edit controls.
	for(int i = 0; i < 5; ++i) {
		SendDlgItemMessage(dialog, IDC_BUTTON0 + i, BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(m_bitmap));
		m_edit[i] = GetDlgItem(dialog, IDC_EDIT0 + i);
	}

	// Add the "About..." menu item to the system menu.  IDM_ABOUT must be
	// in the system command range.
	static_assert((IDM_ABOUT & 0xfff0) == IDM_ABOUT, "invalid IDM_ABOUT");
	static_assert(IDM_ABOUT < 0xf000, "IDM_ABOUT out of range");
	HMENU systemMenu = GetSystemMenu(dialog, false);
	if(systemMenu != NULL) {
		TCHAR aboutString[32];
		LoadString(g_instance, IDS_ABOUT, aboutString, _countof(aboutString));
		AppendMenu(systemMenu, MF_SEPARATOR, 0, NULL);
		AppendMenu(systemMenu, MF_STRING, IDM_ABOUT, aboutString);
	}

	SetLastError(0);
	m_hWndCBChain = SetClipboardViewer(dialog);
	ASSERT(GetLastError() == 0);

	// Allow the system to set the focus.
	return TRUE;
}

static void OnSize(HWND dialog, UINT state, int cx, int cy) {
	if(state != SIZE_MINIMIZED) {
		UpdateLayout(dialog, cx, cy);
	}
}

static INT_PTR CALLBACK HandleMessage(HWND dialog, UINT messageId, WPARAM wParam, LPARAM lParam) {
	switch(messageId) {
		HANDLE_MSG(dialog, WM_CHANGECBCHAIN, OnChangeCbChain), TRUE;
		HANDLE_MSG(dialog, WM_COMMAND, OnCommand), TRUE;
		HANDLE_MSG(dialog, WM_DRAWCLIPBOARD, OnDrawClipboard), TRUE;
		HANDLE_MSG(dialog, WM_INITDIALOG, OnInitDialog);
		HANDLE_MSG(dialog, WM_SIZE, OnSize);
	case WM_SYSCOMMAND:
		return OnSysCommand(dialog, wParam);
	}
	return FALSE;
}

void MainFrame::ShowModal() {
	m_hIcon = LoadIcon(g_instance, MAKEINTRESOURCE(IDR_MAINFRAME));
	m_bitmap = LoadBitmap(g_instance, MAKEINTRESOURCE(IDB_COPY));
	DialogBox(g_instance, MAKEINTRESOURCE(IDR_MAINFRAME), NULL, HandleMessage);
}
