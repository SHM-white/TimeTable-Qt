// 定义应用程序的入口点。
//
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <Windows.h>
#include <strsafe.h>
#include <windowsx.h>
#include <tchar.h>

#include "include\json\json.h"

#include "framework.h"
#include "Lesson.h"
#include "TextFormat.h"
#include "WindowSettings.h"
#include "Timetable.h"


#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
TCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
WindowSettings windowsettings{ ".\\Config.json" };
HWND hStaticText;
TimeTable timetable{ windowsettings.msLessonInfoFile };
// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    AddLesson(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    DialogMore(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    TextView(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ShowAll(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Settings(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Futures(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ImportLessons(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。
    

    // 初始化全局字符串
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_TIMETABLE20, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TIMETABLE20));
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
    return (int) msg.wParam;
}

//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TIMETABLE20));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_TIMETABLE20);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中
   
   HWND hWnd = CreateWindowA(szWindowClass, szTitle,
       WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
       windowsettings.miWindowX,
       windowsettings.miWindowY,
       windowsettings.miWindowWeight,
       windowsettings.miWindowHeight,
       nullptr, nullptr, hInstance, nullptr);
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
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //主窗口消息处理函数
    //HFONT hfont;
    static std::string Text;
    HMENU hMenu, hMenuPopup;
    TCHAR szMenuItem[64]{ 0 };
    POINT pt;
    switch (message)
    {
    case WM_CREATE:
    {
        SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);//默认置顶窗口
        SetTimer(hWnd, IDT_TIMER1, 1000, (TIMERPROC)NULL);//创建时间为1秒的计时器
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    case WM_TIMER:
        switch (wParam)
        {
        case IDT_TIMER1://每秒更新窗口内容
            // process the 1-second timer 
            InvalidateRect(hWnd, NULL, TRUE);
            UpdateWindow(hWnd);
            return 0;
        }
        break;
    case WM_COMMAND:
        {
        HMENU hMenu = GetMenu(hWnd);
        int wmId = LOWORD(wParam);
        LPMENUITEMINFO lpMenuItemInfo;
            // 分析菜单选择:
            switch (wmId)
            {
            //MessageBox(hWnd, TEXT("咕咕咕"), TEXT("咕咕咕"), MB_OK);
            //上方注释用来暂时占位未完成的功能
            case IDC_AbotButton:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_ADD:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ADDLESSON), hWnd, AddLesson);
                break;
            case IDM_TOTOP://切换是否置顶窗口
                lpMenuItemInfo=windowsettings.mSwitchMenuItemCheck(hWnd, hMenu, IDM_TOTOP);
                //返回的结构是已经更改了勾选状态的菜单项属性
                if (lpMenuItemInfo->fState & MFS_CHECKED) {//菜单项checked
                    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                }
                else {//菜单项unchecked
                    SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                }
                break;
            case IDM_SHOWALL:
                ShowWindow(CreateDialog(hInst, MAKEINTRESOURCE(IDD_SHOWALL), hWnd, ShowAll), SW_SHOW);
                break;
            case IDM_SETTINGS:
                MessageBox(hWnd, TEXT("咕咕咕"), TEXT("咕咕咕"), MB_OK);
                //DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGS), hWnd, Settings);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case IDM_IMPORT:
                ShowWindow(CreateDialog(hInst, MAKEINTRESOURCE(IDD_IMPORT), hWnd, ImportLessons), SW_SHOW);
                break;
            case IDM_MoreInfo:
                ShowWindow(CreateDialog(hInst, MAKEINTRESOURCE(IDD_MOREINFO), hWnd, DialogMore), SW_SHOW);
                break;
            case IDM_FUTURES:
                ShowWindow(CreateDialog(hInst, MAKEINTRESOURCE(IDD_TEXTVIEW), hWnd, Futures), SW_SHOW);
                break;
            case IDM_MINIMIZE:
                ShowWindow(hWnd, SW_MINIMIZE);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT://hdc绘图函数
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            
            //SetBkColor(hdc, windowsettings.mcBackGroundColor);
            windowsettings.mPrintText(hdc,timetable);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_LBUTTONDOWN:
        SetCursor(LoadCursor(NULL, IDC_SIZEALL));
        SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        break;
    case WM_LBUTTONUP:
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        break;
    case WM_RBUTTONDOWN:
        hMenu = GetMenu(hWnd);
        hMenuPopup = CreatePopupMenu();
        for (int i{ 0 }; i < GetMenuItemCount(hMenu); i++) {
            GetMenuString(hMenu, i, szMenuItem, _countof(szMenuItem), MF_BYPOSITION);
            AppendMenu(hMenuPopup, MF_STRING | MF_POPUP, (UINT_PTR)GetSubMenu(hMenu, i), szMenuItem);
        }
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        ClientToScreen(hWnd, &pt);
        TrackPopupMenu(hMenuPopup, TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, 0, hWnd, NULL);
        break;
    case WM_DESTROY:
        KillTimer(hWnd, IDT_TIMER1);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK DialogMore(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    std::vector<std::string> Infos;
    TCHAR szWeek[16];
    TCHAR szInfo[256];
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        if (timetable.mGetTodayMoreInfo(Infos)) {
            for (auto a : Infos) {
                SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_ADDSTRING, 0, (LPARAM)a.c_str());
            }
        }
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {

        case IDOK:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
            break;
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
            break;
        case IDC_ADD://检测文本框是否有文本，并存储
            if (
                GetWindowTextA(GetDlgItem(hDlg, IDC_EDIT2), szWeek, sizeof(szWeek)) &&
                GetWindowTextA(GetDlgItem(hDlg, IDC_EDIT1), szInfo, sizeof(szInfo))
                )
            {
                timetable.mAddMoreInfo(szWeek, szInfo);
                MessageBox(hDlg, TEXT("成功"), TEXT("提示"), MB_OK);
                {//刷新列表框内容
                    SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_RESETCONTENT, 0, 0);
                    if (timetable.mGetTodayMoreInfo(Infos)) {
                        for (auto a : Infos) {
                            SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_ADDSTRING, 0, (LPARAM)a.c_str());
                        }
                    }
                }
                break;
            }
            MessageBox(hDlg, TEXT("失败"), TEXT("提示"), MB_OK);
            break;
        case IDC_FLASH:
            SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_RESETCONTENT, 0, 0);
            if (timetable.mGetTodayMoreInfo(Infos)) {
                for (auto a : Infos) {
                    SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_ADDSTRING, 0, (LPARAM)a.c_str());
                }
            }
            return (INT_PTR)TRUE;
            break;
        default:
            break;
        }
        break;
    }
    return (INT_PTR)FALSE;
};
INT_PTR CALLBACK AddLesson(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    TCHAR szLessonName[50]{};
    TCHAR szBeginTime[16]{};
    TCHAR szEndTime[16]{};
    TCHAR szWeek[16]{};
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_ADD:
            if (
                GetWindowTextA(GetDlgItem(hDlg, IDC_EDIT4), szWeek, sizeof(szWeek))&&
                GetWindowTextA(GetDlgItem(hDlg, IDC_EDIT1), szLessonName, sizeof(szLessonName))&&
                GetWindowTextA(GetDlgItem(hDlg, IDC_EDIT2), szBeginTime, sizeof(szBeginTime))&&
                GetWindowTextA(GetDlgItem(hDlg, IDC_EDIT3), szEndTime, sizeof(szEndTime))
                ) {
                timetable.mAddLesson(szWeek, szLessonName, szBeginTime, szEndTime);
                MessageBox(hDlg, TEXT("成功"), TEXT("提示"), MB_OK);
                break;
            };
            MessageBox(hDlg, TEXT("失败"), TEXT("提示"), MB_OK);
            break;
        case IDOK:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
            break;
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
            break;
        default:
            break;
        } 
        break;
    }
    return (INT_PTR)FALSE;
};
INT_PTR CALLBACK ShowAll(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    std::vector<std::string> Lessons;
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        if (timetable.mGetLesson(Lessons)){
            for (auto a : Lessons) {
                SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_ADDSTRING, 0, (LPARAM)a.c_str());
            }
        }
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {

        case IDOK:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
            break;
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
            break;
        case IDC_FLASH:
            SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_RESETCONTENT, 0, 0);
            if (timetable.mGetLesson(Lessons)){
                for (auto a : Lessons) {
                    SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_ADDSTRING, 0, (LPARAM)a.c_str());
                }
            }
            return (INT_PTR)TRUE;
            break;
        default:
            break;
        }
        break;
    }
    return (INT_PTR)FALSE;
};
INT_PTR CALLBACK Settings(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
            break;
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
            break;
        default:
            break;
        }
        break;
    }
    return (INT_PTR)FALSE;
};
INT_PTR CALLBACK ImportLessons(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndText1;
    static HWND hwndText2;
    TCHAR szFile[256] = { 0 };   // 返回用户选择的文件名的缓冲区大一点，本程序允许多选
    TCHAR szFileTitle[256] = { 0 };    // 返回用户所选文件的文件名和扩展名的缓冲区
    HANDLE hf;
    OPENFILENAME ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetParent(hDlg);
    ofn.lpstrFilter =
        TEXT("ANSI编码的csv文件(*.csv)\0*.csv\0JSON文件(*.json)\0*.json\0All(*.*)\0*.*\0");
    ofn.nFilterIndex = 1;                       // 默认选择第1个过滤器
    ofn.lpstrFile = szFile;                     // 返回用户选择的文件名的缓冲区
    ofn.lpstrFile[0] = NULL;                    // 不需要初始化文件名编辑控件
    ofn.nMaxFile = _countof(szFile);
    ofn.lpstrFileTitle = szFileTitle;	        // 返回用户选择的文件的文件名和扩展名的缓冲区
    ofn.nMaxFileTitle = _countof(szFileTitle);
    ofn.lpstrInitialDir = TEXT("C:\\");         // 初始目录

    LPTSTR lpStr;
    TCHAR szDir[256] = { 0 };
    TCHAR szBuf[256] = { 0 };

    TCHAR szCSVPath[256];
    TCHAR szTargetJsonPath[256];
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        hwndText1 = GetDlgItem(hDlg, IDC_EDIT1);
        hwndText2 = GetDlgItem(hDlg, IDC_EDIT2);
        SetWindowText(hwndText2, (LPCSTR)timetable.mGetLessonInfoPath().c_str());
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            if (
                GetWindowTextA(hwndText1, szCSVPath, sizeof(szCSVPath)) &&
                GetWindowTextA(hwndText2, szTargetJsonPath, sizeof(szTargetJsonPath))
                ) {
                if (timetable.mImportLessonsFromCsv(szCSVPath, szTargetJsonPath)) {
                    MessageBox(hDlg, TEXT("成功"), TEXT("提示"), MB_OK);
                    EndDialog(hDlg, LOWORD(wParam));
                    return (INT_PTR)TRUE;
                    break;
                }
            }
            MessageBox(hDlg, TEXT("失败"), TEXT("提示"), MB_OK);
            return (INT_PTR)TRUE;
            break;
        case IDC_OPEN:
            if (!MessageBox(hDlg, TEXT("该按钮会影响文件读写，是否打开选择文件对话框？"), TEXT("提示"), MB_OKCANCEL)) {
                ofn.nFilterIndex = 1;
                ofn.lpstrTitle = TEXT("请选择要打开的文件");// 对话框标题栏中显示的字符串
                ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_CREATEPROMPT;
                if (GetOpenFileName(&ofn)) {
                    /*hf = CreateFile(ofn.lpstrFile,
                        GENERIC_READ,
                        0,
                        (LPSECURITY_ATTRIBUTES)NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        (HANDLE)NULL);*/
                        //更改文本框显示内容
                    SetWindowText(hwndText1, ofn.lpstrFile);
                    //CloseHandle(hf);
                }
            }
            //ofn = {};
            break;
        case IDC_SAVEAS:
            if (!MessageBox(hDlg, TEXT("该按钮会影响文件读写，是否打开选择文件对话框？"), TEXT("提示"), MB_OKCANCEL)) {
                ofn.nFilterIndex = 2;
                ofn.lpstrTitle = TEXT("请选择要保存的文件名");  // 对话框标题栏中显示的字符串
                ofn.lpstrDefExt = TEXT("json");                  // 默认扩展名
                ofn.Flags = OFN_EXPLORER | OFN_CREATEPROMPT;
                if (GetSaveFileName(&ofn))
                {
                    SetWindowText(hwndText2, ofn.lpstrFile);
                }
            }
            //ofn = {};
            break;
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
            break;
        default:
            break;
        }
        break;
    }
    return (INT_PTR)FALSE;
};
INT_PTR CALLBACK Futures(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    std::string futures;
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        windowsettings.mGetTextItem("futures", futures);
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT1), futures.c_str());
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
            break;
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
            break;
        default:
            break;
        }
        break;
    }
    return (INT_PTR)FALSE;
};
//以下为暂时没啥用的
INT_PTR CALLBACK TextView(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
            break;
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
            break;
        default:
            break;
        }
        break;
    }
    return (INT_PTR)FALSE;
};
// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    std::string About;
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        windowsettings.mGetTextItem("About", About);
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT1), About.c_str());
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
