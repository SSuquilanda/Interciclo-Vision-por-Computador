#include "dicom_reader.h"
#include "itkGDCMImageIOFactory.h"
#include "itkStatisticsImageFilter.h"
#include <iostream>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

namespace DicomIO {

ImagePointer readDicomImage(const std::string& filename) {
    // Registrar la fábrica DICOM
    static bool factoryRegistered = false;
    if (!factoryRegistered) {
        itk::GDCMImageIOFactory::RegisterOneFactory();
        factoryRegistered = true;
    }

    // Verificar existencia del archivo
    std::cout << "Ruta absoluta del archivo DICOM:\n  " << filename << "\n";
    if (!fs::exists(filename)) {
        throw std::runtime_error("El archivo no existe: " + filename);
    }

    // Lector
    using ReaderType = itk::ImageFileReader<ImageType>;
    auto reader = ReaderType::New();
    reader->SetFileName(filename);

    // Leer el archivo
    try {
        reader->Update();
    }
    catch (itk::ExceptionObject& ex) {
        throw std::runtime_error("Error al leer el archivo DICOM: " + std::string(ex.GetDescription()));
    }

    return reader->GetOutput();
}

std::map<std::string, std::string> extractMetadata(itk::ImageFileReader<ImageType>::Pointer reader) {
    std::map<std::string, std::string> metadata;
    
    itk::MetaDataDictionary& dictionary = reader->GetImageIO()->GetMetaDataDictionary();

    // Tags DICOM
    std::map<std::string, std::string> tags = {
        {"0010|0010", "PatientName"},
        {"0010|0020", "PatientID"},
        {"0008|1030", "StudyDescription"},
        {"0008|0060", "Modality"},
        {"0018|0050", "SliceThickness"},
        {"0020|0013", "InstanceNumber"}
    };

    for (const auto& [tag, name] : tags) {
        std::string value;
        if (itk::ExposeMetaData<std::string>(dictionary, tag, value)) {
            metadata[name] = value;
        }
    }

    return metadata;
}

void displayImageInfo(ImagePointer image, const std::map<std::string, std::string>& metadata) {
    std::cout << "Metadata DICOM \n";
    for (const auto& [key, value] : metadata) {
        std::cout << "  " << key << ": " << value << "\n";
    }

    std::cout << "\nPropiedades de la Imagen \n";
    std::cout << "  Dimensiones: " << image->GetLargestPossibleRegion().GetSize() << "\n";
    std::cout << "  Espaciado: " << image->GetSpacing() << "\n";
    std::cout << "  Origen: " << image->GetOrigin() << "\n";
}

void displayImageStatistics(ImagePointer image) {
    using StatsFilterType = itk::StatisticsImageFilter<ImageType>;
    auto statsFilter = StatsFilterType::New();
    statsFilter->SetInput(image);
    statsFilter->Update();

    std::cout << "\nEstadisticas de la Imagen \n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  Minimo (HU): " << statsFilter->GetMinimum() << "\n";
    std::cout << "  Maximo (HU): " << statsFilter->GetMaximum() << "\n";
    std::cout << "  Media (HU): " << statsFilter->GetMean() << "\n";
    std::cout << "  Desviacion Estandar: " << statsFilter->GetSigma() << "\n";
    std::cout << "  Varianza: " << statsFilter->GetVariance() << "\n\n";
}

} // namespace DicomIO
