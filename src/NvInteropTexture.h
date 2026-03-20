#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <string>

class NvInteropTexture {
public:
    NvInteropTexture() = default;

    virtual ~NvInteropTexture() = default;

    bool Init();

    void ReadTexture(HANDLE handle);

    void ShareTexture(uint32_t handle);

private:
    ID3D11Device* m_Device;
    ID3D11DeviceContext* m_Context;
    ID3D11Texture2D* m_SharedTexture;
    HANDLE m_InteropDevice;
    HANDLE m_InteropObject;
};