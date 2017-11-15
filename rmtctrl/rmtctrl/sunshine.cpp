// sunshine.cpp: 定义应用程序的入口点。
//

#include "sunshine.h"

#include "../condll/fileop.h"
#include "../condll/mymalloc.h"
#include "../condll/MyMutex.h"
#include "../condll/socket_util.h"
#include "../condll/SockSendFileManager.h"
#include <process.h>
#include <CommCtrl.h>
#include <windowsx.h>

#include <list>
#include <map>

#pragma comment(lib, "comctl32.lib");

#define MAX_LOADSTRING 100

// 全局变量: 
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:  
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

HWND g_hWnd[8];
#define IP_SEL			1
#define RMT_D_SEL		2
#define LCL_D_SEL		3
#define RMT_FLST_SEL	4
#define LCL_FLST_SEL	5
#define UPDF_BTN		6
#define DLDF_BTN		7

HINSTANCE g_hInst;
bool g_exit = false;
struct MSKTINFO
{
	SocketInfo* osi;
	SocketInfo* fsi;
	MSKTINFO()
	{
		osi = NULL;
		fsi = NULL;
	}
	MSKTINFO(SocketInfo* osi_)
	{
		osi = osi_;
		fsi = NULL;
	}
	~MSKTINFO()
	{
		delete osi;
		delete fsi;
	}
};

std::map<std::string, MSKTINFO> g_cmap;
CRITICAL_SECTION g_cs_list;
SockSendFileManager g_ssfm;

