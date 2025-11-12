# TODO - Proyecto Integrador Parte I (Interciclo)

## Análisis de Imágenes CT con ITK + OpenCV C++

---

## Información General del Proyecto

**Objetivo:** Desarrollar una aplicación en C++ usando OpenCV e ITK para procesar imágenes CT (DICOM), extraer slices, y resaltar áreas de interés (pulmones, corazón, estructuras óseas) para análisis médico.

**Dataset:** CT Low Dose Reconstruction (Kaggle)

- URL: <https://www.kaggle.com/datasets/andrewmvd/ct-low-dose-reconstruction/data>
- Paciente seleccionado: L291
- Formato: DICOM (.IMA)
- Slices disponibles: 343 por modalidad

**Fecha de Entrega:** Semana de revisión de exámenes interciclo

**Ponderación:** 15 puntos proyecto + 5 puntos informe = 20 puntos total

---

## FASE 0: PREPARACIÓN Y CONFIGURACIÓN

### 0.1 Configuración del Entorno

- [X] Verificar instalación de ITK 6.0.0
- [X] Verificar instalación de OpenCV 4.10.0
- [X] Verificar instalación de Qt6 6.10.0
- [X] Configurar CMake 3.16+ con todas las dependencias
- [X] Probar compilación del proyecto base
- [X] Configurar estructura modular del proyecto

**Estado:** Completado. Entorno configurado con todas las bibliotecas necesarias y sistema de compilación CMake funcional.

### 0.2 Obtención y Exploración del Dataset

- [X] Descargar dataset "CT Low Dose Reconstruction" desde Kaggle
- [X] Organizar dataset en estructura de directorios data/
- [X] Seleccionar paciente L291 (único por grupo)
- [X] Explorar estructura del dataset:
  - [X] Full Dose (L291_fd/): 343 slices
  - [X] Quarter Dose (L291_qd/): 343 slices
  - [X] Formato: DICOM (.IMA), 512x512 píxeles, 16-bit
- [X] Crear herramienta de análisis comparativo (ExploreDataset)

**Estado:** Completado. Dataset L291 seleccionado y analizado completamente.

### 0.3 Investigación Médica

- [ ] Consultar con radiólogo sobre criterios de segmentación
- [ ] Documentar valores de HU (Unidades Hounsfield) por tejido:
  - [ ] Pulmones: -1000 a -400 HU
  - [ ] Corazón: 0 a 100 HU
  - [ ] Estructuras óseas: mayor a 200 HU
- [ ] Validar criterios médicos para segmentación correcta
- [ ] Documentar metodología en archivo separado

**Estado:** Pendiente. Requiere consulta con especialista médico.

---

## FASE 1 (UI): APLICACIÓN DE ESCRITORIO Qt6

### 1.1 Configuración Base de Qt6

- [X] Integrar Qt6 en CMakeLists.txt
  - [X] find_package(Qt6 COMPONENTS Core Widgets Gui)
  - [X] Configurar CMAKE_AUTOMOC/AUTORCC/AUTOUIC
  - [X] Vincular bibliotecas Qt6 con VisionApp
- [X] Crear estructura de directorios f1_ui
- [X] Configurar compilación multiplataforma (Linux/Windows)

**Estado:** Completado. Qt6 6.10.0 detectado y configurado.

### 1.2 Implementación de Ventana Principal

- [X] Crear clase MainWindow (QMainWindow)
  - [X] Interfaz con QTabWidget para organizar 7 fases
  - [X] Dimensionamiento inteligente (80% pantalla, centrado)
  - [X] Título y configuración de ventana
- [X] Implementar barra de menú
  - [X] Menú File: Open Dataset, Export Slices, Exit
  - [X] Menú Help: About
  - [X] Shortcuts de teclado (Ctrl+O, Ctrl+E, Ctrl+Q)
- [X] Implementar barra de herramientas
  - [X] Botones de acceso rápido con iconos
  - [X] Tooltips informativos
- [X] Implementar barra de estado
  - [X] Mensajes de estado persistentes
  - [X] QProgressBar para operaciones largas

**Archivos implementados:**

- src/main.cpp (18 líneas - entry point)
- src/f1_ui/mainwindow.h (84 líneas)
- src/f1_ui/mainwindow.cpp (420+ líneas)

**Estado:** Completado. Ventana principal funcional con todos los componentes.

### 1.3 Diseño de Tabs por Fase

- [X] Tab 0: Welcome
  - [X] Información del proyecto e integrantes
  - [X] Quick start con botones de acceso rápido
  - [X] Texto descriptivo del proyecto
- [X] Tab 1: I/O (Input/Output)
  - [X] QPushButton "Load Dataset" con QFileDialog
  - [X] QPushButton "Export Slices"
  - [X] QLabel para mostrar dataset cargado
  - [X] Layout organizado con descripciones
- [X] Tab 2: Preprocessing (stub)
  - [X] Espacio para controles de ecualización
  - [X] Espacio para controles de filtros
  - [X] Área de visualización before/after
- [X] Tab 3: Segmentation (stub)
  - [X] Espacio para controles de umbralización
  - [X] Botones para segmentar pulmones, corazón, huesos
- [X] Tab 4: Morphology (stub)
  - [X] Espacio para operaciones morfológicas
