IncludeDir                                = {}
IncludeDir["cef"]                         = "%{wks.location}/vendor/cef"
IncludeDir["Glad"]                        = "%{wks.location}/vendor/Glad/include"
IncludeDir["GLFW"]                        = "%{wks.location}/vendor/GLFW/include"

LibraryDir                                = {}
LibraryDir["cef_debug"]                   = "%{IncludeDir.cef}/Debug"
LibraryDir["cef_release"]                 = "%{IncludeDir.cef}/Release"

Library                                   = {}
Library["cef_debug"]                      = "%{LibraryDir.cef_debug}/libcef.lib"
Library["cef_wrapper_debug"]              = "%{LibraryDir.cef_debug}/libcef_dll_wrapper.lib"

Library["cef_release"]                    = "%{LibraryDir.cef_release}/libcef.lib"
Library["cef_wrapper_release"]            = "%{LibraryDir.cef_release}/libcef_dll_wrapper.lib"