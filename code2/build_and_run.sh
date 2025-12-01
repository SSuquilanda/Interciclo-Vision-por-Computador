#!/bin/bash

# =============================================================================
# SCRIPT MAESTRO - BUILD + RUN - MedicalVisionApp (GUI)
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
echo "  BUILD & RUN AUTOMATION (GUI)"
echo -e "==========================================${NC}"
echo ""

# Paso 1: Compilar
echo -e "${YELLOW}═══ FASE 1: COMPILACIÓN ═══${NC}"
"$PROJECT_DIR/build.sh"

# Paso 2: Ejecutar GUI
echo ""
echo -e "${YELLOW}═══ FASE 2: EJECUCIÓN (GUI) ═══${NC}"
"$PROJECT_DIR/run.sh"
