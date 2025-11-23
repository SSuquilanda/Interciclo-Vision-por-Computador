#!/bin/bash

# ============================================================================
# Script para ejecutar Pipelines Especializados
# ============================================================================

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m'

# Verificar directorio
if [ ! -f "CMakeLists.txt" ]; then
    echo -e "${RED}Error: Ejecuta este script desde el directorio code/${NC}"
    exit 1
fi

# Banner
echo -e "\n${BLUE}╔══════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║${NC}  ${CYAN}PIPELINES ESPECIALIZADOS - Segmentación CT${NC}      ${BLUE}║${NC}"
echo -e "${BLUE}╚══════════════════════════════════════════════════════╝${NC}\n"

# Función para compilar
compile_pipelines() {
    echo -e "${YELLOW}═══ Compilando Pipelines ═══${NC}\n"
    cd build
    
    if ! cmake .. > /dev/null 2>&1; then
        echo -e "${RED}✗${NC} Error en CMake"
        return 1
    fi
    
    echo -e "${BLUE}→${NC} Compilando PipelinePulmones..."
    make PipelinePulmones -j$(nproc) > /dev/null 2>&1
    
    echo -e "${BLUE}→${NC} Compilando PipelineHuesos..."
    make PipelineHuesos -j$(nproc) > /dev/null 2>&1
    
    echo -e "${BLUE}→${NC} Compilando PipelineArterias..."
    make PipelineArterias -j$(nproc) > /dev/null 2>&1
    
    echo -e "\n${GREEN}✓${NC} Compilación completada"
    cd ..
    return 0
}

# Función para seleccionar archivo
select_dicom_file() {
    echo -e "\n${CYAN}Selecciona el archivo DICOM:${NC}"
    echo "  1) Quarter Dose (QD) - Primer slice"
    echo "  2) Full Dose (FD) - Primer slice"
    echo "  3) Ruta personalizada"
    read -p "Opción [1]: " file_choice
    
    case $file_choice in
        2)
            DICOM_FILE="/home/felipep/Documentos/universidad/universidad 7mo/vision por computador/Interciclo-Vision-por-Computador/code/../data/L291_qd/L291_QD_3_1.CT.0003.0068.2015.12.23.17.49.43.831724.127636233.IMA"
            ;;
        3)
            read -p "Ingresa la ruta al archivo .IMA: " DICOM_FILE
            ;;
        *)
            DICOM_FILE="/home/felipep/Documentos/universidad/universidad 7mo/vision por computador/Interciclo-Vision-por-Computador/code/../data/L291_qd/L291_QD_3_1.CT.0003.0068.2015.12.23.17.49.43.831724.127636233.IMA"
            ;;
    esac
    
    if [ ! -f "$DICOM_FILE" ]; then
        echo -e "${RED}✗${NC} Archivo no encontrado: $DICOM_FILE"
        exit 1
    fi
}

# Función principal
main() {
    # Si hay argumento, compilar primero
    if [ "$1" == "compile" ] || [ "$1" == "build" ]; then
        compile_pipelines
        exit 0
    fi
    
    # Verificar que los ejecutables existan
    if [ ! -f "build/PipelinePulmones" ] || [ ! -f "build/PipelineHuesos" ] || [ ! -f "build/PipelineArterias" ]; then
        echo -e "${YELLOW}→${NC} Los ejecutables no existen. Compilando..."
        if ! compile_pipelines; then
            exit 1
        fi
    fi
    
    # Seleccionar archivo
    select_dicom_file
    
    # Menú de pipelines
    echo -e "\n${CYAN}¿Qué pipeline deseas ejecutar?${NC}"
    echo -e "  ${BLUE}1)${NC} 🫁 Pulmones (segmentación precisa de pulmones)"
    echo -e "  ${RED}2)${NC} 🦴 Huesos (costillas, columna, esternón)"
    echo -e "  ${GREEN}3)${NC} 🫀 Arterias Pulmonares (detección del 'pulpo')"
    echo -e "  ${MAGENTA}4)${NC} 🔄 Ejecutar TODOS secuencialmente"
    echo -e "  ${YELLOW}5)${NC} 🔧 Recompilar pipelines"
    read -p "Opción [1]: " pipeline_choice
    
    case $pipeline_choice in
        1)
            echo -e "\n${BLUE}══════════════════════════════════════${NC}"
            echo -e "${BLUE}   Ejecutando Pipeline: PULMONES 🫁${NC}"
            echo -e "${BLUE}══════════════════════════════════════${NC}\n"
            ./build/PipelinePulmones "$DICOM_FILE"
            ;;
        2)
            echo -e "\n${RED}══════════════════════════════════════${NC}"
            echo -e "${RED}   Ejecutando Pipeline: HUESOS 🦴${NC}"
            echo -e "${RED}══════════════════════════════════════${NC}\n"
            ./build/PipelineHuesos "$DICOM_FILE"
            ;;
        3)
            echo -e "\n${GREEN}══════════════════════════════════════${NC}"
            echo -e "${GREEN}   Ejecutando Pipeline: ARTERIAS 🫀${NC}"
            echo -e "${GREEN}══════════════════════════════════════${NC}\n"
            ./build/PipelineArterias "$DICOM_FILE"
            ;;
        4)
            echo -e "\n${MAGENTA}═══════════════════════════════════════════${NC}"
            echo -e "${MAGENTA}   Ejecutando TODOS los pipelines${NC}"
            echo -e "${MAGENTA}═══════════════════════════════════════════${NC}\n"
            
            echo -e "${BLUE}[1/3] Pipeline: PULMONES${NC}"
            ./build/PipelinePulmones "$DICOM_FILE"
            
            echo -e "\n${RED}[2/3] Pipeline: HUESOS${NC}"
            ./build/PipelineHuesos "$DICOM_FILE"
            
            echo -e "\n${GREEN}[3/3] Pipeline: ARTERIAS${NC}"
            ./build/PipelineArterias "$DICOM_FILE"
            
            echo -e "\n${MAGENTA}✓ Todos los pipelines completados${NC}"
            ;;
        5)
            compile_pipelines
            ;;
        *)
            # Por defecto: Pulmones
            echo -e "\n${BLUE}Ejecutando Pipeline: PULMONES (opción por defecto)${NC}\n"
            ./build/PipelinePulmones "$DICOM_FILE"
            ;;
    esac
}

# Ejecutar
main "$@"

echo -e "\n${GREEN}Fin del script${NC}\n"
