#pragma once

namespace EW
{
	namespace Resource
	{
		template<typename T>
		class EW_Buffer : public EW_Base
		{
		public:
			EW_Buffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer) : m_IsConstantBuffer(isConstantBuffer)
			{
				m_ElementByteSize = sizeof(T);

				if (isConstantBuffer)
					m_ElementByteSize = CalcConstantBufferByteSize(sizeof(T));

				D3D12_HEAP_PROPERTIES Properties = new D3D12_HEAP_PROPERTIES();
				Properties.Type = D3D12_HEAP_TYPE_UPLOAD;
				Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				Properties.CreationNodeMask = 1;
				Properties.VisibleNodeMask = 1;

				ThrowIfFailed(device->CreateCommittedResource(
					&Properties,
					D3D12_HEAP_FLAG_NONE,
					&D3D12_RESOURCE_DESC::Buffer(m_ElementByteSize * elementCount),
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(&m_UploadBuffer)));

				ThrowIfFailed(m_UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData)));
			}

			EW_Buffer(const EW_Buffer& rhs) = delete;
			EW_Buffer& operator=(const EW_Buffer& rhs) = delete;
			~EW_Buffer()
			{
				if (m_UploadBuffer != nullptr)
					m_UploadBuffer->Unmap(0, nullptr);

				m_MappedData = nullptr;
			}

			ID3D12Resource* Resource()const
			{
				return m_UploadBuffer.Get();
			}

			void CopyData(int elementIndex, const T& data)
			{
				memcpy(&m_MappedData[elementIndex * m_ElementByteSize], &data, sizeof(T));
			}

		private:
			Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadBuffer;
			BYTE* m_MappedData = nullptr;

			UINT m_ElementByteSize = 0;
			bool m_IsConstantBuffer = false;
		};
	}
}

