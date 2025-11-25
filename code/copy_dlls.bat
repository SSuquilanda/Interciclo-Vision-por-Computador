@echo off
echo ════════════════════════════════════════════════════════════
echo   Copiando DLLs necesarias al ejecutable
echo ════════════════════════════════════════════════════════════
echo.

set "OPENCV_BIN=C:\dev\opencv\build_cpu\bin\Release"
set "QT_BIN=C:\Qt\6.10.1\msvc2022_64\bin"
set "BUILD_DIR=C:\dev\Proyecto - Vision Por Computador\Interciclo-Vision-por-Computador\code\build\Release"

REM Verificar que los directorios existen
if not exist "%OPENCV_BIN%" (
    echo [ERROR] No se encuentra: %OPENCV_BIN%
    pause
    exit /b 1
)

if not exist "%QT_BIN%" (
    echo [ERROR] No se encuentra: %QT_BIN%
    pause
    exit /b 1
)

if not exist "%BUILD_DIR%" (
    echo [ERROR] No se encuentra: %BUILD_DIR%
    echo Compila el proyecto primero
    pause
    exit /b 1
)

echo [1/3] Copiando DLLs de OpenCV...
copy "%OPENCV_BIN%\opencv_*.dll" "%BUILD_DIR%\" /Y >nul 2>&1
if errorlevel 1 (
    echo [ERROR] No se pudieron copiar DLLs de OpenCV
) else (
    echo   [OK] OpenCV DLLs copiadas
)

echo.
echo [2/3] Copiando DLLs de Qt6...
copy "%QT_BIN%\Qt6Core.dll" "%BUILD_DIR%\" /Y >nul 2>&1
copy "%QT_BIN%\Qt6Gui.dll" "%BUILD_DIR%\" /Y >nul 2>&1
copy "%QT_BIN%\Qt6Widgets.dll" "%BUILD_DIR%\" /Y >nul 2>&1
if errorlevel 1 (
    echo [ERROR] No se pudieron copiar DLLs de Qt6
) else (
    echo   [OK] Qt6 DLLs copiadas
)

echo.
echo [3/3] Copiando plugins de Qt6 (necesarios para mostrar ventanas)...
if not exist "%BUILD_DIR%\platforms" mkdir "%BUILD_DIR%\platforms"
copy "%QT_BIN%\..\plugins\platforms\qwindows.dll" "%BUILD_DIR%\platforms\" /Y >nul 2>&1
if errorlevel 1 (
    echo [ADVERTENCIA] Plugin qwindows.dll no copiado
) else (
    echo   [OK] Plugin qwindows copiado
)

echo.
echo ════════════════════════════════════════════════════════════
echo   ✓ DLLS COPIADAS EXITOSAMENTE
echo ════════════════════════════════════════════════════════════
echo.
echo Archivos en build\Release:
dir "%BUILD_DIR%\*.dll" /b
echo.

pause