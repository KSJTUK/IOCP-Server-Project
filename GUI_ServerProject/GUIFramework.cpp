#include "pch.h"
#include "GUIFramework.h"
#include "framework.h"
#include "Resource.h"

bool MainGuiFramework::Init(HINSTANCE inst) {
    m_instance = inst;

    MyResisterClass();
    if (!CreateMainWindow()) {
        return false;
    }

    m_backBuffer.CreateBuffers(m_window);
    CreateEditBox();

	return true;
}

bool MainGuiFramework::CreateMainWindow() {
    m_window = ::CreateWindowW(m_titleW.c_str(), m_titleW.c_str(), WS_OVERLAPPEDWINDOW,
        100, 100, m_clientArea.width, m_clientArea.height, nullptr, nullptr, m_instance, nullptr);

    if (!m_window) {
        return false;
    }

    ::ShowWindow(m_window, SW_SHOW);
    ::UpdateWindow(m_window);

    return true;
}

ATOM MainGuiFramework::MyResisterClass() {
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = MainGuiFramework::WndMsgProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = m_instance;
    wcex.hIcon = NULL; //LoadIcon(m_instance, MAKEINTRESOURCE(IDI_GUISERVERPROJECT));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = m_titleW.c_str();
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//bool MainGuiFramework::CreateEditBox() {
//    m_editWindow = ::CreateWindowW(L"edit", NULL, WS_BORDER | WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_RIGHT, 20, 20, 100, 25, m_window, NULL, m_instance, NULL);
//    return true;
//}

void MainGuiFramework::FrameAdvance() {
    /*m_backBuffer.ClearBuffer();

    m_backBuffer.Present();*/
}

void MainGuiFramework::Run() {
    HACCEL hAccelTable = LoadAccelerators(m_instance, MAKEINTRESOURCE(IDC_GUISERVERPROJECT));
    MSG msg;

    // 기본 메시지 루프입니다:
    while (true) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                break;

            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else {
            FrameAdvance();
        }
    }
}

LRESULT __stdcall MainGuiFramework::WndMsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

SwapBuffer::SwapBuffer() { }

SwapBuffer::~SwapBuffer() {
    ::DeleteObject(m_bgBrush);
    ::DeleteObject(m_bgPen);
    ::DeleteObject(m_backBuffer);
    ::DeleteDC(m_memDc);
    ::ReleaseDC(m_window, m_mainDc);
}

void SwapBuffer::CreateBuffers(HWND window) {
    m_window = window;
    m_mainDc = ::GetDC(m_window);
    m_memDc = ::CreateCompatibleDC(m_mainDc);
    ::GetClientRect(window, &m_clientRc);
    m_backBuffer = ::CreateCompatibleBitmap(m_memDc, m_clientRc.right - m_clientRc.left, m_clientRc.bottom - m_clientRc.top);

    m_bgPen = ::CreatePen(PS_SOLID, 0, m_bgColor);
    m_bgBrush = ::CreateSolidBrush(m_bgColor);

    ::SelectObject(m_memDc, m_backBuffer);
    ::SetBkMode(m_memDc, TRANSPARENT);
}

void SwapBuffer::ClearBuffer() {
    HPEN oldPen = (HPEN)::SelectObject(m_memDc, m_bgPen);
    HBRUSH oldBrush = (HBRUSH)::SelectObject(m_memDc, m_bgBrush);

    ::Rectangle(m_memDc, m_clientRc.left, m_clientRc.top, m_clientRc.right, m_clientRc.top);

    ::SelectObject(m_memDc, oldPen);
    ::SelectObject(m_memDc, oldBrush);
}

void SwapBuffer::Present() {
    int w = m_clientRc.right - m_clientRc.left;
    int h = m_clientRc.bottom - m_clientRc.top;
    ::BitBlt(m_mainDc, m_clientRc.left, m_clientRc.top, w, h,
        m_memDc, m_clientRc.left, m_clientRc.top, SRCCOPY);
}
