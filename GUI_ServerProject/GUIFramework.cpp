#include "pch.h"
#include "GUIFramework.h"
#include "framework.h"
#include "Resource.h"

bool GUIFramework::Init(HINSTANCE inst) {
    m_instance = inst;

    auto result = MyResisterClass();
    auto r = ::GetLastError();
    if (!CreateMainWindow()) {
        return false;
    }

	return true;
}

bool GUIFramework::CreateMainWindow() {
    m_window = ::CreateWindowW(m_titleW.c_str(), m_titleW.c_str(), WS_OVERLAPPEDWINDOW,
        100, 100, m_clientArea.width, m_clientArea.height, nullptr, nullptr, m_instance, nullptr);

    if (!m_window) {
        return false;
    }

    ::ShowWindow(m_window, SW_SHOW);
    ::UpdateWindow(m_window);

    return true;
}

ATOM GUIFramework::MyResisterClass() {
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = GUIFramework::WndMsgProc;
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

void GUIFramework::FrameAdvance() {

}

void GUIFramework::Run() {
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

LRESULT __stdcall GUIFramework::WndMsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
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