- [X] Tab 5: Visualization (stub)
  - [X] Área de visualización avanzada
- [X] Tab 6: Metrics (stub)
  - [X] Panel para mostrar resultados y métricas

**Estado:** Completado. Todas las interfaces stub creadas, listas para implementación.

### 1.4 Integración con Lógica de Procesamiento

- [ ] Conectar botón "Load Dataset" con DicomReader
  - [ ] Cargar serie DICOM completa (343 slices)
  - [ ] Mostrar información del dataset en QLabel
  - [ ] Actualizar barra de estado con progreso
- [ ] Conectar botón "Export Slices" con funcionalidad ExportSlices
  - [ ] Reutilizar código de export_slices.cpp
  - [ ] Actualizar QProgressBar durante exportación
  - [ ] Mostrar diálogo de completado
- [ ] Implementar slots para todas las acciones del menú
- [ ] Crear sistema de señales para comunicación entre componentes

**Estado:** Pendiente. Requiere integración con módulos existentes (f2_io).

### 1.5 Sistema de Compilación y Ejecución

- [X] Actualizar CMakeLists.txt con target VisionApp
  - [X] Agregar main.cpp y UI_SOURCES
  - [X] Vincular con COMMON_SOURCES
  - [X] Configurar Qt6::Widgets y Qt6::Core
- [X] Actualizar run.sh con opciones VisionApp
  - [X] Opción 2: Run VisionApp
  - [X] Opción 5: Compile + Run VisionApp
  - [X] Comando de línea: ./run.sh app|gui|vision

**Ejecutable generado:**

- build/VisionApp (8.9 MB)

**Estado:** Completado. VisionApp compila y ejecuta correctamente.

---

## FASE 2: LECTURA Y EXPLORACIÓN DE IMÁGENES DICOM

### 2.1 Implementación de Lectura DICOM con ITK

- [X] Implementar módulo de lectura DICOM (f2_io/dicom_reader)
  - [X] Función readDicomImage() con itkImageFileReader
  - [X] Extracción de metadata DICOM (PatientID, StudyDate, etc.)
  - [X] Manejo de errores y validación de archivos
- [X] Implementar puente ITK-OpenCV (utils/itk_opencv_bridge)
  - [X] Conversión ITK::Image a cv::Mat
  - [X] Normalización de intensidades 16-bit a 8-bit
  - [X] Preservación de información de HU
- [X] Implementar módulo de visualización (f6_visualization)
  - [X] Visualización de imágenes con histogramas
  - [X] Comparación lado a lado de múltiples imágenes
  - [X] Ajustes de ventana/nivel para visualización médica

**Archivos implementados:**

- src/f2_io/dicom_reader.h/cpp
- src/utils/itk_opencv_bridge.h/cpp
- src/f6_visualization/visualization.h/cpp

**Estado:** Completado. Sistema completo de lectura y visualización DICOM funcional.

### 2.2 Herramientas de Exploración del Dataset

- [X] Desarrollar ExportSlices (Fase 2.1)
  - [X] Exportación batch de 343 slices DICOM a PNG
  - [X] Numeración secuencial (slice_0001.png - slice_0343.png)
  - [X] Selección de modalidad (FD/QD) por línea de comandos
  - [X] Barra de progreso y reporte de estadísticas
  - [X] Visualización de slices representativos (0%, 25%, 50%, 75%, 100%)
  
- [X] Desarrollar ExploreDataset (Fase 2.2)
  - [X] Comparación interactiva FD vs QD lado a lado
  - [X] Cálculo de métricas de calidad (PSNR, SNR, MSE)
  - [X] Estadísticas por slice (media, desviación estándar, min/max)
  - [X] Navegación entre slices con teclado
  - [X] Generación de reporte CSV comparativo
  - [X] Identificación automática de slices representativos

**Ejecutables generados:**

- build/ExportSlices (8.8 MB)
- build/ExploreDataset (8.8 MB)

**Resultados obtenidos:**

- 343 slices exportados como PNG (42 MB total)
- Reporte CSV con análisis comparativo FD vs QD
- Métricas de calidad calculadas para todo el dataset

**Estado:** Completado. Herramientas CLI completamente funcionales.

---

## FASE 3: PREPROCESAMIENTO DE IMÁGENES

### 3.1 Implementación de Módulo de Preprocesamiento

- [ ] Crear estructura de f3_preprocessing
  - [ ] preprocessing.h: Declaración de funciones
  - [ ] preprocessing.cpp: Implementación
  - [ ] Integrar en CMakeLists.txt con COMMON_SOURCES

**Estado:** Pendiente. Estructura de archivos por crear.

### 3.2 Normalización de Intensidad

- [ ] Implementar normalizeIntensity()
  - [ ] Conversión de rangos HU a escala 0-255
  - [ ] Ajuste de window/level para visualización médica
  - [ ] Preservación de información diagnóstica
- [ ] Probar con diferentes ventanas de visualización
  - [ ] Ventana pulmón: W=1500, L=-600
  - [ ] Ventana abdomen: W=400, L=50
  - [ ] Ventana hueso: W=2000, L=300

**Estado:** Pendiente.

### 3.3 Ecualización de Histograma

- [ ] Implementar ecualización global
  - [ ] Función equalizeHistogram() con cv::equalizeHist
  - [ ] Aplicable a slices individuales o dataset completo
