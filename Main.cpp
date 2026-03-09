// Direct2D.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Direct2D.h"

#include <d2d1.h>                 // *** DIRECT2D ADDED ***
#pragma comment(lib, "d2d1.lib")  // *** DIRECT2D ADDED ***
#define MAX_LOADSTRING 100
#include <vector>

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

// *** DIRECT2D ADDED: Global graphics objects ***
ID2D1Factory* pFactory = nullptr;
ID2D1HwndRenderTarget* pRenderTarget = nullptr;
ID2D1SolidColorBrush* pBrush = nullptr;










struct MyRect
{
    float x, y;       // Top-left position
    float width, height;
    D2D1_COLOR_F color; // Color

    // Helper function to get D2D rectangle
    D2D1_RECT_F getRectF() const
    {
        return D2D1::RectF(x, y, x + width, y + height);
    }

    // Helper function to check if point is inside
    bool contains(float px, float py) const
    {
        return px >= x && px <= x + width &&
            py >= y && py <= y + height;
    }
};



// In your globals
std::vector<MyRect> rectangles;

// Dragging state
int draggingIndex = -1;
float dragOffsetX = 0;
float dragOffsetY = 0;


// Forward declarations
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR    lpCmdLine,
    int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DIRECT2D, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
        return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DIRECT2D));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DIRECT2D));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_DIRECT2D);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
        return FALSE;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

        // *** DIRECT2D ADDED: Initialize Direct2D when window is created ***
    case WM_CREATE:
    {
        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);

        RECT rc;
        GetClientRect(hWnd, &rc);

        pFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(
                hWnd,
                D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)
            ),
            &pRenderTarget
        );

        pRenderTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::DeepSkyBlue),
            &pBrush
        );


        // --- Initialize rectangles here ---
        rectangles.push_back({ 800, 100, 200, 200, D2D1::ColorF(D2D1::ColorF::Red) });
        rectangles.push_back({ 100, 100, 200, 200, D2D1::ColorF(D2D1::ColorF::DeepSkyBlue) });
    }
    break;

    case WM_LBUTTONDOWN:
    {
        int mx = LOWORD(lParam);
        int my = HIWORD(lParam);

        // Loop over rectangles in reverse to select top-most first
        for (int i = rectangles.size() - 1; i >= 0; --i)
        {
            if (rectangles[i].contains(mx, my))
            {
                draggingIndex = i;
                dragOffsetX = mx - rectangles[i].x;
                dragOffsetY = my - rectangles[i].y;
                SetCapture(hWnd);
                break;
            }
        }
    }
    break;

    case WM_MOUSEMOVE:
    {
        if (draggingIndex != -1)
        {
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);
            rectangles[draggingIndex].x = mx - dragOffsetX;
            rectangles[draggingIndex].y = my - dragOffsetY;
            InvalidateRect(hWnd, nullptr, FALSE);
        }
    }
    break;

    case WM_LBUTTONUP:
    {
        if (draggingIndex != -1)
        {
            draggingIndex = -1;
            ReleaseCapture();
        }
    }
    break;

    case WM_KEYDOWN: // dynamically create new rectangles
    {
        switch (wParam)
        {
        case 'N':  // Press 'N' to add a new rectangle
        {
            // Add a new rectangle at (50,50), size 100x100, green
            rectangles.push_back({ 50, 50, 100, 100, D2D1::ColorF(D2D1::ColorF::LimeGreen) });

            // Force repaint
            InvalidateRect(hWnd, nullptr, FALSE);
        }
        break;
        }
    }
    break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
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

    // *** DIRECT2D MODIFIED: Drawing with GPU instead of GDI ***
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);

        pRenderTarget->BeginDraw();
        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));



        for (auto& r : rectangles)
        {
            pBrush->SetColor(r.color);
            pRenderTarget->FillRectangle(r.getRectF(), pBrush);
        }

        pRenderTarget->EndDraw();
        EndPaint(hWnd, &ps);
    }
    break;

    // *** DIRECT2D MODIFIED: Release GPU resources ***
    case WM_DESTROY:
        if (pBrush) pBrush->Release();
        if (pRenderTarget) pRenderTarget->Release();
        if (pFactory) pFactory->Release();
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

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
