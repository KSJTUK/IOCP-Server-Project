// GUI_ServerProject.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "pch.h"
#include "framework.h"
#include "GUI_ServerProject.h"
#include "GUIFramework.h"

INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    std::unique_ptr<MainGuiFramework> game = std::make_unique<MainGuiFramework>();
    if (!game->Init(hInstance)) {
        return EXIT_FAILURE;
    }

    game->Run();
}