- [ ] Implementar CLAHE
  - [ ] Función applyCLAHE() con parámetros configurables
  - [ ] Clip limit ajustable (típico: 2.0-4.0)
  - [ ] Grid size ajustable (típico: 8x8)
- [ ] Comparación visual y cuantitativa
  - [ ] Visualización before/after
  - [ ] Métricas de contraste y detalle

**Estado:** Pendiente.

### 3.4 Reducción de Ruido - Métodos Tradicionales

- [ ] Implementar filtro Gaussiano
  - [ ] Función applyGaussianFilter()
  - [ ] Parámetros: kernel size, sigma
  - [ ] Ideal para ruido Gaussiano
- [ ] Implementar filtro Mediano
  - [ ] Función applyMedianFilter()
  - [ ] Parámetro: kernel size
  - [ ] Eficaz para ruido sal y pimienta
- [ ] Implementar filtro Bilateral
  - [ ] Función applyBilateralFilter()
  - [ ] Parámetros: diameter, sigmaColor, sigmaSpace
  - [ ] Preserva bordes mientras reduce ruido
- [ ] Comparar efectividad
  - [ ] Calcular PSNR, SNR, SSIM
  - [ ] Comparar Full Dose vs Quarter Dose procesados

**Estado:** Pendiente.

### 3.5 Reducción de Ruido - Deep Learning (Opcional)

- [ ] Investigar modelos de denoising
  - [ ] DnCNN, RED-Net, FFDNet
  - [ ] Modelos específicos para CT
- [ ] Integrar modelo preentrenado
  - [ ] Adaptar formato de entrada/salida
  - [ ] Inferencia con OpenCV DNN o PyTorch C++
- [ ] Comparar con métodos tradicionales
  - [ ] Métricas cuantitativas (PSNR, SSIM)
  - [ ] Evaluación visual con radiólogo

**Estado:** Opcional. Considerar según tiempo disponible.

### 3.6 Integración con VisionApp

- [ ] Implementar controles en tab Preprocessing
  - [ ] Sliders para parámetros de filtros
  - [ ] ComboBox para selección de método
  - [ ] Botones Apply/Reset
- [ ] Visualización comparativa
  - [ ] Vista dividida Original vs Procesada
  - [ ] Histogramas lado a lado
  - [ ] Métricas en tiempo real
- [ ] Batch processing
  - [ ] Aplicar preprocesamiento a todo el dataset
  - [ ] Guardar resultados procesados
  - [ ] Progress bar durante procesamiento

**Estado:** Pendiente. Requiere completar 3.1-3.4 primero.

---

## FASE 4: SEGMENTACIÓN DE ÁREAS DE INTERÉS

### 4.1 Implementación de Módulo de Segmentación

- [ ] Crear estructura de f4_segmentation
  - [ ] segmentation.h: Declaración de funciones de segmentación
  - [ ] segmentation.cpp: Implementación de algoritmos
  - [ ] Integrar en CMakeLists.txt

**Estado:** Pendiente. Estructura de archivos por crear.

### 4.2 Segmentación de Pulmones

- [ ] Investigar rangos HU específicos
  - [ ] Consultar con radiólogo valores exactos
  - [ ] Rango típico: -1000 a -400 HU (aire y tejido pulmonar)
- [ ] Implementar segmentPulmones()
  - [ ] Umbralización por rango HU
  - [ ] Morfología: erosión → dilatación para limpiar
  - [ ] Detección de componentes conectados
  - [ ] Filtrado por área y posición (identificar 2 regiones principales)
  - [ ] Generación de máscara binaria
- [ ] Validación
  - [ ] Superposición sobre imagen original
  - [ ] Cálculo de área segmentada
  - [ ] Verificación visual con radiólogo

**Estado:** Pendiente. Requiere consulta médica para valores HU.

### 4.3 Segmentación del Corazón

- [ ] Investigar rangos HU específicos
  - [ ] Consultar con radiólogo valores exactos
  - [ ] Rango típico: 0 a 100 HU (músculo cardíaco)
- [ ] Implementar segmentCorazon()
  - [ ] Umbralización por rango HU
  - [ ] Morfología: clausura para conectar regiones
  - [ ] Filtrado por área (eliminar pequeñas regiones)
  - [ ] Filtrado por posición (región mediastínica central)
  - [ ] Generación de máscara binaria
- [ ] Validación
  - [ ] Superposición sobre imagen original
  - [ ] Verificación de posición anatómica correcta

**Estado:** Pendiente. Requiere consulta médica para valores HU.

### 4.4 Segmentación de Estructuras Óseas

- [ ] Investigar rangos HU específicos
  - [ ] Consultar con radiólogo valores exactos
  - [ ] Rango típico: > 200 HU (hueso cortical: 400-1000 HU)
- [ ] Implementar segmentHuesos()
  - [ ] Umbralización por valor HU alto
  - [ ] Detección de bordes (Canny opcional)
  - [ ] Morfología: dilatación para conectar fragmentos
  - [ ] Identificación de vértebras y costillas
  - [ ] Generación de máscara binaria
- [ ] Validación
  - [ ] Superposición sobre imagen original
  - [ ] Verificación de estructuras óseas principales

**Estado:** Pendiente. Requiere consulta médica para valores HU.

