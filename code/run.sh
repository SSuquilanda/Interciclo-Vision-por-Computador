#!/bin/bash

# ============================================================================
# Script unificado para compilar y ejecutar programas del proyecto
# Vision por Computador - Interciclo
# ============================================================================

# Uso rápido:
# ./run.sh app        # Ejecutar VisionApp (interfaz Qt6)
# ./run.sh compile    # Compilar
# ./run.sh export     # ExportSlices
# ./run.sh explore    # ExploreDataset
# ./run.sh clean      # Limpiar

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Función para mostrar el menú
show_menu() {
    echo ""
    echo -e "${BLUE}╔════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║${NC}  ${GREEN}VISION POR COMPUTADOR - Menu Principal${NC}              ${BLUE}║${NC}"
    echo -e "${BLUE}╚════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    echo "  1) Compilar proyecto completo"
    echo "  2) Ejecutar VisionApp (Aplicación de escritorio Qt6)"
    echo "  3) Ejecutar ExportSlices (Fase 2.1 - Exportar slices a PNG)"
    echo "  4) Ejecutar ExportSlices3Views (Fase 2.1 - Exportar 3 vistas)"
    echo "  5) Ejecutar ExploreDataset (Fase 2.2 - Comparar FD vs QD)"
    echo "  6) Compilar y ejecutar VisionApp"
    echo "  7) Compilar y ejecutar ExportSlices"
    echo "  8) Compilar y ejecutar ExportSlices3Views"
    echo "  9) Compilar y ejecutar ExploreDataset"
    echo " 10) Limpiar build"
    echo "  0) Salir"
    echo ""
}

# Función para compilar
compile() {
    echo -e "\n${YELLOW}═══ Compilando Proyecto ═══${NC}\n"
    
    # Crear directorio build si no existe
    if [ ! -d "build" ]; then
        echo -e "${BLUE}→${NC} Creando directorio build..."
        mkdir build
    fi
    
    cd build
    
    # Configurar con CMake
    echo -e "\n${BLUE}→${NC} Configurando con CMake..."
    if ! cmake ..; then
        echo -e "${RED}✗${NC} Error en CMake"
        return 1
    fi
    
    # Compilar
    echo -e "\n${BLUE}→${NC} Compilando con make..."
    if ! make -j$(nproc); then
        echo -e "${RED}✗${NC} Error en la compilación"
        return 1
    fi
    
    echo -e "\n${GREEN}✓${NC} Compilación exitosa"
    cd ..
    return 0
}

# Función para limpiar build
clean_build() {
    echo -e "\n${YELLOW}═══ Limpiando Build ═══${NC}\n"
    if [ -d "build" ]; then
        echo -e "${BLUE}→${NC} Eliminando directorio build..."
        rm -rf build
        echo -e "${GREEN}✓${NC} Build limpiado"
    else
        echo -e "${YELLOW}⚠${NC} No hay directorio build para limpiar"
    fi
}

# Función para ejecutar ExportSlices
run_export_slices() {
    echo -e "\n${YELLOW}═══ ExportSlices - Exportación de Slices DICOM ═══${NC}\n"
    
    if [ ! -f "build/ExportSlices" ]; then
        echo -e "${RED}✗${NC} El ejecutable ExportSlices no existe."
        echo -e "${YELLOW}→${NC} Compila el proyecto primero (opción 1)"
        return 1
    fi
    
    cd build
    
    echo -e "${BLUE}Selecciona el dataset:${NC}"
    echo "  1) Quarter Dose (QD)"
    echo "  2) Full Dose (FD)"
    read -p "Opción [1]: " dataset_choice
    
    case $dataset_choice in
        2)
            echo -e "\n${GREEN}→${NC} Ejecutando con Full Dose...\n"
            ./ExportSlices fd
            ;;
        *)
            echo -e "\n${GREEN}→${NC} Ejecutando con Quarter Dose...\n"
            ./ExportSlices qd
            ;;
    esac
    
    cd ..
    echo -e "\n${GREEN}✓${NC} Programa finalizado"
}

