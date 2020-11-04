#include "pch.h"

namespace EW
{


	namespace Graphic
	{
		WindowInfo::WindowInfo(int _Width, int _Height)
		{
			Resizing(_Width, _Height);
		}

		WindowInfo::~WindowInfo()
		{
			CommandList.Reset();
			CommandList = nullptr;
			CmdListAlloc.Reset();
			CmdListAlloc = nullptr;
		}

		void WindowInfo::Resizing(int _Width, int _Height)
		{
			ClientWidth = _Width;
			ClientHeight = _Height;

			ScreenViewport.TopLeftX = 0;
			ScreenViewport.TopLeftY = 0;
			ScreenViewport.Width = static_cast<float>(ClientWidth);
			ScreenViewport.Height = static_cast<float>(ClientHeight);
			ScreenViewport.MinDepth = 0.0f;
			ScreenViewport.MaxDepth = 1.0f;

			ScissorRect = { 0, 0, ClientWidth, ClientHeight };
		}

		ID3D12Resource* WindowInfo::CurrentBackBuffer()
		{
			return m_SwapChainBuffer[m_CurrBackBuffer].Get();
		}



		EW_Frame* EW_Device::GetLastFrame()
		{
			EW_Frame* CurrentFrame = nullptr;

			if (nullptr != m_DoingQueue)
			{
				m_DoingQueue->try_pop(CurrentFrame);
			}

			return CurrentFrame;
		}

		Microsoft::WRL::ComPtr<ID3D12CommandList> EW_Frame::ReturnCommandList()
		{
			Microsoft::WRL::ComPtr<ID3D12CommandList> Cmdlist = nullptr;



			return Cmdlist;
		}


		EW_Device* EW_Device::m_Instance = nullptr;

		void EW_Device::CreateInstance(int _ClientWidth, int _ClientHeight)
		{
			if (nullptr == m_Instance)
				m_Instance = new EW_Device(_ClientWidth, _ClientHeight);
		}

		EW_Device* EW_Device::GetInstance()
		{
			return m_Instance;
		}

		void EW_Device::DestroyInstance()
		{
			m_Instance->InstanceRelease();
		}

		void EW_Device::TestDrawing()
		{
			D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetViewHandle = D3D12_CPU_DESCRIPTOR_HANDLE();
			RenderTargetViewHandle.ptr = m_RenderTargetView->GetCPUDescriptorHandleForHeapStart().ptr + (m_WindowInfo->m_CurrBackBuffer * m_RtvDescriptorSize);

			D3D12_RESOURCE_BARRIER StartBarrier = D3D12_RESOURCE_BARRIER();
			ZeroMemory(&StartBarrier, sizeof(StartBarrier));
			StartBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			StartBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			StartBarrier.Transition.pResource = m_WindowInfo->CurrentBackBuffer();
			StartBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			StartBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			StartBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			D3D12_RESOURCE_BARRIER EndBarrier = D3D12_RESOURCE_BARRIER();
			ZeroMemory(&EndBarrier, sizeof(EndBarrier));
			EndBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			EndBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			EndBarrier.Transition.pResource = m_WindowInfo->CurrentBackBuffer();
			EndBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			EndBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			EndBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;


			ThrowIfFailed(m_WindowInfo->CmdListAlloc->Reset());
			
			ThrowIfFailed(m_WindowInfo->CommandList->Reset(m_WindowInfo->CmdListAlloc.Get(), nullptr));
			
			m_WindowInfo->CommandList->ResourceBarrier(1, &StartBarrier);

			m_WindowInfo->CommandList->RSSetViewports(1, &m_WindowInfo->ScreenViewport);
			m_WindowInfo->CommandList->RSSetScissorRects(1, &m_WindowInfo->ScissorRect);


			m_WindowInfo->CommandList->ClearRenderTargetView(RenderTargetViewHandle, DirectX::Colors::Black, 0, nullptr);
			m_WindowInfo->CommandList->ClearDepthStencilView(m_DepthStancilView->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

			m_WindowInfo->CommandList->OMSetRenderTargets(1, &RenderTargetViewHandle, true, &m_DepthStancilView->GetCPUDescriptorHandleForHeapStart());

			///GPU Start
			//m_WindowInfo->CommandList->SetGraphicsRootDescriptorTable(0, m_ConstantBufferView->GetGPUDescriptorHandleForHeapStart());

			
			///GPU End

			m_WindowInfo->CommandList->ResourceBarrier(1, &EndBarrier);
			ThrowIfFailed(m_WindowInfo->CommandList->Close());

			ID3D12CommandList* cmdsLists[] = { m_WindowInfo->CommandList.Get() };
			m_GPUQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

			ThrowIfFailed(m_WindowInfo->m_SwapChain->Present(0, 0));
			m_WindowInfo->m_CurrBackBuffer = (m_WindowInfo->m_CurrBackBuffer + 1) % m_WindowInfo->SwapChainBufferCount;

			FlushCommandQueue();
		}

		void EW_Device::DrawingFrame()
		{
			EW_Frame* CurrentFrame = nullptr;

			CurrentFrame = GetLastFrame();

			if (nullptr != CurrentFrame)
			{
				ID3D12CommandList* cmdsLists[] = { CurrentFrame->ReturnCommandList().Get() };
				m_GPUQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

				//ThrowIfFailed(m_SwapChain->Present(0, 0));
				//m_CurrBackBuffer = (m_CurrBackBuffer + 1) % SwapChainBufferCount;

				CurrentFrame->Fence = ++m_CurrentFence;

				m_GPUQueue->Signal(m_Fence.Get(), m_CurrentFence);
			}
		}

		void EW_Device::DrawObject(EW_Frame _RenderPath, std::queue<Obj::EW_Obj> _RenderTarget)
		{

		}

		ID3D12Device* EW_Device::GetD3D12evice()
		{
			return m_d3dDevice.Get();
		}

		bool EW_Device::InitializeDevice(int _ClientWidth, int _ClientHeight)
		{
//#if defined(DEBUG) || defined(_DEBUG) 
//// Enable the D3D12 debug layer.
//			{
//				Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
//				ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
//				debugController->EnableDebugLayer();
//			}
//#endif

			ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));

