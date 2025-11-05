```
Computación Docente: Vladimir Robles Bykbaev
VISIÓN ARTIFICIAL Período Lectivo: Febrero 2026 Octubre 2025 –
```

### FORMATO DE GUÍA DE PROYECTO INTEGRADOR

### CARRERA : COMPUTACIÓN ASIGNATURA : VISIÓN ARTIFICIAL

### NRO. PRÁCTICA : 2-

**TÍTULO DEL PROYECTO** : Proyecto Integrador Final – Desarrollo de Sistema
Inteligente de Análisis Visual para Seguridad y Diagnóstico Médico.
**OBJETIVO:**
Reforzar los conocimientos adquiridos en clase sobre el trabajo con las etapas de preprocesamiento de imágenes y
extracción de características locales y globales y el empleo de técnicas del dominio de la frecuencia y de
aprendizaje de máquina a fin de realizar tareas de reconocimiento de patrones.

### INSTRUCCIONES

**1.** Revisar el contenido teórico del tema, las presentaciones, tutoriales y vídeo-
    lecturas disponibles en el ambiente virtual de aprendizaje.
**2.** Profundizar los conocimientos revisando los libros guías, los enlaces
    contenidos en los objetos de aprendizaje y la documentación disponible en
    fuentes académicas en línea
**3.
4.** Deberá generar un informe empleando una herramienta Web 2.0 y un
    vídeo-blog en inglés explicando los principales aspectos de la propuesta
    planteada.
**ASIGNATURAS Y CONOCIMIENTOS RELACIONADOS AL PROYECTO INTEGRADOR**

```
Asignatura Contenido de la materia
```

```
Contenido al que aporta de la
asignatura de Visión por Computador
o requerimentos del Proyecto
```

```
Porcentaje de
aporte al
proyecto
```

Álgebra Lineal
Matrices (operaciones con
matrices)

```
Operaciones sobre grupos de puntos de
la imagen (convolución, filtro de la
media, filtro de la mediana, filtro
Gaussiano). Reducción de ruido.
Operaciones morfológicas. HOG.
```

### 25%

Cálculo Diferencial La derivada

```
Detección de Bordes (Operadores
Básicos, Prewitt, Sobel, Laplaciano,
Canny)
```

### 20%

Estadística

```
Introducción a la
estadística y conjuntos
```

```
Probabilidad Variables
aleatorias y
distribuciones de
probabilidad
```

```
Ecualización de Histograma 25%
```

Programación y
plataformas Web
Desarrollo de aplicaciones Web 15%

Inteligencia
Artificial
Redes Neuronales
Aplicación de visión por computador
con aprendizaje profundo.

### 15%

Visión por
Computador

```
Espacios de Color
Operaciones sobre los puntos de
la imagen
Operaciones Morfológicas
Detección de Bordes
Histogram Oriented Gradients
(HOG)
Cascadas de Haar
```

### N/A N/A

```
Resolución CS N° 076-04-2016-04-
```

### PROBLEMA QUE BUSCA SOLUCIONAR EL PROYECTO INTEGRADOR

En el contexto actual de la inteligencia artificial aplicada a la salud y la seguridad, existe una creciente necesidad
de sistemas que integren técnicas clásicas de visión por computador con métodos avanzados basados en
aprendizaje profundo. Este proyecto integrador busca abordar dos problemas reales y de alta relevancia: el
mejoramiento del diagnóstico médico mediante el procesamiento inteligente de imágenes médicas y la
implementación de sistemas de vigilancia que no solo detecten presencia humana, sino que también comprendan
su comportamiento a través de la postura corporal. Al combinar herramientas como OpenCV (C++ y Python),
PyTorch, Flask y Telegram Bot API, los estudiantes consolidarán sus conocimientos teóricos en un entorno
práctico, demostrando la capacidad de construir soluciones end-to-end que van desde el preprocesamiento básico
hasta la inferencia con redes neuronales. Este enfoque no solo fortalece la comprensión de los fundamentos de la
visión por computador, sino que también prepara a los futuros profesionales para enfrentar desafíos tecnológicos
contemporáneos en sectores críticos como la medicina y la seguridad pública.

