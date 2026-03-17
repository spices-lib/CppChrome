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
