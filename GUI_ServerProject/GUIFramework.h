#pragma once

struct Area {
	int width;
	int height;
};

class SwapBuffer {
public:
	SwapBuffer();
	~SwapBuffer();

public:
	void CreateBuffers(HWND window);
	void ClearBuffer();

	void Present();

	HDC GetMemDc() const { return m_memDc; }

private:
	HWND m_window{ };
	HDC m_mainDc{ };
	HDC m_memDc{ };
	HBITMAP m_backBuffer{ };
	RECT m_clientRc{ };

	DWORD m_bgColor{ RGB(255, 0, 0) };

	HPEN m_bgPen{ };
	HBRUSH m_bgBrush{ };
};

class MainGuiFramework {
public:
	MainGuiFramework() { }
	~MainGuiFramework() { }

public:
	bool Init(HINSTANCE inst);
	
	bool CreateMainWindow();
	ATOM MyResisterClass();

	bool CreateEditBox();

	void FrameAdvance();

	void Run();

public:
	static LRESULT __stdcall WndMsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	HINSTANCE m_instance{ };
	HWND m_window{ };
	Area m_clientArea{ 800, 600 };

	HWND m_editWindow{ };

	SwapBuffer m_backBuffer{ };

	std::wstring m_titleW{ L"GUI Server" };
	std::string m_title{ "GUI Server" };
};