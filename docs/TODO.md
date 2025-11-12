# TODO - Proyecto Integrador Parte I (Interciclo)

## Análisis de Imágenes CT con ITK + OpenCV C++

---

## 📋 Información General del Proyecto

**Objetivo:** Desarrollar una aplicación en C++ usando OpenCV e ITK para procesar imágenes CT (DICOM), extraer slices, y resaltar áreas de interés (pulmones, corazón, estructuras óseas) para análisis médico.

**Dataset:** CT Low Dose Reconstruction (Kaggle)

- URL: <https://www.kaggle.com/datasets/andrewmvd/ct-low-dose-reconstruction/data>
- Paciente seleccionado: L096 (o el que elijas)
- Formato: DICOM (.IMA)

**Fecha de Entrega:** Semana de revisión de exámenes interciclo

**Ponderación:** 15 puntos proyecto + 5 puntos informe = 20 puntos total

---

## FASE 1: PREPARACIÓN Y CONFIGURACIÓN (Semana 1)

### 1.1 Configuración del Entorno

- [X] Verificar que ITK esté correctamente instalado
- [X] Verificar que OpenCV C++ esté correctamente instalado
- [X] Probar compilación del proyecto base en `codigo/`
- [X] Crear carpeta de trabajo dentro de `codigo/` para organizar el proyecto

### 1.2 Obtención del Dataset

- [X] Descargar dataset "CT Low Dose Reconstruction" desde Kaggle
- [X] Verificar que el dataset esté en `intercilo01/archive/`
- [X] Seleccionar un paciente único (diferente al de otros grupos)
- [X] Explorar la estructura de carpetas del dataset:
  - Full Dose / Quarter Dose
  - 1mm / 3mm Slice Thickness
  - Sharp Kernel (D45) / Soft Kernel (B30)

### 1.3 Investigación Médica

- [ ] Consultar con un radiólogo o fuentes médicas sobre:
  - Características de las zonas de interés (pulmones, corazón, estructuras óseas)
  - Valores de HU (Unidades Hounsfield) para cada tejido
  - Criterios para la segmentación correcta
- [ ] Documentar los criterios médicos en un archivo separado
- [ ] Definir las 3 áreas principales a extraer:
  1. Pulmones
  2. Corazón
  3. Estructuras óseas

---

## FASE 2: LECTURA Y EXPLORACIÓN DE IMÁGENES DICOM (Semana 1-2)

### 2.1 Implementación de Lectura DICOM con ITK

- [X] Crear función para leer archivos DICOM (.IMA) usando ITK
- [X] Implementar `itkImageFileReader` para cargar imágenes
- [X] Convertir imágenes ITK a formato OpenCV usando `itkOpenCVImageBridge`
- [X] Mostrar información básica de la imagen:
  - Dimensiones
  - Rango de valores (min/max HU)
  - Metadata DICOM
- [X] **REFACTORIZACIÓN:** Código organizado en módulos:
  - `f2_io/dicom_reader` - Lectura y metadata DICOM
  - `utils/itk_opencv_bridge` - Conversión ITK ↔ OpenCV
  - `f6_visualization/visualization` - Visualización e histogramas

### 2.2 Exploración del Dataset

- [X] Cargar y visualizar diferentes slices del paciente seleccionado
- [X] Analizar las diferencias entre Full Dose y Quarter Dose
- [X] Identificar slices representativos para el análisis (ej: slice 100)
- [X] Guardar estadísticas básicas (media, desviación estándar, histograma)
- [X] **HERRAMIENTA:** Creado `ExploreDataset` - programa interactivo para:
  - Comparación visual lado a lado FD vs QD
  - Cálculo de PSNR, SNR y estadísticas completas
  - Identificación automática de slices representativos
  - Generación de reporte CSV detallado
  - Navegación interactiva entre slices

---

## FASE 3: PREPROCESAMIENTO DE IMÁGENES (Semana 2)

### 3.1 Ecualización de Histograma

