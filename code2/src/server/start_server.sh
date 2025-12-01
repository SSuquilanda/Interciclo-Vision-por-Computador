#!/bin/bash

echo "=========================================="
echo "  FLASK DNCNN SERVER - STARTUP"
echo "=========================================="
echo ""

# Verificar que existe el modelo
if [ ! -f "../models/dncnn_grayscale.onnx" ]; then
    echo "[✗] ERROR: Modelo dncnn_grayscale.onnx no encontrado"
    echo "    Ubicación esperada: ../models/dncnn_grayscale.onnx"
    exit 1
fi

echo "[✓] Modelo DnCNN encontrado"
echo ""

# Verificar dependencias de Python
echo "[INFO] Verificando dependencias de Python..."
python3 -c "import flask" 2>/dev/null || { echo "[✗] Flask no instalado. Instalar con: pip install flask"; exit 1; }
python3 -c "import cv2" 2>/dev/null || { echo "[✗] OpenCV no instalado. Instalar con: pip install opencv-python"; exit 1; }
python3 -c "import numpy" 2>/dev/null || { echo "[✗] NumPy no instalado. Instalar con: pip install numpy"; exit 1; }
echo "[✓] Todas las dependencias instaladas"
echo ""

# Iniciar servidor
echo "=========================================="
echo "  Iniciando servidor Flask..."
echo "  URL: http://localhost:5000/denoise"
echo "=========================================="
echo ""

python3 app.py
