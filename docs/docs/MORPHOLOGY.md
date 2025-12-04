# 🔧 Morphology - Operaciones Morfológicas sobre Máscaras

## 📋 Descripción

El módulo **Morphology** (`morphology.h/cpp`) implementa **operaciones morfológicas binarias** para refinar y mejorar máscaras de segmentación. Permite rellenar huecos, eliminar ruido, suavizar bordes y realizar operaciones de conectividad sobre regiones segmentadas.

## 🎯 Responsabilidad

> **Única responsabilidad**: Aplicar transformaciones morfológicas a máscaras binarias para mejorar su calidad topológica

## 🏗️ Arquitectura

```cpp
namespace Morphology {
    // Operaciones básicas
    cv::Mat erode(const cv::Mat& mask, int kernelSize = 5);
    cv::Mat dilate(const cv::Mat& mask, int kernelSize = 5);
    
    // Operaciones compuestas
    cv::Mat opening(const cv::Mat& mask, int kernelSize = 5);
    cv::Mat closing(const cv::Mat& mask, int kernelSize = 5);
    
    // Procesamiento de regiones
    cv::Mat fillHoles(const cv::Mat& mask);
    cv::Mat removeSmallRegions(const cv::Mat& mask, int minArea = 100);
    cv::Mat largestConnectedComponent(const cv::Mat& mask);
    
    // Análisis
    int countConnectedComponents(const cv::Mat& mask);
    std::vector<std::vector<cv::Point>> extractContours(const cv::Mat& mask);
}
```

## 📚 API Pública

### 1. `erode()` - Erosión

**Descripción**: **Reduce** el tamaño de regiones blancas, eliminando píxeles en los bordes

**Firma**:

```cpp
cv::Mat erode(const cv::Mat& mask, int kernelSize = 5);
```

**Parámetros**:

- `mask`: Máscara binaria (CV_8U, valores 0 o 255)
- `kernelSize`: Tamaño del elemento estructurante (impar: 3, 5, 7...)

**Efecto visual**:

```bash
Antes:        Después (kernel 3×3):
█████████     
███████████   ███████
█████████     █████
              ███
```

**Cuándo usar**:

- ✅ **Eliminar puentes delgados** entre regiones
- ✅ **Separar objetos tocándose**
- ✅ **Reducir tamaño** de regiones (por ej., antes de dilatar)
- ⚠️ Puede eliminar estructuras pequeñas importantes

**Ejemplo**:

```cpp
cv::Mat lungMask = segmentLungs(...);

// Eliminar pequeñas conexiones espurias
cv::Mat eroded = Morphology::erode(lungMask, 3);

// Luego dilatar de vuelta para recuperar tamaño
cv::Mat cleaned = Morphology::dilate(eroded, 3);
```

**Implementación**:

```cpp
cv::Mat Morphology::erode(const cv::Mat& mask, int kernelSize) {
    cv::Mat result;
    cv::Mat kernel = cv::getStructuringElement(
        cv::MORPH_ELLIPSE, 
        cv::Size(kernelSize, kernelSize)
    );
    cv::erode(mask, result, kernel);
    return result;
}
```

---

### 2. `dilate()` - Dilatación

**Descripción**: **Expande** el tamaño de regiones blancas, agregando píxeles en los bordes

**Firma**:

```cpp
cv::Mat dilate(const cv::Mat& mask, int kernelSize = 5);
```

**Parámetros**:

- `mask`: Máscara binaria
- `kernelSize`: Tamaño del elemento estructurante

**Efecto visual**:

```bash
Antes:        Después (kernel 3×3):
  ███         █████████
 █████        ███████████
  ███         ███████████
              █████████
```

**Cuándo usar**:

- ✅ **Rellenar pequeños huecos** internos
- ✅ **Conectar regiones cercanas**
- ✅ **Suavizar bordes irregulares**
- ⚠️ Puede unir objetos que deberían estar separados

**Ejemplo**:

```cpp
cv::Mat boneMask = segmentBones(...);

// Conectar fragmentos de la misma costilla
cv::Mat dilated = Morphology::dilate(boneMask, 5);

// Luego erosionar para volver al tamaño original
cv::Mat connected = Morphology::erode(dilated, 5);
```

---

### 3. `opening()` - Apertura (Erosión + Dilatación)