- [ ] Implementar ecualización de histograma clásica (`cv::equalizeHist`)
- [ ] Implementar CLAHE (Contrast Limited Adaptive Histogram Equalization)
- [ ] Comparar resultados visualmente
- [ ] Documentar cuál técnica funciona mejor para CT

### 3.2 Reducción de Ruido - Métodos Tradicionales

- [ ] Implementar filtro de la media
- [ ] Implementar filtro de la mediana
- [ ] Implementar filtro Gaussiano
- [ ] Implementar filtro bilateral
- [ ] Comparar efectividad de cada filtro (PSNR, SSIM)

### 3.3 Reducción de Ruido - Deep Learning

- [ ] Investigar e integrar modelo DnCNN preentrenado
- [ ] Aplicar denoising con la red neuronal
- [ ] Comparar resultados con métodos tradicionales
- [ ] Calcular métricas de calidad (PSNR, SSIM)

---

## FASE 4: SEGMENTACIÓN DE ÁREAS DE INTERÉS (Semana 3)

### 4.1 Segmentación de Pulmones

- [ ] Aplicar umbralización para aislar regiones pulmonares
  - Rango HU típico: -1000 a -400 HU
- [ ] Aplicar operaciones morfológicas:
  - Erosión para eliminar ruido
  - Dilatación para recuperar forma
  - Apertura/Cierre para suavizar bordes
- [ ] Identificar las dos regiones pulmonares principales
- [ ] Crear máscara binaria de pulmones

### 4.2 Segmentación del Corazón

- [ ] Aplicar umbralización para tejido cardíaco
  - Rango HU típico: 0 a 100 HU
- [ ] Usar operaciones morfológicas para refinar
- [ ] Aplicar filtros de área para eliminar regiones pequeñas
- [ ] Crear máscara binaria del corazón

### 4.3 Segmentación de Estructuras Óseas

- [ ] Aplicar umbralización para huesos
  - Rango HU típico: > 200 HU
- [ ] Usar detección de bordes (Canny, Sobel) si es necesario
- [ ] Aplicar operaciones morfológicas para conectar fragmentos
- [ ] Crear máscara binaria de estructuras óseas

---

## FASE 5: DETECCIÓN DE BORDES Y REFINAMIENTO (Semana 3)

### 5.1 Detección de Bordes

- [ ] Implementar detector Canny con ajuste de umbrales
- [ ] Implementar detector Sobel
- [ ] Aplicar detección de bordes a cada región de interés
- [ ] Comparar resultados y seleccionar el mejor método

### 5.2 Operaciones Morfológicas Avanzadas

- [ ] Implementar closing para cerrar huecos
- [ ] Implementar opening para eliminar ruido
- [ ] Implementar gradient morfológico para resaltar bordes
- [ ] Aplicar transformada de distancia si es necesario

---

## FASE 6: RESALTADO Y VISUALIZACIÓN DE ÁREAS (Semana 4)

### 6.1 Creación de Máscaras de Color

- [ ] Convertir imagen original a color (BGR)
- [ ] Asignar colores distintivos a cada área:
  - Pulmones: Azul/Cian
  - Corazón: Rojo/Magenta
  - Estructuras óseas: Verde/Amarillo
- [ ] Crear imagen con superposición de máscaras (overlay)

### 6.2 Operaciones sobre Puntos (AND, OR, XOR, NOT)

- [ ] Implementar operaciones lógicas entre máscaras
- [ ] Usar AND para intersecciones
- [ ] Usar OR para uniones
- [ ] Usar XOR para diferencias
- [ ] Usar NOT para inversión

### 6.3 Mejora de Contraste y Visualización

- [ ] Aplicar contrast stretching a regiones de interés
- [ ] Ajustar ventana/nivel (window/level) para visualización óptima
- [ ] Crear imagen final con todas las áreas resaltadas

---

## FASE 7: INTERFAZ DE USUARIO Y CONTROLES (Semana 4)

### 7.1 Implementación de Interfaz

