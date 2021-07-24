@echo off

set CODE_PATH=%~dp0/../../../code
set GLSLC_EXE=%CODE_PATH%/Extern/VulkanSDK/1.2.162.1/bin/win64/glslc.exe

echo ----------------------
echo --- Vertex Shaders ---
echo ----------------------
for %%f in (*.vert) do (
	echo %%~nf
	%GLSLC_EXE% %%~nf.vert -o %%~nf_vert.spv
)

echo ------------------------
echo --- Fragment Shaders ---
echo ------------------------
for %%f in (*.frag) do (
	echo %%~nf
	%GLSLC_EXE% %%~nf.frag -o %%~nf_frag.spv
)

%GLSLC_EXE% basicRendererTuto.vert -o basicRendererTuto_vert.spv
%GLSLC_EXE% basicRendererTuto.frag -o basicRendererTuto_frag.spv

%GLSLC_EXE% phong.vert -o phong.vert.spv
%GLSLC_EXE% phong.frag -o phong.frag.spv
%GLSLC_EXE% toon.vert -o toon.vert.spv
%GLSLC_EXE% toon.frag -o toon.frag.spv
%GLSLC_EXE% wireframe.vert -o wireframe.vert.spv
%GLSLC_EXE% wireframe.frag -o wireframe.frag.spv

pause