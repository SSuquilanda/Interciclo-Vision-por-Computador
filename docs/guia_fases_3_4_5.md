# Desglose con las técnicas recomendadas para **cada fase**, ejemplos y su justificación

---

## **Fase 3 – Preprocesamiento (`f3_preprocessing`)**

**Objetivo:**
Mejorar la calidad visual del slice médico antes de segmentar, reduciendo ruido y aumentando contraste.

**Técnicas recomendadas:**

| Técnica                                                                     | Descripción                                                 | Función / Justificación                                                         |
| --------------------------------------------------------------------------- | ----------------------------------------------------------- | ------------------------------------------------------------------------------- |
| **Conversión a escala de grises** (`cv::cvtColor(img, cv::COLOR_BGR2GRAY)`) | Simplifica el análisis y reduce dimensionalidad.            | Todas las operaciones de realce son más efectivas en gris.                      |
| **Normalización o Stretching lineal**                                       | `cv::normalize(img, img, 0, 255, cv::NORM_MINMAX)`          | Expande los niveles de intensidad — útil en CTs con bajo contraste.             |
| **Ecualización de histograma** (`cv::equalizeHist`)                         | Redistribuye intensidades para mejorar el contraste global. | Permite ver estructuras internas más claras (pulmones, huesos).                 |
| **CLAHE (Contrast Limited Adaptive Histogram Equalization)**                | Versión local adaptativa del histograma (`cv::createCLAHE`) | Mejora contraste sin sobreexponer zonas brillantes.                             |
| **Filtrado Gaussiano / Mediana / Bilateral**                                | `cv::GaussianBlur`, `cv::medianBlur`, `cv::bilateralFilter` | Elimina ruido sin distorsionar bordes (el bilateral es ideal para tomografías). |
| **Filtro de realce Laplaciano** (`cv::Laplacian`)                           | Acentúa bordes y cambios de intensidad.                     | Mejora percepción de estructuras finas.                                         |
| **Detección de bordes (Canny, Sobel, Prewitt)**                             | `cv::Canny`, `cv::Sobel`                                    | Para visualizar contornos anatómicos, antes de segmentar.                       |

**Ejemplo de flujo:**

```cpp
cv::Mat gray, eq, blur, edges;
cv::cvtColor(slice, gray, cv::COLOR_BGR2GRAY);
cv::equalizeHist(gray, eq);
cv::GaussianBlur(eq, blur, cv::Size(5,5), 0);
cv::Canny(blur, edges, 50, 150);
```

**Nota:** En tomografías, los filtros suaves tipo mediana + CLAHE ayudan a resaltar pulmones sin sobreexponer huesos.

---

## **Fase 4 – Segmentación (`f4_segmentation`)**

**Objetivo:**
Delimitar áreas anatómicas de interés (ROI): pulmones, corazón, huesos.
Esta etapa extrae regiones relevantes mediante umbrales y operaciones de agrupamiento.

**Técnicas recomendadas:**

| Técnica                                   | Descripción                                                           | Aplicación práctica                                                                               |
| ----------------------------------------- | --------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------- |
| **Thresholding global (Otsu o Manual)**   | `cv::threshold(img, mask, t, 255, cv::THRESH_BINARY+cv::THRESH_OTSU)` | Divide fondo y área anatómica según intensidad.                                                   |
| **Binarización por color / rango**        | `cv::inRange(img, minVal, maxVal)`                                    | Aísla regiones con valores de densidad específicos (pulmones más oscuros, huesos más brillantes). |
| **Double threshold + hysteresis**         | Como en Canny, separa estructuras fuertes, medias y débiles.          | Permite refinar máscaras con mayor precisión.                                                     |
| **Watershed Segmentation**                | `cv::watershed()` sobre marcadores previos.                           | Útil para separar regiones superpuestas (pulmón/tejido).                                          |
| **K-means clustering**                    | Agrupa píxeles en regiones similares (`cv::kmeans`).                  | Permite segmentar sin depender de umbral fijo.                                                    |
| **Region growing o connected components** | `cv::connectedComponents()` o ITK region growing.                     | Permite expandir regiones homogéneas desde semillas.                                              |
| **Active Contours (Snake, opcional)**     | En ITK existen implementaciones de contornos activos.                 | Refinamiento final para bordes anatómicos complejos.                                              |
| **Color map para resaltar**               | `cv::applyColorMap(mask, colorMask, cv::COLORMAP_JET)`                | Visualización intuitiva del ROI.                                                                  |

