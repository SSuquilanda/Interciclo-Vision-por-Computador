cmake_minimum_required(VERSION 3.16)
project(ProyectoVision CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Encontrar OpenCV ---
# Le damos a CMake una PISTA de dónde está tu carpeta 'build' de OpenCV
# Esta es la carpeta que CONTIENE el archivo "OpenCVConfig.cmake"
set(OpenCV_DIR "C:/dev/opencv/build_cpu")
find_package(OpenCV REQUIRED)

# --- Encontrar ITK ---
# Le damos a CMake una PISTA de dónde está tu carpeta 'itk_build'
# Esta es la carpeta que CONTIENE el archivo "ITKConfig.cmake"
set(ITK_DIR "C:/Program Files/ITK")
find_package(ITK REQUIRED COMPONENTS ITKVideoBridgeOpenCV ITKIOGDCM)

# --- Configurar el Proyecto ---
# Le decimos a CMake que busque los archivos .h de las librerías
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${ITK_INCLUDE_DIRS})

# Le decimos a CMake dónde está tu código fuente
add_executable(MyApp src/main.cpp)

# Le decimos a CMake que enlace tu app con las librerías de OpenCV e ITK
target_link_libraries(MyApp PRIVATE
    ${OpenCV_LIBS}
    ${ITK_LIBRARIES}
)