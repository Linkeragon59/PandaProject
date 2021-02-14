@echo off

set CODE_PATH=%~dp0/../../../code
set GLSLC_EXE=%CODE_PATH%/Extern/VulkanSDK/1.2.162.1/bin/win64/glslc.exe

%GLSLC_EXE% triangleRenderer.vert -o triangleRenderer_vert.spv
%GLSLC_EXE% triangleRenderer.frag -o triangleRenderer_frag.spv

%GLSLC_EXE% basicRenderer.vert -o basicRenderer_vert.spv
%GLSLC_EXE% basicRenderer.frag -o basicRenderer_frag.spv

pause