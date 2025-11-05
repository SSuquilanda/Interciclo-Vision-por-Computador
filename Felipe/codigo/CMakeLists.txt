cmake_minimum_required(VERSION 3.16)
project(IntercicloProject)

# Establecer estándar C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Buscar ITK
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

# Buscar OpenCV
find_package(OpenCV REQUIRED)

# Buscar GStreamer (requerido por OpenCV)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED
    gstreamer-1.0
    gstreamer-base-1.0
    gstreamer-video-1.0
    gstreamer-app-1.0
)

# Incluir directorios
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${GST_INCLUDE_DIRS})

# Ejecutable
add_executable(main main.cpp)

# Enlazar bibliotecas
target_link_libraries(main 
    ${ITK_LIBRARIES}
    ${OpenCV_LIBS}
    ${GST_LIBRARIES}
)

# Habilitar warnings
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(main PRIVATE -Wall -Wextra)
endif()
