#pragma once

namespace EW
{
	namespace Graphic
	{
		namespace RenderTarget
		{


			class VertexInfo
			{
			public:
				DirectX::XMFLOAT4 COLOR;
				DirectX::XMFLOAT3 Pos;
				DirectX::XMFLOAT2 UV;
			};

			class DrawInfoInfo
			{
			public:
				long VertexSize = 0;
				long IndexSize = 0;

				std::unique_ptr<std::vector<VertexInfo>> Vertex = nullptr;
				std::unique_ptr<std::vector<std::uint16_t>> Index = nullptr;
				Microsoft::WRL::ComPtr<ID3D12Resource> VertexData = nullptr;
				Microsoft::WRL::ComPtr<ID3D12Resource> IndexData = nullptr;
			};

			class EW_RenderTarget : public EW_Base
			{

			};
		}
	}
}
