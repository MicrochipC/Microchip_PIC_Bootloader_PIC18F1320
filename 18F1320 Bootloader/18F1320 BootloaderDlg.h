// 18F1320 BootloaderDlg.h : header file
//

#pragma once

#include "Bootloader.h"
#include "Comm.h"
#include "afxwin.h"

// CMy18F1320BootloaderDlg dialog
class CMy18F1320BootloaderDlg : public CDialog
{
// Construction
public:
	Bootloader bootloader;
	ComPort com;

	CMy18F1320BootloaderDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MY18F1320BOOTLOADER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CString m_File;
	afx_msg void OnBnClickedButton1();
	CString m_Status;
	afx_msg void OnBnClickedButton2();
	CComboBox m_ComPort;
	afx_msg void OnTimer(UINT nIDEvent);
};
