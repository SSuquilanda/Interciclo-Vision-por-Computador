cmake_minimum_required(VERSION 3.16)
project(ProyectoVision CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Configuración de rutas según sistema operativo ---
if(WIN32)
    # Rutas de Sami (Windows)
    message(STATUS "Configurando para Windows (Sami)")
    set(OpenCV_DIR "C:/dev/opencv/build_cpu")
    set(ITK_DIR "C:/Program Files/ITK")
    set(CMAKE_CONFIGURATION_TYPES "Release")
else()
    # Rutas de Felipe (Linux)
    message(STATUS "Configurando para Linux (Felipe)")
    # OpenCV_DIR debe apuntar a la carpeta que contiene OpenCVConfig.cmake
    set(OpenCV_DIR "/home/felipep/Documentos/universidad/universidad 7mo/vision por computador/opencv-dev/install/lib/cmake/opencv4")
    # ITK_DIR debe apuntar a la carpeta build de ITK que contiene ITKConfig.cmake
    set(ITK_DIR "/home/felipep/Documentos/universidad/universidad 7mo/vision por computador/ITK/build")
endif()

# --- Buscar paquetes ---
find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
    message(STATUS "OpenCV encontrado: ${OpenCV_VERSION}")
    message(STATUS "   OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "OpenCV no encontrado")
endif()

find_package(ITK REQUIRED COMPONENTS ITKCommon ITKIOImageBase ITKIOGDCM ITKVideoBridgeOpenCV)
if(ITK_FOUND)
    message(STATUS "ITK encontrado: ${ITK_VERSION}")
    include(${ITK_USE_FILE})
else()
    message(FATAL_ERROR "ITK no encontrado")
endif()

# --- Buscar GStreamer (solo en Linux, requerido por OpenCV) ---
if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GST REQUIRED
        gstreamer-1.0
        gstreamer-base-1.0
        gstreamer-video-1.0
        gstreamer-app-1.0
    )
    if(GST_FOUND)
        message(STATUS "GStreamer encontrado: ${GST_VERSION}")
    endif()
endif()

# --- Incluir directorios ---
include_directories(${OpenCV_INCLUDE_DIRS})

# Inyectar la ruta del proyecto como macro para C++
add_definitions(-DPROJECT_SOURCE_DIR=${CMAKE_SOURCE_DIR})


# --- Crear ejecutable ---
add_executable(MyApp src/main.cpp)

# --- Enlazar bibliotecas ---
if(UNIX AND NOT APPLE)
    # Linux: incluir GStreamer
    target_link_libraries(MyApp PRIVATE
        ${OpenCV_LIBS}
        ${ITK_LIBRARIES}
        ${GST_LIBRARIES}
    )
    target_include_directories(MyApp PRIVATE ${GST_INCLUDE_DIRS})
else()
    # Windows: solo OpenCV e ITK
    target_link_libraries(MyApp PRIVATE
        ${OpenCV_LIBS}
        ${ITK_LIBRARIES}
    )
endif()

# --- Habilitar warnings ---
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(MyApp PRIVATE -Wall -Wextra)
elseif(MSVC)
    target_compile_options(MyApp PRIVATE /W4)
endif()

# --- Información de compilación ---
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "Configuración del proyecto completada")
message(STATUS "   Sistema: ${CMAKE_SYSTEM_NAME}")
message(STATUS "   Compilador: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "   Ejecutable: MyApp")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")