- [ ] Crear máscara binaria de estructuras óseas

---

### 4.5 Integración con VisionApp

- [ ] Implementar controles en tab Segmentation
  - [ ] Botones "Segment Lungs", "Segment Heart", "Segment Bones"
  - [ ] Sliders para ajuste de umbrales HU
  - [ ] Checkbox para mostrar/ocultar máscaras
- [ ] Visualización de resultados
  - [ ] Vista original con overlay de máscaras en colores
  - [ ] Vista de máscaras individuales
  - [ ] Estadísticas de áreas segmentadas
- [ ] Validación y ajuste
  - [ ] Controles para refinar segmentación
  - [ ] Guardar máscaras generadas

**Estado:** Pendiente. Requiere completar 4.2-4.4 primero.

---

## FASE 5: OPERACIONES MORFOLÓGICAS

### 5.1 Implementación de Módulo de Morfología

- [ ] Crear estructura de f5_morphology
  - [ ] morphology.h: Declaración de operaciones morfológicas
  - [ ] morphology.cpp: Implementación
  - [ ] Integrar en CMakeLists.txt

**Estado:** Pendiente. Estructura de archivos por crear.

### 5.2 Operaciones Morfológicas Básicas

- [ ] Implementar erosión
  - [ ] Función erode() con kernel configurable
  - [ ] Aplicable a máscaras binarias
- [ ] Implementar dilatación
  - [ ] Función dilate() con kernel configurable
  - [ ] Aplicable a máscaras binarias
- [ ] Implementar apertura (opening)
  - [ ] Erosión seguida de dilatación
  - [ ] Eliminar ruido pequeño
- [ ] Implementar clausura (closing)
  - [ ] Dilatación seguida de erosión
  - [ ] Cerrar huecos internos

**Estado:** Pendiente.

### 5.3 Operaciones Morfológicas Avanzadas

- [ ] Implementar gradiente morfológico
  - [ ] Diferencia entre dilatación y erosión
  - [ ] Resaltar bordes de regiones
- [ ] Implementar transformada de distancia
  - [ ] cv::distanceTransform
  - [ ] Útil para encontrar centros de objetos
- [ ] Implementar top-hat y black-hat
  - [ ] Top-hat: resaltar objetos brillantes sobre fondo oscuro
  - [ ] Black-hat: resaltar objetos oscuros sobre fondo brillante

**Estado:** Pendiente.

### 5.4 Detección y Refinamiento de Bordes

- [ ] Implementar detector Canny
  - [ ] Función applyCanny() con umbrales ajustables
  - [ ] Aplicar a regiones de interés
- [ ] Implementar detector Sobel
  - [ ] Función applySobel() en X e Y
  - [ ] Combinar para magnitud de gradiente
- [ ] Comparar métodos
  - [ ] Evaluación visual de calidad de bordes
  - [ ] Selección del mejor método según estructura anatómica

**Estado:** Pendiente.

### 5.5 Integración con VisionApp

- [ ] Implementar controles en tab Morphology
  - [ ] ComboBox para selección de operación
  - [ ] Sliders para parámetros (kernel size, iterations)
  - [ ] Botones Apply/Reset
- [ ] Visualización comparativa
  - [ ] Vista antes y después de operación
  - [ ] Aplicar en tiempo real (preview)
- [ ] Pipeline de operaciones
  - [ ] Permitir encadenar múltiples operaciones
  - [ ] Guardar secuencia de operaciones aplicadas

**Estado:** Pendiente. Requiere completar 5.2-5.4 primero.

---

## FASE 6: VISUALIZACIÓN Y RESALTADO

### 6.1 Implementación de Visualización Avanzada

- [ ] Mejorar módulo f6_visualization existente
  - [ ] Añadir funciones de overlay de máscaras
  - [ ] Añadir funciones de visualización multi-panel

**Estado:** Pendiente. Módulo básico ya existe, requiere extensión.

### 6.2 Creación de Máscaras de Color

- [ ] Implementar createColorOverlay()
  - [ ] Convertir imagen grayscale a BGR
  - [ ] Asignar colores a cada máscara:
    - Pulmones: Azul/Cian (0, 255, 255)
    - Corazón: Rojo (0, 0, 255)
    - Estructuras óseas: Verde (0, 255, 0)
  - [ ] Combinar con transparencia ajustable (alpha blending)
- [ ] Implementar visualización de contornos
  - [ ] cv::findContours para extraer bordes de máscaras
  - [ ] cv::drawContours para dibujar sobre imagen original

**Estado:** Pendiente.

### 6.3 Operaciones Lógicas entre Máscaras

- [ ] Implementar operaciones bit a bit
  - [ ] AND: intersección de máscaras (cv::bitwise_and)
  - [ ] OR: unión de máscaras (cv::bitwise_or)
  - [ ] XOR: diferencia simétrica (cv::bitwise_xor)
  - [ ] NOT: inversión de máscara (cv::bitwise_not)
- [ ] Aplicaciones prácticas
  - [ ] Detectar superposiciones entre estructuras
  - [ ] Aislar regiones específicas
  - [ ] Crear máscaras compuestas

**Estado:** Pendiente.

### 6.4 Mejora de Contraste para Visualización

- [ ] Implementar contrast stretching
  - [ ] Ajuste de histograma a rango completo 0-255
  - [ ] Aplicable a ROIs específicas
