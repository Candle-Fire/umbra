@echo off
echo Compiling GLSL shaders to SPIR-V Binary
for /r %%i in (*.vert;*.frag) do %VULKAN_SDK%\Bin\glslangValidator.exe -V "%%i" -o "%%~dpiSPIRV\%%~nxi".spv

PAUSE