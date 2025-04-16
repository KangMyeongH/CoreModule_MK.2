#include "Grid.h"

#include "EditorCamera.h"
#include "Material.h"

engine::editor::Grid::Grid()
= default;

engine::editor::Grid::~Grid()
= default;

void engine::editor::Grid::InitGrid(const ComPtr<ID3D11Device>& device, int maxGridRange)
{
	int maxLineCount = (2 * maxGridRange + 1) * 2; 	// 가로 + 세로 라인 수
	m_MaxGirdVerts = maxLineCount * 2;				// 라인 1개당 정점 2개

	_uint bufferSize = m_MaxGirdVerts * sizeof(VTX_GRID);

	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = bufferSize;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;          // CPU에서 쓰기 가능
	bd.MiscFlags = 0;
	bd.StructureByteStride = sizeof(VTX_GRID);

	HRESULT hr = device->CreateBuffer(&bd, nullptr, m_GirdVB.GetAddressOf());
	if (FAILED(hr))
	{
		m_MaxGirdVerts = 0;
		std::cerr << "ERROR : Failed Create Grid Vertex Buffer !\n";
		return;
	}

	m_PrevSnappedX = 99999999.0f;
	m_PrevSnappedZ = 99999999.0f;

	m_Material = Material::Create(nullptr);
	m_Material->LoadShader(L".\\resource\\Shader\\EditorGrid.hlsl");
}

void engine::editor::Grid::UpdateGridVertices(ComPtr<ID3D11DeviceContext> context, const Vector3 cameraPos, _float gridStep,
	_int gridRange)
{
	// (1) 카메라 위치를 그리드 스텝 단위로 스냅
	_float snappedX = std::floorf(cameraPos.Value.x / gridStep) * gridStep;
	_float snappedZ = std::floorf(cameraPos.Value.z / gridStep) * gridStep;

	// (2) 이전과 동일하면 갱신 불필요
	if (std::fabsf(snappedX - m_PrevSnappedX) < 0.0001f &&
		std::fabsf(snappedZ - m_PrevSnappedZ) < 0.0001f)
	{
		return;
	}
	m_PrevSnappedX = snappedX;
	m_PrevSnappedZ = snappedZ;

	// --------------------------------------------------------------------------------
	// 정점 생성
	// --------------------------------------------------------------------------------
	std::vector<VTX_GRID> vertices;
	vertices.reserve((2 * gridRange + 1) * 4);

	// 색상 설정
	// normalColor = 0x40AAAAAA
	// A = 0x40, R = 0xAA, G = 0xAA, B = 0xAA
	_float4 normalColor = _float4(
		170.0f / 255.0f, // R
		170.0f / 255.0f, // G
		170.0f / 255.0f, // B
		64.0f / 255.0f  // A
	);

	// xAxisColor = 0x40FF4444
	// A = 0x40, R = 0xFF, G = 0x44, B = 0x44
	_float4 xAxisColor = _float4(
		255.0f / 255.0f, // R
		68.0f / 255.0f, // G
		68.0f / 255.0f, // B
		64.0f / 255.0f  // A
	);

	// zAxisColor = 0x4000FF00
	// A = 0x40, R = 0x00, G = 0xFF, B = 0x00
	_float4 zAxisColor = _float4(
		0.0f / 255.0f, // R
		255.0f / 255.0f, // G
		0.0f / 255.0f, // B
		64.0f / 255.0f  // A
	);

	// (3) X 축 방향 평행선 (Z : -gridRange ~ +gridRange)
	for (int iz = -gridRange; iz <= gridRange; ++iz)
	{
		float zPos = snappedZ + iz * gridStep;

		_float4 lineColor = normalColor;
		// Z=0 근처 라인은 X 축이므로 빨강
		if (std::fabsf(zPos) < (gridStep * 0.5f))
			lineColor = xAxisColor;

		float xMin = snappedX + (-gridRange) * gridStep;
		float xMax = snappedX + (gridRange)*gridStep;

		Vector3 vec1{ xMin, 0.0f, zPos };
		Vector3 vec2{ xMax, 0.0f, zPos };

		VTX_GRID v1 = { vec1.Value, lineColor };
		VTX_GRID v2 = { vec2.Value, lineColor };

		vertices.push_back(v1);
		vertices.push_back(v2);
	}

	// (4) Z 축 방향 평행선 (X : -gridRange ~ +gridRange)
	for (int ix = -gridRange; ix <= gridRange; ++ix)
	{
		float xPos = snappedX + ix * gridStep;

		_float4 lineColor = normalColor;
		// X=0 근처 라인은 Z 축이므로 초록
		if (std::fabsf(xPos) < (gridStep * 0.5f))
			lineColor = zAxisColor;

		float zMin = snappedZ + (-gridRange) * gridStep;
		float zMax = snappedZ + (gridRange)*gridStep;

		Vector3 vec1{ xPos, 0.0f, zMin };
		Vector3 vec2{ xPos, 0.0f, zMax };

		VTX_GRID v1 = { vec1.Value, lineColor };
		VTX_GRID v2 = { vec2.Value, lineColor };

		vertices.push_back(v1);
		vertices.push_back(v2);
	}

	m_CurrentVerts = static_cast<int>(vertices.size());
	if (m_CurrentVerts > m_MaxGirdVerts)
	{
		m_CurrentVerts = m_MaxGirdVerts;
	}

	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT hr = context->Map(m_GirdVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	if (SUCCEEDED(hr))
	{
		std::memcpy(mapped.pData, vertices.data(), m_CurrentVerts * sizeof(VTX_GRID));
		context->Unmap(m_GirdVB.Get(), 0);
	}
}

void engine::editor::Grid::Bind(const ComPtr<ID3D11DeviceContext>& context, const _float4X4& viewMat, const _float4X4& projMat)
{
	_float4X4 identity;
	XMStoreFloat4x4(&identity, XMMatrixTranspose(DirectX::XMMatrixIdentity()));

	m_Material->SetMatrix("g_WorldMatrix", identity);
	m_Material->SetMatrix("g_ViewMatrix", viewMat);
	m_Material->SetMatrix("g_ProjMatrix", projMat);

	m_Material->Bind(context.Get());
}

void engine::editor::Grid::RenderGird(const ComPtr<ID3D11DeviceContext>& context)
{
	_uint stride = sizeof(VTX_GRID);
	_uint offset = 0;

	context->IASetVertexBuffers(0, 1, m_GirdVB.GetAddressOf(), &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	if (m_CurrentVerts > 0)
	{
		context->Draw(m_CurrentVerts, 0);
	}
}
