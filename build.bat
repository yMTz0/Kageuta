@echo off
echo ========================================
echo   Kageuta C++ - Build Script
echo ========================================
echo.

echo [1/3] Configurando ambiente MSYS2...
set MSYS2_DIR=C:\msys64
set MINGW_DIR=%MSYS2_DIR%\mingw64
set PATH=%MINGW_DIR%\bin;%PATH%

echo Verificando compilador...
g++ --version >nul 2>nul
if %errorlevel% neq 0 (
    echo ERRO: g++ nao encontrado!
    echo Execute este script dentro do MSYS2 ou instale o MinGW-w64.
    pause
    exit /b 1
)
echo Compilador encontrado!
echo.

echo [2/3] Configurando CMake...
cmake --version >nul 2>nul
if %errorlevel% neq 0 (
    echo ERRO: CMake nao encontrado!
    pause
    exit /b 1
)
echo CMake encontrado!
echo.

echo [3/3] Compilando Kageuta...
cd /d "%~dp0"
if not exist "build" mkdir build
cd build

cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
if %errorlevel% neq 0 (
    echo ERRO: Falha ao configurar CMake!
    pause
    exit /b 1
)

cmake --build . --config Release
if %errorlevel% neq 0 (
    echo ERRO: Falha na compilacao!
    pause
    exit /b 1
)

echo.
echo ========================================
echo   Build concluido com sucesso!
echo   Executavel: build\Kageuta.exe
echo ========================================
echo.

copy "Kageuta.exe" "%USERPROFILE%\Desktop\Kageuta.exe" >nul 2>nul
if exist "%USERPROFILE%\Desktop\Kageuta.exe" (
    echo Executavel copiado para a area de trabalho!
)

echo.
pause
