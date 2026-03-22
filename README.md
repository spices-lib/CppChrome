Test Project of Cpp with CEF

Cef Run in Windowless mode.

Draw with OnAcceleratedPaint interface

Must satifiy:
Cef version above 125
settings.windowless_rendering_enabled = true;
windowInfo.shared_texture_enabled = true;
windowInfo.external_begin_frame_enabled = false;

reference:
https://github.com/jtorjo/cef-spout?tab=readme-ov-file


这个工程的目的是验证Cef的OnAcceleratedPaint使用场景
工程使用OpenGL作为渲染API，存在两处限制
    1.无法直接使用Cef返回的D3D11纹理
    2.无法使用OpenGL ES 提供的零成本转换到OpenGL纹理

验证方案如下：
    1.D3D11 Texture -> Staging Texture -> CPU -> Staging Buffer -> OpenGL Texture 耗时与 OnPaint 基本一致
    2.WGL_NV_DX_interop2 Cef创建的资源带有GDI标志，无法共享给OpenGL，但是性能消耗只有方案一的三分之一左右
    3.使用 EGL_ANGLE_d3d_share_handle 拓展，利用浏览器的Angle渲染后端，将 D3D11纹理转换成 OpenGL纹理，但是硬性需求 OpenGL ES环境
    4.D3D11 Texture 复制 -> D3D11 Texture without  GDI，使用 WGL_NV_DX_interop2 共享 耗时基本是 OnPaint 一半

多线程：
    验证结果良好，多个浏览器实例彼此无影响

Build:
1. 下载cef138版本到 vendor/cef
2. 手动编译 libcef_dll 到 Debug/Release
2. 复制Resource里面所有文件到 Debug/Release
2. WebClient 中 设置资源路径 
    CefString(&settings.resources_dir_path)
    CefString(&settings.locales_dir_path)
3. 运行Start.bat

切换 OnAcceleratedPaint / OnPaint :
    WebClient.Init(ture/false)