- [ ] Implementar ajuste window/level
  - [ ] Parámetros configurables para diferentes tejidos
  - [ ] Presets médicos estándar (pulmón, abdomen, hueso)
- [ ] Crear imagen final compuesta
  - [ ] Combinar imagen original mejorada con overlays
  - [ ] Agregar leyenda de colores
  - [ ] Incluir información del paciente y slice

**Estado:** Pendiente.

### 6.5 Integración con VisionApp

- [ ] Implementar controles en tab Visualization
  - [ ] Checkboxes para mostrar/ocultar cada máscara
  - [ ] Sliders para transparencia de overlays
  - [ ] ComboBox para presets de window/level
  - [ ] Color pickers para personalizar colores de máscaras
- [ ] Visualización multi-panel
  - [ ] Panel 1: Imagen original
  - [ ] Panel 2: Imagen con overlays
  - [ ] Panel 3: Máscaras individuales
  - [ ] Panel 4: Operaciones lógicas
- [ ] Exportación de visualizaciones
  - [ ] Guardar como PNG con anotaciones
  - [ ] Exportar serie completa con overlays
  - [ ] Generar video de navegación por slices

**Estado:** Pendiente. Requiere completar 6.2-6.4 primero.

---

## FASE 7: INVESTIGACIÓN Y TÉCNICA ADICIONAL

- [ ] Revisar literatura científica
  - [ ] Buscar papers recientes sobre segmentación CT de tórax
  - [ ] Considerar: Watershed, Active Contours, Region Growing, Graph Cuts, U-Net
  - [ ] Seleccionar técnica no vista en clase (15% del proyecto)
- [ ] Documentar técnica seleccionada
  - [ ] Fundamentos teóricos
  - [ ] Ventajas y desventajas
  - [ ] Aplicabilidad a imágenes CT
  - [ ] Citas bibliográficas en formato APA

**Estado:** Pendiente. Actividad de investigación.

### 7.2 Implementación de Técnica Nueva

- [ ] Crear módulo f7_research
  - [ ] research_technique.h/cpp
  - [ ] Integrar en CMakeLists.txt
- [ ] Implementar técnica seleccionada
  - [ ] Adaptar algoritmo a imágenes CT
  - [ ] Optimizar parámetros para dataset L291
  - [ ] Asegurar compatibilidad con pipeline existente
- [ ] Validar resultados
  - [ ] Comparar con métodos tradicionales (Fase 4)
  - [ ] Calcular métricas de segmentación
  - [ ] Documentar mejoras o limitaciones observadas

**Estado:** Pendiente. Requiere completar investigación 7.1.

### 7.3 Integración de Técnica Nueva

- [ ] Agregar opción en VisionApp
  - [ ] Botón en tab Segmentation para técnica nueva
  - [ ] Controles específicos según técnica
  - [ ] Visualización comparativa con métodos tradicionales
- [ ] Documentar en informe
  - [ ] Sección dedicada a técnica de investigación
  - [ ] Comparación cuantitativa con otros métodos
  - [ ] Conclusiones sobre aplicabilidad

**Estado:** Pendiente. Requiere completar 7.2.

---

## FASE 8: MÉTRICAS Y VALIDACIÓN

### 8.1 Implementación de Módulo de Métricas

- [ ] Crear estructura de f8_metrics
  - [ ] metrics.h: Declaración de funciones de cálculo
  - [ ] metrics.cpp: Implementación
  - [ ] Integrar en CMakeLists.txt

**Estado:** Pendiente. Estructura de archivos por crear.

### 8.2 Creación de Ground Truth

- [ ] Consulta médica profesional
  - [ ] Presentar casos del dataset L291 a radiólogo
  - [ ] Obtener segmentación manual experta
  - [ ] Documentar criterios médicos utilizados
- [ ] Digitalizar ground truth
  - [ ] Crear máscaras binarias de referencia
  - [ ] Almacenar en formato estándar (PNG)
  - [ ] Organizar en directorio data/ground_truth/

**Estado:** Pendiente. Requiere consulta médica profesional.

### 8.3 Cálculo de Precisión y Métricas

- [ ] Implementar métricas de segmentación
  - [ ] Precisión (Accuracy): (TP+TN)/(TP+TN+FP+FN) > 90% requerido
  - [ ] Sensibilidad (Recall/TPR): TP/(TP+FN)
  - [ ] Especificidad (TNR): TN/(TN+FP)
  - [ ] Dice Coefficient: 2*TP/(2*TP+FP+FN)
  - [ ] IoU (Jaccard Index): TP/(TP+FP+FN)
  - [ ] F1-Score: 2*(Precision*Recall)/(Precision+Recall)
- [ ] Aplicar métricas por estructura anatómica
  - [ ] Métricas para segmentación de pulmones
  - [ ] Métricas para segmentación de corazón
  - [ ] Métricas para segmentación de huesos
- [ ] Análisis estadístico
  - [ ] Calcular media y desviación estándar por métrica
  - [ ] Identificar casos con peor rendimiento
  - [ ] Proponer mejoras según resultados

**Estado:** Pendiente. Requiere ground truth (8.2).

### 8.4 Estadísticas de Rendimiento del Sistema

