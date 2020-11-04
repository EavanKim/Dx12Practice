// EW_Engine.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "pch.h"

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void ManagementWakeUp(HINSTANCE hInstance, int nCmdShow, LRESULT(__stdcall* _FunctionPoint)(HWND, UINT, WPARAM, LPARAM));
void ManagementSleep();
void Idle();
void Update();
void MessageingLogic(MSG& _msg, HACCEL& _hAccelTable);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_EWENGINE));

	ManagementWakeUp(hInstance, nCmdShow, WndProc);

	MSG msg = {};
	
	// 기본 메시지 루프입니다:

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			MessageingLogic(msg, hAccelTable);
		}
		else
		{
			Update();
			Idle();
		}
	}

	ManagementSleep();

	return (int)msg.wParam;
}

void ManagementWakeUp(HINSTANCE hInstance, int nCmdShow, LRESULT(__stdcall* _FunctionPoint)(HWND, UINT, WPARAM, LPARAM))
{
	EW::Window::EW_Window::CreateInstance(L"Dx12", L"Dx12");
	EW::Window::EW_Window::GetInstance()->InitInstance(hInstance, nCmdShow, _FunctionPoint);
	EW::Graphic::EW_Device::CreateInstance(800, 600);
}

void ManagementSleep()
{
	EW::Window::EW_Window::GetInstance()->DestroyInstance();
	EW::Graphic::EW_Device::GetInstance()->DestroyInstance();
}

void Idle()
{
	//EW::Graphic::EW_Device::GetInstance()->DrawingFrame();
	EW::Graphic::EW_Device::GetInstance()->TestDrawing();
}

void Update()
{

}

void MessageingLogic(MSG& _msg, HACCEL& _hAccelTable)
{
	if (!TranslateAccelerator(_msg.hwnd, _hAccelTable, &_msg))
	{
		TranslateMessage(&_msg);
		DispatchMessage(&_msg);
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_COMMAND:
			{
				int wmId = LOWORD(wParam);
				// 메뉴 선택을 구문 분석합니다:
				switch (wmId)
				{
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
				// TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
				EndPaint(hWnd, &ps);
			}
			break;
		case WM_KEYDOWN:
			switch (wParam)
			{
				case VK_ESCAPE:
					{
						PostMessage(hWnd, WM_DESTROY, 0, 0);
					}
					break;
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			PostMessage(hWnd, WM_QUIT, 0, 0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}