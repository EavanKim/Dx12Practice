#pragma once

namespace EW
{
	namespace Obj
	{
		struct Texture
		{
			// Unique material name for lookup.
			std::string Name;

			std::wstring Filename;

			Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
			Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;
		};

		using uint16 = std::uint16_t;
		using uint32 = std::uint32_t;

		struct Vertex
		{
			Vertex() {}
			Vertex(
				const DirectX::XMFLOAT3& p,
				const DirectX::XMFLOAT3& n,
				const DirectX::XMFLOAT3& t,
				const DirectX::XMFLOAT2& uv) :
				Position(p),
				Normal(n),
				TangentU(t),
				TexC(uv) {}
			Vertex(
				float px, float py, float pz,
				float nx, float ny, float nz,
				float tx, float ty, float tz,
				float u, float v) :
				Position(px, py, pz),
				Normal(nx, ny, nz),
				TangentU(tx, ty, tz),
				TexC(u, v) {}

			DirectX::XMFLOAT3 Position;
			DirectX::XMFLOAT3 Normal;
			DirectX::XMFLOAT3 TangentU;
			DirectX::XMFLOAT2 TexC;
		};



		struct MeshData
		{
			std::vector<Vertex> Vertices;
			std::vector<uint32> Indices32;

			std::vector<uint16>& GetIndices16()
			{
				if (mIndices16.empty())
				{
					mIndices16.resize(Indices32.size());
					for (size_t i = 0; i < Indices32.size(); ++i)
						mIndices16[i] = static_cast<uint16>(Indices32[i]);
				}

				return mIndices16;
			}

			~MeshData()
			{
				if (0 != Vertices.size())
					Vertices.clear();

				if(0 != Indices32.size())
					Indices32.clear();
			}
		private:
			std::vector<uint16> mIndices16;
		};

		inline Vertex MidPoint(const Vertex& v0, const Vertex& v1)
		{
			DirectX::XMVECTOR p0 = XMLoadFloat3(&v0.Position);
			DirectX::XMVECTOR p1 = XMLoadFloat3(&v1.Position);

			DirectX::XMVECTOR n0 = XMLoadFloat3(&v0.Normal);
			DirectX::XMVECTOR n1 = XMLoadFloat3(&v1.Normal);

			DirectX::XMVECTOR tan0 = XMLoadFloat3(&v0.TangentU);
			DirectX::XMVECTOR tan1 = XMLoadFloat3(&v1.TangentU);

			DirectX::XMVECTOR tex0 = XMLoadFloat2(&v0.TexC);
			DirectX::XMVECTOR tex1 = XMLoadFloat2(&v1.TexC);

			// Compute the midpoints of all the attributes.  Vectors need to be normalized
			// since linear interpolating can make them not unit length.  
			DirectX::XMVECTOR pos = DirectX::XMVectorScale(DirectX::XMVectorAdd(p0, p1), 0.5f);
			DirectX::XMVECTOR normal = DirectX::XMVectorScale(DirectX::XMVectorAdd(n0, n1), 0.5f);
			DirectX::XMVECTOR tangent = DirectX::XMVectorScale(DirectX::XMVectorAdd(tan0, tan1), 0.5f);
			DirectX::XMVECTOR tex = DirectX::XMVectorScale(DirectX::XMVectorAdd(tex0, tex1), 0.5f);

			Vertex v;
			XMStoreFloat3(&v.Position, pos);
			XMStoreFloat3(&v.Normal, normal);
			XMStoreFloat3(&v.TangentU, tangent);
			XMStoreFloat2(&v.TexC, tex);

			return v;
		}