- [ ] Crear ventanas con `cv::imshow` para visualización
- [ ] Implementar trackbars para ajustar parámetros:
  - Umbrales de segmentación
  - Parámetros de filtros
  - Niveles de visualización
- [ ] Agregar controles de teclado para navegación
- [ ] Permitir selección de diferentes slices

### 7.2 Guardar Resultados

- [ ] Crear carpeta de salida para imágenes procesadas
- [ ] Guardar imagen original
- [ ] Guardar máscaras individuales de cada área
- [ ] Guardar imagen final con áreas resaltadas
- [ ] Guardar imágenes intermedias del proceso

---

## FASE 8: TÉCNICA NUEVA (INVESTIGACIÓN) (Semana 4-5)

### 8.1 Investigar Técnica Adicional

- [ ] Buscar papers sobre procesamiento de imágenes CT
- [ ] Seleccionar una técnica no vista en clase:
  - Watershed segmentation
  - Active contours (snakes)
  - Region growing
  - Graph cuts
  - Otro método avanzado
- [ ] Documentar la técnica con citas

### 8.2 Implementar y Validar

- [ ] Implementar la técnica seleccionada
- [ ] Aplicarla a las imágenes CT
- [ ] Comparar resultados con métodos tradicionales
- [ ] Documentar mejoras o diferencias

---

## FASE 9: MÉTRICAS Y VALIDACIÓN (Semana 5)

### 9.1 Cálculo de Precisión

- [ ] Crear ground truth (manualmente o con ayuda médica)
- [ ] Calcular precisión de segmentación (>90% requerido)
- [ ] Calcular métricas adicionales:
  - Sensibilidad
  - Especificidad
  - Dice Coefficient
  - IoU (Intersection over Union)

### 9.2 Estadísticas del Sistema

- [ ] Medir uso de memoria RAM durante procesamiento
- [ ] Calcular tiempo de procesamiento por imagen
- [ ] Generar gráficos comparativos de rendimiento
- [ ] Documentar estadísticas de las zonas de interés

---

## FASE 10: INFORME Y DOCUMENTACIÓN (Semana 5-6)

### 10.1 Diseño de la Propuesta

- [ ] Crear diagrama de flujo del proceso completo
- [ ] Diseñar esquema explicativo similar a Ilustración 1
- [ ] Documentar cada etapa del pipeline
- [ ] Incluir capturas de pantalla de resultados

### 10.2 Redacción del Informe

- [ ] Escribir introducción
- [ ] Describir el problema a resolver
- [ ] Explicar la propuesta de solución
- [ ] Incluir resultados de pruebas con gráficas
- [ ] Agregar análisis de métricas
- [ ] Escribir conclusiones
- [ ] Compilar bibliografía con citas correctas

### 10.3 Preparación de Figuras y Tablas

- [ ] Crear tabla comparativa de métodos de filtrado
- [ ] Generar gráficos de PSNR/SSIM
- [ ] Incluir histogramas de las imágenes
- [ ] Mostrar resultados de segmentación paso a paso

---

## FASE 11: VIDEO-BLOG EN INGLÉS (Semana 6)

### 11.1 Preparación del Guion

- [ ] Escribir guion en inglés explicando:
  - Introducción al problema
  - Dataset utilizado
  - Técnicas aplicadas
  - Resultados obtenidos
  - Conclusiones
- [ ] Revisar gramática y pronunciación
- [ ] Practicar lectura del guion

### 11.2 Grabación y Edición

- [ ] Grabar video mostrando:
  - Código funcionando en vivo
  - Comparación de resultados
  - Explicación de técnicas
- [ ] Editar video (máximo 5-7 minutos)
- [ ] Agregar subtítulos si es necesario
- [ ] Subir a plataforma (YouTube, Vimeo, etc.)

---

## FASE 12: PREPARACIÓN DE LA PRESENTACIÓN EN VIVO (Semana 6)

### 12.1 Pruebas Finales