**Descripción**: **Elimina ruido pequeño** preservando forma general

**Firma**:

```cpp
cv::Mat opening(const cv::Mat& mask, int kernelSize = 5);
```

**Secuencia**:

```bash
1. Erode  → Elimina protuberancias pequeñas
2. Dilate → Recupera tamaño original
```

**Efecto**:

- ✅ **Elimina píxeles aislados** (ruido de sal)
- ✅ **Rompe istmos delgados** entre objetos
- ✅ **Suaviza contornos convexos**

**Cuándo usar**:

- ✅ Máscara tiene muchos **píxeles ruidosos aislados**
- ✅ Objetos conectados por **puentes delgados** que deben separarse

**Ejemplo**:

```cpp
cv::Mat noisyMask = binaryThreshold(...);

// Eliminar píxeles aislados
cv::Mat clean = Morphology::opening(noisyMask, 3);

// Antes:  ███ █ ██ █ ███
// Después: ███   ██   ███  (píxeles aislados eliminados)
```

---

### 4. `closing()` - Cierre (Dilatación + Erosión)

**Descripción**: **Rellena huecos pequeños** preservando forma general

**Firma**:

```cpp
cv::Mat closing(const cv::Mat& mask, int kernelSize = 5);
```

**Secuencia**:

```bash
1. Dilate → Cierra huecos pequeños
2. Erode  → Recupera tamaño original
```

**Efecto**:

- ✅ **Rellena huecos internos pequeños**
- ✅ **Conecta regiones cercanas**
- ✅ **Suaviza contornos cóncavos**

**Cuándo usar**:

- ✅ Región tiene **huecos pequeños** que deben rellenarse
- ✅ Objetos fragmentados que son **parte de la misma estructura**

**Ejemplo**:

```cpp
cv::Mat aortaMask = segmentAorta(...);

// Aorta fragmentada por calcificaciones
// Closing conecta los fragmentos
cv::Mat continuous = Morphology::closing(aortaMask, 7);

// Antes:  ███  ███  ███  (aorta fragmentada)
// Después: █████████████  (aorta continua)
```

---

### 5. `fillHoles()` - Relleno de Huecos ⭐

**Descripción**: Rellena **todos los huecos internos** de una región, sin importar su tamaño

**Firma**:

```cpp
cv::Mat fillHoles(const cv::Mat& mask);
```

**Parámetros**:

- `mask`: Máscara binaria con posibles huecos internos

**Retorna**:

- Máscara con todos los huecos rellenados

**Algoritmo**:

```bash
1. Invertir máscara (blanco ↔ negro)
2. Flood-fill desde (0,0) - marca el fondo exterior
3. Invertir resultado - lo no marcado son huecos internos
4. Combinar con máscara original
```

**Cuándo usar**:

- ✅ **Segmentación de órganos sólidos** (hígado, riñones)
- ✅ **Rellenar vasos internos** en segmentación pulmonar
- ✅ Cuando huecos son **artefactos** no reales

**Ejemplo**:

```cpp
cv::Mat liverMask = segmentLiver(...);

// El hígado puede tener vasos que aparecen como huecos
cv::Mat solidLiver = Morphology::fillHoles(liverMask);

// Antes:  ███████        Después: ███████████
//         ██   ███                ███████████
//         ███████                 ███████████
```

**Implementación completa**:

```cpp
cv::Mat Morphology::fillHoles(const cv::Mat& mask) {
    // Crear imagen temporal con borde de 1 píxel
    cv::Mat padded;
    cv::copyMakeBorder(mask, padded, 1, 1, 1, 1, cv::BORDER_CONSTANT, cv::Scalar(0));
    
    // Invertir
    cv::Mat inverted = 255 - padded;
    
    // Flood-fill desde (0,0) - marca el fondo exterior
    cv::Mat floodFilled = inverted.clone();
    cv::floodFill(floodFilled, cv::Point(0, 0), cv::Scalar(0));
    
    // Invertir resultado - lo blanco ahora son los huecos internos
    cv::Mat holes = 255 - floodFilled;
    
    // Remover borde temporal
    holes = holes(cv::Rect(1, 1, mask.cols, mask.rows));
    
    // Combinar con máscara original
    cv::Mat result;
    cv::bitwise_or(mask, holes, result);
    
    return result;
}
```