**PARTE I. INTERCICLO**
Al realizar un proyecto integrador que involucre el uso de la librería ITK junto con OpenCV C++ para procesar
imágenes médicas en formato NIfTI, y su posterior envío a un servidor Flask que aplica denoising mediante una red
neuronal preentrenada (DnCNN), los estudiantes de la materia de Visión por Computador pueden consolidar y
aplicar de manera práctica los conocimientos adquiridos en las Unidades 1 y 2. En primer lugar, al extraer slices de
imágenes volumétricas, los estudiantes profundizan en conceptos fundamentales de digitalización de imágenes y
espacios de color (Unidad 1), al traducir datos médicos complejos a formatos visualizables y manipulables. Al
aplicar técnicas de preprocesamiento como ecualización de histograma, binarización por umbral y detección de
bordes (Unidad 2), los alumnos mejoran el contraste y facilitan la identificación de regiones relevantes, como
posibles tumores o lesiones. Además, al implementar filtros de reducción de ruido (media, mediana o Gaussiano) y
operaciones morfológicas (erosión y dilatación), los estudiantes aprenden a eliminar artefactos y refinar las
máscaras para obtener resultados más precisos. La comparación cuantitativa entre métodos clásicos de denoising
y la red DnCNN permite a los estudiantes apreciar el impacto del aprendizaje profundo en la calidad de las
imágenes médicas, fomentando una comprensión crítica sobre la evolución de las técnicas de visión por
computador. Este proyecto no solo integra herramientas avanzadas de procesamiento de imágenes, sino que
también introduce a los estudiantes en el paradigma moderno de la visión artificial basada en tensores y redes
neuronales, permitiéndoles apreciar el impacto de estas tecnologías en campos críticos como el diagnóstico
médico asistido.

### PARTE II. FINAL

En el presente Proyecto Integrador (Final), los estudiantes de la Carrera de Computación deberán desarrollar un
sistema de visión por computador capaz de detectar la presencia de personas en tiempo real mediante técnicas
clásicas como HOG + SVM o LBP + Cascades, y enviar automáticamente la imagen a un Bot de Telegram para su
análisis posterior mediante aprendizaje profundo. Este proceso se realizará mediante la técnica de detección de
postura humana utilizando modelos preentrenados como MMPose o OpenPose en PyTorch, lo que permite no solo
identificar la ubicación de las personas, sino también analizar su comportamiento físico (postura erguida, caída,
agacharse, etc.). El Bot responderá al usuario enviando tres archivos: la imagen original, la imagen con las
posturas humanas superpuestas y un video corto que muestra la secuencia de detección en tiempo real. Este
proyecto se enmarca dentro de un contexto práctico y profesional donde los estudiantes deben aplicar técnicas de
visión por computador para la seguridad y monitoreo inteligente, integrando conceptos de extracción de
características locales y globales (Unidad 3) y reconocimiento de patrones mediante descriptores avanzados y
aprendizaje profundo (Unidad 4). Al evitar el uso de la Transformada de Hough y enfocarse en la detección de
postura, el proyecto promueve una aplicación más sofisticada y útil en entornos de vigilancia, atención médica en
hogares o seguridad industrial, preparando a los estudiantes para resolver problemas reales con soluciones
tecnológicamente actualizadas y comunicativas.

### ACTIVIDADES POR DESARROLLAR

- **Desarrollar una aplicación basada en visión artificial y organizada en dos etapas. La primera etapa**
    **se debe presentar en el interciclo, y la segunda etapa previo a finalizar el ciclo lectivo. La aplicación**
    **deberá cumplir con las siguientes especificaciones:**

## a) Parte I (Interciclo)

## ▪ Debe desarrollar una aplicación de escritorio usando OpenCV C++ que permita procesar extraer

```
imágenes (slices) de archivos de CT Scan (computed tomography) (formato IMA – DICOM). Para
```

```
Computación Docente: Vladimir Robles Bykbaev
VISIÓN ARTIFICIAL Período Lectivo: Febrero 2026 Octubre 2025 –
```

```
ello, deberá seleccionar un conjunto de imágenes médicas (desde KAGGLE) único por grupo. El
dataset que debe emplear es CT Low Dose Reconstruction. Con esta información, deberá
extraer los cortes (slices) y resaltar las áreas de interés en las imágenes extraídas empleando la
librería de acceso abierto ITK:
```

## • (<https://github.com/InsightSoftwareConsortium/ITK?tab=readme-ov-file>)

## • (<https://itk.org/>)

