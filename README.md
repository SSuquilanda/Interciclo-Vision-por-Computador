# Medical Vision App - Sistema de Análisis de Imágenes CT

## Descripción General

Medical Vision App es una aplicación de escritorio desarrollada en C++ para el análisis y procesamiento avanzado de imágenes médicas en formato DICOM, específicamente orientada a tomografías computarizadas (CT). La aplicación integra técnicas de visión por computador, aprendizaje profundo y morfología matemática para proporcionar herramientas profesionales de segmentación anatómica y análisis cuantitativo.

## Características Principales

### 1. Carga y Visualización de Datasets DICOM

- Lectura nativa de archivos DICOM médicos usando ITK (Insight Segmentation and Registration Toolkit)
- Soporte para datasets completos con navegación por slices
- Visualización de imágenes en escala de grises de 16-bit con conversión automática a 8-bit
- Interfaz de navegación intuitiva con lista de archivos y control deslizante

### 2. Preprocesamiento de Imágenes

El sistema ofrece múltiples técnicas de reducción de ruido y mejora de contraste:

#### Reducción de Ruido

- **Red Neuronal DnCNN**: Denoising basado en aprendizaje profundo con soporte para servidor Flask o modelo ONNX local
- **Filtro Gaussiano**: Suavizado con kernel ajustable (1-21 píxeles)
- **Filtro de Mediana**: Eliminación de ruido sal y pimienta
- **Filtro Bilateral**: Preservación de bordes durante el suavizado

#### Mejora de Contraste

- **Ecualización de Histograma**: Distribución uniforme de intensidades
- **CLAHE** (Contrast Limited Adaptive Histogram Equalization): Mejora de contraste adaptativo local

#### Pipeline de Procesamiento

El sistema aplica los filtros en orden óptimo: primero reducción de ruido, luego mejora de contraste. Calcula métricas PSNR (Peak Signal-to-Noise Ratio) y SNR (Signal-to-Noise Ratio) para evaluar la calidad.

### 3. Segmentación Anatómica Automática

#### Segmentación de Pulmones

- Umbralización adaptativa basada en valores de Hounsfield Units (HU)
- Rango de detección: -1024 a -400 HU
- Eliminación de componentes pequeñas mediante análisis de área
- Filtrado inteligente basado en posición anatómica

#### Segmentación de Huesos

- Detección de estructuras óseas (rango: 400-3071 HU)
- Algoritmo especializado de cierre de esternón mediante morfología matemática
- Filtrado de ruido en región de columna vertebral
- Criterios de filtrado: distancia al centroide y área de componentes

#### Segmentación de Aorta

- Detección de estructuras vasculares (rango: 120-300 HU)
- Análisis de componentes conectadas
- Filtrado por área y posición anatómica relativa

#### Características Comunes

- Soporte para imágenes originales o preprocesadas
- Adaptación automática de umbrales según el tipo de imagen
- Visualización con colores distintivos por estructura
- Cálculo de métricas por región segmentada

### 4. Operaciones Morfológicas Avanzadas

El sistema implementa 7 operaciones morfológicas fundamentales:

- **Erosión**: Reducción de regiones blancas
- **Dilatación**: Expansión de regiones blancas
- **Apertura**: Eliminación de ruido pequeño (erosión + dilatación)
- **Cierre**: Relleno de huecos pequeños (dilatación + erosión)
- **Gradiente Morfológico**: Detección de bordes
- **Top Hat**: Resalta estructuras brillantes pequeñas
- **Black Hat**: Resalta estructuras oscuras pequeñas

#### Parámetros Configurables

- Tamaño de kernel: 1-21 píxeles (valores impares)
- Número de iteraciones: 1-10
- Operación de relleno de huecos mediante flood fill

#### Algoritmo de Relleno de Huecos

Implementa un algoritmo de flood fill inverso que:

1. Rellena el fondo desde las esquinas
2. Invierte el resultado
3. Combina con la máscara original mediante operación OR

### 5. Visualización Interactiva

#### Modos de Visualización

- Vista de imagen original en escala de grises
- Overlay de segmentaciones con transparencia ajustable
- Visualización multi-región con códigos de color

#### Controles de Overlay

- Activación/desactivación por región anatómica
- Opacidad ajustable del overlay (0-100%)
- Actualización en tiempo real

### 6. Sistema de Métricas y Validación

#### Métricas Calculadas Automáticamente

