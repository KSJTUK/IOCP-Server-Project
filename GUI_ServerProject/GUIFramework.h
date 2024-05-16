#pragma once

struct Area {
	int width;
	int height;
};

class GUIFramework {
public:
	GUIFramework() { }
	~GUIFramework() { }

public:
	bool Init(HINSTANCE inst);
	
	bool CreateMainWindow();
	ATOM MyResisterClass();

	void FrameAdvance();

	void Run();

public:
	static LRESULT __stdcall WndMsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	HINSTANCE m_instance{ };
	HWND m_window{ };
	Area m_clientArea{ 800, 600 };

	std::wstring m_titleW{ L"GUI Server" };
	std::string m_title{ "GUI Server" };
};