#ifndef DICOM_READER_H
#define DICOM_READER_H

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkGDCMImageIO.h"
#include "itkMetaDataObject.h"
#include <string>
#include <map>

namespace DicomIO {

// Tipos básicos de imagen
const unsigned int Dimension = 2;
using PixelType = short;
using ImageType = itk::Image<PixelType, Dimension>;
using ImagePointer = ImageType::Pointer;

// Lee un archivo DICOM y retorna un puntero a la imagen ITK
ImagePointer readDicomImage(const std::string& filename);

// Extrae metadata de un archivo DICOM
std::map<std::string, std::string> extractMetadata(itk::ImageFileReader<ImageType>::Pointer reader);

// Muestra información básica de la imagen y metadata
void displayImageInfo(ImagePointer image, const std::map<std::string, std::string>& metadata);

// Calcular y muestra estadísticas de la imagen
void displayImageStatistics(ImagePointer image);

} // namespace DicomIO

#endif // DICOM_READER_H
