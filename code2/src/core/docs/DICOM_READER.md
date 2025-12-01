# 📄 DICOM Reader - Lectura de Imágenes Médicas

## 📋 Descripción

El módulo **DICOM Reader** (`dicom_reader.h/cpp`) se encarga de leer archivos de imagen médica en formato DICOM (Digital Imaging and Communications in Medicine), extraer los datos de píxeles y metadatos relevantes para su procesamiento posterior.

## 🎯 Responsabilidad

> **Única responsabilidad**: Cargar imágenes DICOM desde disco y convertirlas a formato ITK

## 🏗️ Arquitectura

```cpp
namespace DicomIO {
    struct DicomData {
        itk::Image<short, 2>::Pointer image;  // Imagen 16-bit con valores HU
        std::string patientName;               // Nombre del paciente
        std::string studyDate;                 // Fecha del estudio
        std::string modality;                  // Modalidad (CT, MRI, etc.)
        std::map<std::string, std::string> metadata;  // Otros metadatos
    };

    DicomData readDicomImage(const std::string& filePath);
}
```

## 📚 API Pública

### `readDicomImage(const std::string& filePath)`

**Descripción**: Lee un archivo DICOM y extrae imagen + metadatos

**Parámetros**:

- `filePath`: Ruta absoluta al archivo `.dcm` o `.IMA`

**Retorna**:

- `DicomData`: Estructura con imagen ITK y metadatos

**Excepciones**:

- `std::runtime_error`: Si el archivo no existe o no es DICOM válido

**Ejemplo de uso**:

```cpp
try {
    auto data = DicomIO::readDicomImage("/data/scan001.dcm");
    
    std::cout << "Paciente: " << data.patientName << std::endl;
    std::cout << "Modalidad: " << data.modality << std::endl;
    
    // Obtener dimensiones
    auto size = data.image->GetLargestPossibleRegion().GetSize();
    std::cout << "Dimensiones: " << size[0] << "x" << size[1] << std::endl;
    
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

## 🔍 Implementación Interna

### **Pipeline de Lectura**

```bash
1. Verificar que archivo existe
   ↓
2. Crear ImageFileReader de ITK
   ↓
3. Configurar ImageIO (GDCM para DICOM)
   ↓
4. Ejecutar lectura (reader->Update())
   ↓
5. Extraer metadatos con MetaDataDictionary
   ↓
6. Retornar DicomData completo
```

### **Código Simplificado**

```cpp
DicomData DicomIO::readDicomImage(const std::string& filePath) {
    // 1. Validación
    if (!std::filesystem::exists(filePath)) {
        throw std::runtime_error("Archivo no encontrado: " + filePath);
    }

    // 2. Crear lector ITK
    using ImageType = itk::Image<short, 2>;
    using ReaderType = itk::ImageFileReader<ImageType>;
    auto reader = ReaderType::New();
    reader->SetFileName(filePath);

    // 3. Configurar GDCM IO para DICOM
    auto dicomIO = itk::GDCMImageIO::New();
    reader->SetImageIO(dicomIO);

    // 4. Leer imagen
    try {
        reader->Update();
    } catch (const itk::ExceptionObject& ex) {
        throw std::runtime_error("Error leyendo DICOM: " + std::string(ex.what()));
    }

    // 5. Extraer metadatos
    DicomData data;
    data.image = reader->GetOutput();
    
    const auto& dict = dicomIO->GetMetaDataDictionary();
    
    // Patient Name (0010,0010)
    std::string patientName;
    if (itk::ExposeMetaData<std::string>(dict, "0010|0010", patientName)) {
        data.patientName = patientName;
    }
    
    // Study Date (0008,0020)
    std::string studyDate;
    if (itk::ExposeMetaData<std::string>(dict, "0008|0020", studyDate)) {
        data.studyDate = studyDate;
    }
    
    // Modality (0008,0060)
    std::string modality;
    if (itk::ExposeMetaData<std::string>(dict, "0008|0060", modality)) {
        data.modality = modality;
    }

    return data;
}
```

## 📊 Formato de Datos

### **Imagen ITK**

- **Tipo**: `itk::Image<short, 2>::Pointer`
- **Profundidad**: 16-bit signed (`short`)
- **Dimensiones**: 2D (una slice de CT)
- **Valores**: Hounsfield Units (HU)
  - Aire: -1000 HU
  - Agua: 0 HU
  - Tejido blando: +20 a +80 HU
  - Hueso: +200 a +3000 HU

### **Metadatos DICOM**

| Campo | Tag DICOM | Tipo | Ejemplo |
|-------|-----------|------|---------|
| Patient Name | (0010,0010) | String | "DOE^JOHN" |
| Study Date | (0008,0020) | String | "20231115" |
| Modality | (0008,0060) | String | "CT" |
| Series Description | (0008,103E) | String | "Chest CT" |
| Slice Thickness | (0018,0050) | Float | "5.0" |

## 🔧 Configuración

### **Dependencias**

```cmake
# En CMakeLists.txt
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

# Componentes necesarios:
# - ITKCommon
# - ITKIOGDCM (lectura DICOM)
# - ITKIOImageBase
```

### **Includes Necesarios**

```cpp
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkGDCMImageIO.h>
#include <itkMetaDataDictionary.h>
#include <itkMetaDataObject.h>
```

## ⚡ Optimizaciones

### **Lectura Eficiente**

- ITK usa **lazy loading**: la imagen se carga solo al llamar `Update()`
- Para series completas, usar `ImageSeriesReader` en lugar de leer slice por slice

### **Caché de Metadatos**

```cpp
// Si se va a leer el mismo archivo múltiples veces:
static std::map<std::string, DicomData> cache;