- [ ] Implementar profiling de rendimiento
  - [ ] Medir tiempo de procesamiento por fase
  - [ ] Medir uso de memoria RAM
  - [ ] Medir uso de CPU
- [ ] Calcular estadísticas por slice
  - [ ] Tiempo promedio de procesamiento
  - [ ] Memoria pico utilizada
  - [ ] Throughput (slices por segundo)
- [ ] Generar reportes
  - [ ] Tabla de tiempos por operación
  - [ ] Gráficos de rendimiento
  - [ ] Identificar cuellos de botella

**Estado:** Pendiente.

### 8.5 Integración con VisionApp

- [ ] Implementar tab Metrics
  - [ ] Tabla de métricas por estructura
  - [ ] Gráficos de barras comparativos
  - [ ] Botón "Generate Report"
- [ ] Visualización de resultados
  - [ ] Superposición de ground truth vs segmentación
  - [ ] Mapa de errores (FP en rojo, FN en azul)
  - [ ] Estadísticas en tiempo real
- [ ] Exportación de resultados
  - [ ] Guardar reporte CSV con todas las métricas
  - [ ] Exportar gráficos como PNG
  - [ ] Generar resumen ejecutivo

**Estado:** Pendiente. Requiere completar 8.3-8.4.

---

## FASE 9: INFORME Y DOCUMENTACIÓN

### 9.1 Diseño de la Propuesta

- [ ] Crear diagrama de flujo del pipeline completo
  - [ ] Diagrama de arquitectura del sistema
  - [ ] Flujo de datos desde carga DICOM hasta visualización
  - [ ] Integración de todos los módulos (f1-f8)
- [ ] Diseñar diagramas explicativos
  - [ ] Esquema del proceso de segmentación
  - [ ] Comparación visual de técnicas aplicadas
  - [ ] Resultados antes/después por fase
- [ ] Capturar pantallas de VisionApp
  - [ ] Screenshots de cada tab funcional
  - [ ] Visualizaciones de resultados intermedios
  - [ ] Interfaz completa en operación

**Estado:** Pendiente. Requiere completar implementación de fases.

### 9.2 Redacción del Informe Técnico

- [ ] Estructura del documento
  - [ ] Portada con información del proyecto
  - [ ] Resumen ejecutivo (Abstract)
  - [ ] Tabla de contenidos
  
- [ ] Sección 1: Introducción
  - [ ] Contexto y motivación (imágenes CT low-dose)
  - [ ] Objetivos del proyecto
  - [ ] Alcance y limitaciones
  
- [ ] Sección 2: Marco Teórico
  - [ ] Imágenes DICOM y unidades Hounsfield
  - [ ] Técnicas de preprocesamiento (ecualización, filtrado)
  - [ ] Métodos de segmentación
  - [ ] Operaciones morfológicas
  - [ ] Técnica de investigación seleccionada
  
- [ ] Sección 3: Metodología
  - [ ] Descripción del dataset L291 (343 slices FD/QD)
  - [ ] Pipeline de procesamiento implementado
  - [ ] Herramientas y tecnologías (C++, OpenCV, ITK, Qt6)
  - [ ] Criterios de segmentación médica
  
- [ ] Sección 4: Implementación
  - [ ] Arquitectura de software (módulos f1-f8)
  - [ ] Descripción de VisionApp
  - [ ] Algoritmos implementados con pseudocódigo
  - [ ] Optimizaciones realizadas
  
- [ ] Sección 5: Resultados
  - [ ] Métricas de precisión por estructura (>90%)
  - [ ] Comparación de técnicas de preprocesamiento
  - [ ] Evaluación de técnica de investigación
  - [ ] Análisis estadístico de rendimiento
  - [ ] Gráficos y tablas comparativas
  
- [ ] Sección 6: Discusión
  - [ ] Interpretación de resultados
  - [ ] Comparación con trabajos relacionados
  - [ ] Limitaciones encontradas
  - [ ] Trabajo futuro
  
- [ ] Sección 7: Conclusiones
  - [ ] Cumplimiento de objetivos
  - [ ] Aportes del proyecto
  - [ ] Lecciones aprendidas
  
- [ ] Referencias bibliográficas
  - [ ] Formato APA
  - [ ] Incluir papers de técnica de investigación
  - [ ] Documentación técnica de OpenCV, ITK, Qt

**Estado:** Pendiente. Documento en preparación paralela a implementación.

### 9.3 Preparación de Material Visual

- [ ] Figuras y diagramas
  - [ ] Diagrama de arquitectura del sistema
  - [ ] Flujo de datos del pipeline
  - [ ] Ejemplos de preprocesamiento (before/after)
  - [ ] Resultados de segmentación por estructura
  - [ ] Comparación de técnicas
  
- [ ] Tablas de datos
  - [ ] Tabla de métricas de precisión
  - [ ] Comparativa de métodos de filtrado (PSNR/SSIM)
  - [ ] Estadísticas de rendimiento
  - [ ] Parámetros óptimos encontrados
  
- [ ] Gráficos estadísticos
  - [ ] Histogramas de imágenes FD vs QD
  - [ ] Gráficos de barras de métricas
  - [ ] Gráficos de línea de PSNR/SSIM
  - [ ] Box plots de distribuciones de valores HU

**Estado:** Pendiente. Creación paralela a obtención de resultados.

---