- [ ] Compilar proyecto en computadora de presentación
- [ ] Verificar que todas las bibliotecas estén instaladas
- [ ] Probar con diferentes imágenes del dataset
- [ ] Preparar dataset de demostración

### 12.2 Presentación

- [ ] Preparar slides o demostración en vivo
- [ ] Practicar explicación del código
- [ ] Preparar respuestas a preguntas frecuentes
- [ ] Llevar respaldo del proyecto (USB, GitHub)

---

## 📊 CHECKLIST DE ENTREGABLES

### Código

- [ ] Aplicación en C++ con OpenCV + ITK funcionando
- [ ] Código comentado y bien estructurado
- [ ] README con instrucciones de compilación
- [ ] Script de compilación (`compile_and_run.sh`)

### Resultados

- [ ] Carpeta con imágenes procesadas
- [ ] Máscaras de las 3 áreas de interés
- [ ] Imágenes con áreas resaltadas a color
- [ ] CSV con estadísticas y métricas

### Documentación

- [ ] Informe completo en formato Web 2.0
- [ ] Diagrama explicativo del proceso
- [ ] Bibliografía con citas correctas
- [ ] Video-blog en inglés (5-7 min)

### Presentación

- [ ] Proyecto funcionando en vivo
- [ ] Demostración con diferentes slices
- [ ] Explicación de técnicas aplicadas
- [ ] Respuestas a preguntas del docente

---

## 📚 RECURSOS Y REFERENCIAS

### Documentación Técnica

- ITK Documentation: <https://itk.org/>
- ITK Examples: <https://examples.itk.org/>
- OpenCV C++ Tutorials: <https://docs.opencv.org/>
- Dataset: <https://www.kaggle.com/datasets/andrewmvd/ct-low-dose-reconstruction/data>

### Papers Recomendados

- [ ] Buscar papers sobre segmentación de CT
- [ ] Investigar técnicas de denoising en imágenes médicas
- [ ] Leer sobre valores HU en diferentes tejidos
- [ ] Consultar guías de visualización médica

### Librerías Adicionales

- Albumentations (para data augmentation): <https://albumentations.ai/>
- DnCNN (para denoising con deep learning)
- SimpleITK (alternativa más simple a ITK)

---

## ⚠️ NOTAS IMPORTANTES

1. **Ningún grupo puede tener el mismo dataset/paciente**
2. **Precisión mínima requerida: 90%**
3. **Implementar en C++ con OpenCV + ITK para obtener el 35% de ponderación**
   - Implementación en Python solo vale 10%
4. **Citar correctamente todo código de terceros**
5. **Validar criterios médicos con un radiólogo**
6. **Probar el proyecto en la computadora de presentación antes de la entrega**

---

## 🎯 CRITERIOS DE EVALUACIÓN (Rúbrica)

### Proyecto (15 puntos = 100%)

- **25%** - Aplicación de técnicas vistas en clase con criterios médicos
- **10%** - Uso efectivo de red neuronal para denoising
- **15%** - Análisis médico sustentado y precisión >90%
- **15%** - Técnica nueva investigada e implementada
- **35%** - Implementación en C++ con OpenCV + ITK

### Informe (5 puntos = 100%)

- **35%** - Descripción detallada del trabajo
- **25%** - Resultados de pruebas con gráficas
- **20%** - Redacción, citas y esquema explicativo
- **20%** - Video-blog en inglés con guion claro

---

## 📅 CRONOGRAMA SUGERIDO

| Semana | Tareas Principales |
|--------|-------------------|
| 1 | Configuración, obtención dataset, investigación médica, lectura DICOM |
| 2 | Preprocesamiento, ecualización, reducción ruido |
| 3 | Segmentación de áreas, detección de bordes |
| 4 | Resaltado visual, interfaz, técnica nueva |
| 5 | Métricas, validación, inicio informe |
| 6 | Completar informe, video-blog, preparar presentación |

---

¡Éxito en tu proyecto! 🚀
