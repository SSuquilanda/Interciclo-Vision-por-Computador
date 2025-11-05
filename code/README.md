# Proyecto Interciclo - Análisis de Imágenes CT

## CMakeLists.txt Multi-plataforma (Windows + Linux)

---

## Correcciones Realizadas al CMakeLists.txt

### **Problemas encontrados:**

1. **Typo crítico**: `make_minimum_required` → debería ser `cmake_minimum_required`
2. **Rutas incorrectas en Linux**:
    - `OpenCV_DIR` apuntaba a `/include/opencv4` (debería ser `/lib/cmake/opencv4`)
    - `ITK_DIR` apuntaba a la raíz de ITK (debería ser `/ITK/build`)
3. **Falta de GStreamer en Linux**: OpenCV en Linux requiere GStreamer
4. **Sintaxis inconsistente**: Usaba `IF/ELSE` en mayúsculas (debería ser `if/else`)
5. **Falta de validación**: No verificaba si los paquetes se encontraron correctamente
6. **Componentes ITK incompletos**: Faltaban componentes necesarios

### **Correcciones aplicadas:**

1. **Sintaxis corregida**: `cmake_minimum_required` con 'c' minúscula
2. **Rutas corregidas**:
    - Linux OpenCV: `.../opencv-dev/install/lib/cmake/opencv4`
    - Linux ITK: `.../ITK/build`
3. **Agregado soporte GStreamer** para Linux
4. **Validación de paquetes** con mensajes informativos
5. **Componentes ITK completos**: `ITKCommon ITKIOImageBase ITKIOGDCM ITKVideoBridgeOpenCV`
6. **Warnings habilitados** para mejor calidad de código

---

## Estructura del Proyecto

```bash
code/
├── CMakeLists.txt           Multi-plataforma (Windows + Linux)
├── compile_linux.sh         Script de compilación para Linux
├── compile_windows.bat      Script de compilación para Windows
├── src/
│   └── main.cpp             Código de prueba de integración
└── build/                   Carpeta de compilación (auto-generada)
```

---

## Cómo Compilar y Ejecutar

### **En Linux (Felipe):**

```bash
cd code
./compile_linux.sh
```

O manualmente:

```bash
cd code
mkdir build && cd build
cmake ..
make
./MyApp
```

### **En Windows (Sami):**

```batch
cd code
compile_windows.bat
```

O manualmente:

```batch
cd code
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
Release\MyApp.exe
```

---

## Requisitos Previos

### **Linux:**

- CMake 3.16+
- GCC/G++ con soporte C++17
- OpenCV 4.x instalado
- ITK 5.x/6.x instalado y compilado
- GStreamer (gstreamer-1.0, gstreamer-base-1.0, etc.)

### **Windows:**

- CMake 3.16+
- Visual Studio 2019 o superior
- OpenCV 4.x compilado en `C:/dev/opencv/build_cpu`
- ITK instalado en `C:/Program Files/ITK`

---

## Configuración de Rutas

### **Modificar rutas en CMakeLists.txt si es necesario:**

**Para Linux (Felipe):**

```cmake
set(OpenCV_DIR "/tu/ruta/a/opencv-dev/install/lib/cmake/opencv4")
set(ITK_DIR "/tu/ruta/a/ITK/build")
```

**Para Windows (Sami):**

```cmake
set(OpenCV_DIR "C:/dev/opencv/build_cpu")
set(ITK_DIR "C:/Program Files/ITK")
```

---

## Verificar la Instalación

Al ejecutar `./MyApp` (Linux) o `MyApp.exe` (Windows), deberías ver:

```bash
╔═══════════════════════════════════════════════════════════╗
║     Proyecto Interciclo - Visión por Computador          ║
║           Análisis de Imágenes CT (DICOM)                ║
║              OpenCV + ITK Integration                     ║
╚═══════════════════════════════════════════════════════════╝

Información de bibliotecas:
    OpenCV version: 4.10.0
    ITK version: 6.0.0

Probando OpenCV...
    OpenCV funcionando correctamente
    Tamaño de imagen de prueba: 640x480

Probando ITK...
    ITK funcionando correctamente
    Imagen ITK creada: 512x512

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
¡Todas las bibliotecas están funcionando correctamente!
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Siguiente paso: Agregar código para procesar imágenes DICOM
```

---

## Solución de Problemas

### **Error: "OpenCV not found"**

- Verifica que `OpenCV_DIR` apunte a la carpeta que contiene `OpenCVConfig.cmake`
- En Linux: `.../lib/cmake/opencv4/`
- En Windows: `.../build/` o `.../build_cpu/`

### **Error: "ITK not found"**

- Verifica que `ITK_DIR` apunte a la carpeta `build` de ITK que contiene `ITKConfig.cmake`
- Asegúrate de haber compilado ITK antes de usarlo

### **Error: "GStreamer not found" (Linux)**

- Instala GStreamer: `sudo apt install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev`

### **Error de compilación con Visual Studio (Windows)**

- Asegúrate de tener Visual Studio 2019 instalado
- Ajusta el generador CMake según tu versión: `-G "Visual Studio 16 2019"` o `-G "Visual Studio 17 2022"`

---

## Próximos Pasos

1. Verificar que el proyecto compila en ambos sistemas
2. Probar la lectura de archivos DICOM
3. Implementar extracción de slices
4. Aplicar técnicas de procesamiento de imágenes
5. Desarrollar la interfaz de usuario

---

## Colaboradores

- **Felipe** - Desarrollo en Linux
- **Sami** - Desarrollo en Windows

---

## Referencias

- [CMake Documentation](https://cmake.org/documentation/)
- [OpenCV Documentation](https://docs.opencv.org/)
- [ITK Documentation](https://itk.org/Doxygen/html/index.html)
- [ITK + OpenCV Bridge](https://examples.itk.org/src/bridge/videobridgeopencv/index.html)