---

### 6. `removeSmallRegions()` - Filtrado por Área

**Descripción**: Elimina **componentes conectadas pequeñas** consideradas ruido

**Firma**:

```cpp
cv::Mat removeSmallRegions(const cv::Mat& mask, int minArea = 100);
```

**Parámetros**:

- `mask`: Máscara binaria con posibles regiones ruidosas
- `minArea`: Área mínima en píxeles para conservar región

**Retorna**:

- Máscara con solo regiones grandes (área ≥ minArea)

**Algoritmo**:

```bash
1. Análisis de componentes conectadas
2. Para cada componente:
   - Si área < minArea → Eliminar
   - Si área ≥ minArea → Conservar
3. Reconstruir máscara filtrada
```

**Cuándo usar**:

- ✅ Después de umbralización con **píxeles aislados**
- ✅ Segmentación ruidosa con **artefactos pequeños**
- ✅ Conservar solo **estructuras principales**

**Ejemplo**:

```cpp
cv::Mat noisySegmentation = thresholdImage(...);

// Contar componentes antes
int before = Morphology::countConnectedComponents(noisySegmentation);
std::cout << "Regiones antes: " << before << std::endl;  // Ej: 347

// Eliminar regiones < 500 píxeles
cv::Mat filtered = Morphology::removeSmallRegions(noisySegmentation, 500);

int after = Morphology::countConnectedComponents(filtered);
std::cout << "Regiones después: " << after << std::endl;  // Ej: 3
```

**Implementación**:

```cpp
cv::Mat Morphology::removeSmallRegions(const cv::Mat& mask, int minArea) {
    cv::Mat labels, stats, centroids;
    int numLabels = cv::connectedComponentsWithStats(mask, labels, stats, centroids);
    
    cv::Mat result = cv::Mat::zeros(mask.size(), CV_8U);
    
    for (int i = 1; i < numLabels; ++i) {  // Saltar label 0 (fondo)
        int area = stats.at<int>(i, cv::CC_STAT_AREA);
        
        if (area >= minArea) {
            // Conservar esta región
            result.setTo(255, labels == i);
        }
    }
    
    return result;
}
```

---

### 7. `largestConnectedComponent()` - Región Más Grande

**Descripción**: Conserva **solo la componente conectada de mayor área**, elimina el resto

**Firma**:

```cpp
cv::Mat largestConnectedComponent(const cv::Mat& mask);
```

**Parámetros**:

- `mask`: Máscara binaria con múltiples componentes

**Retorna**:

- Máscara con solo la región más grande

**Cuándo usar**:

- ✅ **Segmentación de órgano único** (hígado, bazo)
- ✅ Cuando hay **falsos positivos pequeños**
- ✅ **Simplificar máscara** a una sola región

**Ejemplo**:

```cpp
cv::Mat liverMask = segmentLiver(...);

// Puede haber pequeñas regiones falsas (riñón, bazo)
// Conservar solo el hígado (región más grande)
cv::Mat onlyLiver = Morphology::largestConnectedComponent(liverMask);
```

**Implementación**:

```cpp
cv::Mat Morphology::largestConnectedComponent(const cv::Mat& mask) {
    cv::Mat labels, stats, centroids;
    int numLabels = cv::connectedComponentsWithStats(mask, labels, stats, centroids);
    
    if (numLabels <= 1) return mask;  // Solo fondo o vacío
    
    // Encontrar label con mayor área
    int largestLabel = 1;
    int maxArea = stats.at<int>(1, cv::CC_STAT_AREA);
    
    for (int i = 2; i < numLabels; ++i) {
        int area = stats.at<int>(i, cv::CC_STAT_AREA);
        if (area > maxArea) {
            maxArea = area;
            largestLabel = i;
        }
    }
    
    // Crear máscara con solo esa región
    cv::Mat result = (labels == largestLabel);
    result.convertTo(result, CV_8U, 255);
    
    return result;
}
```

---

### 8. `countConnectedComponents()` - Análisis

**Descripción**: Cuenta el número de **componentes conectadas** en la máscara

**Firma**:

```cpp
int countConnectedComponents(const cv::Mat& mask);
```

**Ejemplo**:

