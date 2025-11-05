@echo off
REM Script de compilación para Windows

echo ╔════════════════════════════════════════════════════════════╗
echo ║  Compilando Proyecto Interciclo (Windows)                 ║
echo ╚════════════════════════════════════════════════════════════╝
echo.

REM Crear directorio build si no existe
if not exist "build" (
    echo Creando directorio build...
    mkdir build
)

REM Entrar al directorio build
cd build

REM Limpiar build anterior
echo Limpiando compilación anterior...
del /Q *.*
for /d %%p in (*) do rmdir "%%p" /s /q

REM Configurar con CMake
echo.
echo Configurando con CMake...
cmake .. -G "Visual Studio 16 2019" -A x64
if errorlevel 1 (
    echo Error en CMake
    pause
    exit /b 1
)

REM Compilar
echo.
echo Compilando...
cmake --build . --config Release
if errorlevel 1 (
    echo Error en la compilación
    pause
    exit /b 1
)

REM Ejecutar si la compilación fue exitosa
echo.
echo ╔════════════════════════════════════════════════════════════╗
echo ║  Compilación exitosa - Ejecutando programa...              ║
echo ╚════════════════════════════════════════════════════════════╝
echo.

Release\MyApp.exe

echo.
echo Programa finalizado
pause