		inline void Subdivide(MeshData& meshData)
		{
			// Save a copy of the input geometry.
			MeshData inputCopy = meshData;


			meshData.Vertices.resize(0);
			meshData.Indices32.resize(0);

			//       v1
			//       *
			//      / \
			//     /   \
			//  m0*-----*m1
			//   / \   / \
			//  /   \ /   \
			// *-----*-----*
			// v0    m2     v2

			uint32 numTris = (uint32)inputCopy.Indices32.size() / 3;
			for (uint32 i = 0; i < numTris; ++i)
			{
				Vertex v0 = inputCopy.Vertices[inputCopy.Indices32[i * 3 + 0]];
				Vertex v1 = inputCopy.Vertices[inputCopy.Indices32[i * 3 + 1]];
				Vertex v2 = inputCopy.Vertices[inputCopy.Indices32[i * 3 + 2]];

				//
				// Generate the midpoints.
				//

				Vertex m0 = MidPoint(v0, v1);
				Vertex m1 = MidPoint(v1, v2);
				Vertex m2 = MidPoint(v0, v2);

				//
				// Add new geometry.
				//

				meshData.Vertices.push_back(v0); // 0
				meshData.Vertices.push_back(v1); // 1
				meshData.Vertices.push_back(v2); // 2
				meshData.Vertices.push_back(m0); // 3
				meshData.Vertices.push_back(m1); // 4
				meshData.Vertices.push_back(m2); // 5

				meshData.Indices32.push_back(i * 6 + 0);
				meshData.Indices32.push_back(i * 6 + 3);
				meshData.Indices32.push_back(i * 6 + 5);

				meshData.Indices32.push_back(i * 6 + 3);
				meshData.Indices32.push_back(i * 6 + 4);
				meshData.Indices32.push_back(i * 6 + 5);

				meshData.Indices32.push_back(i * 6 + 5);
				meshData.Indices32.push_back(i * 6 + 4);
				meshData.Indices32.push_back(i * 6 + 2);

				meshData.Indices32.push_back(i * 6 + 3);
				meshData.Indices32.push_back(i * 6 + 1);
				meshData.Indices32.push_back(i * 6 + 4);
			}
		}

		inline MeshData CreateRectangle(float _Width, float _Height, uint32 _numSubdivisions)
		{
			MeshData Result;

			Vertex vPoints[4];

			float harfwidth = 0.5f * _Width;
			float harfheight = 0.5f * _Height;

			vPoints[0] = Vertex(-harfwidth, -harfheight, 0.f
								, 0.f, 0.f, -1.f
								, 0.f, 0.f, -1.f
								, 0.f, 0.f);

			vPoints[1] = Vertex(harfwidth, -harfheight, 0.f
								, 0.f, 0.f, -1.f
								, 0.f, 0.f, -1.f
								, 1.f, 0.f);

			vPoints[2] = Vertex(harfwidth, harfheight, 0.f
								, 0.f, 0.f, -1.f
								, 0.f, 0.f, -1.f
								, 1.f, 1.f);

			vPoints[3] = Vertex(-harfwidth, harfheight, 0.f
								, 0.f, 0.f, -1.f
								, 0.f, 0.f, -1.f
								, 0.f, 1.f);

			Result.Vertices.assign(&vPoints[0], &vPoints[3]);

			uint32 Indexs[6];

			Indexs[0] = 0;
			Indexs[1] = 1;
			Indexs[2] = 2;
			Indexs[3] = 0;
			Indexs[4] = 2;
			Indexs[5] = 3;

			Result.Indices32.assign(&Indexs[0], &Indexs[5]);

			_numSubdivisions = std::min<uint32>(_numSubdivisions, 6u);

			for (uint32 i = 0; i < _numSubdivisions; ++i)
				Subdivide(Result);

			return Result;
		}

		struct SubmeshGeometry
		{
			UINT IndexCount = 0;
			UINT StartIndexLocation = 0;
			INT BaseVertexLocation = 0;

			DirectX::BoundingBox Bounds;
		};

		struct MeshGeometry
		{
			std::string Name;
 
			Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
			Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

			Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
			Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

			Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
			Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

			UINT VertexByteStride = 0;
			UINT VertexBufferByteSize = 0;
			DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
			UINT IndexBufferByteSize = 0;

			std::unordered_map<std::string, SubmeshGeometry> DrawPaths;

			D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const
			{
				D3D12_VERTEX_BUFFER_VIEW vbv;
				vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
				vbv.StrideInBytes = VertexByteStride;
				vbv.SizeInBytes = VertexBufferByteSize;

				return vbv;
			}

			D3D12_INDEX_BUFFER_VIEW IndexBufferView()const
			{
				D3D12_INDEX_BUFFER_VIEW ibv;
				ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
				ibv.Format = IndexFormat;
				ibv.SizeInBytes = IndexBufferByteSize;

				return ibv;
			}

			void DisposeUploaders()
			{
				VertexBufferUploader = nullptr;
				IndexBufferUploader = nullptr;
			}
		};



		class EW_Obj : public EW_Base
		{
		public:
			EW_Obj() = default;
			EW_Obj(const EW_Obj& rhs) = delete;
			virtual ~EW_Obj();

			DirectX::XMFLOAT4X4 World = Identity4x4();
			DirectX::XMFLOAT4X4 Transform = Identity4x4();

			UINT ObjectIndex = -1;

			MeshData ObjMesh;
		};
	}
}

