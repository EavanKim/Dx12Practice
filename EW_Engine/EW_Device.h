#pragma once

namespace EW
{

	namespace Graphic
	{
		struct WindowInfo
		{
			static const int SwapChainBufferCount = 2;

			explicit WindowInfo(int _Width, int _Height);
			~WindowInfo();

			void Resizing(int _Width, int _Height);
			ID3D12Resource* CurrentBackBuffer();

			int ClientWidth = 800;
			int ClientHeight = 600;
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList;
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

			int m_CurrBackBuffer = 0;
			Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;
			Microsoft::WRL::ComPtr<ID3D12Resource> m_SwapChainBuffer[SwapChainBufferCount];
			Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencilBuffer;

			bool m_4xMsaaState = false;
			UINT m_4xMsaaQuality = 0;

			DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
			DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
			D3D12_VIEWPORT ScreenViewport;
			D3D12_RECT ScissorRect;
		};

		struct EW_ObjectConstants
		{
			DirectX::XMFLOAT4X4 World = Identity4x4();
			DirectX::XMFLOAT4X4 TexTransform = Identity4x4();
			UINT     MaterialIndex;
			UINT     ObjPad0;
			UINT     ObjPad1;
			UINT     ObjPad2;
		};

		class EW_CameraConstance
		{
			DirectX::XMFLOAT4X4 View = Identity4x4();
			DirectX::XMFLOAT4X4 InvView = Identity4x4();
			DirectX::XMFLOAT4X4 Proj = Identity4x4();
			DirectX::XMFLOAT4X4 InvProj = Identity4x4();
			DirectX::XMFLOAT4X4 ViewProj = Identity4x4();
			DirectX::XMFLOAT4X4 InvViewProj = Identity4x4();
			DirectX::XMFLOAT4X4 ViewProjTex = Identity4x4();
			DirectX::XMFLOAT4X4 ShadowTransform = Identity4x4();
			DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
			float cbPerObjectPad1 = 0.0f;
			DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
			DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
			float NearZ = 0.0f;
			float FarZ = 0.0f;
			float TotalTime = 0.0f;
			float DeltaTime = 0.0f;

			//DirectX::XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

			// Indices [0, NUM_DIR_LIGHTS) are directional lights;
			// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
			// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
			// are spot lights for a maximum of MaxLights per object.
			//Light Lights[MaxLights];
		};

		class EW_Frame : public EW_Base
		{
		public:
			Microsoft::WRL::ComPtr<ID3D12CommandList> ReturnCommandList();



			UINT64 Fence = 0;
		};

		class EW_Device : public EW_Base
		{
		public:
			static void CreateInstance(int _ClientWidth, int _ClientHeight);

			static EW_Device* GetInstance();

			void DestroyInstance();

			void TestDrawing();

			void DrawingFrame();

			void DrawObject(EW_Frame _RenderPath, std::queue<Obj::EW_Obj> _RenderTarget);

			ID3D12Device* GetD3D12evice();

			void FlushCommandQueue();

			static UINT CalcConstantBufferByteSize(UINT byteSize)
			{
				// Constant buffers must be a multiple of the minimum hardware
				// allocation size (usually 256 bytes).  So round up to nearest
				// multiple of 256.  We do this by adding 255 and then masking off
				// the lower 2 bytes which store all bits < 256.
				// Example: Suppose byteSize = 300.
				// (300 + 255) & ~255
				// 555 & ~255
				// 0x022B & ~0x00ff
				// 0x022B & 0xff00
				// 0x0200
				// 512
				return (byteSize + 255) & ~255;
			}

			Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
				const std::wstring& filename,
				const D3D_SHADER_MACRO* defines,
				const std::string& entrypoint,
				const std::string& target)
			{
				UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
				compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

				HRESULT hr = S_OK;

				Microsoft::WRL::ComPtr<ID3DBlob> byteCode = nullptr;
				Microsoft::WRL::ComPtr<ID3DBlob> errors;
				hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
					entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

				if (errors != nullptr)
					OutputDebugStringA((char*)errors->GetBufferPointer());

				ThrowIfFailed(hr);

				return byteCode;
			}

		protected:
			EW_Frame* GetLastFrame();

			bool InitializeDevice(int _ClientWidth, int _ClientHeight);
			void OnResize(int _ClientWidth, int _ClientHeight);
			void CreateCommandObjects();
			void CreateSwapChain(int _ClientWidth, int _ClientHeight);
			void CreatePipelineHeaps();

		private:
			explicit EW_Device(int _ClientWidth, int _ClientHeight);
			virtual ~EW_Device();

			Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_GPUQueue = nullptr;
			Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
			UINT64 m_CurrentFence = 0;

			Microsoft::WRL::ComPtr<IDXGIFactory4> m_dxgiFactory = nullptr;

			std::unique_ptr<WindowInfo> m_WindowInfo = nullptr;
			
			Microsoft::WRL::ComPtr<ID3D12Device> m_d3dDevice = nullptr;

			concurrency::concurrent_queue<EW_Frame*>* m_DoingQueue = nullptr;

			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RenderTargetView = nullptr;
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DepthStancilView = nullptr;

			Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature = nullptr;
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_ConstantBufferView = nullptr;

			Microsoft::WRL::ComPtr<ID3DBlob> m_VertexShader = nullptr;
			Microsoft::WRL::ComPtr<ID3DBlob> m_PixelShader = nullptr;

			std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;

			Microsoft::WRL::ComPtr<ID3D12PipelineState> mPSO = nullptr;

			UINT m_RtvDescriptorSize = 0;
			UINT m_DsvDescriptorSize = 0;
			UINT m_CbvSrvUavDescriptorSize = 0;

			D3D_DRIVER_TYPE m_d3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
			DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
			DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
			static EW_Device* m_Instance;
		};
	}
}