DicomData readDicomImageCached(const std::string& path) {
    if (cache.find(path) != cache.end()) {
        return cache[path];
    }
    auto data = DicomIO::readDicomImage(path);
    cache[path] = data;
    return data;
}
```

## 🐛 Manejo de Errores

### **Errores Comunes**

- Archivo no existe**

```cpp
Error: Archivo no encontrado: /data/missing.dcm
```

- Formato inválido**

```cpp
Error: El archivo no es un DICOM válido
```

- DICOM corrupto**

```cpp
Error leyendo DICOM: itk::ImageFileReaderException (0x...)
```

### **Validación Robusta**

```cpp
DicomData readDicomImageSafe(const std::string& filePath) {
    // Verificar extensión
    std::filesystem::path p(filePath);
    auto ext = p.extension().string();
    if (ext != ".dcm" && ext != ".IMA" && ext != ".DCM") {
        std::cerr << "Advertencia: extensión inusual: " << ext << std::endl;
    }
    
    // Verificar tamaño mínimo
    auto fileSize = std::filesystem::file_size(filePath);
    if (fileSize < 1024) {  // DICOMs son típicamente > 1KB
        throw std::runtime_error("Archivo demasiado pequeño para ser DICOM");
    }
    
    return DicomIO::readDicomImage(filePath);
}
```

## 📖 Estándar DICOM

### **Tags Útiles Adicionales**

```cpp
// Window Center/Width (para visualización)
"0028|1050"  // Window Center
"0028|1051"  // Window Width

// Información del equipo
"0008|0070"  // Manufacturer
"0008|1090"  // Manufacturer's Model Name

// Parámetros de adquisición (CT)
"0018|0060"  // KVP (kilo voltage peak)
"0018|1152"  // Exposure (mAs)
"0018|0050"  // Slice Thickness
```

### **Lectura de Tags Personalizados**

```cpp
void extractAllMetadata(const itk::GDCMImageIO::Pointer& dicomIO, 
                        DicomData& data) {
    const auto& dict = dicomIO->GetMetaDataDictionary();
    auto itr = dict.Begin();
    
    while (itr != dict.End()) {
        std::string key = itr->first;
        std::string value;
        
        if (itk::ExposeMetaData<std::string>(dict, key, value)) {
            data.metadata[key] = value;
        }
        
        ++itr;
    }
}
```

## 🧪 Testing

### **Test Básico**

```cpp
void testDicomReading() {
    auto data = DicomIO::readDicomImage("test_data/sample.dcm");
    
    assert(data.image != nullptr);
    assert(!data.patientName.empty());
    assert(data.modality == "CT");
    
    auto size = data.image->GetLargestPossibleRegion().GetSize();
    assert(size[0] > 0 && size[1] > 0);
    
    std::cout << "✓ Test de lectura DICOM pasado" << std::endl;
}
```

### **Test de Serie Completa**

```cpp
void testSeriesReading() {
    std::vector<std::string> files = {
        "slice001.dcm", "slice002.dcm", "slice003.dcm"
    };
    
    std::vector<DicomData> slices;
    for (const auto& file : files) {
        slices.push_back(DicomIO::readDicomImage(file));
    }
    
    // Verificar que todas las slices sean del mismo paciente
    for (size_t i = 1; i < slices.size(); ++i) {
        assert(slices[i].patientName == slices[0].patientName);
    }
    
    std::cout << "✓ Test de serie completa pasado" << std::endl;
}
```

## 🚀 Extensiones Futuras

### **Soporte para 3D**

```cpp
// Para leer volúmenes completos
using ImageType3D = itk::Image<short, 3>;
ImageType3D::Pointer readDicomSeries(const std::string& directory);
```

### **Conversión a NIfTI**

```cpp
// Para exportar a formatos de investigación
void convertDicomToNifti(const std::string& dicomPath, 
                         const std::string& niftiPath);
```

### **Anonimización**

```cpp
// Remover información identificable del paciente
DicomData anonymizeDicom(const DicomData& data);
```

## 📚 Referencias

- [DICOM Standard](https://www.dicomstandard.org/)
- [ITK GDCM Documentation](https://itk.org/Doxygen/html/classitk_1_1GDCMImageIO.html)
- [Hounsfield Units](https://radiopaedia.org/articles/hounsfield-unit)
- [DICOM Tag Browser](https://dicom.innolitics.com/ciods)

## 🎓 Conceptos Clave

### **¿Qué son los Hounsfield Units?**

Escala cuantitativa para describir radiodensidad en TC:

- **HU = ((μ_tissue - μ_water) / μ_water) × 1000**
- Agua = 0 HU por definición
- Permite segmentación precisa de tejidos

### **¿Por qué 16-bit signed?**

- Rango HU: típicamente -1024 a +3071
- `short` cubre -32768 a +32767
- Suficiente para todos los valores clínicos

---

**Versión**: 1.0  
**Última actualización**: Noviembre 2025  
**Archivo**: `src/core/dicom_reader.h`, `src/core/dicom_reader.cpp`