- **Área**: Número de píxeles por estructura segmentada
- **Densidad Media (HU)**: Valor promedio de intensidad en unidades Hounsfield
- **SNR**: Relación señal-ruido por región
- **PSNR**: Comparación con imagen original (si hay preprocesamiento)
- **Precisión IoU**: Índice de similitud con Ground Truth (opcional)

#### Validación con Ground Truth

- Carga de máscaras de validación manual
- Cálculo de IoU (Intersection over Union)
- Visualización comparativa en tres paneles:
  - Segmentación automática (azul)
  - Ground Truth (verde)
  - Diferencias (blanco=coincidencia, rojo=falsos positivos, amarillo=falsos negativos)
- Métricas detalladas de intersección y unión

#### Conversión Automática de HU

Sistema inteligente de detección de rango de valores:

- Para imágenes 12-bit (0-4095): HU = valor - 1024
- Para imágenes ya en HU: usa valores directamente
- Evita valores negativos incorrectos

### 7. Exportación de Resultados

El sistema permite exportar todas las etapas del procesamiento en formato PNG:

1. **Imagen Original**: Slice DICOM normalizado a 8-bit
2. **Imagen Preprocesada**: Resultado de filtros aplicados
3. **Máscara de Segmentación**: Imagen binaria de regiones detectadas
4. **Overlay de Segmentación**: Visualización coloreada sobre original
5. **Máscara Morfológica**: Resultado de operaciones morfológicas
6. **Visualización Final**: Composición completa con todas las regiones
7. **Histograma**: Distribución de intensidades de ROI

#### Características de Exportación

- Selección de carpeta de destino mediante diálogo
- Nombrado automático: `slice_XXXX_##_descripcion.png`
- Exportación selectiva (solo etapas realizadas)
- Resumen detallado de archivos generados

## Arquitectura del Sistema

### Estructura del Proyecto

```bash
code2/
├── src/
│   ├── main.cpp                    # Punto de entrada
│   ├── ui/
│   │   ├── mainwindow.h/cpp        # Interfaz gráfica principal
│   ├── core/
│   │   ├── dicom_reader.h/cpp      # Lectura de archivos DICOM
│   │   ├── itk_opencv_bridge.h/cpp # Conversión ITK-OpenCV
│   │   ├── preprocessing.h/cpp     # Filtros y DnCNN
│   │   ├── segmentation.h/cpp      # Algoritmos de segmentación
│   │   ├── morphology.h/cpp        # Operaciones morfológicas
│   ├── server/
│   │   └── server.py               # Servidor Flask para DnCNN
│   └── models/
│       └── dncnn_grayscale.onnx    # Modelo de denoising
├── build/                          # Archivos compilados
├── CMakeLists.txt                  # Configuración de CMake
├── build.sh                        # Script de compilación
└── run.sh                          # Script de ejecución
```

### Tecnologías y Librerías

#### Lenguajes

- C++17 (Lógica principal y procesamiento)
- Python 3.x (Servidor de inferencia DnCNN)

#### Librerías Principales

- **Qt 6.10.1**: Framework de interfaz gráfica
- **OpenCV 4.10.0**: Procesamiento de imágenes y morfología
- **ITK 6.0.0**: Lectura y manipulación de DICOM
- **libcurl**: Comunicación con servidor Flask
- **Flask**: Servidor web para inferencia de red neuronal

#### Herramientas de Desarrollo

- CMake 3.x: Sistema de construcción
- GCC/G++ 15.2.1: Compilador C++
- Python pip: Gestor de paquetes

## Requisitos del Sistema

### Hardware Mínimo

- Procesador: Intel Core i5 o equivalente
- RAM: 8 GB
- Espacio en disco: 2 GB para aplicación y dependencias
- Resolución de pantalla: 1366x768 o superior

### Hardware Recomendado

- Procesador: Intel Core i7 o superior
- RAM: 16 GB o más
- GPU compatible con CUDA (opcional, para DnCNN)
- Resolución de pantalla: 1920x1080 o superior

### Software

- Sistema Operativo: Linux (Ubuntu 20.04 o superior recomendado)
- Soporte para X11 (interfaz gráfica)
- Python 3.8 o superior (para servidor DnCNN)

## Instalación

### 1. Clonar el Repositorio

```bash
git clone https://github.com/SSuquilanda/Interciclo-Vision-por-Computador.git
cd Interciclo-Vision-por-Computador/code2
```

### 2. Instalar Dependencias del Sistema

#### Ubuntu/Debian

