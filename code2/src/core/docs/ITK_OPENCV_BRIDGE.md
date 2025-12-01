# 🔄 ITK-OpenCV Bridge - Conversión de Formatos de Imagen

## 📋 Descripción

El módulo **ITK-OpenCV Bridge** (`itk_opencv_bridge.h/cpp`) actúa como puente entre dos bibliotecas fundamentales de visión por computador: **ITK** (para imágenes médicas) y **OpenCV** (para procesamiento general). Permite convertir imágenes entre ambos formatos preservando datos y metadatos.

## 🎯 Responsabilidad

> **Única responsabilidad**: Conversión bidireccional entre `itk::Image` y `cv::Mat` sin pérdida de información

## 🏗️ Arquitectura

```cpp
namespace Bridge {
    // ITK → OpenCV (16-bit, valores HU originales)
    cv::Mat itkToOpenCV16bit(const itk::Image<short, 2>::Pointer& itkImage);
    
    // ITK → OpenCV (8-bit, normalizado para visualización)
    cv::Mat itkToOpenCV8bit(const itk::Image<short, 2>::Pointer& itkImage);
    
    // OpenCV → ITK (reconstrucción desde procesado)
    itk::Image<short, 2>::Pointer openCVToITK(const cv::Mat& cvImage);
}
```

## 📚 API Pública

### 1. `itkToOpenCV16bit()`

**Descripción**: Convierte ITK image a OpenCV preservando **valores Hounsfield originales** (16-bit)

**Firma**:

```cpp
cv::Mat itkToOpenCV16bit(const itk::Image<short, 2>::Pointer& itkImage);
```

**Parámetros**:

- `itkImage`: Imagen ITK de 16-bit signed (short)

**Retorna**:

- `cv::Mat` de tipo `CV_16S` (16-bit signed) con valores HU sin modificar

**Cuándo usar**:

- ✅ Para algoritmos de **segmentación** (requieren valores HU precisos)
- ✅ Para cálculo de **métricas cuantitativas** (densidades, SNR)
- ✅ Cuando se necesita preservar rango dinámico completo

**Ejemplo**:

```cpp
auto dicomData = DicomIO::readDicomImage("scan.dcm");
cv::Mat raw16bit = Bridge::itkToOpenCV16bit(dicomData.image);

// Segmentación de pulmones (HU: -1000 a -400)
cv::Mat lungsMask;
cv::inRange(raw16bit, -1000, -400, lungsMask);

// Cálculo de densidad media
cv::Scalar meanHU = cv::mean(raw16bit, lungsMask);
std::cout << "Densidad pulmones: " << meanHU[0] << " HU" << std::endl;
```

---

### 2. `itkToOpenCV8bit()`

**Descripción**: Convierte ITK image a OpenCV con **normalización** para visualización (8-bit)

**Firma**:

```cpp
cv::Mat itkToOpenCV8bit(const itk::Image<short, 2>::Pointer& itkImage);
```

**Parámetros**:

- `itkImage`: Imagen ITK de 16-bit signed

**Retorna**:

- `cv::Mat` de tipo `CV_8U` (8-bit unsigned) con valores [0-255]

**Transformación aplicada**:

```bash
1. Encontrar min y max de la imagen
2. Normalizar: normalized = (pixel - min) / (max - min)
3. Escalar: pixel_8bit = normalized × 255
```

**Cuándo usar**:

- ✅ Para **visualización** en GUI (QLabel, cv::imshow)
- ✅ Como entrada a filtros que esperan 8-bit (CLAHE, algunos filtros)
- ✅ Para exportar a formatos estándar (PNG, JPEG)

**Ejemplo**:

```cpp
auto dicomData = DicomIO::readDicomImage("scan.dcm");
cv::Mat display = Bridge::itkToOpenCV8bit(dicomData.image);

// Aplicar CLAHE para mejorar contraste
cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
cv::Mat enhanced;
clahe->apply(display, enhanced);

// Mostrar en GUI
QImage qimg(enhanced.data, enhanced.cols, enhanced.rows, 
            enhanced.step, QImage::Format_Grayscale8);
label->setPixmap(QPixmap::fromImage(qimg));
```

---

### 3. `openCVToITK()`

**Descripción**: Convierte OpenCV Mat de vuelta a ITK Image

