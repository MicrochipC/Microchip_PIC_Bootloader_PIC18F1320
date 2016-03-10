// 18F1320 BootloaderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "18F1320 Bootloader.h"
#include "18F1320 BootloaderDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CMy18F1320BootloaderDlg dialog



CMy18F1320BootloaderDlg::CMy18F1320BootloaderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMy18F1320BootloaderDlg::IDD, pParent)
	, m_File(_T(""))
	, m_Status(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMy18F1320BootloaderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_File);
	DDX_Text(pDX, IDC_EDIT2, m_Status);
	DDX_Control(pDX, IDC_COMBO1, m_ComPort);
}

BEGIN_MESSAGE_MAP(CMy18F1320BootloaderDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CMy18F1320BootloaderDlg message handlers

BOOL CMy18F1320BootloaderDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMy18F1320BootloaderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMy18F1320BootloaderDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMy18F1320BootloaderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMy18F1320BootloaderDlg::OnBnClickedButton1()
{
	CFileDialog dlg(TRUE, "Hex files", "*.hex");
	if(dlg.DoModal() == IDOK)
	{
		m_File = dlg.GetPathName();
	}
	UpdateData(FALSE);
}
UINT bootloaderthread( LPVOID pParam )
{
	Bootloader *b = (Bootloader *)pParam;
	b->Update();
	return 0;
}

void CMy18F1320BootloaderDlg::OnBnClickedButton2()
{
	UpdateData(TRUE);
	if(com.Open(m_ComPort.GetCurSel()+1, 19200, NONE, 8, ONESB) != COMM_OK)
	{
		MessageBox("Error opening port.");
		return;
	}
	SetTimer(1, 100, NULL);
	bootloader.SetFilename((char *)m_File.GetString());
	bootloader.SetComPort(&com);

	UpdateData(FALSE);
	CWinThread *thread;
	thread = AfxBeginThread( bootloaderthread, (void *)&bootloader );
}

void CMy18F1320BootloaderDlg::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	UpdateData(TRUE);
	m_Status = bootloader.GetStatus();
	UpdateData(FALSE);
	CDialog::OnTimer(nIDEvent);
}
