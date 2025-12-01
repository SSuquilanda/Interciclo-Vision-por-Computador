cmake_minimum_required(VERSION 3.16)
project(MedicalVisionApp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# 1. CONFIGURACIÓN DE QT6
# ==============================================================================
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ==============================================================================
# 2. RUTAS Y DEPENDENCIAS (Copiado de tu config funcional)
# ==============================================================================
if(WIN32)
    message(STATUS "Configurando para Windows")
    set(OpenCV_DIR "C:/dev/opencv/build_cpu")
    set(ITK_DIR "C:/Program Files/ITK")
    set(Qt6_DIR "C:/Qt/6.10.1/msvc2022_64/lib/cmake/Qt6")
    set(CMAKE_PREFIX_PATH "C:/Qt/6.10.1/msvc2022_64")
else()
    message(STATUS "Configurando para Linux")
    set(OpenCV_DIR "/home/felipep/Documentos/universidad/universidad 7mo/vision por computador/opencv-dev/install/lib/cmake/opencv4")
    set(ITK_DIR "/home/felipep/Documentos/universidad/universidad 7mo/vision por computador/ITK/build")
endif()

# Buscar Paquetes
find_package(Qt6 COMPONENTS Core Widgets Gui REQUIRED)
find_package(OpenCV REQUIRED)
find_package(ITK REQUIRED COMPONENTS ITKCommon ITKIOImageBase ITKIOGDCM ITKVideoBridgeOpenCV ITKImageStatistics)
find_package(CURL REQUIRED)

# (Opcional) GStreamer para Linux
if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GST gstreamer-1.0 gstreamer-video-1.0)
endif()

include(${ITK_USE_FILE})

# ==============================================================================
# 3. DIRECTORIOS E INCLUDES
# ==============================================================================
# Incluir src para que los #include "core/..." funcionen bien
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${OpenCV_INCLUDE_DIRS})

# Macro para encontrar la carpeta 'data' y 'models' desde el código
add_definitions(-DPROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

# ==============================================================================
# 4. DEFINICIÓN DE ARCHIVOS FUENTE (LA LIMPIEZA)
# ==============================================================================

# Lógica Matemática (Backend)
set(CORE_SOURCES
    src/core/dicom_reader.cpp
    src/core/itk_opencv_bridge.cpp
    src/core/preprocessing.cpp
    src/core/segmentation.cpp
    src/core/morphology.cpp
)

# Interfaz Gráfica (Frontend)
set(UI_SOURCES
    src/ui/mainwindow.cpp
)

# ==============================================================================
# 5. CREACIÓN DEL EJECUTABLE FINAL
# ==============================================================================
add_executable(MedicalApp
    src/main.cpp      # Tu punto de entrada limpio
    ${CORE_SOURCES}
    ${UI_SOURCES}
)

# ==============================================================================
# 6. VINCULACIÓN DE LIBRERÍAS
# ==============================================================================
if(UNIX AND NOT APPLE)
    target_link_libraries(MedicalApp PRIVATE
        Qt6::Core Qt6::Widgets Qt6::Gui
        ${OpenCV_LIBS}
        ${ITK_LIBRARIES}
        ${GST_LIBRARIES}
        CURL::libcurl
    )
else()
    target_link_libraries(MedicalApp PRIVATE
        Qt6::Core Qt6::Widgets Qt6::Gui
        ${OpenCV_LIBS}
        ${ITK_LIBRARIES}
        CURL::libcurl
    )
endif()

# ==============================================================================
# 7. MENSAJES DE ÉXITO
# ==============================================================================
message(STATUS "-------------------------------------------------")
message(STATUS "PROYECTO: MedicalVisionApp (Clean Architecture)")
message(STATUS "-------------------------------------------------")
message(STATUS "Compilando ejecutable: MedicalApp")
message(STATUS "OpenCV: ${OpenCV_VERSION}")
message(STATUS "ITK:    ${ITK_VERSION}")
message(STATUS "Qt:     ${Qt6_VERSION}")