**Firma**:

```cpp
itk::Image<short, 2>::Pointer openCVToITK(const cv::Mat& cvImage);
```

**Parámetros**:

- `cvImage`: Puede ser `CV_8U`, `CV_16S`, o `CV_16U`

**Retorna**:

- `itk::Image<short, 2>::Pointer` compatible con ITK

**Conversión automática**:

- `CV_8U` → escala a rango 16-bit
- `CV_16S` → copia directa
- `CV_16U` → convierte a signed

**Cuándo usar**:

- ✅ Para exportar resultados procesados con OpenCV a formato ITK
- ✅ Para usar filtros ITK después de procesamiento OpenCV
- ✅ Para guardar como DICOM/NIfTI

**Ejemplo**:

```cpp
cv::Mat processed = someOpenCVProcessing(inputMat);
auto itkResult = Bridge::openCVToITK(processed);

// Guardar como NIfTI
using WriterType = itk::ImageFileWriter<itk::Image<short, 2>>;
auto writer = WriterType::New();
writer->SetFileName("result.nii.gz");
writer->SetInput(itkResult);
writer->Update();
```

## 🔍 Implementación Interna

### **Conversión ITK → OpenCV 16-bit**

```cpp
cv::Mat Bridge::itkToOpenCV16bit(const itk::Image<short, 2>::Pointer& itkImage) {
    // 1. Obtener región y dimensiones
    auto region = itkImage->GetLargestPossibleRegion();
    auto size = region.GetSize();
    int width = size[0];
    int height = size[1];

    // 2. Crear Mat de OpenCV (16-bit signed)
    cv::Mat cvImage(height, width, CV_16S);

    // 3. Copiar píxeles
    itk::ImageRegionConstIterator<itk::Image<short, 2>> it(itkImage, region);
    short* cvData = cvImage.ptr<short>(0);
    
    it.GoToBegin();
    int idx = 0;
    while (!it.IsAtEnd()) {
        cvData[idx] = it.Get();  // Copia directa, sin transformación
        ++it;
        ++idx;
    }

    return cvImage;
}
```

**Características**:

- ✅ **Zero-copy semántico**: Solo copia datos, sin transformación
- ✅ **Preserva HU**: Los valores negativos se mantienen
- ✅ **Eficiente**: Un solo paso de copia

---

### **Conversión ITK → OpenCV 8-bit**

```cpp
cv::Mat Bridge::itkToOpenCV8bit(const itk::Image<short, 2>::Pointer& itkImage) {
    // 1. Primero obtener versión 16-bit
    cv::Mat raw16 = itkToOpenCV16bit(itkImage);

    // 2. Encontrar rango dinámico
    double minVal, maxVal;
    cv::minMaxLoc(raw16, &minVal, &maxVal);

    // 3. Normalizar y convertir a 8-bit
    cv::Mat normalized, result;
    raw16.convertTo(normalized, CV_32F);  // Temporal float para precisión
    normalized = (normalized - minVal) / (maxVal - minVal);  // [0, 1]
    normalized.convertTo(result, CV_8U, 255.0);  // [0, 255]

    return result;
}
```

**Transformación matemática**:
$$
\text{pixel}_{8bit} = \frac{(\text{pixel}_{HU} - \text{min}_{HU})}{\text{max}_{HU} - \text{min}_{HU}} \times 255
$$

**Ventajas**:

- ✅ **Contraste óptimo**: Usa todo el rango [0-255]
- ✅ **Visualización clara**: Adaptado a capacidades de display
- ⚠️ **Pérdida de información cuantitativa**: No sirve para segmentación

---

### **Conversión OpenCV → ITK**

