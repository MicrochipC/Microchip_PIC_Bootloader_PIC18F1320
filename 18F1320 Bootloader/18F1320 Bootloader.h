// 18F1320 Bootloader.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CMy18F1320BootloaderApp:
// See 18F1320 Bootloader.cpp for the implementation of this class
//

class CMy18F1320BootloaderApp : public CWinApp
{
public:
	CMy18F1320BootloaderApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CMy18F1320BootloaderApp theApp;