## FASE 10: VIDEO-BLOG EN INGLÉS

### 10.1 Preparación del Guion

- [ ] Escribir guion estructurado (5-10 minutos)
  
  **Sección 1: Introduction (1 min)**
  - [ ] Presentación personal y del proyecto
  - [ ] Problema: Low-dose CT reconstruction challenges
  - [ ] Objetivo: Automated organ segmentation
  
  **Sección 2: Dataset & Tools (1 min)**
  - [ ] Dataset L291: 343 slices, Full Dose vs Quarter Dose
  - [ ] Tools: C++, OpenCV 4.10, ITK 6.0, Qt6
  - [ ] Desktop application: VisionApp
  
  **Sección 3: Methodology (2-3 min)**
  - [ ] DICOM reading with ITK
  - [ ] Preprocessing: histogram equalization, noise reduction
  - [ ] Segmentation: lungs, heart, bones
  - [ ] Morphological operations
  - [ ] Research technique (explicar técnica seleccionada)
  
  **Sección 4: Results & Demo (2-3 min)**
  - [ ] Live demonstration of VisionApp
  - [ ] Visual results of segmentation
  - [ ] Quantitative metrics (>90% accuracy)
  - [ ] Performance statistics
  
  **Sección 5: Conclusions (1 min)**
  - [ ] Achievements and challenges
  - [ ] Future work
  - [ ] Thank you message
  
- [ ] Revisar gramática y pronunciación
  - [ ] Corrección con herramientas (Grammarly, ChatGPT)
  - [ ] Práctica de pronunciación de términos técnicos
  - [ ] Preparar tarjetas de apoyo visual

**Estado:** Pendiente. Requiere resultados finales del proyecto.

### 10.2 Grabación y Edición del Video

- [ ] Preparación técnica
  - [ ] Configurar OBS Studio o software de grabación
  - [ ] Iluminación y audio de calidad
  - [ ] Fondo neutro y profesional
  - [ ] VisionApp preparada para demo en vivo
  
- [ ] Grabación del contenido
  - [ ] Grabación de presentación personal
  - [ ] Screen recording de VisionApp funcionando
  - [ ] Grabación de explicaciones técnicas sobre resultados
  - [ ] Captura de visualizaciones clave
  
- [ ] Edición del video
  - [ ] Cortar y ensamblar secciones
  - [ ] Añadir transiciones profesionales
  - [ ] Insertar títulos y subtítulos en inglés
  - [ ] Overlay de gráficos y resultados
  - [ ] Música de fondo suave (libre de derechos)
  
- [ ] Revisión y publicación
  - [ ] Control de calidad (audio, video, timing)
  - [ ] Exportar en formato HD (1080p)
  - [ ] Subir a plataforma requerida
  - [ ] Verificar accesibilidad del enlace

**Estado:** Pendiente. Última fase del proyecto.

---

## CONSULTA MÉDICA Y VALIDACIÓN

### Consulta con Radiólogo

- [ ] Agendar cita con profesional médico especializado
  - [ ] Presentar dataset L291 y objetivos del proyecto
  - [ ] Consultar rangos HU específicos por estructura
    - Pulmones (típico: -1000 a -400 HU)
    - Corazón (típico: 0 a 100 HU)
    - Huesos (típico: >200 HU, cortical 400-1000 HU)
  - [ ] Obtener ground truth manualmente segmentado
  - [ ] Documentar criterios médicos utilizados
  
- [ ] Validación de resultados
  - [ ] Presentar segmentaciones automáticas al radiólogo
  - [ ] Recibir feedback sobre precisión clínica
  - [ ] Ajustar parámetros según recomendaciones médicas
  - [ ] Documentar validación profesional en informe

**Estado:** Crítico. Requerido para alcanzar 90% de precisión y validez médica.

---

## PRIORIDADES Y CRONOGRAMA

### Prioridad Alta (Crítico para funcionalidad básica)

1. **Fase 1 (UI): Integración con lógica existente**
   - Conectar botón "Load Dataset" con DicomReader
   - Integrar ExportSlices en interfaz
   - Sistema de progress bar funcional

2. **Consulta Médica**
   - Obtener rangos HU validados
   - Crear ground truth para validación

3. **Fase 3: Preprocesamiento**
   - Implementar ecualización y CLAHE
   - Implementar filtros básicos (Gaussiano, Mediano, Bilateral)

4. **Fase 4: Segmentación**
   - Implementar segmentación de las 3 estructuras
   - Validar con ground truth (>90% precisión)

### Prioridad Media (Necesario para proyecto completo)

1. **Fase 5: Morfología**
   - Operaciones morfológicas para refinar segmentaciones
   - Detección de bordes

2. **Fase 7: Investigación**
   - Seleccionar e implementar técnica adicional
   - Documentar con citas bibliográficas

3. **Fase 8: Métricas**
   - Implementar cálculo de métricas de precisión
   - Generar reportes estadísticos

4. **Fase 6: Visualización avanzada**
   - Overlays de color para máscaras
   - Visualización multi-panel

### Prioridad Baja (Mejoras y documentación)

1. **Fase 9: Informe**
   - Redacción del documento técnico
   - Generación de figuras y tablas

2. **Fase 10: Video-blog**
    - Grabación y edición en inglés
    - Demostración de VisionApp