			// Try to create hardware device.
			HRESULT hardwareResult = D3D12CreateDevice(
				nullptr,             // default adapter
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&m_d3dDevice));

			// Fallback to WARP device.
			if (FAILED(hardwareResult))
			{
				Microsoft::WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
				ThrowIfFailed(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

				ThrowIfFailed(D3D12CreateDevice(
					pWarpAdapter.Get(),
					D3D_FEATURE_LEVEL_11_0,
					IID_PPV_ARGS(&m_d3dDevice)));
			}

			ThrowIfFailed(m_d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
				IID_PPV_ARGS(&m_Fence)));

			m_WindowInfo = std::make_unique<WindowInfo>(_ClientWidth, _ClientHeight);
			m_RtvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			m_DsvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			m_CbvSrvUavDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			// Check 4X MSAA quality support for our back buffer format.
			// All Direct3D 11 capable devices support 4X MSAA for all render 
			// target formats, so we only need to check quality support.

			D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
			msQualityLevels.Format = m_BackBufferFormat;
			msQualityLevels.SampleCount = 4;
			msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
			msQualityLevels.NumQualityLevels = 0;
			ThrowIfFailed(m_d3dDevice->CheckFeatureSupport(
				D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
				&msQualityLevels,
				sizeof(msQualityLevels)));

			m_WindowInfo->m_4xMsaaQuality = msQualityLevels.NumQualityLevels;
			assert(m_WindowInfo->m_4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

			//#ifdef _DEBUG
			//	LogAdapters();
			//#endif

			CreateCommandObjects();
			CreateSwapChain(_ClientWidth, _ClientHeight);
			CreatePipelineHeaps();

			OnResize(_ClientWidth, _ClientHeight);

			D3D12_DESCRIPTOR_HEAP_DESC ConstantBufferViewDesc;
			ConstantBufferViewDesc.NumDescriptors = 1;
			ConstantBufferViewDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			ConstantBufferViewDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			ConstantBufferViewDesc.NodeMask = 0;
			ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&ConstantBufferViewDesc,
				IID_PPV_ARGS(&m_ConstantBufferView)));

			D3D12_ROOT_PARAMETER slotRootParameter[1];

