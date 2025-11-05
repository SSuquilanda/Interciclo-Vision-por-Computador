#!/bin/bash

# Script para compilar y ejecutar el proyecto

echo "╔════════════════════════════════════════════════════════════╗"
echo "║  Compilando proyecto ITK + OpenCV...                      ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""

# Crear directorio build si no existe
if [ ! -d "build" ]; then
    echo "Creando directorio build..."
    mkdir build
fi

# Entrar al directorio build
cd build

# Configurar con CMake
echo "Configurando con CMake..."
cmake .. || { echo "Error en CMake"; exit 1; }

# Compilar
echo ""
echo "Compilando..."
make || { echo "Error en la compilación"; exit 1; }

# Ejecutar si la compilación fue exitosa
echo ""
echo "╔════════════════════════════════════════════════════════════╗"
echo "║  Compilación exitosa - Ejecutando programa...              ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""

./main

echo ""
echo "Programa finalizado"
