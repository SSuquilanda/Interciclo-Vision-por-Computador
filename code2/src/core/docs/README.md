# 📚 Módulos Core - Backend de Procesamiento Médico

## 📋 Descripción General

Los módulos **Core** constituyen el backend de la aplicación de visión por computador médico. Implementan todos los algoritmos de procesamiento de imágenes CT (Tomografía Computarizada) de forma independiente de la interfaz gráfica, siguiendo los principios de **Arquitectura Limpia**.

## 🏗️ Arquitectura

```bash
src/core/
├── dicom_reader.h/cpp        # Lectura de imágenes DICOM
├── itk_opencv_bridge.h/cpp   # Conversión ITK ↔ OpenCV
├── preprocessing.h/cpp        # Filtros y red neuronal DnCNN
├── segmentation.h/cpp         # Segmentación de órganos
└── morphology.h/cpp           # Operaciones morfológicas
```

## ✨ Características Principales

### 1. **Separación de Responsabilidades**

- Cada módulo tiene una responsabilidad única y bien definida
- Sin dependencias circulares
- Fácil de probar y mantener

### 2. **Independencia de UI**

- Los módulos core NO conocen Qt
- Pueden ser reutilizados en aplicaciones CLI o web
- Testeable sin interfaz gráfica

### 3. **APIs Consistentes**

- Funciones estáticas organizadas en namespaces
- Documentación clara de parámetros y retornos
- Manejo robusto de errores con excepciones

### 4. **Rendimiento Optimizado**

- Uso eficiente de memoria con `cv::Mat`
- Operaciones vectorizadas cuando es posible
- Cacheo de resultados costosos (ej: modelo DnCNN)

## 🔗 Dependencias Externas

### **ITK (Insight Toolkit) 6.0.0**

- Lectura/escritura de formatos médicos (DICOM, NIfTI)
- Manipulación de metadatos médicos
- Filtros especializados para imágenes médicas

### **OpenCV 4.10.0**

- Procesamiento de imágenes general
- Operaciones morfológicas
- Módulo DNN para redes neuronales (DnCNN)
- Visualización y métricas

## 📊 Flujo de Datos Típico

```bash
1. DICOM File
   ↓
2. dicom_reader.cpp (ITK) → Imagen 16-bit + Metadata
   ↓
3. itk_opencv_bridge.cpp → cv::Mat (OpenCV)
   ↓
4. preprocessing.cpp → Filtros + DnCNN
   ↓
5. segmentation.cpp → Máscaras binarias por órgano
   ↓
6. morphology.cpp → Refinamiento de máscaras
   ↓
7. Visualización (UI) + Métricas
```

## 🎯 Casos de Uso

### **Aplicación GUI (Qt6)**

```cpp
// En mainwindow.cpp
auto dicomData = DicomIO::readDicomImage("scan.dcm");
cv::Mat img8bit = Bridge::itkToOpenCV8bit(dicomData.image);
cv::Mat denoised = Preprocessing::applyDnCNN(img8bit, "model.onnx");
auto regions = Segmentation::segmentLungs(dicomData.image);
```

### **Script CLI**

```cpp
// En un main.cpp standalone
int main(int argc, char** argv) {
    auto data = DicomIO::readDicomImage(argv[1]);
    auto lungs = Segmentation::segmentLungs(data.image);
    // Guardar resultados...
    return 0;
}
```

### **Pipeline Batch**

```cpp
// Procesar múltiples estudios
for (auto& file : dicomFiles) {
    auto data = DicomIO::readDicomImage(file);
    cv::Mat processed = Preprocessing::applyCLAHE(
        Bridge::itkToOpenCV8bit(data.image)
    );
    // Análisis automatizado...
}
```

## 📝 Convenciones de Código

### **Estilo de Nombres**

- **Namespaces**: `PascalCase` (ej: `DicomIO`, `Preprocessing`)
- **Funciones**: `camelCase` (ej: `readDicomImage`, `segmentLungs`)
- **Constantes**: `UPPER_SNAKE_CASE` (ej: `MIN_HU_BONE`)

### **Retorno de Valores**

- Imágenes: por valor (`cv::Mat` usa reference counting)
- Estructuras complejas: por valor o `std::vector`
- Datos primitivos: por valor
- Evitar punteros crudos

### **Manejo de Errores**

```cpp
// Lanzar excepciones descriptivas
if (!std::filesystem::exists(path)) {
    throw std::runtime_error("Archivo no encontrado: " + path);
}

// En UI, capturar y mostrar al usuario
try {
    auto data = DicomIO::readDicomImage(path);
} catch (const std::exception& e) {
    QMessageBox::critical(this, "Error", e.what());
}
```

## 🧪 Testing

Cada módulo debe ser testeable de forma aislada:

```cpp
// Ejemplo de test unitario (pseudo-código)
TEST(PreprocessingTest, GaussianFilterReducesNoise) {
    cv::Mat noisy = generateNoisyImage();
    cv::Mat filtered = Preprocessing::applyGaussianFilter(noisy, 5);
    
    double snrBefore = Preprocessing::calculateSNR(noisy);
    double snrAfter = Preprocessing::calculateSNR(filtered);
    
    EXPECT_GT(snrAfter, snrBefore);
}
```

## 📖 Documentación Individual

Para información detallada de cada módulo, consulta:

- [📄 DICOM Reader](./DICOM_READER.md) - Lectura de archivos médicos
- [🔄 ITK-OpenCV Bridge](./ITK_OPENCV_BRIDGE.md) - Conversión de formatos
- [🎨 Preprocessing](./PREPROCESSING.md) - Filtros y DnCNN
- [🔍 Segmentation](./SEGMENTATION.md) - Segmentación de órganos
- [🔧 Morphology](./MORPHOLOGY.md) - Operaciones morfológicas

## 🚀 Extensibilidad

Para agregar nuevos algoritmos:

1. **Crear nueva función** en el namespace apropiado
2. **Documentar** parámetros y comportamiento
3. **Testear** con casos de uso reales
4. **Integrar** en la UI si es necesario

Ejemplo:

```cpp
// En segmentation.h
namespace Segmentation {
    // ... funciones existentes ...
    
    /**
     * @brief Segmenta el hígado usando umbralización adaptativa
     * @param image Imagen CT 16-bit con valores HU
     * @return Vector de regiones hepáticas detectadas
     */
    std::vector<SegmentedRegion> segmentLiver(const itk::Image<short, 2>::Pointer& image);
}
```

## 📊 Métricas de Código

- **Líneas de código**: ~2000 (5 módulos)
- **Funciones públicas**: ~25
- **Cobertura de casos de uso**: Medical CT workflows
- **Dependencias externas**: ITK, OpenCV

## 🔐 Consideraciones de Seguridad

- **Validación de entrada**: Siempre verificar que archivos existan
- **Sanitización**: No confiar en metadatos DICOM sin validar
- **Límites de memoria**: Controlar tamaño de imágenes cargadas
- **PHI (Protected Health Information)**: Respetar privacidad de pacientes

## 🎓 Referencias

- [ITK Documentation](https://itk.org/Doxygen/html/)
- [OpenCV Documentation](https://docs.opencv.org/)
- [DICOM Standard](https://www.dicomstandard.org/)
- [Hounsfield Units](https://en.wikipedia.org/wiki/Hounsfield_scale)

---

**Versión**: 1.0  
**Última actualización**: Noviembre 2025  
**Autores**: Equipo de Visión por Computador