SOCKET g_order_sock;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	CSktUtil::CSUInit();
	MyMutex::InitCS(&g_cs_list);

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
	g_hInst = hInstance;
    // TODO: 在此放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SUNSHINE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化: 
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SUNSHINE));

    MSG msg;

    // 主消息循环: 
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

	CSktUtil::CSUCleanup();
    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SUNSHINE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(COLOR_GRAYTEXT + 1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SUNSHINE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_SYSMENU, 
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CREATE:
	{
		RECT rect;
		GetClientRect(hWnd, &rect);

		//创建控件
		g_hWnd[IP_SEL] = CreateWindow(WC_COMBOBOX, TEXT(""),
			CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_BORDER,
			0, 0, rect.right, 200, hWnd, (HMENU)IP_SEL, g_hInst,
			NULL);

		g_hWnd[RMT_D_SEL] = CreateWindow(WC_COMBOBOX, TEXT(""),
			CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_BORDER,
			0, 30, rect.right / 2 - 30, 200, hWnd, (HMENU)RMT_D_SEL, g_hInst,
			NULL);
		g_hWnd[LCL_D_SEL] = CreateWindow(WC_COMBOBOX, TEXT(""),
			CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_BORDER|WS_HSCROLL|WS_VSCROLL,
			rect.right / 2 + 30, 30, rect.right / 2 - 30, 200, hWnd, (HMENU)LCL_D_SEL, g_hInst,
			NULL);
		g_hWnd[RMT_FLST_SEL] = CreateWindowEx(WS_EX_STATICEDGE | LVS_EX_FULLROWSELECT,WC_LISTVIEW,
			NULL, WS_CHILD | WS_VISIBLE | LVS_SINGLESEL | WS_BORDER | LVS_REPORT,
			0, 55, rect.right / 2 - 30, rect.bottom - 55, hWnd, (HMENU)RMT_FLST_SEL, g_hInst,
			NULL);
		ListView_SetExtendedListViewStyle(g_hWnd[RMT_FLST_SEL], ListView_GetExtendedListViewStyle(g_hWnd[RMT_FLST_SEL])| LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

		g_hWnd[LCL_FLST_SEL] = CreateWindowEx(WS_EX_STATICEDGE| LVS_EX_FULLROWSELECT, WC_LISTVIEW,
			NULL, WS_CHILD | WS_VISIBLE | LVS_SINGLESEL | WS_BORDER | LVS_REPORT  ,
			rect.right / 2 + 30, 55, rect.right / 2 - 30, rect.bottom - 55,
			hWnd, (HMENU)LCL_FLST_SEL, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
		ListView_SetExtendedListViewStyle(g_hWnd[LCL_FLST_SEL], ListView_GetExtendedListViewStyle(g_hWnd[LCL_FLST_SEL]) | LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

		g_hWnd[UPDF_BTN] = CreateWindow(WC_BUTTON, TEXT("上传"),
			WS_BORDER | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
			rect.right / 2 - 25, rect.bottom / 2 - 30, 50, 25, hWnd, (HMENU)UPDF_BTN, g_hInst,
			NULL);
		g_hWnd[DLDF_BTN] = CreateWindow(WC_BUTTON, TEXT("下载"),
			WS_BORDER | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
			rect.right / 2 - 25, rect.bottom / 2, 50, 25, hWnd, (HMENU)DLDF_BTN, g_hInst,
			NULL);
		
		//设置本地文件地址栏内容
		char buf[128];
		DWORD dwLen = ::GetLogicalDriveStringsA(MAX_PATH, buf);
		int dstrlen = dwLen;
		char* tdstr = buf;
		int tstrlen = 0;
		while (dstrlen > 0)
		{
			SendMessage(g_hWnd[LCL_D_SEL], (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)tdstr);
			tstrlen = strlen(tdstr) + 1;
			dstrlen -= tstrlen;
			tdstr += tstrlen;
		}

		LVCOLUMN col = { 0 };
		col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
		col.fmt = LVCFMT_CENTER;
		col.cx =  800;
		col.cchTextMax = MAX_PATH;
		col.pszText = "文件列表";
		col.iSubItem = 0;
		ListView_InsertColumn(g_hWnd[LCL_FLST_SEL], 0, &col);
		ListView_InsertColumn(g_hWnd[RMT_FLST_SEL], 0, &col);


		//创建图片列表  
		HICON hiconItem;     // Icon for list-view items.
		HIMAGELIST hLarge;   // Image list for icon view.
		HIMAGELIST hSmall;   // Image list for other views.

							 // Create the full-sized icon image lists. 
		hLarge = ImageList_Create(GetSystemMetrics(SM_CXICON),
			GetSystemMetrics(SM_CYICON),
			ILC_MASK, 1, 1);

		hSmall = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
			GetSystemMetrics(SM_CYSMICON),
			ILC_MASK, 1, 1);

		// Add an icon to each image list.  
		hiconItem = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_SMALL));

		ImageList_AddIcon(hLarge, hiconItem);
		ImageList_AddIcon(hSmall, hiconItem);
		DestroyIcon(hiconItem);
		ListView_SetImageList(g_hWnd[LCL_FLST_SEL], hLarge, LVSIL_NORMAL);
		ListView_SetImageList(g_hWnd[LCL_FLST_SEL], hSmall, LVSIL_SMALL);

		CSktUtil::CSUStartAndListen(g_order_sock, SEVER_PORT);
		_beginthread(MyAccept, 0, AcceptProc);
		_beginthread(MyFileAccept, 0, FileAcceptProc);
	}
	break;
	case WM_NOTIFY:
	{
		switch (((LPNMHDR)lParam)->code)
		{
		case NM_DBLCLK:
		{
			if (((LPNMHDR)lParam)->idFrom == LCL_FLST_SEL)
			{
				//ListView_Scroll(,);
				char buf[MAX_PATH];
				char path[MAX_PATH];
				LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)lParam;
				if (lpnmitem->iItem >= 0)
				{
					ListView_GetItemText(g_hWnd[LCL_FLST_SEL], lpnmitem->iItem, lpnmitem->iSubItem, buf, MAX_PATH);
					if (!strncmp(buf, "[D]", 3))
					{
						GetWindowText(g_hWnd[LCL_D_SEL], path, MAX_PATH);
						strcat(path, "\\");
						strcat(path, buf + 3);
						ListView_DeleteAllItems(g_hWnd[LCL_FLST_SEL]);
						SendMessage(g_hWnd[LCL_D_SEL], (UINT)CB_INSERTSTRING, (WPARAM)0, (LPARAM)path);
						SendMessage(g_hWnd[LCL_D_SEL], (UINT)CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

						TraverFolder(MyVisitFile, path, NULL, 0);
					}
				}
			}
			else if (((LPNMHDR)lParam)->idFrom == RMT_FLST_SEL)
			{
				//ListView_Scroll(,);
				char buf[MAX_PATH];
				char path[MAX_PATH];
				LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)lParam;
				if (lpnmitem->iItem >= 0)
				{
					ListView_GetItemText(g_hWnd[RMT_FLST_SEL], lpnmitem->iItem, lpnmitem->iSubItem, buf, MAX_PATH);
					if (!strncmp(buf, "[D]", 3))
					{
						GetWindowText(g_hWnd[RMT_D_SEL], path, MAX_PATH);
						strcat(path, "\\");
						strcat(path, buf + 3);
						ListView_DeleteAllItems(g_hWnd[RMT_FLST_SEL]);
						SendMessage(g_hWnd[RMT_D_SEL], (UINT)CB_INSERTSTRING, (WPARAM)0, (LPARAM)path);
						SendMessage(g_hWnd[RMT_D_SEL], (UINT)CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

						char ip[32];
						int len = 4;
						GetWindowText(g_hWnd[IP_SEL], ip, 32);

						*(int*)(buf + len) = htonl(2);
						len += 4;
						strcpy(buf + len, path);
						len += strlen(path) + 1;
						*(int*)buf = htonl(len - 4);

						CSktUtil::CSUSend(g_cmap[ip].osi->sock, buf, len);

					}
				}
			}
		}
		}
	}break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
			//组合框，选择了选项
			if (HIWORD(wParam) == CBN_SELCHANGE)
				// If the user makes a selection from the list:
				//   Send CB_GETCURSEL message to get the index of the selected list item.
				//   Send CB_GETLBTEXT message to get the item.
				//   Display the item in a messagebox.
			{
				switch (wmId)
				{
				case IP_SEL: 
				{
					int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL,
						(WPARAM)0, (LPARAM)0);
					char  ListItem[MAX_PATH];
					(TCHAR)SendMessage((HWND)lParam, (UINT)CB_GETLBTEXT,
						(WPARAM)ItemIndex, (LPARAM)ListItem);
					//MessageBox(hWnd, ListItem, TEXT("Item Selected"), MB_OK);
					if (g_cmap[ListItem].osi)
					{
						ComboBox_ResetContent(g_hWnd[RMT_D_SEL]);
						char buf[8];
						*(int*)buf = htonl(4);
						*(int*)(buf + 4) = htonl(1);
						CSktUtil::CSUSend(g_cmap[ListItem].osi->sock, buf, 8);
					}
				}
				break;
				case RMT_D_SEL:
				{
					int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL,
						(WPARAM)0, (LPARAM)0);

					char  ListItem[MAX_PATH];
					char* buf = (char*)malloc(512);
					char ip[32];
					int len = 4;
					(TCHAR)SendMessage((HWND)lParam, (UINT)CB_GETLBTEXT,
						(WPARAM)ItemIndex, (LPARAM)ListItem);
					GetWindowText(g_hWnd[IP_SEL], ip, 32);

					*(int*)(buf + len) = htonl(2);
					len += 4;
					strcpy(buf + len, ListItem);
					len += strlen(ListItem) + 1;
					*(int*)buf = htonl(len - 4);
					ListView_DeleteAllItems(g_hWnd[RMT_FLST_SEL]);

					CSktUtil::CSUSend(g_cmap[ip].osi->sock, buf, len);
					free(buf);
				}
				break;
				case LCL_D_SEL:
				{
					int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL,
						(WPARAM)0, (LPARAM)0);
					char  ListItem[MAX_PATH];
					(TCHAR)SendMessage((HWND)lParam, (UINT)CB_GETLBTEXT,
						(WPARAM)ItemIndex, (LPARAM)ListItem);
					ListView_DeleteAllItems(g_hWnd[LCL_FLST_SEL]);
					TraverFolder(MyVisitFile, ListItem, NULL,0);
				}break;
				default:
					break;
				}
			}

			//点击了按钮
			if (HIWORD(wParam) == BN_CLICKED)
			{
				int ItemIndex = SendMessage((HWND)g_hWnd[LCL_D_SEL], (UINT)CB_GETCURSEL,
					(WPARAM)0, (LPARAM)0);
				char  rmtpath[MAX_PATH];
				char  lclpath[MAX_PATH];
				(TCHAR)SendMessage((HWND)g_hWnd[LCL_D_SEL], (UINT)CB_GETLBTEXT,
					(WPARAM)ItemIndex, (LPARAM)lclpath);
				ItemIndex = SendMessage((HWND)g_hWnd[RMT_D_SEL], (UINT)CB_GETCURSEL,
					(WPARAM)0, (LPARAM)0);
				(TCHAR)SendMessage((HWND)g_hWnd[RMT_D_SEL], (UINT)CB_GETLBTEXT,
					(WPARAM)ItemIndex, (LPARAM)rmtpath);
				char ip[32];
				GetWindowText(g_hWnd[IP_SEL], ip, 32);

				//点击了上传按钮
				if (LOWORD(wParam) == UPDF_BTN && 0 != strlen(lclpath) && 0 != strlen(rmtpath))
				{
					char  ListItem[256];
					int listcnt = ListView_GetItemCount(g_hWnd[LCL_FLST_SEL]);

					for (int i = 0; i < listcnt; i++)
					{
						if (ListView_GetCheckState(g_hWnd[LCL_FLST_SEL], i))
						{
							ListView_GetItemText(g_hWnd[LCL_FLST_SEL],i,0, ListItem,256);
							if (!strncmp(ListItem,"[D]",3))
							{
								MessageBox(hWnd, "暂不支持上传文件夹！", "warning", 1);
							}
							else
							{
								//上传文件
								CFileTransInfo* cft = new CFileTransInfo();
								strcpy(cft->file_name, ListItem + 3);
								strcpy(cft->file_path,lclpath);
								strcpy(cft->remote_file_path,rmtpath);
								SockSendFileManager* pssm = (SockSendFileManager*)g_cmap[ip].fsi->data;
								pssm->ReqSendFile(cft);
							}
						}
					}
					break;
				}
				//点击下载按钮
				else if (LOWORD(wParam) == DLDF_BTN)
				{
					char ListItem[256];
					int listcnt = ListView_GetItemCount(g_hWnd[RMT_FLST_SEL]);
					int len = 0;
					char* sendbuf = (char*)malloc(1024);

					for (int i = 0; i < listcnt; i++)
					{
						if (ListView_GetCheckState(g_hWnd[RMT_FLST_SEL], i))
						{
							ListView_GetItemText(g_hWnd[RMT_FLST_SEL], i, 0, ListItem, 256);
							if (!strncmp(ListItem, "[D]", 3))
							{
								MessageBox(hWnd, "暂不支持下载文件夹！", "warning", 1);
							}
							else
							{
								char* buf = sendbuf;
								//空出前四个字节存信息长度
								len = 4;
								//命令类型
								*(_uint32*)(buf + len) = htonl(3);		//
								len += 4;

								//本地路径
								_uint32 filepathlen = strlen(lclpath);				//路径长度
								*(_uint32*)(buf + len) = htonl(filepathlen);		//
								len += 4;
								memcpy(buf + len, lclpath, filepathlen);			//文件路径
								len += filepathlen;

								//远程路径
								filepathlen = strlen(rmtpath);				//路径长度
								*(_uint32*)(buf + len) = htonl(filepathlen);		//
								len += 4;
								memcpy(buf + len, rmtpath, filepathlen);			//文件路径
								len += filepathlen;
								
								//文件名
								_uint32 filenamelen = strlen(ListItem + 3);			//文件名长度
								*(_uint32*)(buf + len) = htonl(filenamelen);			//
								len += 4;
								memcpy(buf + len, ListItem + 3, filenamelen);		//文件名
								len += filenamelen;

								*(_uint32*)sendbuf = htonl(len - 4);				//文件信息头长度

								CSktUtil::CSUSend(g_cmap[ip].osi->sock, sendbuf, len);
							}
						}
					}
					free(sendbuf);
					break;
				}
			}
			// 分析菜单选择: 
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		g_exit = true;
		
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void MyAccept(void* data)
{
	//LPAcceptProc func = (LPAcceptProc)data;
	CSktUtil::CSUAccept(g_order_sock, AcceptProc, NULL, g_exit);
}

