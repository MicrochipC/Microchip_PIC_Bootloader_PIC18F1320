// 18F1320 Bootloader.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "18F1320 Bootloader.h"
#include "18F1320 BootloaderDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMy18F1320BootloaderApp

BEGIN_MESSAGE_MAP(CMy18F1320BootloaderApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CMy18F1320BootloaderApp construction

CMy18F1320BootloaderApp::CMy18F1320BootloaderApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CMy18F1320BootloaderApp object

CMy18F1320BootloaderApp theApp;


// CMy18F1320BootloaderApp initialization

BOOL CMy18F1320BootloaderApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();


	CMy18F1320BootloaderDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
