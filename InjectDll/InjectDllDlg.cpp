
// InjectDllDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "InjectDll.h"
#include "InjectDllDlg.h"
#include "afxdialogex.h"
#include <Windows.h>
#include <string>
#include <Psapi.h>



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CInjectDllDlg 对话框



CInjectDllDlg::CInjectDllDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_INJECTDLL_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CInjectDllDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CInjectDllDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CInjectDllDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CInjectDllDlg 消息处理程序

BOOL CInjectDllDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CInjectDllDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CInjectDllDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CInjectDllDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL InjectDll(DWORD dwProcessId, const char* dllPath)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
	if (hProcess == NULL) {
		return FALSE;
	}

	LPVOID pDllPath = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(hProcess, pDllPath, dllPath, strlen(dllPath) + 1, NULL);

	LPTHREAD_START_ROUTINE pfnThreadRtn = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("Kernelbase.dll"), "LoadLibraryA");
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pfnThreadRtn, pDllPath, 0, NULL);

	CloseHandle(hThread);
	CloseHandle(hProcess);

	return TRUE;
}

std::string GetFullPath(const std::string& relativePath)
{
	char fullPath[MAX_PATH];
	if (GetFullPathNameA(relativePath.c_str(), MAX_PATH, fullPath, nullptr) == 0)
	{
		// 处理获取路径失败的情况
		return "";
	}

	return std::string(fullPath);
}


DWORD GetProcessIdByName(const std::string& processName)
{
	DWORD processId = 0;
	DWORD processes[1024];
	DWORD cbNeeded;

	// 获取系统中所有进程的列表
	if (!EnumProcesses(processes, sizeof(processes), &cbNeeded))
	{
		// 处理获取进程列表失败的情况
		return 0;
	}

	// 计算进程列表中的进程数量
	DWORD count = cbNeeded / sizeof(DWORD);

	for (DWORD i = 0; i < count; i++)
	{
		if (processes[i] != 0)
		{
			// 打开进程句柄，获取进程名
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
			if (hProcess != NULL)
			{
				char szProcessName[MAX_PATH];
				if (GetModuleBaseNameA(hProcess, NULL, szProcessName, sizeof(szProcessName) / sizeof(wchar_t)) != 0)
				{
					std::string processNameStr(szProcessName);
					if (processNameStr == processName)
					{
						// 匹配到指定进程名，返回进程 ID
						processId = processes[i];
						break;
					}
				}
			}

			// 关闭进程句柄
			CloseHandle(hProcess);
		}
	}

	return processId;
}

void CInjectDllDlg::OnBnClickedButton1()
{
	std::string relativePath = "DEEP_ROCK.dll";
	std::string fullPath = GetFullPath(relativePath);
	printf("%s\n", fullPath.c_str());


	DWORD pid = GetProcessIdByName("DRG Survivor.exe");
	printf("pid:%d\n", pid);
	if (pid !=0) {
		InjectDll(pid, fullPath.c_str()); //调试运行的时候要注意路径是否正确 
	}
	else {
		printf("pid = 0\n");
	}
	

	// TODO: 在此添加控件通知处理程序代码
}