void AcceptProc(void* data)
{
	SocketInfo* si = (SocketInfo*)data;
	size_t len = BUFFSIZE;
	char* buf = (char*)MyMalloc(len);

	{
		MyMutex m(&g_cs_list);
		inet_ntop(AF_INET, &si->addr.sin_addr.S_un.S_addr, buf, len);

		g_cmap[buf].osi = si;
		//MessageBox(NULL, inet_ntop(AF_INET, &si->addr.sin_addr.S_un.S_addr, buf, 128), "oooo", 1);
		//update 
		SendMessage(g_hWnd[IP_SEL], (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)buf);

	}
	int tlen = 0;;
	char* tbuf = buf;
	int cmd = 0;
	while (!g_exit)
	{
		//recv and to do
		memset(buf, 0x00, len * sizeof(char));
		tbuf = buf;
		if (CSktUtil::CSURecv(si->sock, tbuf, 4) > 0)
		{
			tlen = ntohl(*(int*)tbuf);
			if (tlen > len)
			{
				free(buf);
				len = tlen*1.5;
				buf = (char*)MyMalloc(len);
				memset(buf, 0x00, len * sizeof(char));
				tbuf = buf;
			}
			if (CSktUtil::CSURecv(si->sock, tbuf, tlen) > 0)
			{
				tbuf[tlen] = 0;

				cmd = ntohl(*(int*)tbuf);
				tbuf += 4;
				switch (cmd)
				{
				case 1:
				{
					int dstrlen = tlen - 4;
					char* tdstr = tbuf;
					int tstrlen = 0;
					while (dstrlen > 0)
					{
						SendMessage(g_hWnd[RMT_D_SEL], (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)tdstr);
						tstrlen = strlen(tdstr) + 1;
						dstrlen -= tstrlen;
						tdstr += tstrlen;
					}
				}
				break;
				case 2:
				{
					char buf[MAX_PATH];
					char* tpos;
					int fnamelen = 0;
					LV_ITEM itm = { 0 };
					itm.mask = LVFIF_TEXT;
					itm.iItem = 0;
					itm.iSubItem = 0;
					itm.iImage = 2;

					while (tpos = strchr(tbuf + 1, '['))
					{
						fnamelen = tpos - tbuf;
						strncpy(buf, tbuf, fnamelen);
						buf[fnamelen] = 0;
						itm.pszText = (LPSTR)buf;
						tbuf += fnamelen;
						ListView_InsertItem(g_hWnd[RMT_FLST_SEL], &itm);
					}
					itm.pszText = (LPSTR)tbuf;
					ListView_InsertItem(g_hWnd[RMT_FLST_SEL], &itm);
				}
				break;
				default:
					break;
				}
			}
		}

	}
	free(buf);
}
void MyFileAccept(void *)
{
	//LPAcceptProc func = (LPAcceptProc)data;
	g_ssfm.InitListen(SEVER_FPORT, FileAcceptProc);
}
void FileAcceptProc(void* data)
{
	SocketInfo* si = (SocketInfo*)data;
	SockSendFileManager* psssm = new SockSendFileManager();
	si->data = psssm;

	char buf[128];
	//MessageBox(NULL, , "fffff", 1);

	{
		MyMutex m(&g_cs_list); 
		inet_ntop(AF_INET, &si->addr.sin_addr.S_un.S_addr, buf, 128);
		
		g_cmap[buf].fsi = si;
		
	}
	g_ssfm.Accept(data);

}

bool MyVisitFile(const char* lpPath, int kind, void* data)
{
	static int row = 0;
	LV_ITEM itm = { 0 };
	itm.mask = LVFIF_TEXT;

	char buf[MAX_PATH];
	sprintf_s(buf, "[%c]%s", (kind & FILE_ATTRIBUTE_DIRECTORY) ? 'D' : 'F', lpPath);
	itm.iItem = row;
	itm.iSubItem = 0;
	itm.pszText = (LPSTR)buf;
	itm.iImage = 2;

	ListView_InsertItem(g_hWnd[LCL_FLST_SEL], &itm);

//	ListView_SetItem(g_hWnd[LCL_FLST_SEL], &itm);

	return true;
}