# Función para ejecutar VisionApp
run_vision_app() {
    echo -e "\n${YELLOW}═══ VisionApp - Aplicación de Escritorio Qt6 ═══${NC}\n"
    
    if [ ! -f "build/VisionApp" ]; then
        echo -e "${RED}✗${NC} El ejecutable VisionApp no existe."
        echo -e "${YELLOW}→${NC} Compila el proyecto primero (opción 1)"
        return 1
    fi
    
    cd build
    echo -e "${GREEN}→${NC} Iniciando aplicación de escritorio...\n"
    ./VisionApp
    cd ..
    
    echo -e "\n${GREEN}✓${NC} Aplicación cerrada"
}

# Función para ejecutar ExportSlices3Views
run_export_slices_3views() {
    echo -e "\n${YELLOW}═══ ExportSlices3Views - Exportación de 3 Vistas ═══${NC}\n"
    
    if [ ! -f "build/ExportSlices3Views" ]; then
        echo -e "${RED}✗${NC} El ejecutable ExportSlices3Views no existe."
        echo -e "${YELLOW}→${NC} Compila el proyecto primero (opción 1)"
        return 1
    fi
    
    cd build
    
    echo -e "${BLUE}Selecciona el dataset:${NC}"
    echo "  1) Quarter Dose (QD)"
    echo "  2) Full Dose (FD)"
    read -p "Opción [1]: " dataset_choice
    
    case $dataset_choice in
        2)
            echo -e "\n${GREEN}→${NC} Ejecutando con Full Dose...\n"
            ./ExportSlices3Views fd
            ;;
        *)
            echo -e "\n${GREEN}→${NC} Ejecutando con Quarter Dose...\n"
            ./ExportSlices3Views qd
            ;;
    esac
    
    cd ..
    echo -e "\n${GREEN}✓${NC} Programa finalizado"
}

# Función para ejecutar ExploreDataset
run_explore_dataset() {
    echo -e "\n${YELLOW}═══ ExploreDataset - Comparación FD vs QD ═══${NC}\n"
    
    if [ ! -f "build/ExploreDataset" ]; then
        echo -e "${RED}✗${NC} El ejecutable ExploreDataset no existe."
        echo -e "${YELLOW}→${NC} Compila el proyecto primero (opción 1)"
        return 1
    fi
    
    cd build
    echo -e "${GREEN}→${NC} Iniciando exploración del dataset...\n"
    ./ExploreDataset
    cd ..
    
    echo -e "\n${GREEN}✓${NC} Programa finalizado"
}

# Script principal
main() {
    # Verificar que estamos en el directorio correcto
    if [ ! -f "CMakeLists.txt" ]; then
        echo -e "${RED}Error: Ejecuta este script desde el directorio code/${NC}"
        exit 1
    fi
    
    # Si hay argumentos, ejecutar directamente
    if [ $# -gt 0 ]; then
        case $1 in
            compile|build)
                compile
                ;;
            app|gui|vision)
                run_vision_app
                ;;
            export)
                run_export_slices
                ;;
            export3views|3views)
                run_export_slices_3views
                ;;
            explore)
                run_explore_dataset
                ;;
            clean)
                clean_build
                ;;
            *)
                echo -e "${RED}Comando no reconocido: $1${NC}"
                echo "Uso: $0 [compile|app|export|export3views|explore|clean]"
                exit 1
                ;;
        esac
        exit 0
    fi
    
    # Modo interactivo
    while true; do
        show_menu
        read -p "Selecciona una opción: " choice
        
        case $choice in
            1)
                compile
                ;;
            2)
                run_vision_app
                ;;
            3)
                run_export_slices
                ;;
            4)
                run_export_slices_3views
                ;;
            5)
                run_explore_dataset
                ;;
            6)
                if compile; then
                    run_vision_app
                fi
                ;;
            7)
                if compile; then
                    run_export_slices
                fi
                ;;
            8)
                if compile; then
                    run_export_slices_3views
                fi
                ;;
            9)
                if compile; then
                    run_explore_dataset
                fi
                ;;
            10)
                clean_build
                ;;
            0)
                echo -e "\n${GREEN}¡Hasta luego!${NC}\n"
                exit 0
                ;;
            *)
                echo -e "${RED}✗${NC} Opción inválida"
                ;;
        esac
        
        echo ""
        read -p "Presiona Enter para continuar..."
    done
}

# Ejecutar script principal
main "$@"
