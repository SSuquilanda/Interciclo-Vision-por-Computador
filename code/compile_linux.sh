#!/bin/bash

# Script de compilación para Linux

echo "╔════════════════════════════════════════════════════════════╗"
echo "║  Compilando Proyecto Interciclo (Linux)                   ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""

# Crear directorio build si no existe
if [ ! -d "build" ]; then
    echo "📁 Creando directorio build..."
    mkdir build
fi

# Entrar al directorio build
cd build

# Limpiar build anterior si existe
echo "🧹 Limpiando compilación anterior..."
rm -rf *

# Configurar con CMake
echo ""
echo "⚙️  Configurando con CMake..."
cmake .. || { echo "❌ Error en CMake"; exit 1; }

# Compilar
echo ""
echo "🔨 Compilando..."
make || { echo "❌ Error en la compilación"; exit 1; }

# Ejecutar si la compilación fue exitosa
echo ""
echo "╔════════════════════════════════════════════════════════════╗"
echo "║  ✅ Compilación exitosa - Ejecutando programa...          ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""

./MyApp

echo ""
echo "✅ Programa finalizado"