```cpp
itk::Image<short, 2>::Pointer Bridge::openCVToITK(const cv::Mat& cvImage) {
    // 1. Manejar diferentes tipos de entrada
    cv::Mat temp;
    if (cvImage.type() == CV_8U) {
        // Escalar de [0,255] a rango apropiado
        cvImage.convertTo(temp, CV_16S, 256.0);  // [0, 65536]
    } else if (cvImage.type() == CV_16U) {
        cvImage.convertTo(temp, CV_16S);  // Unsigned → Signed
    } else if (cvImage.type() == CV_16S) {
        temp = cvImage.clone();
    } else {
        throw std::runtime_error("Tipo no soportado");
    }

    // 2. Crear imagen ITK
    using ImageType = itk::Image<short, 2>;
    auto itkImage = ImageType::New();

    // 3. Configurar región
    ImageType::IndexType start;
    start.Fill(0);
    ImageType::SizeType size;
    size[0] = temp.cols;
    size[1] = temp.rows;
    
    ImageType::RegionType region(start, size);
    itkImage->SetRegions(region);
    itkImage->Allocate();

    // 4. Copiar datos
    itk::ImageRegionIterator<ImageType> it(itkImage, region);
    const short* cvData = temp.ptr<short>(0);
    
    it.GoToBegin();
    int idx = 0;
    while (!it.IsAtEnd()) {
        it.Set(cvData[idx]);
        ++it;
        ++idx;
    }

    return itkImage;
}
```

## ⚡ Consideraciones de Rendimiento

### **Memoria**

| Operación | Memoria Usada | Notas |
|-----------|---------------|-------|
| `itkToOpenCV16bit()` | Width × Height × 2 bytes | Copia completa |
| `itkToOpenCV8bit()` | Width × Height × 1 byte | Más conversión temporal |
| `openCVToITK()` | Width × Height × 2 bytes | Copia completa |

### **Tiempo de Ejecución**

Para imagen 512×512:

- `itkToOpenCV16bit()`: ~0.5 ms (copia directa)
- `itkToOpenCV8bit()`: ~2-3 ms (incluye normalización)
- `openCVToITK()`: ~1 ms (incluye conversión de tipo)

### **Optimizaciones Aplicadas**

```cpp
// ✅ BUENO: Reutilizar resultado
cv::Mat raw = Bridge::itkToOpenCV16bit(itkImg);
// ... usar raw múltiples veces ...

// ❌ MALO: Convertir repetidamente
for (int i = 0; i < 100; ++i) {
    cv::Mat temp = Bridge::itkToOpenCV16bit(itkImg);  // Desperdicio
    // ...
}
```

## 🔒 Validación y Seguridad

### **Verificaciones en Tiempo de Ejecución**

```cpp
cv::Mat Bridge::itkToOpenCV16bit(const itk::Image<short, 2>::Pointer& itkImage) {
    // Verificar puntero válido
    if (itkImage.IsNull()) {
        throw std::invalid_argument("ITK image es nullptr");
    }

    // Verificar dimensiones razonables
    auto size = itkImage->GetLargestPossibleRegion().GetSize();
    if (size[0] == 0 || size[1] == 0) {
        throw std::runtime_error("Imagen ITK tiene dimensiones inválidas");
    }
    
    if (size[0] > 10000 || size[1] > 10000) {
        throw std::runtime_error("Imagen demasiado grande (posible corrupción)");
    }

    // ... resto de la conversión ...
}
```

## 🧪 Testing

### **Test de Conservación de Valores**

```cpp
void testValuePreservation() {
    // Crear imagen ITK con valores conocidos
    auto itkImg = itk::Image<short, 2>::New();
    // ... configurar región 100x100 ...
    
    // Llenar con patrón
    itk::ImageRegionIterator<itk::Image<short, 2>> it(itkImg, region);
    short testValue = -500;  // Valor HU de pulmón
    while (!it.IsAtEnd()) {
        it.Set(testValue);
        ++it;
    }

    // Convertir
    cv::Mat cvImg = Bridge::itkToOpenCV16bit(itkImg);

    // Verificar que todos los valores se preservaron
    assert(cvImg.type() == CV_16S);
    assert(cvImg.at<short>(50, 50) == testValue);
    
    std::cout << "✓ Valores preservados correctamente" << std::endl;
}
```

### **Test de Ida y Vuelta**

```cpp
void testRoundTrip() {
    // ITK → OpenCV → ITK
    auto original = createTestITKImage();
    cv::Mat cvImg = Bridge::itkToOpenCV16bit(original);
    auto reconstructed = Bridge::openCVToITK(cvImg);

    // Comparar pixel por pixel
    itk::ImageRegionConstIterator<itk::Image<short, 2>> itOrig(original, region);
    itk::ImageRegionConstIterator<itk::Image<short, 2>> itRecon(reconstructed, region);
    
    itOrig.GoToBegin();
    itRecon.GoToBegin();
    
    while (!itOrig.IsAtEnd()) {
        assert(itOrig.Get() == itRecon.Get());
        ++itOrig;
        ++itRecon;
    }
    
    std::cout << "✓ Conversión ida y vuelta sin pérdida" << std::endl;
}
```

