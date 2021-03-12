@echo off

set CODE_PATH=%~dp0/../../../code
set GLSLC_EXE=%CODE_PATH%/Extern/VulkanSDK/1.2.162.1/bin/win64/glslc.exe

%GLSLC_EXE% basicRenderer.vert -o basicRenderer_vert.spv
%GLSLC_EXE% basicRenderer.frag -o basicRenderer_frag.spv

%GLSLC_EXE% vulkanRendererDefault.vert -o vulkanRendererDefault.vert.spv
%GLSLC_EXE% vulkanRendererDefault.frag -o vulkanRendererDefault.frag.spv

pause