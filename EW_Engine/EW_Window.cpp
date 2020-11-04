#include "pch.h"

namespace EW
{
	namespace Window
	{
		EW_Window* EW_Window::m_Instance = nullptr;



		HWND EW_Window::GetHandle()
		{
			return hWnd;
		}

		EW_Window::EW_Window(std::wstring _Title, std::wstring _WindowClass)
		{
			ZeroMemory(szTitle, sizeof(szTitle));
			memcpy_s(szTitle, 200, _Title.c_str(), 200);
			ZeroMemory(szWindowClass, sizeof(szWindowClass));
			memcpy_s(szWindowClass, 200, _WindowClass.c_str(), 200);
		}

		EW_Window::~EW_Window()
		{
		}

		BOOL EW_Window::InitInstance(HINSTANCE hInstance, int nCmdShow, LRESULT(__stdcall* _FunctionPoint)(HWND, UINT, WPARAM, LPARAM))
		{
			hInst = hInstance;

			MyRegisterClass(_FunctionPoint);

			hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

			if (!hWnd)
			{
				return FALSE;
			}

			ShowWindow(hWnd, nCmdShow);
			UpdateWindow(hWnd);

			return TRUE;
		}

		ATOM EW_Window::MyRegisterClass(LRESULT(__stdcall* _FunctionPoint)(HWND, UINT, WPARAM, LPARAM))
		{
			WNDCLASSEXW wcex;

			wcex.cbSize = sizeof(WNDCLASSEX);

			wcex.style = CS_HREDRAW | CS_VREDRAW;
			wcex.lpfnWndProc = (WNDPROC)_FunctionPoint;
			wcex.cbClsExtra = 0;
			wcex.cbWndExtra = 0;
			wcex.hInstance = hInst;
			wcex.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SIRRANG));
			wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
			wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
			wcex.lpszMenuName = MAKEINTRESOURCEW(IDI_SIRRANG);
			wcex.lpszClassName = szWindowClass;
			wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

			return RegisterClassExW(&wcex);
		}
	}
}