### Cronograma Sugerido

Semana 1-2: Completar base técnica

- Integrar UI con procesamiento existente
- Consulta médica y ground truth
- Implementar Fase 3 (Preprocesamiento)

Semana 3: Segmentación y validación

- Implementar Fase 4 (Segmentación)
- Fase 5 (Morfología) para refinamiento
- Validar precisión >90%

Semana 4: Investigación y métricas

- Fase 7 (Técnica de investigación)
- Fase 8 (Métricas y validación)
- Fase 6 (Visualización avanzada)

Semana 5: Documentación

- Redacción de informe técnico
- Preparación de figuras y resultados
- Pruebas finales del sistema

Semana 6: Entrega

- Grabación de video-blog en inglés
- Revisión final de documentación
- Entrega del proyecto completo

---

## NOTAS TÉCNICAS

### Dependencias del Proyecto

- **Sistema Operativo:** Linux (Ubuntu/Debian) o Windows
- **Compilador:** GCC 7+ / MSVC 2019+ (soporte C++17)
- **CMake:** 3.16 o superior
- **Qt6:** 6.10.0 (Core, Widgets, Gui)
- **OpenCV:** 4.10.0 con módulos core, imgproc, imgcodecs, highgui
- **ITK:** 6.0.0 (InsightToolkit)
- **Git:** Para control de versiones

### Estructura de Archivos Implementados

```bash
code/
├── src/
│   ├── main.cpp                    [COMPLETADO] Entry point VisionApp
│   ├── export_slices.cpp           [COMPLETADO] CLI tool Fase 2.1
│   ├── explore_dataset.cpp         [COMPLETADO] CLI tool Fase 2.2
│   ├── f1_ui/
│   │   ├── mainwindow.h            [COMPLETADO] 
│   │   └── mainwindow.cpp          [COMPLETADO] 420+ líneas
│   ├── f2_io/
│   │   ├── dicom_reader.h          [COMPLETADO]
│   │   └── dicom_reader.cpp        [COMPLETADO]
│   ├── f3_preprocessing/           [PENDIENTE] Por crear
│   ├── f4_segmentation/            [PENDIENTE] Por crear
│   ├── f5_morphology/              [PENDIENTE] Por crear
│   ├── f6_visualization/
│   │   ├── visualization.h         [COMPLETADO] Funciones básicas
│   │   └── visualization.cpp       [COMPLETADO] Histogramas, etc.
│   ├── f7_research/                [PENDIENTE] Por crear
│   ├── f8_metrics/                 [PENDIENTE] Por crear
│   └── utils/
│       ├── itk_opencv_bridge.h     [COMPLETADO]
│       └── itk_opencv_bridge.cpp   [COMPLETADO]
├── CMakeLists.txt                  [ACTUALIZADO] Con Qt6
├── run.sh                          [ACTUALIZADO] 9 opciones
└── build/
    ├── VisionApp                   [COMPILADO] 8.9 MB
    ├── ExportSlices                [COMPILADO] 8.8 MB
    └── ExploreDataset              [COMPILADO] 8.8 MB
```

### Comandos Útiles

```bash
# Compilar todo el proyecto
./run.sh compile

# Ejecutar VisionApp (interfaz Qt6)
./run.sh app

# Ejecutar herramientas CLI
./run.sh export
./run.sh explore

# Limpiar build
./run.sh clean
```

---

## CRITERIOS DE ÉXITO

### Mínimo Viable (Aprobar el proyecto)

- [X] Dataset L291 cargado y explorado
- [X] Herramientas CLI funcionales (ExportSlices, ExploreDataset)
- [X] Aplicación de escritorio VisionApp base funcional
- [ ] Preprocesamiento implementado y funcional
- [ ] Segmentación de 3 estructuras con >90% precisión
- [ ] Técnica de investigación implementada y documentada
- [ ] Informe técnico completo con resultados
- [ ] Video-blog en inglés explicando el proyecto

### Excelencia (Máxima calificación)

- Todo lo anterior, más:
- [ ] Interfaz Qt6 completamente integrada y pulida
- [ ] Múltiples técnicas de preprocesamiento comparadas
- [ ] Ground truth validado por profesional médico
- [ ] Métricas exhaustivas (6+ métricas calculadas)
- [ ] Técnica de investigación con resultados superiores
- [ ] Optimizaciones de rendimiento documentadas
- [ ] Código limpio, modular y bien comentado
- [ ] Documentación técnica excepcional
- [ ] Video-blog profesional y claro

---

## RECURSOS Y REFERENCIAS

### Documentación Técnica

- OpenCV Documentation: <https://docs.opencv.org/4.10.0/>
- ITK Software Guide: <https://itk.org/ItkSoftwareGuide.pdf>
- Qt6 Documentation: <https://doc.qt.io/qt-6/>
- DICOM Standard: <https://www.dicomstandard.org/>

### Literatura Científica (Por completar en Fase 7)

- Papers sobre segmentación de CT de tórax
- Técnicas de reducción de ruido en imágenes médicas
- Métodos de validación de segmentación automática
- Técnica de investigación seleccionada

### Datasets y Ground Truth

- Dataset L291: 343 slices Full Dose + Quarter Dose
- Ground truth: Por obtener con validación médica
- Metadata: Disponible en data/dataset/metadata.csv

---
