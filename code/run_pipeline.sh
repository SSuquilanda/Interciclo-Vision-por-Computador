#!/bin/bash

# ============================================================================
# Script para compilar y ejecutar MainPipeline
# Vision por Computador - Interciclo
# ============================================================================

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Verificar que estamos en el directorio correcto
if [ ! -f "CMakeLists.txt" ]; then
    echo -e "${RED}Error: Ejecuta este script desde el directorio code/${NC}"
    exit 1
fi

# Banner
echo -e "\n${BLUE}╔════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║${NC}  ${CYAN}MainPipeline - Pipeline Completo de Segmentación${NC}      ${BLUE}║${NC}"
echo -e "${BLUE}║${NC}  ${GREEN}Fases 3, 4, 5: Preprocessing + Segmentación + Morfología${NC} ${BLUE}║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════════════════════════╝${NC}\n"

# Función para compilar
compile_pipeline() {
    echo -e "${YELLOW}═══ Compilando MainPipeline ═══${NC}\n"
    
    # Crear directorio build si no existe
    if [ ! -d "build" ]; then
        echo -e "${BLUE}→${NC} Creando directorio build..."
        mkdir build
    fi
    
    cd build
    
    # Configurar con CMake
    echo -e "\n${BLUE}→${NC} Configurando con CMake..."
    if ! cmake .. > /dev/null 2>&1; then
        echo -e "${RED}✗${NC} Error en CMake"
        cd ..
        return 1
    fi
    
    # Compilar solo MainPipeline
    echo -e "${BLUE}→${NC} Compilando MainPipeline..."
    if ! make MainPipeline -j$(nproc); then
        echo -e "${RED}✗${NC} Error en la compilación"
        cd ..
        return 1
    fi
    
    echo -e "\n${GREEN}✓${NC} Compilación exitosa"
    cd ..
    return 0
}

# Función para ejecutar
run_pipeline() {
    if [ ! -f "build/MainPipeline" ]; then
        echo -e "${RED}✗${NC} El ejecutable MainPipeline no existe."
        echo -e "${YELLOW}→${NC} Intentando compilar..."
        if ! compile_pipeline; then
            exit 1
        fi
    fi
    
    echo -e "\n${YELLOW}═══ Ejecutando MainPipeline ═══${NC}\n"
    
    # Solicitar archivo DICOM
    if [ -z "$1" ]; then
        echo -e "${CYAN}Selecciona el archivo DICOM a procesar:${NC}"
        echo "  1) Quarter Dose (QD) - Primer slice"
        echo "  2) Full Dose (FD) - Primer slice"
        echo "  3) Ingresar ruta manualmente"
        read -p "Opción [1]: " file_choice
        
        case $file_choice in
            2)
                DICOM_FILE="../data/L291_fd/L291_FD_3_1.CT.0005.0001.2015.12.23.17.48.23.868235.127709571.IMA"
                ;;
            3)
                read -p "Ingresa la ruta completa al archivo .IMA: " DICOM_FILE
                ;;
            *)
                # Por defecto usar QD
                DICOM_FILE="../data/L291_qd/L291_QD_3_1.CT.0004.0001.2015.12.23.17.46.30.761031.127695337.IMA"
                ;;
        esac
    else
        DICOM_FILE="$1"
    fi
    
    # Verificar que el archivo existe
    if [ ! -f "$DICOM_FILE" ]; then
        echo -e "${RED}✗${NC} El archivo DICOM no existe: $DICOM_FILE"
        exit 1
    fi
    
    echo -e "\n${GREEN}→${NC} Procesando: $DICOM_FILE\n"
    echo -e "${CYAN}─────────────────────────────────────────────────────────${NC}\n"
    
    # Ejecutar el pipeline
    ./build/MainPipeline "$DICOM_FILE"
    
    RESULT=$?
    
    echo -e "\n${CYAN}─────────────────────────────────────────────────────────${NC}"
    
    if [ $RESULT -eq 0 ]; then
        echo -e "\n${GREEN}✓${NC} Pipeline ejecutado exitosamente"
    else
        echo -e "\n${RED}✗${NC} El pipeline finalizó con errores (código: $RESULT)"
    fi
}

# Mostrar menú si no hay argumentos
if [ $# -eq 0 ]; then
    echo -e "${CYAN}¿Qué deseas hacer?${NC}"
    echo "  1) Compilar y ejecutar"
    echo "  2) Solo compilar"
    echo "  3) Solo ejecutar"
    read -p "Opción [1]: " menu_choice
    
    case $menu_choice in
        2)
            compile_pipeline
            ;;
        3)
            run_pipeline
            ;;
        *)
            if compile_pipeline; then
                run_pipeline
            fi
            ;;
    esac
else
    # Si se proporciona un archivo como argumento, compilar y ejecutar directamente
    if compile_pipeline; then
        run_pipeline "$1"
    fi
fi

echo -e "\n${GREEN}Fin del script${NC}\n"