```bash
sudo apt update
sudo apt install -y build-essential cmake git
sudo apt install -y qt6-base-dev libqt6core6 libqt6gui6 libqt6widgets6
sudo apt install -y libopencv-dev
sudo apt install -y libinsighttoolkit5-dev
sudo apt install -y libcurl4-openssl-dev
sudo apt install -y python3 python3-pip python3-venv
```

### 3. Instalar Dependencias de Python (Servidor DnCNN)

```bash
cd src/server
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
cd ../..
```

### 4. Compilar la Aplicación

```bash
chmod +x build.sh
./build.sh
```

El script realizará:

- Limpieza de compilaciones previas
- Configuración con CMake
- Compilación del proyecto
- Verificación del ejecutable

### 5. Ejecutar el Servidor DnCNN (Opcional)

En una terminal separada:

```bash
cd src/server
source venv/bin/activate
python server.py
```

El servidor se ejecutará en `http://localhost:5000`

Nota: La aplicación puede funcionar sin el servidor usando el modelo ONNX local.

## Uso de la Aplicación

### Inicio Rápido

```bash
./run.sh
```

O de forma manual:

```bash
./build/MedicalApp
```

### Flujo de Trabajo Típico

#### 1. Cargar Dataset

- Ir a pestaña "I/O Dataset"
- Clic en "Cargar Dataset"
- Seleccionar carpeta con archivos DICOM
- Navegar entre slices usando el control deslizante

#### 2. Preprocesamiento (Opcional)

- Ir a pestaña "Preprocesamiento"
- Seleccionar filtros deseados:
  - DnCNN para denoising profesional
  - Filtros clásicos para casos específicos
  - CLAHE para mejorar contraste en CT
- Ajustar parámetros según necesidad
- Clic en "Aplicar Preprocesamiento"
- Revisar métricas PSNR y SNR

#### 3. Segmentación

- Ir a pestaña "Segmentación"
- Marcar checkbox "Usar imagen preprocesada" si aplicaste filtros
- Ejecutar segmentaciones:
  - "Segmentar Pulmones": Detecta tejido pulmonar
  - "Segmentar Huesos": Detecta estructura ósea
  - "Segmentar Aorta": Detecta vaso principal
- Las máscaras se visualizan automáticamente

#### 4. Refinamiento Morfológico (Opcional)

- Ir a pestaña "Morfología"
- Seleccionar operación deseada
- Ajustar tamaño de kernel (valores impares)
- Configurar número de iteraciones
- Clic en "Aplicar Operación"
- Usar "Rellenar Huecos" para cerrar regiones

#### 5. Visualización

- Ir a pestaña "Visualización"
- Activar/desactivar regiones en el overlay
- Ajustar opacidad del overlay (0-100%)
- Clic en "Actualizar Visualización"

#### 6. Análisis de Métricas

- Ir a pestaña "Métricas"
- Clic en "Actualizar Métricas" para calcular estadísticas
- Revisar tabla con área, densidad HU, SNR, PSNR por región

#### 7. Validación con Ground Truth (Opcional)

- Colocar imagen de validación en `src/core/img/`
- Nombrar como: `gt_<estructura>_slice_<número>.png`
- Clic en "Cargar Validación (Ground Truth)"
- Revisar comparación visual y métrica IoU

#### 8. Exportar Resultados

- Clic en "Exportar Imágenes"
- Seleccionar carpeta de destino
- Todas las etapas se guardan automáticamente en PNG

## Casos de Uso Clínicos

### Análisis de Enfermedad Pulmonar

1. Cargar CT de tórax
2. Aplicar DnCNN para reducir ruido de dosis baja
3. Segmentar pulmones
4. Aplicar apertura morfológica para eliminar artefactos
5. Calcular densidad media (HU) para detectar opacidades
6. Comparar con Ground Truth del radiólogo

### Detección de Fracturas Óseas

1. Cargar CT de trauma
2. Preprocesar con filtro bilateral (preserva bordes)
3. Segmentar huesos
4. Aplicar gradiente morfológico para resaltar discontinuidades
5. Analizar área y fragmentación

### Evaluación de Aneurisma Aórtico

1. Cargar CT con contraste
2. Segmentar aorta
3. Aplicar cierre morfológico para suavizar contornos
4. Calcular área transversal
5. Exportar visualización para reporte

## Formato de Datos

### Entrada: Archivos DICOM

- Extensión: `.dcm` o `.IMA`
- Modalidad: CT (Computed Tomography)
- Profundidad de bits: 12-bit o 16-bit
- Orientación: Axial preferida
- Formato de píxel: Escala de grises

### Salida: Imágenes PNG

