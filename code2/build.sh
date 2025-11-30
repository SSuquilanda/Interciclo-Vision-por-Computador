#!/bin/bash

# =============================================================================
# SCRIPT DE COMPILACIÓN Y EJECUCIÓN AUTOMÁTICA - MedicalVisionApp
# =============================================================================

set -e  # Detener si hay errores

echo "=========================================="
echo "  MEDICAL VISION APP - BUILD SYSTEM"
echo "=========================================="
echo ""

# Colores para mensajes
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Directorio del proyecto
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

echo -e "${YELLOW}[1/4] Limpiando build anterior...${NC}"
if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
    echo "      ✓ Build anterior eliminado"
fi
mkdir -p "$BUILD_DIR"
echo ""

echo -e "${YELLOW}[2/4] Configurando con CMake...${NC}"
cd "$BUILD_DIR"
cmake .. || {
    echo -e "${RED}✗ Error en configuración CMake${NC}"
    exit 1
}
echo -e "${GREEN}      ✓ Configuración exitosa${NC}"
echo ""

echo -e "${YELLOW}[3/4] Compilando proyecto...${NC}"
make -j$(nproc) || {
    echo -e "${RED}✗ Error en compilación${NC}"
    exit 1
}
echo -e "${GREEN}      ✓ Compilación exitosa${NC}"
echo ""

echo -e "${YELLOW}[4/4] Verificando ejecutable...${NC}"
if [ -f "$BUILD_DIR/MedicalApp" ]; then
    echo -e "${GREEN}      ✓ Ejecutable generado: $BUILD_DIR/MedicalApp${NC}"
    echo ""
    echo "=========================================="
    echo -e "${GREEN}  ✓ BUILD COMPLETADO CON ÉXITO${NC}"
    echo "=========================================="
    echo ""
    echo "Para ejecutar la aplicación:"
    echo "  $BUILD_DIR/MedicalApp <ruta_archivo_DICOM>"
    echo ""
    echo "Ejemplo:"
    echo "  $BUILD_DIR/MedicalApp ../data/L291_fd/L291_FD_3_1.CT.0005.0100.2015.12.23.17.48.23.868235.130270509.IMA"
    echo ""
else
    echo -e "${RED}✗ Error: Ejecutable no encontrado${NC}"
    exit 1
fi