```cpp
cv::Mat boneMask = segmentBones(...);

int numBones = Morphology::countConnectedComponents(boneMask);
std::cout << "Estructuras óseas: " << numBones << std::endl;
// Output típico: 12 (costillas, vértebras, esternón)
```

---

### 9. `extractContours()` - Extracción de Contornos

**Descripción**: Extrae los **contornos externos** de todas las regiones

**Firma**:

```cpp
std::vector<std::vector<cv::Point>> extractContours(const cv::Mat& mask);
```

**Retorna**:

- Vector de contornos (cada contorno es un vector de puntos)

**Ejemplo**:

```cpp
cv::Mat lungMask = segmentLungs(...);
auto contours = Morphology::extractContours(lungMask);

// Dibujar contornos en color
cv::Mat display;
cv::cvtColor(originalImage, display, cv::COLOR_GRAY2BGR);
cv::drawContours(display, contours, -1, cv::Scalar(0, 255, 0), 2);

cv::imshow("Contornos pulmonares", display);
```

## 🎨 Comparación Visual de Operaciones

| Operación | Efecto | Mejor Para | Kernel Típico |
|-----------|--------|------------|---------------|
| **Erode** | Encoge regiones | Separar objetos | 3×3 |
| **Dilate** | Expande regiones | Conectar objetos | 5×5 |
| **Opening** | Elimina ruido pequeño | Limpiar píxeles aislados | 3×3 |
| **Closing** | Rellena huecos pequeños | Conectar fragmentos | 5×5 - 7×7 |
| **Fill Holes** | Rellena TODO hueco | Órganos sólidos | N/A |
| **Remove Small** | Filtra por área | Eliminar artefactos | N/A |
| **Largest** | Conserva mayor región | Un solo órgano | N/A |

## 🔍 Elementos Estructurantes

### **Formas Disponibles**

```cpp
// Rectangular (cuadrado)
cv::Mat kernel = cv::getStructuringElement(
    cv::MORPH_RECT, 
    cv::Size(5, 5)
);

// Elíptico (circular) - MÁS ISOTRÓPICO
cv::Mat kernel = cv::getStructuringElement(
    cv::MORPH_ELLIPSE, 
    cv::Size(5, 5)
);

// Cruz (conectividad 4)
cv::Mat kernel = cv::getStructuringElement(
    cv::MORPH_CROSS, 
    cv::Size(5, 5)
);
```

### **Recomendación**

- ✅ **MORPH_ELLIPSE** para la mayoría de casos médicos (isotrópico, no introduce sesgo direccional)
- ⚠️ MORPH_RECT puede causar artefactos en esquinas
- 🔧 MORPH_CROSS para conectividad estricta

## ⚡ Optimizaciones

### **Operaciones en Cadena**

```cpp
// ❌ INEFICIENTE: Múltiples copias intermedias
cv::Mat temp1 = Morphology::closing(mask, 5);
cv::Mat temp2 = Morphology::opening(temp1, 3);
cv::Mat temp3 = Morphology::fillHoles(temp2);
cv::Mat result = Morphology::removeSmallRegions(temp3, 100);

// ✅ EFICIENTE: Encadenar sin copias innecesarias
cv::Mat result = mask.clone();
cv::Mat kernel5 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
cv::Mat kernel3 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));

cv::morphologyEx(result, result, cv::MORPH_CLOSE, kernel5);
cv::morphologyEx(result, result, cv::MORPH_OPEN, kernel3);
result = Morphology::fillHoles(result);
result = Morphology::removeSmallRegions(result, 100);
```

### **Procesamiento en GPU**

```cpp
// OpenCV CUDA para operaciones morfológicas
#ifdef HAVE_CUDA
cv::cuda::GpuMat d_mask, d_result;
d_mask.upload(mask);

auto morphFilter = cv::cuda::createMorphologyFilter(
    cv::MORPH_CLOSE, CV_8U, kernel
);
morphFilter->apply(d_mask, d_result);

d_result.download(result);
#endif
```

## 🎯 Flujos de Trabajo Típicos

### **Pipeline para Pulmones**