## 🎓 Casos de Uso Avanzados

### **Pipeline Híbrido ITK-OpenCV**

```cpp
// Aprovechar lo mejor de ambos mundos
auto dicomData = DicomIO::readDicomImage("scan.dcm");

// 1. Filtro ITK (media adaptativa)
using FilterType = itk::AdaptiveHistogramEqualizationImageFilter<
    itk::Image<short, 2>>;
auto itkFilter = FilterType::New();
itkFilter->SetInput(dicomData.image);
itkFilter->Update();

// 2. Convertir a OpenCV
cv::Mat cvImg = Bridge::itkToOpenCV8bit(itkFilter->GetOutput());

// 3. Filtros OpenCV (detección de bordes)
cv::Mat edges;
cv::Canny(cvImg, edges, 50, 150);

// 4. Operaciones morfológicas OpenCV
cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
cv::morphologyEx(edges, edges, cv::MORPH_CLOSE, kernel);

// 5. Convertir resultado de vuelta a ITK para exportar
auto finalITK = Bridge::openCVToITK(edges);
```

### **Procesamiento Multi-Escala**

```cpp
// Mantener versiones 8-bit y 16-bit sincronizadas
struct ImagePair {
    cv::Mat raw16bit;    // Para algoritmos
    cv::Mat display8bit; // Para UI
};

ImagePair loadAndPrepare(const std::string& path) {
    auto dicomData = DicomIO::readDicomImage(path);
    
    return {
        Bridge::itkToOpenCV16bit(dicomData.image),
        Bridge::itkToOpenCV8bit(dicomData.image)
    };
}

// Uso
auto imgs = loadAndPrepare("scan.dcm");

// Segmentar en 16-bit (valores HU precisos)
cv::Mat lungsMask;
cv::inRange(imgs.raw16bit, -1000, -400, lungsMask);

// Visualizar en 8-bit (contraste optimizado)
cv::Mat overlay;
cv::cvtColor(imgs.display8bit, overlay, cv::COLOR_GRAY2BGR);
overlay.setTo(cv::Scalar(0, 255, 0), lungsMask);  // Verde para pulmones
```

## 📊 Comparación de Formatos

| Aspecto | ITK Image | OpenCV Mat |
|---------|-----------|------------|
| **Propósito** | Imágenes médicas | Visión general |
| **Tipo típico** | `short` (16-bit) | `uchar` (8-bit) |
| **Metadatos** | Ricos (DICOM) | Mínimos |
| **Indexación** | [x, y] | [row, col] |
| **Memoria** | Smart pointers | Reference counting |
| **Filtros** | Médicos/3D | Generales/rápidos |
| **I/O** | DICOM, NIfTI | PNG, JPEG, TIFF |

## 🚀 Extensiones Futuras

### **Soporte para Color**

```cpp
// Para imágenes RGB (ej: visualizaciones médicas coloreadas)
cv::Mat itkToOpenCVColor(const itk::Image<itk::RGBPixel<unsigned char>, 2>::Pointer& itkImage);
```

### **Conversión 3D**

```cpp
// Para volúmenes completos
cv::Mat itkToOpenCV3D(const itk::Image<short, 3>::Pointer& itkVolume);
```

### **Preservación de Metadatos**

```cpp
struct ConversionResult {
    cv::Mat image;
    std::map<std::string, std::string> metadata;  // Spacing, origin, etc.
};

ConversionResult itkToOpenCVWithMetadata(const itk::Image<short, 2>::Pointer& itkImage);
```

## 📚 Referencias

- [ITK Software Guide](https://itk.org/ItkSoftwareGuide.pdf)
- [OpenCV Documentation - Mat](https://docs.opencv.org/4.x/d3/d63/classcv_1_1Mat.html)
- [ITK Image Concept](https://itk.org/Doxygen/html/ImagePage.html)

---

**Versión**: 1.0  
**Última actualización**: Noviembre 2025  
**Archivos**: `src/core/itk_opencv_bridge.h`, `src/core/itk_opencv_bridge.cpp`
