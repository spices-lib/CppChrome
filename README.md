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