- Profundidad de bits: 8-bit (0-255)
- Canales:
  - Escala de grises para máscaras
  - RGB para visualizaciones coloreadas
- Resolución: Mantiene dimensiones originales

## Valores de Referencia en Unidades Hounsfield (HU)

| Estructura | Rango HU | Aplicación |
|-----------|----------|------------|
| Aire | -1000 | Exterior/cavidades |
| Pulmón | -900 a -400 | Detección de tejido pulmonar |
| Grasa | -120 a -90 | Tejido adiposo |
| Agua | 0 | Referencia estándar |
| Tejido blando | 40 a 80 | Músculos, órganos |
| Contraste vascular | 120 a 300 | Vasos con medio de contraste |
| Hueso | 400 a 1000+ | Estructura ósea |

## Solución de Problemas

### La aplicación no compila

**Problema**: Errores de CMake o compilación

**Solución**:

```bash
# Limpiar completamente
rm -rf build/
mkdir build

# Verificar versiones
cmake --version  # Debe ser 3.x
g++ --version    # Debe ser compatible con C++17

# Reinstalar dependencias
sudo apt install --reinstall qt6-base-dev libopencv-dev
```

### DnCNN no funciona

**Problema**: Error al conectar con servidor Flask

**Solución**:

1. Verificar que el servidor esté ejecutándose: `curl http://localhost:5000`
2. Iniciar servidor manualmente: `cd src/server && python server.py`
3. La aplicación usará el modelo ONNX local como fallback automáticamente

### Valores de densidad negativos incorrectos

**Problema**: Densidades HU incorrectas (muy negativas)

**Causa**: Rango de valores DICOM no detectado correctamente

**Solución**: El sistema detecta automáticamente el rango. Si persiste:

- Verificar que el archivo sea CT válido
- Revisar metadata DICOM del archivo
- Usar herramienta externa para validar: `gdcmdump archivo.dcm`

### Segmentación imprecisa

**Problema**: Regiones no detectadas correctamente

**Solución**:

1. Aplicar preprocesamiento (CLAHE recomendado para CT)
2. Marcar checkbox "Usar imagen preprocesada"
3. Ajustar parámetros de filtros si necesario
4. Aplicar operaciones morfológicas de refinamiento

### Exportación no genera archivos

**Problema**: Botón exportar no crea archivos

**Causa**: No hay etapas procesadas

**Solución**:

1. Ejecutar al menos una segmentación
2. Verificar permisos de escritura en carpeta destino
3. Revisar log de la aplicación para errores

## Limitaciones Conocidas

1. **Formato de entrada**: Solo soporta archivos DICOM, no admite otros formatos médicos (NIFTI, ANALYZE)
2. **Orientación**: Optimizado para slices axiales, puede requerir ajustes para sagitales/coronales
3. **Contraste**: La segmentación de aorta requiere medio de contraste para mejores resultados
4. **Resolución**: Rendimiento óptimo con imágenes 512x512, resoluciones mayores pueden ser lentas
5. **3D**: No hay visualización volumétrica, solo procesamiento slice por slice
6. **Multithreading**: El procesamiento es secuencial, no aprovecha múltiples cores

## Contribuciones

Este proyecto fue desarrollado como parte del curso de Visión por Computador. Para contribuir:

1. Fork el repositorio
2. Crear branch para feature: `git checkout -b feature/nueva-funcionalidad`
3. Commit cambios: `git commit -am 'Añadir nueva funcionalidad'`
4. Push al branch: `git push origin feature/nueva-funcionalidad`
5. Crear Pull Request

## Licencia

Este proyecto es de código abierto para fines educativos y de investigación.

## Contacto

- Repositorio: <https://github.com/SSuquilanda/Interciclo-Vision-por-Computador>
- Universidad: [Nombre de la Universidad]
- Curso: Visión por Computador

## Referencias

1. K. Zhang, W. Zuo, Y. Chen, D. Meng, and L. Zhang, "Beyond a Gaussian Denoiser: Residual Learning of Deep CNN for Image Denoising," IEEE TIP, 2017.
2. ITK Software Guide: <https://itk.org/ItkSoftwareGuide.pdf>
3. OpenCV Documentation: <https://docs.opencv.org/>
4. DICOM Standard: <https://www.dicomstandard.org/>

## Agradecimientos

- Comunidad de ITK por las herramientas de procesamiento médico
- Desarrolladores de OpenCV por las funciones de visión por computador
- Equipo de Qt por el framework de interfaz gráfica
- Autores del modelo DnCNN por el denoising con deep learning