```cpp
cv::Mat refineLungMask(const cv::Mat& rawMask) {
    // 1. Closing para conectar fragmentos del mismo pulmón
    cv::Mat connected = Morphology::closing(rawMask, 7);
    
    // 2. Rellenar vasos internos (aparecen como huecos)
    cv::Mat filled = Morphology::fillHoles(connected);
    
    // 3. Opening para suavizar bordes
    cv::Mat smoothed = Morphology::opening(filled, 3);
    
    // 4. Eliminar regiones pequeñas (artefactos)
    cv::Mat clean = Morphology::removeSmallRegions(smoothed, 5000);
    
    return clean;
}
```

### **Pipeline para Huesos**

```cpp
cv::Mat refineBoneMask(const cv::Mat& rawMask) {
    // 1. Closing para conectar fragmentos de la misma costilla
    cv::Mat connected = Morphology::closing(rawMask, 5);
    
    // 2. Opening para eliminar píxeles ruidosos
    cv::Mat denoised = Morphology::opening(connected, 3);
    
    // 3. Eliminar regiones muy pequeñas (calcificaciones aisladas)
    cv::Mat filtered = Morphology::removeSmallRegions(denoised, 100);
    
    return filtered;
}
```

### **Pipeline para Aorta**

```cpp
cv::Mat refineAortaMask(const cv::Mat& rawMask) {
    // 1. Closing agresivo para conectar aorta fragmentada
    cv::Mat connected = Morphology::closing(rawMask, 9);
    
    // 2. Rellenar huecos (calcificaciones internas)
    cv::Mat filled = Morphology::fillHoles(connected);
    
    // 3. Conservar solo componente más grande (la aorta verdadera)
    cv::Mat aorta = Morphology::largestConnectedComponent(filled);
    
    // 4. Suavizar contorno
    cv::Mat smoothed = Morphology::closing(aorta, 3);
    
    return smoothed;
}
```

## 🐛 Solución de Problemas

### **Problema: Opening elimina detalles importantes**

```cpp
// Usar kernel más pequeño o menos iteraciones
cv::Mat result = Morphology::opening(mask, 3);  // En lugar de 7
```

### **Problema: Closing une objetos que deben estar separados**

```cpp
// Reducir tamaño de kernel
cv::Mat result = Morphology::closing(mask, 3);  // En lugar de 9

// O usar erosión después para separar
cv::Mat closed = Morphology::closing(mask, 7);
cv::Mat separated = Morphology::erode(closed, 2);
```

### **Problema: fillHoles() es muy lento en imágenes grandes**

```cpp
// Alternativa más rápida para huecos pequeños-medianos
cv::Mat approxFilled = Morphology::closing(mask, 15);  // Cierra huecos < 15px
```

## 🧪 Testing

### **Test de Idempotencia**

```cpp
void testIdempotence() {
    cv::Mat mask = createTestMask();
    
    // Opening aplicado dos veces debe dar mismo resultado
    cv::Mat open1 = Morphology::opening(mask, 5);
    cv::Mat open2 = Morphology::opening(open1, 5);
    
    int diff = cv::countNonZero(open1 != open2);
    assert(diff == 0);  // Debe ser idempotente
    
    std::cout << "✓ Test de idempotencia pasado" << std::endl;
}
```

### **Test de Conservación de Área**

```cpp
void testAreaPreservation() {
    cv::Mat mask = createTestMask();
    
    int areaBefore = cv::countNonZero(mask);
    
    // Opening + Closing debe conservar área aproximadamente
    cv::Mat processed = Morphology::opening(mask, 3);
    processed = Morphology::closing(processed, 3);
    
    int areaAfter = cv::countNonZero(processed);
    
    double diff = std::abs(areaBefore - areaAfter) / (double)areaBefore;
    assert(diff < 0.05);  // Menos de 5% diferencia
    
    std::cout << "✓ Área conservada: " << diff * 100 << "%" << std::endl;
}
```

## 📚 Referencias

- [Mathematical Morphology](https://en.wikipedia.org/wiki/Mathematical_morphology)
- [OpenCV Morphological Transformations](https://docs.opencv.org/4.x/d9/d61/tutorial_py_morphological_ops.html)
- [Structuring Elements](https://homepages.inf.ed.ac.uk/rbf/HIPR2/strctel.htm)
- [Hole Filling Algorithm](https://www.mathworks.com/help/images/ref/imfill.html)

---

**Versión**: 1.0  
**Última actualización**: Noviembre 2025  
**Archivos**: `src/core/morphology.h`, `src/core/morphology.cpp`
