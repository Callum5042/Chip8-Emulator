#include "Shader.h"
#include <fstream>
#include <vector>

DX::Shader::Shader(Renderer* renderer) : m_DxRenderer(renderer)
{
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = 1000.0f;

	DX::Check(m_DxRenderer->GetDevice()->CreateSamplerState(&samplerDesc, &m_AnisotropicSampler));
}

void DX::Shader::LoadVertexShader(std::string&& vertex_shader_path)
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Load the binary file into memory
	std::ifstream file(vertex_shader_path, std::fstream::in | std::fstream::binary);
	std::vector<char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	// Create the vertex shader
	DX::Check(d3dDevice->CreateVertexShader(data.data(), data.size(), nullptr, m_d3dVertexShader.ReleaseAndGetAddressOf()));

	// Describe the memory layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXTURE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);
	DX::Check(d3dDevice->CreateInputLayout(layout, numElements, data.data(), data.size(), m_d3dVertexLayout.ReleaseAndGetAddressOf()));
}

void DX::Shader::LoadPixelShader(std::string&& pixel_shader_path)
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Load the binary file into memory
	std::ifstream file(pixel_shader_path, std::fstream::in | std::fstream::binary);
	std::vector<char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	// Create pixel shader
	DX::Check(d3dDevice->CreatePixelShader(data.data(), data.size(), nullptr, m_d3dPixelShader.ReleaseAndGetAddressOf()));
}

void DX::Shader::Use()
{
	auto d3dDeviceContext = m_DxRenderer->GetDeviceContext();

	// Bind the input layout to the pipeline's Input Assembler stage
	d3dDeviceContext->IASetInputLayout(m_d3dVertexLayout.Get());

	// Bind the vertex shader to the pipeline's Vertex Shader stage
	d3dDeviceContext->VSSetShader(m_d3dVertexShader.Get(), nullptr, 0);

	// Bind the pixel shader to the pipeline's Pixel Shader stage
	d3dDeviceContext->PSSetShader(m_d3dPixelShader.Get(), nullptr, 0);

	// Bind pixel shader texture sampler
	d3dDeviceContext->PSSetSamplers(0, 1, m_AnisotropicSampler.GetAddressOf());
}