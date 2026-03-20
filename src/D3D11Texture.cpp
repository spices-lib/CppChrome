#include "D3D11Texture.h"
#include <d3d11_1.h>
#include <d3d11_4.h>

void D3D11Texture::Init()
{
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &m_Device,
        nullptr,
        &m_Context
    );
}

std::shared_ptr<std::vector<uint8_t>> D3D11Texture::ReadTexture(HANDLE handle, uint32_t w, uint32_t h)
{
    if (!m_Device || !m_Context) return {};

    ID3D11Device4* device = nullptr;
    HRESULT hr = m_Device->QueryInterface(__uuidof(ID3D11Device4), (void**)&device);
    if (FAILED(hr)) return nullptr;

    // 1. 获取纹理描述
    ID3D11Texture2D* texture = nullptr;
    hr = device->OpenSharedResource1(handle, __uuidof(ID3D11Texture2D), (void**)&texture);
    device->Release();
    if (FAILED(hr)) return nullptr;

    // 2. 获取纹理描述
    D3D11_TEXTURE2D_DESC sharedDesc;
    texture->GetDesc(&sharedDesc);

    // 3. 创建 Staging Texture
    D3D11_TEXTURE2D_DESC stagingDesc = sharedDesc;
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.BindFlags = 0;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingDesc.MiscFlags = 0;

    ID3D11Texture2D* stagingTexture = nullptr;
    hr = m_Device->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);
    if (FAILED(hr)) {
        texture->Release();
        return {};
    }

    // 4. 拷贝数据
    m_Context->CopyResource(stagingTexture, texture);

    // 5. 映射读取
    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = m_Context->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(hr)) {
        stagingTexture->Release();
        texture->Release();
        return {};
    }

    // 6. 拷贝到输出缓冲区
    auto data = std::make_shared<std::vector<uint8_t>>();
    size_t totalSize = h * mapped.RowPitch;
    data->resize(totalSize);
    memcpy(data->data(), mapped.pData, totalSize);

    // 7. 清理
    m_Context->Unmap(stagingTexture, 0);
    stagingTexture->Release();
    texture->Release();

    return data;
}