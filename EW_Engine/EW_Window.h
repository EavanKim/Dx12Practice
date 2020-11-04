#pragma once

#define MAX_LOADSTRING 100

namespace EW
{
	namespace Window
	{
		class EW_Window : public EW_Base
		{
		public:
			static void CreateInstance(std::wstring _Title, std::wstring _WindowClass)
			{
				if (nullptr == m_Instance)
					m_Instance = new EW_Window(_Title, _WindowClass);
			}

			static EW_Window* GetInstance()
			{
				return m_Instance;
			}


			BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, LRESULT(__stdcall* _FunctionPoint)(HWND, UINT, WPARAM, LPARAM));

			ATOM MyRegisterClass(LRESULT(__stdcall* _FunctionPoint)(HWND, UINT, WPARAM, LPARAM));

			void DestroyInstance()
			{
				DestroyWindow(hWnd);
				delete this;
				PostQuitMessage(0);
			}

			HWND GetHandle();

		private:
			EW_Window(std::wstring _Title, std::wstring _WindowClass);
			~EW_Window();

			HWND hWnd = NULL;						
			HINSTANCE hInst = NULL;					
			WCHAR szTitle[MAX_LOADSTRING];			
			WCHAR szWindowClass[MAX_LOADSTRING];	

			static EW_Window* m_Instance;
		};
	}
}