## ▪ Un ejemplo del proceso a seguir se observa en la Ilustración 1. Tome en cuenta que esta es la

```
línea base y deberá diseñar su propio proceso, indicando qué técnicas aplicó para separar
determinadas áreas en función del corte de la imagen para ello se sugiere consulte con un
radiólogo (slice):
```

# Ilustración 1 : Línea base del proceso para separar 3 áreas

# principales de la imagen: pulmones, corazón y estructuras

# óseas

## ▪ Un ejemplo del resultado que se obtiene al extraer los cortes se puede apreciar en la Tabla 1

```
Recuerde que una imagen de CT puede contener cientos de cortes (slices), sin embargo, para el
dataset seleccionado, principalmente contiene solo 1 corte por archivo.
```

```
Resolución CS N° 076-04-2016-04-
```

```
Slice del archivo
L067_QD_3_SHARP_1.CT.0003.0001.2016.01.21.18.12.01.
1560.405032914.IMA
```

```
Detección de la zona de las estructuras óseas del Slice del
archivo
L067_QD_3_SHARP_1.CT.0003.0001.2016.01.21.18.12.01.
1560.405032914.IMA
```

```
Resaltado de las estructuras óseas del Slice del archivo con ajuste de parámetros de Double threshold e histérisis
L067_QD_3_SHARP_1.CT.0003.0001.2016.01.21.18.12.01.921560.405032914.IMA
```

