#!/bin/bash

# =============================================================================
# SCRIPT MAESTRO - BUILD + RUN - MedicalVisionApp
# =============================================================================

set -e

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Colores
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m'

echo ""
echo -e "${BLUE}=========================================="
echo "  MEDICAL VISION APP"
echo "  BUILD & RUN AUTOMATION"
echo -e "==========================================${NC}"
echo ""

# Paso 1: Compilar
echo -e "${YELLOW}═══ FASE 1: COMPILACIÓN ═══${NC}"
"$PROJECT_DIR/build.sh"

# Verificar si se proporcionó un archivo DICOM
if [ $# -eq 1 ]; then
    DICOM_FILE="$1"
    echo ""
    echo -e "${YELLOW}═══ FASE 2: EJECUCIÓN ═══${NC}"
    "$PROJECT_DIR/run.sh" "$DICOM_FILE"
else
    echo ""
    echo -e "${GREEN}✓ Compilación completada${NC}"
    echo ""
    echo "Para ejecutar la aplicación:"
    echo -e "  ${YELLOW}./run.sh <ruta_archivo_DICOM>${NC}"
    echo ""
    echo "O ejecuta este script con un archivo DICOM:"
    echo -e "  ${YELLOW}./build_and_run.sh <ruta_archivo_DICOM>${NC}"
    echo ""
fi
