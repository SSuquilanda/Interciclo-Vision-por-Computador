cmake_minimum_required(VERSION 3.16)
project(ProyectoVision CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configuración de Qt6
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Configuración de rutas según sistema operativo
if(WIN32)
    message(STATUS "Configurando para Windows")
    set(OpenCV_DIR "C:/dev/opencv/build_cpu")
    set(ITK_DIR "C:/Program Files/ITK")
    set(CMAKE_CONFIGURATION_TYPES "Release")
else()
    message(STATUS "Configurando para Linux")
    set(OpenCV_DIR "/home/felipep/Documentos/universidad/universidad 7mo/vision por computador/opencv-dev/install/lib/cmake/opencv4")
    set(ITK_DIR "/home/felipep/Documentos/universidad/universidad 7mo/vision por computador/ITK/build")
endif()

# Buscar paquetes
find_package(Qt6 COMPONENTS Core Widgets Gui REQUIRED)
if(Qt6_FOUND)
    message(STATUS "Qt6 encontrado: ${Qt6_VERSION}")
else()
    message(FATAL_ERROR "Qt6 no encontrado")
endif()

find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
    message(STATUS "OpenCV encontrado: ${OpenCV_VERSION}")
    message(STATUS "   OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "OpenCV no encontrado")
endif()

find_package(ITK REQUIRED COMPONENTS ITKCommon ITKIOImageBase ITKIOGDCM ITKVideoBridgeOpenCV ITKImageStatistics)
if(ITK_FOUND)
    message(STATUS "ITK encontrado: ${ITK_VERSION}")
    include(${ITK_USE_FILE})
else()
    message(FATAL_ERROR "ITK no encontrado")
endif()

# Buscar GStreamer en Linux
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

# Incluir directorios
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/src)

# Inyectar la ruta del proyecto como macro para C++
add_definitions(-DPROJECT_SOURCE_DIR=${CMAKE_SOURCE_DIR})

# Definir archivos fuente comunes
set(COMMON_SOURCES
    src/f2_io/dicom_reader.cpp
    src/f2_io/dataset_explorer.cpp
    src/f3_preprocessing/preprocessing.cpp
    src/f4_segmentation/segmentation.cpp
    src/f5_morphology/morphology.cpp
    src/utils/itk_opencv_bridge.cpp
    src/f6_visualization/visualization.cpp
)

# Definir archivos fuente de la UI Qt
set(UI_SOURCES
    src/f1_ui/mainwindow.cpp
)

# Crear ejecutable principal con interfaz Qt (Aplicación de escritorio)
add_executable(VisionApp src/main.cpp ${UI_SOURCES} ${COMMON_SOURCES})

# Crear ejecutable para exportación de slices (Fase 2.1)
add_executable(ExportSlices src/export_slices.cpp ${COMMON_SOURCES})

# Crear ejecutable para exportación de slices en 3 vistas (Fase 2.1 extendida)
add_executable(ExportSlices3Views src/export_slices_3_views.cpp ${COMMON_SOURCES})

# Crear ejecutable para exploración del dataset (Fase 2.2)
add_executable(ExploreDataset src/explore_dataset.cpp ${COMMON_SOURCES})

# Enlazar bibliotecas para todos los ejecutables
if(UNIX AND NOT APPLE)
    # Incluir GStreamer en Linux
    target_link_libraries(VisionApp PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        ${OpenCV_LIBS}
        ${ITK_LIBRARIES}
        ${GST_LIBRARIES}
    )
    target_include_directories(VisionApp PRIVATE ${GST_INCLUDE_DIRS})
    
    target_link_libraries(ExportSlices PRIVATE
        ${OpenCV_LIBS}
        ${ITK_LIBRARIES}
        ${GST_LIBRARIES}
    )
    target_include_directories(ExportSlices PRIVATE ${GST_INCLUDE_DIRS})
    
    target_link_libraries(ExportSlices3Views PRIVATE
        ${OpenCV_LIBS}
        ${ITK_LIBRARIES}
        ${GST_LIBRARIES}
    )
    target_include_directories(ExportSlices3Views PRIVATE ${GST_INCLUDE_DIRS})
    
    target_link_libraries(ExploreDataset PRIVATE
        ${OpenCV_LIBS}
        ${ITK_LIBRARIES}
        ${GST_LIBRARIES}
    )
    target_include_directories(ExploreDataset PRIVATE ${GST_INCLUDE_DIRS})
else()
    # OpenCV e ITK en Windows
    target_link_libraries(VisionApp PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        ${OpenCV_LIBS}
        ${ITK_LIBRARIES}
    )
    
    target_link_libraries(ExportSlices PRIVATE
        ${OpenCV_LIBS}
        ${ITK_LIBRARIES}
    )
    
    target_link_libraries(ExportSlices3Views PRIVATE
        ${OpenCV_LIBS}
        ${ITK_LIBRARIES}
    )
    
    target_link_libraries(ExploreDataset PRIVATE
        ${OpenCV_LIBS}
        ${ITK_LIBRARIES}
    )
endif()

# Habilitar warnings para todos los ejecutables
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(VisionApp PRIVATE -Wall -Wextra)
    target_compile_options(ExportSlices PRIVATE -Wall -Wextra)
    target_compile_options(ExportSlices3Views PRIVATE -Wall -Wextra)
    target_compile_options(ExploreDataset PRIVATE -Wall -Wextra)
elseif(MSVC)
    target_compile_options(VisionApp PRIVATE /W4)
    target_compile_options(ExportSlices PRIVATE /W4)
    target_compile_options(ExportSlices3Views PRIVATE /W4)
    target_compile_options(ExploreDataset PRIVATE /W4)
endif()

message(STATUS "   Sistema: ${CMAKE_SYSTEM_NAME}")
message(STATUS "   Compilador: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "   Ejecutables: VisionApp (GUI), ExportSlices, ExploreDataset")