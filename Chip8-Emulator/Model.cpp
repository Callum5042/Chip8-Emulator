#include "Model.h"
#include <DirectXMath.h>
#include <vector>

DX::Model::Model(DX::Renderer* renderer) : m_DxRenderer(renderer)
{
}

void DX::Model::Create()
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Vertex data
	std::vector<Vertex> vertices =
	{
		{ -0.5f, +0.5f, 0.0f }, // Top left vertex
		{ +0.5f, +0.5f, 0.0f }, // Top right vertex
		{ -0.5f, -0.5f, 0.0f } , // Bottom left vertex
		{ +0.5f, -0.5f, 0.0f }, // Bottom right vertex
	};

	m_VertexCount = static_cast<UINT>(vertices.size());

	// Index data
	std::vector<UINT> indices =
	{
		0, 1, 2,
		2, 1, 3,
	};

	m_IndexCount = static_cast<UINT>(indices.size());

	// Create vertex buffer
	D3D11_BUFFER_DESC vertexbuffer_desc = {};
	vertexbuffer_desc.Usage = D3D11_USAGE_DEFAULT;
	vertexbuffer_desc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertices.size());
	vertexbuffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertex_subdata = {};
	vertex_subdata.pSysMem = vertices.data();

	DX::Check(d3dDevice->CreateBuffer(&vertexbuffer_desc, &vertex_subdata, m_d3dVertexBuffer.ReleaseAndGetAddressOf()));

	// Create index buffer
	D3D11_BUFFER_DESC index_buffer_desc = {};
	index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	index_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(UINT) * indices.size());
	index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA index_subdata = {};
	index_subdata.pSysMem = indices.data();

	DX::Check(d3dDevice->CreateBuffer(&index_buffer_desc, &index_subdata, m_d3dIndexBuffer.ReleaseAndGetAddressOf()));
}

void DX::Model::Render()
{
	auto d3dDeviceContext = m_DxRenderer->GetDeviceContext();

	// We need the stride and offset
	UINT vertex_stride = sizeof(Vertex);
	auto vertex_offset = 0u;

	// Bind the vertex buffer to the Input Assembler
	d3dDeviceContext->IASetVertexBuffers(0, 1, m_d3dVertexBuffer.GetAddressOf(), &vertex_stride, &vertex_offset);

	// Bind the index buffer to the Input Assembler
	d3dDeviceContext->IASetIndexBuffer(m_d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Bind the geometry topology to the Input Assembler
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Bind texture to the pixel shader
	// d3dDeviceContext->PSSetShaderResources(0, 1, m_DiffuseTexture.GetAddressOf());

	// Render geometry
	d3dDeviceContext->DrawIndexed(m_IndexCount, 0, 0);
}
