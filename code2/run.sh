#!/bin/bash

# =============================================================================
# SCRIPT DE EJECUCIÓN - MedicalVisionApp (GUI)
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

echo "=========================================="
echo "  EJECUTANDO MEDICAL VISION APP (GUI)"
echo "=========================================="
echo ""
echo -e "${YELLOW}Iniciando interfaz gráfica...${NC}"
echo ""

# Ejecutar la aplicación GUI (no necesita argumentos)
"$EXECUTABLE"

EXIT_CODE=$?
echo ""
if [ $EXIT_CODE -eq 0 ]; then
    echo -e "${GREEN}✓ Aplicación cerrada correctamente${NC}"
else
    echo -e "${RED}✗ Error en la ejecución (código: $EXIT_CODE)${NC}"
fi

exit $EXIT_CODE
