#!/bin/bash

# =============================================================================
# SCRIPT DE EJECUCIÓN - MedicalVisionApp
# =============================================================================

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
EXECUTABLE="$PROJECT_DIR/build/MedicalApp"

# Colores
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

# Verificar que el ejecutable existe
if [ ! -f "$EXECUTABLE" ]; then
    echo -e "${RED}✗ Error: Ejecutable no encontrado${NC}"
    echo "Por favor, ejecuta primero: ./build.sh"
    exit 1
fi

# Si se proporciona un argumento, usarlo como ruta DICOM
if [ $# -eq 1 ]; then
    DICOM_FILE="$1"
else
    # Usar un archivo de ejemplo por defecto
    DICOM_FILE="$PROJECT_DIR/../data/L291_fd/L291_FD_3_1.CT.0005.0100.2015.12.23.17.48.23.868235.130270509.IMA"
fi

# Verificar que el archivo existe
if [ ! -f "$DICOM_FILE" ]; then
    echo -e "${RED}✗ Error: Archivo DICOM no encontrado: $DICOM_FILE${NC}"
    echo ""
    echo "Uso:"
    echo "  ./run.sh <ruta_archivo_DICOM>"
    echo ""
    echo "Ejemplo:"
    echo "  ./run.sh ../data/L291_fd/L291_FD_3_1.CT.0005.0100.2015.12.23.17.48.23.868235.130270509.IMA"
    exit 1
fi

echo "=========================================="
echo "  EJECUTANDO MEDICAL VISION APP"
echo "=========================================="
echo ""
echo -e "${YELLOW}Archivo DICOM:${NC} $DICOM_FILE"
echo ""

# Ejecutar la aplicación
"$EXECUTABLE" "$DICOM_FILE"

EXIT_CODE=$?
echo ""
if [ $EXIT_CODE -eq 0 ]; then
    echo -e "${GREEN}✓ Ejecución completada exitosamente${NC}"
else
    echo -e "${RED}✗ Error en la ejecución (código: $EXIT_CODE)${NC}"
fi

exit $EXIT_CODE