_Tabla 1 : Ejemplo de dos imágenes extraídas de la imagen de CT
L067_QD_3_SHARP_1.CT.0003.0001.2016.01.21.18.12.01.921560.405032914.IMA del dataset "CT Low Dose Reconstruction"
(URL: <https://www.kaggle.com/datasets/andrewmvd/ct-low-dose-reconstruction/data>)_

```
▪ Con las imágenes (tanto el corte como la segmentación), deberá aplicar operaciones que permitan
```

```
Computación Docente: Vladimir Robles Bykbaev
VISIÓN ARTIFICIAL Período Lectivo: Febrero 2026 Octubre 2025 –
```

```
resaltar dicha área (por ejemplo, incremento de contraste, operaciones morfológicas, máscaras de
resaltado de color, etc.). Para ello deberá consultar fuentes médicas y definir qué operaciones de
resaltado va a emplear, como se puede apreciar en la Ilustración 1.
```

# Ilustración 2 : Ejemplo que ilustra el proceso a seguir para extraer el

# corte (slice) y el área de interés de dicho corte y luego generar una

# tercera imagen (a color) donde se resalta dicha área de interés a fin

# de que los médicos puedan realizar un mejor análisis

## ▪ Los aspectos y funcionalidades con los que debe contar el proyecto desarrollado son los que se

```
indican seguidamente.
```

## • La aplicación de escritorio permitirá seleccionar el corte de cualquier imagen contenida en la

```
carpeta y generará una imagen donde se resalte el área de interés para cada corte, mostrando
las operaciones intermedias realizadas.
Resolución CS N° 076-04-2016-04-
```

```
Insight Toolkit
```

```
CT Scan Imaging
```

```
NII Images
```

```
Corte de la imagen
DICOM
```

```
Obtención ROI imagen
DICOM
```

```
Módulo de
procesamiento
PDI
```

**- Filtros

- Ecualización Histograma
- Operaciones Morfológicas
- Operadores AND, OR, XOR
- Detección Bordes
- Reducción ruido (métodos
tradicionales y DNN**

```
Zona de interés
resaltada para
análisis médico
```

```
DICOM Images
```

```
Reducción de ruido
técnicas tradicionales y
Deep Learning
```

- La aplicación debe contar con controles para poder realizar el análisis y extracción de
    máscaras, áreas de interés, etc. que se requieran, en función de lo que haya establecido cada
    grupo con la consulta previa a un médico o radiólogo.
- Las imágenes tanto intermedias como finales, se almacenarán en una carpeta.
- Las operaciones para obtener las áreas de interés, máscaras, etc. deberán estar sustentadas
    en un proceso lógico que permita mejorar el proceso de análisis para médicos y/o radiólogos.

```
▪ Las técnicas de visión artificial que deben emplearse en el pre-procesamiento tanto en el
dispositivo móvil como en el servidor web:
```

- Thersholding
- Contrast Stretching
- Binarización por umbral de color
- Operaciones sobre los puntos de la imagen (NOT, AND, OR, XOR)
- Detección de Bordes
- Manipulación de pixeles.
- Filtros de suavizado.
- Operaciones morfológicas
- Uso de una red neuronal de aprendizaje profundo para reducir ruido (no se requiere entrenar la
    red, únicamente usarla).
- **Otras técnicas que se investigue**
▪ Aspectos generales a tomar en consideración:
- Se debe diseñar la propuesta del efecto (de forma similar a cómo se presenta en la Ilustración
1).
- Si usa códigos de terceros, debe indicar qué parte se ha tomado y de dónde (colocar citas), a
fin de valorar correctamente el trabajo.
- El proyecto debe presentarse funcionando en vivo durante la semana de revisión de
exámenes.
- **Fecha de entrega: Semana de revisión de exámenes interciclo.**

**RÚBRICA DE EVALUACIÓN PARA LA PARTE I PROYECTO (15 puntos):
Proyecto Integrador**

**Criterio Ponderación**

La operación para resaltar al menos 3 áreas de interés aplica las técnicas y conceptos vistos
en clase sustentados en criterios médicos de procesamiento de imágenes (por grupo):

- Ecualización de histograma
- Suavizado de imágenes
- Filtros de detección de bordes
- Manipulación de pixeles
- Operaciones morfológicas

### 25%

El uso de la red neuronal de aprendizaje profundo mejora el proceso de análisis y extracción
de áreas de interés, máscaras, etc.

### 10%

Existe un análisis sustentado desde la perspectiva médica y documentado sobre las zonas de
interés que se extraerán y dicho proceso permite obtener al menos 3 zonas con una precisión

de 90% o superior (es decir, existen pocos puntos fuera de la zona de interés). Ver el ejemplo
de la Ilustración 1.

### 15 %

Dentro del proceso se incluye una técnica nueva que se ha investigado por parte de los

estudiantes

### 15%

El código está desarrollado combina las librerías OpenCV C++ e ITK (35%) o en OpenCV e
ITK Python (10%) donde se extraen los cortes y se realiza el proceso de resaltado de las
zonas de interés *****

### 10% 35 %

```
Total (sobre 100%) 75% 10 0%
```

***Este criterio tiene una ponderación distinta para el tipo de implementación que se realice. Si desarrolla la opción para**

```
Computación Docente: Vladimir Robles Bykbaev
VISIÓN ARTIFICIAL Período Lectivo: Febrero 2026 Octubre 2025 –
```

**realizar el procesamiento desde OpenCV C++ e ITK (como lo visto en clase), el puntaje es de 35%. Si por otra parte, en
lugar de ello, se realiza el procesamiento en Python, es del 10%. Esto se da con el objetivo de que los estudiantes
puedan optar por aplicar una alternativa en caso de no lograr implementar la aplicación en C++.**

**RÚBRICA DE EVALUACIÓN PARA EL INFORME DE LA PARTE I (5 puntos):**
A continuación se especifican los criterios de evaluación:

**Video – Blog en inglés (Sobre 4 Puntos)**

**Criterio Ponderación**

El informe incluye una descripción detallada del trabajo realizado (introducción, descripción del
problema, propuesta de solución, conclusiones, bibliografía)

### 35%

El informe incorpora resultados de pruebas realizadas con el sistema (gráficas, reportes, etc.)
de los siguientes aspectos: uso de memoria RAM para procesar un grupo de imágenes, datos
estadísticos de las zonas de interés resaltados.

### 25%

El informe está correctamente redactado, contiene citas a papers y un **esquema explicativo
de la solución planteada**

### 20 %

El vídeo en inglés sigue un guion donde se explican los principales aspectos del proyecto de

una manera clara y concisa

### 20 %

```
Total 100% Total
```

# b) Parte II (Final)

## ▪ Debe implementar un detector de personas/peatones usando técnicas clásicas (HOG + SVM o LBP

```
+ Cascadas de Haar o métodos similares) en una aplicación de escritorio con OpenCV C++.
```

## ▪ Cuando se detecta una persona, la aplicación debe enviar la imagen y vídeo capturado a un Bot de

```
Telegram.
```

## ▪ El Bot debe usar una red de detección de postura humana (mediante PyTorch y modelos como

```
OpenPose, MMPose o similares) para analizar la postura de todas las personas en la imagen.
```

## ▪ El Bot debe enviar al usuario de Telegram tres archivos

- Imagen original con un mensaje de que se ha realizado la detección.
- Imagen con posturas humanas detectadas (superpuestas).
- Vídeo corto (GIF o MP4) de la detección en tiempo real vs original.

# ▪ Detector de personas/peatones usando técnicas clásicas (HOG + SVM o LBP

# + Cascadas de Haar o métodos similares)

## • La aplicación de escritorio (OpenCV C++) debe detectar personas o peatones en vídeos

```
capturados desde una cámara web HOG + SVM, LBP + Cascadas de Haar
( opencv_traincascade ) o un enfoque similar, de forma que se pueda detectar el área donde
está la persona, como se indica en la Ilustración 3.
```

```
Resolución CS N° 076-04-2016-04-
```

```
Ilustración 3 : Ejemplo de una imagen donde se detecta peatones con técnicas como
```

# HOG y LBP (fuente: <http://www.vision-ary.net/2015/03/boost-the-world-pedestrian/>)

- Debe tomar en cuenta que los peatones pueden estar realizando algún movimiento o estar en
    una postura dada, por ello, deberá construir un **_dataset_** que tenga al menos 4.000 imágenes
    positivas de peatones no solo caminando sino también en determinadas posturas y al menos
    4.000 imágenes negativas (cualquier otro objeto que no sean los peatones que se desean
    detectar). Para realizar el proceso de aumento de imágenes/datos ( **_data augmentation_** ) puede
    usar la librería **_Albumentations:_**
    ◦ <https://albumentations.ai/>
- El proceso de entrenamiento lo puede realizar con la librería OpenCV con el método
    **_opencv_traincascade_** que permite entrenar cascadas de detección.
- Para ello, puede basarse en los pasos explicados en el siguiente tutorial:
    <https://rithikachowta.medium.com/object-detection-lbp-cascade-classifier-generation->
    a1d1a1c2d0b
- **Recuerde por favor que ningún grupo puede tener el mismo dataset.**

# ▪ Detección de postura humana usando Deep Learning

## • Deberá implementar un detector de objetos en un bot de Telegram usando mediante PyTorch y

```
modelos como OpenPose, MMPose o similares.
```

## • El Bot realiza el proceso de detección cuando recibe la información de la aplicación de

```
escritorio (desarrollada en OpenCV C++).
```

## • El Bot de Telegram se puede programar en Python, y debe enviar la notificación de detección

```
de postura humana a un número de celular (o cuenta de Telegram) del usuario registrado.
```

## • Asimismo, el Bot enviará al usuario de Telegram dos fotos (original y con los puntos

```
detectados) y un vídeo corto de al menos 5 segundos de la persona o peatón con movimiento y
la detección de puntos de la postura (Ilustración 4).
```

```
Computación Docente: Vladimir Robles Bykbaev
VISIÓN ARTIFICIAL Período Lectivo: Febrero 2026 Octubre 2025 –
```

```
Ilustración 4 : Ejemplo de detección de la postura humana en diferentes
fuentes de vídeo usando OpenPose. Fuente:
https://blog.roboflow.com/what-is-openpose/
```

```
▪ Aspectos generales a tomar en consideración:
```

- Se debe diseñar la propuesta del proyecto para las partes I y II, es decir, cada parte debe tener
    un diagrama explicativo (de forma similar a cómo se presenta en la Ilustración 1).
- Si usa códigos de terceros, debe indicar qué parte se ha tomado y de dónde (colocar citas), a
    fin de valorar correctamente el trabajo.
- El proyecto debe presentarse funcionando en vivo durante la semana de revisión de
    exámenes. Considere que para ello puede ser que necesite traer su _router_ o realizar
    configuración y pruebas previas en la red de la UPS.
- **Fecha de entrega: Semana de revisión de examenes finales.**

**Documentación de Soporte:**

- Tutorial detección de vehículos usando LBP y Cascadas en OpenCV:
    ◦ <https://rithikachowta.medium.com/object-detection-lbp-cascade-classifier-generation-a1d1a1c2d0b>
- Tutorial detector de objetos uscando Cascadas de Haar en OpenCV:
    ◦ <https://machinelearningmastery.com/training-a-haar-cascade-object-detector-in-opencv/>
- Tutorial Seguimiento de Detección de la Postura Humana con OpenPose:
    ◦ <https://blog.roboflow.com/what-is-openpose/>

**RÚBRICA DE EVALUACIÓN PARA EL PROYECTO PARTE II FINAL (15 puntos):**

**Proyecto Integrador
Resolución CS N° 076-04-2016-04-**

**Criterio Ponderación**

La detección de personas o peatones tanto de pie como realizando alguna postura tiene una

precisión igual o mayor al 80%. Los estudiantes usan HOG o LBP para esta tarea junto con
SVM

### 20%

La detección de la postura humana tiene una precisión igual o mayor al 80%. Los estudiantes

usan una red neuronal de aprendizaje profundo y PyTorch para realizar esta tarea.

### 2 0%

La aplicación de escritorio se integra correctamente con el Bot y permite configurar a qué
clientes o usuarios enviará las notificaciones. Se valorará que el envío se realice con algún API
y no escribiendo archivos desde OpenCV C++ que luego serán leídos por el Bot (esta no es la

opción correcta).

### 15%

La aplicación de escritorio y el Bot de Telegram muestran información de uso de memoria,
FPS, puntos detectados (SIFT), nivel de confianza de la detección.

### 10%

La aplicación de escritorio captura el vídeo (25%) o realiza la captura de una secuencia de
imágenes o una sola imagen (10%) que se envía a procesar al bot de Telegram *****

### 10% 35 %

```
Total (sobre 100%) 75% 10 0%
```

***Este criterio tiene una ponderación distinta para el tipo de implementación que se realice. Si desarrolla la opción para
capturar vídeos y procesarlos en el Bot, el puntaje es de 35%. Si por otra parte, en lugar de ello, se captura únicamente
una suencia de imágenes, la ponderación es del 10%. Esto se da con el objetivo de que los estudiantes puedan optar
por aplicar una alternativa en caso de no lograr capturar vídeo.**

**RÚBRICA DE EVALUACIÓN PARA EL INFORME PARTE II FINAL (5 puntos):**
A continuación se especifican los criterios de evaluación:

```
Video – Blog en inglés (Sobre 4 Puntos)
```

```
Criterio Ponderación
```

```
El informe incluye una descripción detallada del trabajo realizado (introducción, descripción
del problema, propuesta de solución, conclusiones, bibliografía)
```

### 30%

```
El informe incorpora un análisis detallado de los resultados obtenidos con las técnicas tanto
LBP o HOG y la Red Neuronal de Aprendizaje Profundo se incluye:
```

- Resultados y gráficos de precisión
- Resultados y gráficos de sensitividad y especifidad
- Uso de memoria de la aplicación de escritorio y del Bot
- FPS que se puede procesar
- Matriz de confusión

### 30%

```
El informe está correctamente redactado, contiene citas a papers y un esquema explicativo
de la solución planteada
```

### 20 %

```
El vídeo en inglés sigue un guion donde se explican los principales aspectos del proyecto de
una manera clara y concisa
```

### 20 %

```
Total 100%
```

```
Computación Docente: Vladimir Robles Bykbaev
VISIÓN ARTIFICIAL Período Lectivo: Febrero 2026 Octubre 2025 –
```

### RESULTADO(S) OBTENIDO(S)

El desarrollo del Proyecto Integrador “Sistema Inteligente de Análisis Visual para Seguridad y Diagnóstico Médico”
permite a los estudiantes consolidar y aplicar de manera práctica las competencias definidas en los cuatro
Resultados de Aprendizaje de la asignatura, demostrando una integración profunda entre teoría y aplicación.

**Habilidades desarrolladas en relación con el Resultado de Aprendizaje 1: Identifica características,
espacios de color y elementos de formación de imágenes.**

- **Parte I (Interciclo)** : Al trabajar con imágenes médicas en formato DICOM/NIfTI, los estudiantes
    comprenden la estructura de datos volumétricos y la digitalización de imágenes complejas. Aplican
    conversiones entre espacios de color (escala de grises, RGB) y manipulan profundidad de bits para
    visualizar y procesar slices.
- **Parte II (Final)** : Al capturar video desde una cámara web, los estudiantes identifican los elementos
    fundamentales de un sistema de visión artificial (cámara, modelo pinhole implícito) y comprenden cómo se
    forman las imágenes digitales en tiempo real.
- **Habilidad adquirida** : Los estudiantes son capaces de **comprender los fenómenos involucrados en la**
    **captación de imágenes** , **identificar las características básicas de una imagen digital** y **transformar**
    **imágenes entre distintos espacios de color** según sea necesario para el preprocesamiento o la
    visualización.

**Habilidades desarrolladas en relación con el Resultado de Aprendizaje 2: Desarrolla algoritmos para
preprocesamiento y manipulación de imágenes.**

- **Parte I (Interciclo)** : Implementan algoritmos de preprocesamiento como ecualización de histograma,
    binarización por umbral, filtros de suavizado (Gaussiano, mediana) y operaciones morfológicas (erosión,
    dilatación). Además, aplican detección de bordes (Canny) y comparan resultados cuantitativamente
    (PSNR/SSIM).
- **Parte II (Final)** : Aunque la detección de personas es el foco, los estudiantes deben preprocesar el video
    (redimensionamiento, conversión a escala de grises) antes de aplicar el detector clásico (HOG/SVM o
    LBP).
- **Habilidad adquirida** : Los estudiantes pueden **obtener y ecualizar histogramas** , **implementar**
    **algoritmos de reducción de ruido** , **aplicar operaciones morfológicas** y **detectar bordes en imágenes** ,
    integrando estas técnicas en flujos de trabajo reales y automatizados.

**Habilidades desarrolladas en relación con el Resultado de Aprendizaje 3: Desarrolla algoritmos para la
extracción de características globales y locales.**

- **Parte I (Interciclo)** : Aunque no se extraen descriptores explícitos, el proceso de segmentación y resaltado
    de áreas de interés (pulmones, corazón, huesos) implica la comprensión de qué características visuales
    (intensidad, textura, forma) son relevantes para un radiólogo, preparando el terreno para la extracción de
    características en la Parte II.
- **Parte II (Final)** : El uso de HOG (Histogram of Oriented Gradients) para la detección de peatones es una
    aplicación directa de la extracción de características globales. Además, la detección de postura humana
    con MMPose/OpenPose implica la extracción de características locales (puntos clave del cuerpo humano).
- **Habilidad adquirida** : Los estudiantes **emplean algoritmos de HOG para extraer características**
    **globales** y **utilizan modelos basados en deep learning para extraer características locales (puntos**
    **clave)** , comprendiendo sus ventajas y aplicaciones en contextos específicos.

**Habilidades desarrolladas en relación con el Resultado de Aprendizaje 4: Desarrolla aplicaciones para**

**extraer descriptores en el dominio del espacio y la frecuencia a fin de sustentar el reconocimiento de
patrones.**

- **Parte I (Interciclo)** : La comparación entre métodos clásicos de denoising y DnCNN introduce a los
    estudiantes en el concepto de "reconocimiento de patrones" mediante redes neuronales, donde el
    descriptor es el conjunto de pesos aprendidos por la red.
- **Parte II (Final)** : La detección de personas con HOG+SVM o LBP+Cascades es una aplicación directa de
    reconocimiento de patrones en el dominio del espacio. La detección de postura con deep learning es una
       **Resolución CS N° 076-04-2016-04-**

```
extensión moderna de este concepto, donde el descriptor es una representación de alto nivel aprendida por
la red.
```

- **Habilidad adquirida** : Los estudiantes **comprenden los fundamentos de los algoritmos de**
    **reconocimiento de patrones** (como HOG y Cascadas de Haar) y **implementan sistemas de**
    **reconocimiento basados en aprendizaje profundo** , aplicando estos conocimientos a problemas reales
    de seguridad y monitoreo.

### CONCLUSIONES

Al completar este Proyecto Integrador, los estudiantes de la Carrera de Computación habrán logrado una
comprensión integral y aplicada de los principios fundamentales de la Visión por Computador, validando todos los
Resultados de Aprendizaje de la asignatura. Las conclusiones específicas son las siguientes:

## 1. Comprendieron la formación y adquisición de imágenes en contextos reales : Los estudiantes

```
identificaron las diferencias entre imágenes médicas (DICOM/NIfTI) y de vigilancia (video en tiempo real),
comprendiendo los elementos físicos y digitales involucrados en su captura y representación. Esta
habilidad les permite elegir adecuadamente los formatos y espacios de color para cualquier aplicación
futura.
```

## 2. Dominaron las técnicas de preprocesamiento y manipulación de imágenes : A través de la

```
implementación de filtros, operaciones morfológicas y detección de bordes, los estudiantes no solo
aprendieron a aplicar estos algoritmos, sino también a evaluar su impacto cuantitativo y cualitativo en la
calidad de la imagen, lo cual es crucial para tareas de análisis médico y seguridad.
```

## 3. Aplicaron con éxito la extracción de características globales y locales : La combinación de HOG

```
(característica global) para la detección de personas y la extracción de puntos clave (características
locales) para la detección de postura humana les permitió entender la diferencia entre ambos tipos de
descriptores y seleccionar el más adecuado para cada problema, fortaleciendo su capacidad de diseño de
sistemas de visión.
```

## 4. Implementaron sistemas de reconocimiento de patrones modernos y tradicionales : El proyecto les

```
permitió experimentar con dos paradigmas: el clásico (HOG+SVM, LBP+Cascades) y el moderno (deep
learning con PyTorch). Esto les proporciona una perspectiva histórica y actualizada del campo,
preparándolos para trabajar en entornos donde ambas tecnologías coexisten.
```

## 5. Desarrollaron habilidades transversales de ingeniería de software y comunicación : La integración de

```
múltiples componentes (OpenCV C++, Flask, PyTorch, Telegram Bot API) y la obligatoriedad de generar un
informe técnico y un vídeo-blog en inglés les enseñó a trabajar en proyectos multidisciplinarios, gestionar
dependencias, documentar su trabajo y comunicar sus hallazgos de manera efectiva.
```

## 6. Conectaron la teoría con la práctica en campos críticos : Al enfocarse en diagnóstico médico y

```
seguridad, los estudiantes apreciaron el impacto social y profesional de la visión por computador,
motivándolos a seguir profundizando en el área y a considerarla como una herramienta poderosa para
resolver problemas reales.
```

### RECOMENDACIONES

**Para los estudiantes:**

## 1. Planificar con anticipación las etapas del proyecto : Dado que el proyecto está dividido en dos fases

```
(Interciclo y final), los estudiantes deben definir un cronograma de trabajo claro. Asegúrate de realizar
pruebas previas al día de la presentación y configurar correctamente la red en la UPS o traer tu propio
router para evitar problemas de conectividad.
```

## 2. Investigar y probar nuevas técnicas de visión artificial : Además de las técnicas vistas en clase

```
(ecualización de histograma, operaciones morfológicas, etc.), es fundamental que los estudiantes
investiguen y apliquen nuevas técnicas no contempladas en el material. Esta iniciativa será valorada
```

```
Computación Docente: Vladimir Robles Bykbaev
VISIÓN ARTIFICIAL Período Lectivo: Febrero 2026 Octubre 2025 –
```

```
positivamente.
```

**Para el docente:**

## 3. Proporcionar soporte técnico continuo : Se recomienda que el docente ofrezca sesiones adicionales de

```
asesoría, donde los estudiantes puedan resolver dudas sobre la configuración de la librería ITK. También
podría proporcionar guías o ejemplos adicionales relacionados con las técnicas avanzadas de
procesamiento de imágenes.
```

## 4. Facilitar recursos de infraestructura : Durante las fases de prueba y presentación en vivo, podría ser

```
beneficioso que el docente coordine con el equipo de TI para garantizar que los estudiantes tengan acceso
a redes Wi-Fi estables y funcionales, especialmente para proyectos que requieran transmisión de datos.
```

**Para la Dirección de Carrera de Computación:**

## 5. Asignar recursos para capacitación técnica : Se sugiere que la Dirección de Carrera organice talleres

```
previos al inicio del ciclo, enfocados en el uso de software de Procesamiento Digital de Imágenes médicas
y el manejo avanzado de librerías de C++ y Python para visión artificial. Esto permitirá que los estudiantes
lleguen mejor preparados a la asignatura.
```

Estas recomendaciones buscan asegurar que tanto estudiantes como docentes y la administración tengan las
herramientas y el soporte necesario para completar el proyecto de manera efectiva y sin contratiempos.

```
Docente / Técnico Docente : Ing. Vladimir Robles Bykbaev
```

```
Firma : ______________________________
```

```
Resolución CS N° 076-04-2016-04-
```
