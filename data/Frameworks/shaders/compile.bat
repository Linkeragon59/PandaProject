@echo off

set CODE_PATH=%~dp0/../../../code
set GLSLC_EXE=%CODE_PATH%/Extern/VulkanSDK/1.2.162.1/bin/win64/glslc.exe

%GLSLC_EXE% triangleRenderer.vert -o triangleRenderer_vert.spv
%GLSLC_EXE% triangleRenderer.frag -o triangleRenderer_frag.spv

%GLSLC_EXE% basicRenderer.vert -o basicRenderer_vert.spv
%GLSLC_EXE% basicRenderer.frag -o basicRenderer_frag.spv

%GLSLC_EXE% basicRendererTuto.vert -o basicRendererTuto_vert.spv
%GLSLC_EXE% basicRendererTuto.frag -o basicRendererTuto_frag.spv

%GLSLC_EXE% phong.vert -o phong.vert.spv
%GLSLC_EXE% phong.frag -o phong.frag.spv
%GLSLC_EXE% toon.vert -o toon.vert.spv
%GLSLC_EXE% toon.frag -o toon.frag.spv
%GLSLC_EXE% wireframe.vert -o wireframe.vert.spv
%GLSLC_EXE% wireframe.frag -o wireframe.frag.spv

pause