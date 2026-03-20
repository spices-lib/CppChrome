#pragma once
#include <Windows.h>
#include <vector>
#include <d3d11.h>
#include <memory>

class D3D11Texture
{
public:

	D3D11Texture() = default;
	virtual ~D3D11Texture() = default;

	void Init();

	std::shared_ptr<std::vector<uint8_t>> ReadTexture(HANDLE handle, uint32_t w, uint32_t h);

private:

	ID3D11Device* m_Device;

	ID3D11DeviceContext* m_Context;

};