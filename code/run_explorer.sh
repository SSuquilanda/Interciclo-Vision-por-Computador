#!/bin/bash

echo "EXPLORADOR DE DATASET - Full Dose vs Quarter Dose"
echo ""

cd build

if [ ! -f "ExploreDataset" ]; then
    echo "Error: El ejecutable ExploreDataset no existe."
    echo "   Compila el proyecto primero con: ./compile_linux.sh"
    exit 1
fi

./ExploreDataset
