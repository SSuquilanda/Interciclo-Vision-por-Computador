@echo off
setlocal enabledelayedexpansion

REM Colores usando caracteres especiales
set "BLUE=[94m"
set "GREEN=[92m"
set "YELLOW=[93m"
set "CYAN=[96m"
set "RED=[91m"
set "MAGENTA=[95m"
set "NC=[0m"

:menu
cls
echo.
echo ════════════════════════════════════════════════════════════
echo   PIPELINES ESPECIALIZADOS - Segmentacion CT (Windows)
echo ════════════════════════════════════════════════════════════
echo.
echo   1) Ejecutar Pipeline PULMONES
echo   2) Ejecutar Pipeline HUESOS
echo   3) Ejecutar Pipeline ARTERIAS PULMONARES
echo   4) Ejecutar TODOS los pipelines secuencialmente
echo   5) Recompilar pipelines
echo   6) Limpiar y recompilar desde cero
echo   0) Salir
echo.
set /p choice="Selecciona una opcion: "

if "%choice%"=="1" goto run_pulmones
if "%choice%"=="2" goto run_huesos
if "%choice%"=="3" goto run_arterias
if "%choice%"=="4" goto run_all
if "%choice%"=="5" goto compile
if "%choice%"=="6" goto clean_compile
if "%choice%"=="0" exit /b 0
goto menu

:run_pulmones
cls
echo.
echo ════════════════════════════════════════════════════════════
echo    Pipeline: PULMONES (segmentacion precisa)
echo ════════════════════════════════════════════════════════════
echo.

REM Usar la ruta que proporcionaste
set "DICOM_FILE=C:\dev\Proyecto - Vision Por Computador\Interciclo-Vision-por-Computador\data\CT_low_dose_reconstruction_dataset\Original Data\Quarter Dose\3mm Slice Thickness\Soft Kernel (B30)\L291\L291_QD_3_1.CT.0003.0068.2015.12.23.17.49.43.831724.127636233.IMA"

if not exist "build\Release\PipelinePulmones.exe" (
    echo Error: PipelinePulmones.exe no encontrado
    echo Ejecuta primero la opcion 5 para compilar
    pause
    goto menu
)

echo Ejecutando PipelinePulmones con:
echo   "%DICOM_FILE%"
echo.

build\Release\PipelinePulmones.exe "%DICOM_FILE%"

echo.
echo Pipeline completado
pause
goto menu

:run_huesos
cls
echo.
echo ════════════════════════════════════════════════════════════
echo    Pipeline: HUESOS (costillas, columna, esternon)
echo ════════════════════════════════════════════════════════════
echo.

set "DICOM_FILE=C:\dev\Proyecto - Vision Por Computador\Interciclo-Vision-por-Computador\data\CT_low_dose_reconstruction_dataset\Original Data\Quarter Dose\3mm Slice Thickness\Soft Kernel (B30)\L291\L291_QD_3_1.CT.0003.0068.2015.12.23.17.49.43.831724.127636233.IMA"

if not exist "build\Release\PipelineHuesos.exe" (
    echo Error: PipelineHuesos.exe no encontrado
    echo Ejecuta primero la opcion 5 para compilar
    pause
    goto menu
)

echo Ejecutando PipelineHuesos con:
echo   "%DICOM_FILE%"
echo.

build\Release\PipelineHuesos.exe "%DICOM_FILE%"

echo.
echo Pipeline completado
pause
goto menu

:run_arterias
cls
echo.
echo ════════════════════════════════════════════════════════════
echo    Pipeline: ARTERIAS PULMONARES (deteccion del 'pulpo')
echo ════════════════════════════════════════════════════════════
echo.

set "DICOM_FILE=C:\dev\Proyecto - Vision Por Computador\Interciclo-Vision-por-Computador\data\CT_low_dose_reconstruction_dataset\Original Data\Quarter Dose\3mm Slice Thickness\Soft Kernel (B30)\L291\L291_QD_3_1.CT.0003.0068.2015.12.23.17.49.43.831724.127636233.IMA"

if not exist "build\Release\PipelineArterias.exe" (
    echo Error: PipelineArterias.exe no encontrado
    echo Ejecuta primero la opcion 5 para compilar
    pause
    goto menu
)

echo Ejecutando PipelineArterias con:
echo   "%DICOM_FILE%"
echo.

build\Release\PipelineArterias.exe "%DICOM_FILE%"

echo.
echo Pipeline completado
pause
goto menu

:run_all
cls
echo.
echo ════════════════════════════════════════════════════════════
echo    Ejecutando TODOS los pipelines secuencialmente
echo ════════════════════════════════════════════════════════════
echo.

set "DICOM_FILE=C:\dev\Proyecto - Vision Por Computador\Interciclo-Vision-por-Computador\data\CT_low_dose_reconstruction_dataset\Original Data\Quarter Dose\3mm Slice Thickness\Soft Kernel (B30)\L291\L291_QD_3_1.CT.0003.0068.2015.12.23.17.49.43.831724.127636233.IMA"

echo [1/3] Ejecutando Pipeline: PULMONES
build\Release\PipelinePulmones.exe "%DICOM_FILE%"

echo.
echo [2/3] Ejecutando Pipeline: HUESOS
build\Release\PipelineHuesos.exe "%DICOM_FILE%"

echo.
echo [3/3] Ejecutando Pipeline: ARTERIAS
build\Release\PipelineArterias.exe "%DICOM_FILE%"

echo.
echo ════════════════════════════════════════════════════════════
echo   Todos los pipelines completados
echo ════════════════════════════════════════════════════════════
pause
goto menu

:compile
cls
echo.
echo ════════════════════════════════════════════════════════════
echo    Compilando Pipelines (incremental)
echo ════════════════════════════════════════════════════════════
echo.

if not exist "build" (
    echo Creando directorio build...
    mkdir build
)

cd build

echo Configurando con CMake...
cmake .. -G "Visual Studio 17 2022" -A x64

if errorlevel 1 (
    echo.
    echo Error en CMake
    cd ..
    pause
    goto menu
)

echo.
echo Compilando...
cmake --build . --config Release

if errorlevel 1 (
    echo.
    echo Error en compilacion
    cd ..
    pause
    goto menu
)

cd ..
echo.
echo Compilacion exitosa
pause
goto menu

:clean_compile
cls
echo.
echo ════════════════════════════════════════════════════════════
echo    Limpiando y Recompilando desde cero
echo ════════════════════════════════════════════════════════════
echo.

if exist "build" (
    echo Eliminando build anterior...
    rmdir /s /q build
)

echo Creando directorio build...
mkdir build
cd build

echo.
echo Configurando con CMake...
cmake .. -G "Visual Studio 17 2022" -A x64

if errorlevel 1 (
    echo.
    echo Error en CMake
    cd ..
    pause
    goto menu
)

echo.
echo Compilando...
cmake --build . --config Release

if errorlevel 1 (
    echo.
    echo Error en compilacion
    cd ..
    pause
    goto menu
)

cd ..
echo.
echo Compilacion completa desde cero exitosa
pause
goto menu