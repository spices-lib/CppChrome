#include "NvInteropTexture.h"
#include <windows.h>
#include <glad/glad.h>
#include <wglext.h>
#include <d3d11.h>
#include <d3d11_4.h>
#include <dxgi1_2.h>
#include <GLFW/glfw3.h>

namespace {

    PFNWGLDXOPENDEVICENVPROC             wglDXOpenDeviceNV             = nullptr;
    PFNWGLDXCLOSEDEVICENVPROC            wglDXCloseDeviceNV            = nullptr;
    PFNWGLDXREGISTEROBJECTNVPROC         wglDXRegisterObjectNV         = nullptr;
    PFNWGLDXUNREGISTEROBJECTNVPROC       wglDXUnregisterObjectNV       = nullptr;
    PFNWGLDXLOCKOBJECTSNVPROC            wglDXLockObjectsNV            = nullptr;
    PFNWGLDXUNLOCKOBJECTSNVPROC          wglDXUnlockObjectsNV          = nullptr;
    PFNWGLDXSETRESOURCESHAREHANDLENVPROC wglDXSetResourceShareHandleNV = nullptr;

    bool InitNVInterop() {

        wglDXOpenDeviceNV = (PFNWGLDXOPENDEVICENVPROC)glfwGetProcAddress("wglDXOpenDeviceNV");

        wglDXCloseDeviceNV = (PFNWGLDXCLOSEDEVICENVPROC)glfwGetProcAddress("wglDXCloseDeviceNV");

        wglDXRegisterObjectNV = (PFNWGLDXREGISTEROBJECTNVPROC)glfwGetProcAddress("wglDXRegisterObjectNV");

        wglDXUnregisterObjectNV = (PFNWGLDXUNREGISTEROBJECTNVPROC)glfwGetProcAddress("wglDXUnregisterObjectNV");

        wglDXLockObjectsNV = (PFNWGLDXLOCKOBJECTSNVPROC)glfwGetProcAddress("wglDXLockObjectsNV");

        wglDXUnlockObjectsNV = (PFNWGLDXUNLOCKOBJECTSNVPROC)glfwGetProcAddress("wglDXUnlockObjectsNV");

        wglDXSetResourceShareHandleNV = (PFNWGLDXSETRESOURCESHAREHANDLENVPROC)glfwGetProcAddress("wglDXSetResourceShareHandleNV");

        return !!wglDXOpenDeviceNV;
    }
}

bool NvInteropTexture::Init() {

    // 1. 获取扩展函数指针
    if (!InitNVInterop()) return false;

    // 2. 创建 D3D11 设备（支持共享资源）
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG;

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

    // 3. 打开 NVIDIA 互操作设备
    m_InteropDevice = wglDXOpenDeviceNV(m_Device);
    if (!m_InteropDevice) {
        return false;
    }

    return true;
}

void NvInteropTexture::ReadTexture(HANDLE handle) {

    if (!m_Device || !m_Context) return;

    // 1. 打开 CEF 的共享纹理
    ID3D11Device4* device = nullptr;
    HRESULT hr = m_Device->QueryInterface(__uuidof(ID3D11Device4), (void**)&device);
    if (FAILED(hr)) return;

    ID3D11Texture2D* texture = nullptr;
    hr = device->OpenSharedResource1(handle, __uuidof(ID3D11Texture2D), (void**)&texture);
    device->Release();
    if (FAILED(hr)) return;

    // 2. 获取源纹理描述
    D3D11_TEXTURE2D_DESC srcDesc;
    texture->GetDesc(&srcDesc);

    // 3. 复制描述，禁用GDI
    srcDesc.MiscFlags  = D3D11_RESOURCE_MISC_SHARED;
    srcDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

    // 4. 创建目标纹理
    ID3D11Texture2D* destTexture = nullptr;
    hr = m_Device->CreateTexture2D(&srcDesc, nullptr, &destTexture);
    if (FAILED(hr)) return;

    m_Context->CopyResource(destTexture, texture);

    // 3. 保存共享纹理引用（用于后续清理）
    if (m_SharedTexture)
    {
        m_SharedTexture->Release();
        m_SharedTexture = nullptr;
    }

    m_SharedTexture = destTexture;
    texture->Release();
}

void NvInteropTexture::ShareTexture(uint32_t handle)
{
    if (!m_SharedTexture) return;

    // 1. 获取纹理描述
    D3D11_TEXTURE2D_DESC sharedDesc;
    m_SharedTexture->GetDesc(&sharedDesc);

    if (m_InteropObject) {

        wglDXUnlockObjectsNV(m_InteropDevice, 1, &m_InteropObject);

        wglDXUnregisterObjectNV(m_InteropDevice, m_InteropObject);
        m_InteropObject = nullptr;

    }

    glBindTexture(GL_TEXTURE_2D, handle);

    // 2. 将 D3D 纹理注册到 OpenGL
    // 注意：注册时 GL 纹理必须已经存在（已创建）
    m_InteropObject = wglDXRegisterObjectNV(
        m_InteropDevice,           // 互操作设备
        m_SharedTexture,           // D3D 纹理
        handle,                    // OpenGL 纹理名称
        GL_TEXTURE_2D,             // 类型
        WGL_ACCESS_READ_ONLY_NV    // 访问模式：只读
    );

    if (!m_InteropObject) {

        DWORD lastError = GetLastError();
        printf("  Last error: %lu\n", lastError);

        return;
    }

    BOOL lockResult = wglDXLockObjectsNV(m_InteropDevice, 1, &m_InteropObject);
    if (!lockResult) {
        printf("wglDXLockObjectsNV failed\n");
        return;
    }
}