**Ejemplo:**

```cpp
cv::Mat mask;
cv::threshold(eq, mask, 90, 255, cv::THRESH_BINARY);
cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, cv::Mat::ones(5,5,CV_8U));
cv::applyColorMap(mask, maskColor, cv::COLORMAP_JET);
```

**Consideraciones médicas:**

* Pulmones → umbrales bajos (regiones oscuras).
* Huesos → intensidades altas.
* Corazón → valores medios + contornos suaves.

---

## **Fase 5 – Morfología (`f5_morphology`)**

**Objetivo:**
Refinar la máscara segmentada, eliminando ruido, uniendo regiones y mejorando la forma del ROI.

**Técnicas recomendadas:**

| Operación                                          | Función                                               | Ejemplo                                          |
| -------------------------------------------------- | ----------------------------------------------------- | ------------------------------------------------ |
| **Erosión (`cv::erode`)**                          | Elimina píxeles en bordes → reduce ruido aislado.     | Ideal tras umbral binario para limpiar.          |
| **Dilatación (`cv::dilate`)**                      | Añade píxeles a bordes → rellena huecos.              | Se usa después de erosión (closing).             |
| **Opening (`cv::morphologyEx(..., MORPH_OPEN)`)**  | Erosión seguida de dilatación.                        | Elimina pequeños artefactos blancos.             |
| **Closing (`cv::morphologyEx(..., MORPH_CLOSE)`)** | Dilatación seguida de erosión.                        | Cierra huecos dentro de regiones detectadas.     |
| **Gradient / Top-hat / Black-hat**                 | Resalta bordes o diferencias estructurales.           | Útil para resaltar estructuras óseas.            |
| **Hit-or-Miss transform**                          | Detecta formas específicas.                           | Aplicable si defines patrones (p.ej. vértebras). |
| **Structuring element personalizado**              | `cv::getStructuringElement(MORPH_ELLIPSE, Size(5,5))` | Ajusta la forma del elemento según el órgano.    |

**Ejemplo práctico:**

```cpp
cv::Mat morph;
cv::morphologyEx(mask, morph, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(7,7)));
```

**Recomendaciones:**

* Utilice **closing** para consolidar zonas de tejido.
* Utilice **opening** para eliminar ruido disperso.
* En pulmones, **gradiente morfológico** puede resaltar pleura o bordes pulmonares.

---

## **Resumen visual del pipeline completo**

```bash
[Entrada DICOM]
    ↓
(F3) Preprocesamiento → Filtros, Ecualización, Bordes
    ↓
(F4) Segmentación → Threshold, K-means, Region Growing
    ↓
(F5) Morfología → Erosión, Dilatación, Closing, Gradient
    ↓
[ROI resaltado + métricas PSNR/SSIM + exportación]
```

---

## **Sugerencia de combinación (flujo recomendado para CT)**

| Paso | Técnica              | Propósito                               |
| ---- | -------------------- | --------------------------------------- |
| 1    | CLAHE + GaussianBlur | Mejorar contraste y suavizar            |
| 2    | Otsu Threshold       | Separar estructuras                     |
| 3    | Morph Close (7×7)    | Rellenar pulmones o huesos              |
| 4    | Morph Gradient       | Resaltar bordes anatómicos              |
| 5    | applyColorMap        | Resaltar área de interés                |
| 6    | PSNR / SSIM          | Evaluar mejora con respecto al original |

---

**Nota final:**
Se puede generar un pipeline en C++ que combine estas tres fases en una función tipo
`processSlice(const cv::Mat &input)` → `{mask, highlighted, metrics}`
para integración directa en `main.cpp` o `mainwindow.cpp`.