// Create a single descriptor table of CBVs.
			D3D12_DESCRIPTOR_RANGE ConstantBufferViewTable;
			ConstantBufferViewTable.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			ConstantBufferViewTable.NumDescriptors = 1;
			ConstantBufferViewTable.BaseShaderRegister = 0;
			ConstantBufferViewTable.RegisterSpace = 0;
			ConstantBufferViewTable.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


			slotRootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			slotRootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			slotRootParameter[0].DescriptorTable.NumDescriptorRanges = 1;
			slotRootParameter[0].DescriptorTable.pDescriptorRanges = &ConstantBufferViewTable;

			// A root signature is an array of root parameters.
			D3D12_ROOT_SIGNATURE_DESC rootSigDesc;
			rootSigDesc.NumParameters = 1;
			rootSigDesc.pParameters = slotRootParameter;
			rootSigDesc.NumStaticSamplers = 0;
			rootSigDesc.pStaticSamplers = nullptr;
			rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

			// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
			Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
			Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
			HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
				serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

			if (errorBlob != nullptr)
			{
				::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			}
			ThrowIfFailed(hr);

			ThrowIfFailed(m_d3dDevice->CreateRootSignature(
				0,
				serializedRootSig->GetBufferPointer(),
				serializedRootSig->GetBufferSize(),
				IID_PPV_ARGS(&m_RootSignature)));

			m_VertexShader = CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
			m_PixelShader = CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

			m_InputLayout =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};

			return true;
		}

		void EW_Device::CreateCommandObjects()
		{
			D3D12_COMMAND_QUEUE_DESC queueDesc = {};
			queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			ThrowIfFailed(m_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_GPUQueue)));

			ThrowIfFailed(m_d3dDevice->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(m_WindowInfo->CmdListAlloc.GetAddressOf())));

			ThrowIfFailed(m_d3dDevice->CreateCommandList(
				0,
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				m_WindowInfo->CmdListAlloc.Get(), // Associated command allocator
				nullptr,                   // Initial PipelineStateObject
				IID_PPV_ARGS(m_WindowInfo->CommandList.GetAddressOf())));

			// Start off in a closed state.  This is because the first time we refer 
			// to the command list we will Reset it, and it needs to be closed before
			// calling Reset.
			m_WindowInfo->CommandList->Close();
		}

		//Ocurr Binding Window Handle and DirectX
		void EW_Device::CreateSwapChain(int _ClientWidth, int _ClientHeight)
		{
			// Release the previous swapchain we will be recreating.
			m_WindowInfo->m_SwapChain.Reset();

			DXGI_SWAP_CHAIN_DESC sd;
			sd.BufferDesc.Width = _ClientWidth;
			sd.BufferDesc.Height = _ClientHeight;
			sd.BufferDesc.RefreshRate.Numerator = 60;
			sd.BufferDesc.RefreshRate.Denominator = 1;
			sd.BufferDesc.Format = m_WindowInfo->m_BackBufferFormat;
			sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			sd.SampleDesc.Count = m_WindowInfo->m_4xMsaaState ? 4 : 1;
			sd.SampleDesc.Quality = m_WindowInfo->m_4xMsaaState ? (m_WindowInfo->m_4xMsaaQuality - 1) : 0;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.BufferCount = m_WindowInfo->SwapChainBufferCount;
			sd.OutputWindow = EW::Window::EW_Window::GetInstance()->GetHandle();
			sd.Windowed = true;
			sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

			// Note: Swap chain uses queue to perform flush.
			ThrowIfFailed(m_dxgiFactory->CreateSwapChain(
				m_GPUQueue.Get(),
				&sd,
				m_WindowInfo->m_SwapChain.GetAddressOf()));
		}

		void EW_Device::CreatePipelineHeaps()
		{
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
			rtvHeapDesc.NumDescriptors = m_WindowInfo->SwapChainBufferCount;
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			rtvHeapDesc.NodeMask = 0;
			ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(
				&rtvHeapDesc, IID_PPV_ARGS(m_RenderTargetView.GetAddressOf())));


			D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
			dsvHeapDesc.NumDescriptors = 1;
			dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			dsvHeapDesc.NodeMask = 0;
			ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(
				&dsvHeapDesc, IID_PPV_ARGS(m_DepthStancilView.GetAddressOf())));
		}

		void EW_Device::FlushCommandQueue()
		{
			// Advance the fence value to mark commands up to this fence point.
			++m_CurrentFence;

			// Add an instruction to the command queue to set a new fence point.  Because we 
			// are on the GPU timeline, the new fence point won't be set until the GPU finishes
			// processing all the commands prior to this Signal().
			ThrowIfFailed(m_GPUQueue->Signal(m_Fence.Get(), m_CurrentFence));

			// Wait until the GPU has completed commands up to this fence point.
			if (m_Fence->GetCompletedValue() < m_CurrentFence)
			{
				HANDLE eventHandle = CreateEventEx(nullptr, L"", false, EVENT_ALL_ACCESS);

				// Fire event when GPU hits current fence.
				ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrentFence, eventHandle));

				// Wait until the GPU hits current fence event is fired.
				WaitForSingleObject(eventHandle, INFINITE);
				CloseHandle(eventHandle);
			}
		}

		void EW_Device::OnResize(int _ClientWidth, int _ClientHeight)
		{
			try 
			{
				assert(m_d3dDevice);
				assert(m_WindowInfo->m_SwapChain);
				assert(m_WindowInfo->CmdListAlloc);

				// Flush before changing any resources.
				FlushCommandQueue();

				ThrowIfFailed(m_WindowInfo->CommandList->Reset(m_WindowInfo->CmdListAlloc.Get(), nullptr));

				// Release the previous resources we will be recreating.
				for (int i = 0; i < m_WindowInfo->SwapChainBufferCount; ++i)
					m_WindowInfo->m_SwapChainBuffer[i].Reset();
				m_WindowInfo->m_DepthStencilBuffer.Reset();

				// Resize the swap chain.
				ThrowIfFailed(m_WindowInfo->m_SwapChain->ResizeBuffers(
					m_WindowInfo->SwapChainBufferCount,
					_ClientWidth, _ClientHeight,
					m_WindowInfo->m_BackBufferFormat,
					DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

				m_WindowInfo->m_CurrBackBuffer = 0;

				D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_RenderTargetView->GetCPUDescriptorHandleForHeapStart());
				for (UINT i = 0; i < m_WindowInfo->SwapChainBufferCount; i++)
				{
					ThrowIfFailed(m_WindowInfo->m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_WindowInfo->m_SwapChainBuffer[i])));
					m_d3dDevice->CreateRenderTargetView(m_WindowInfo->m_SwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
					rtvHeapHandle.ptr += (1 * m_RtvDescriptorSize);
				}

				// Create the depth/stencil buffer and view.
				D3D12_RESOURCE_DESC depthStencilDesc;
				depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				depthStencilDesc.Alignment = 0;
				depthStencilDesc.Width = _ClientWidth;
				depthStencilDesc.Height = _ClientHeight;
				depthStencilDesc.DepthOrArraySize = 1;
				depthStencilDesc.MipLevels = 1;

				// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
				// the depth buffer.  Therefore, because we need to create two views to the same resource:
				//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
				//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
				// we need to create the depth buffer resource with a typeless format.  
				depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

				depthStencilDesc.SampleDesc.Count = m_WindowInfo->m_4xMsaaState ? 4 : 1;
				depthStencilDesc.SampleDesc.Quality = m_WindowInfo->m_4xMsaaState ? (m_WindowInfo->m_4xMsaaQuality - 1) : 0;
				depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

				D3D12_CLEAR_VALUE optClear;
				optClear.Format = m_DepthStencilFormat;
				optClear.DepthStencil.Depth = 1.0f;
				optClear.DepthStencil.Stencil = 0;


				D3D12_HEAP_PROPERTIES Properties = D3D12_HEAP_PROPERTIES();
				Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
				Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				Properties.CreationNodeMask = 1;
				Properties.VisibleNodeMask = 1;


				ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
					&Properties,
					D3D12_HEAP_FLAG_NONE,
					&depthStencilDesc,
					D3D12_RESOURCE_STATE_COMMON,
					&optClear,
					IID_PPV_ARGS(m_WindowInfo->m_DepthStencilBuffer.GetAddressOf())));

				// Create descriptor to mip level 0 of entire resource using the format of the resource.
				D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
				dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
				dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
				dsvDesc.Format = m_DepthStencilFormat;
				dsvDesc.Texture2D.MipSlice = 0;
				m_d3dDevice->CreateDepthStencilView(m_WindowInfo->m_DepthStencilBuffer.Get(), &dsvDesc, m_DepthStancilView->GetCPUDescriptorHandleForHeapStart());


				D3D12_RESOURCE_BARRIER Barrier = D3D12_RESOURCE_BARRIER();
				ZeroMemory(&Barrier, sizeof(Barrier));
				Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				Barrier.Transition.pResource = m_WindowInfo->m_DepthStencilBuffer.Get();
				Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
				Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
				Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				// Transition the resource from its initial state to be used as a depth buffer.
				m_WindowInfo->CommandList->ResourceBarrier(1, &Barrier);

				// Execute the resize commands.

				ThrowIfFailed(m_WindowInfo->CommandList->Close());

				ID3D12CommandList* cmdsLists[] = { m_WindowInfo->CommandList.Get() };
				m_GPUQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

				// Wait until resize is complete.
				FlushCommandQueue();

				m_WindowInfo->Resizing(_ClientWidth, _ClientHeight);
			}
			catch (EW_Exception _Exception)
			{
				switch (_Exception.ErrorCode)
				{
					case E_FAIL:
						MessageBox(EW::Window::EW_Window::GetInstance()->GetHandle(), L"E_Fail : if the command list has already been closed, or an invalid API was called during command list recording.", nullptr, MB_OK);
						break;
					case E_OUTOFMEMORY:
						MessageBox(EW::Window::EW_Window::GetInstance()->GetHandle(), L"E_OutOfMemory : if the operating system ran out of memory during recording.", nullptr, MB_OK);
						break;
					case E_INVALIDARG:
						MessageBox(EW::Window::EW_Window::GetInstance()->GetHandle(), L"E_InvalidArg : if an invalid argument was passed to the command list API during recording.", nullptr, MB_OK);
						break;
					default:
						__asm int 3;
						break;
				}

			}
		}

		EW_Device::EW_Device(int _ClientWidth, int _ClientHeight)
		{
			InitializeDevice(_ClientWidth, _ClientHeight);
		}

		EW_Device::~EW_Device()
		{
			ReleaseUnique(m_WindowInfo);
		}
	}
}
