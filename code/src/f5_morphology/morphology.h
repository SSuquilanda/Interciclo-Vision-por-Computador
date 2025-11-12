#ifndef MORPHOLOGY_H
#define MORPHOLOGY_H

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

/**
 * @file morphology.h
 * @brief Operaciones morfológicas para procesamiento de imágenes médicas
 * 
 * FASE 5: Morfología Matemática
 * 
 * Este módulo implementa operaciones morfológicas para refinar y mejorar
 * las segmentaciones obtenidas en la Fase 4.
 * 
 * Operaciones disponibles:
 * - Operaciones básicas: Erosión, Dilatación, Apertura, Cierre
 * - Operaciones avanzadas: Gradiente morfológico, Top-hat, Black-hat
 * - Hit-or-Miss para detección de patrones
 * - Esqueletización y adelgazamiento
 * - Reconstrucción morfológica
 * - Pipeline de refinamiento
 */

namespace Morphology {

// ============================================================================
// ENUMERACIONES Y ESTRUCTURAS
// ============================================================================

/**
 * @brief Formas de elementos estructurantes
 */
enum StructuringElementShape {
    MORPH_RECT,      ///< Rectangular
    MORPH_ELLIPSE,   ///< Elíptico (circular)
    MORPH_CROSS      ///< Cruz
};

/**
 * @brief Tipos de operaciones morfológicas
 */
enum MorphOperation {
    MORPH_ERODE,        ///< Erosión
    MORPH_DILATE,       ///< Dilatación
    MORPH_OPEN,         ///< Apertura (erosión + dilatación)
    MORPH_CLOSE,        ///< Cierre (dilatación + erosión)
    MORPH_GRADIENT,     ///< Gradiente morfológico
    MORPH_TOPHAT,       ///< Top-hat (diferencia entre imagen y apertura)
    MORPH_BLACKHAT,     ///< Black-hat (diferencia entre cierre e imagen)
    MORPH_HITMISS       ///< Hit-or-Miss
};

/**
 * @brief Parámetros para operaciones morfológicas
 */
struct MorphParams {
    StructuringElementShape shape = MORPH_ELLIPSE;
    cv::Size kernelSize = cv::Size(5, 5);
    int iterations = 1;
    cv::Point anchor = cv::Point(-1, -1);  ///< Centro del kernel por defecto
};

// ============================================================================
// CREACIÓN DE ELEMENTOS ESTRUCTURANTES
// ============================================================================

/**
 * @brief Crea un elemento estructurante
 * @param shape Forma del elemento (rectangular, elíptico, cruz)
 * @param size Tamaño del elemento
 * @param anchor Punto de anclaje (centro por defecto)
 * @return Elemento estructurante como cv::Mat
 */
cv::Mat createStructuringElement(StructuringElementShape shape, 
                                  cv::Size size,
                                  cv::Point anchor = cv::Point(-1, -1));

/**
 * @brief Crea un elemento estructurante personalizado
 * @param kernel Matriz binaria con la forma deseada
 * @return Elemento estructurante
 */
cv::Mat createCustomKernel(const cv::Mat& kernel);

// ============================================================================
// OPERACIONES MORFOLÓGICAS BÁSICAS
// ============================================================================

/**
 * @brief Erosión morfológica
 * 
 * Reduce el tamaño de las regiones blancas, elimina pequeños objetos.
 * Útil para separar objetos conectados y eliminar ruido.
 * 
 * @param image Imagen binaria de entrada
 * @param kernelSize Tamaño del elemento estructurante
 * @param shape Forma del elemento estructurante
 * @param iterations Número de veces que se aplica la erosión
 * @return Imagen erosionada
 */
cv::Mat erode(const cv::Mat& image, 
              cv::Size kernelSize = cv::Size(3, 3),
              StructuringElementShape shape = MORPH_ELLIPSE,
              int iterations = 1);

/**
 * @brief Dilatación morfológica
 * 
 * Expande las regiones blancas, rellena pequeños huecos.
 * Útil para conectar componentes y rellenar agujeros.
 * 
 * @param image Imagen binaria de entrada
 * @param kernelSize Tamaño del elemento estructurante
 * @param shape Forma del elemento estructurante
 * @param iterations Número de veces que se aplica la dilatación
 * @return Imagen dilatada
 */
cv::Mat dilate(const cv::Mat& image, 
               cv::Size kernelSize = cv::Size(3, 3),
               StructuringElementShape shape = MORPH_ELLIPSE,
               int iterations = 1);

/**
 * @brief Apertura morfológica (erosión seguida de dilatación)
 * 
 * Elimina pequeños objetos y ruido manteniendo el tamaño de objetos grandes.
 * Útil para limpiar máscaras de segmentación.
 * 
 * @param image Imagen binaria de entrada
 * @param kernelSize Tamaño del elemento estructurante
 * @param shape Forma del elemento estructurante
 * @return Imagen con apertura aplicada
 */
cv::Mat opening(const cv::Mat& image, 
                cv::Size kernelSize = cv::Size(5, 5),
                StructuringElementShape shape = MORPH_ELLIPSE);

/**
 * @brief Cierre morfológico (dilatación seguida de erosión)
 * 
 * Rellena pequeños huecos y conecta componentes cercanos.
 * Útil para completar contornos y eliminar huecos internos.
 * 
 * @param image Imagen binaria de entrada
 * @param kernelSize Tamaño del elemento estructurante
 * @param shape Forma del elemento estructurante
 * @return Imagen con cierre aplicado
 */
cv::Mat closing(const cv::Mat& image, 
                cv::Size kernelSize = cv::Size(5, 5),
                StructuringElementShape shape = MORPH_ELLIPSE);

// ============================================================================
// OPERACIONES MORFOLÓGICAS AVANZADAS
// ============================================================================

/**
 * @brief Gradiente morfológico
 * 
 * Diferencia entre dilatación y erosión.
 * Resalta los bordes de los objetos.
 * 
 * @param image Imagen de entrada
 * @param kernelSize Tamaño del elemento estructurante
 * @param shape Forma del elemento estructurante
 * @return Gradiente morfológico
 */
cv::Mat morphologicalGradient(const cv::Mat& image, 
                               cv::Size kernelSize = cv::Size(3, 3),
                               StructuringElementShape shape = MORPH_ELLIPSE);

/**
 * @brief Top-hat (sombrero de copa)
 * 
 * Diferencia entre la imagen original y su apertura.
 * Resalta estructuras pequeñas más brillantes que su entorno.
 * 
 * @param image Imagen de entrada
 * @param kernelSize Tamaño del elemento estructurante
 * @param shape Forma del elemento estructurante
 * @return Transformada top-hat
 */
cv::Mat topHat(const cv::Mat& image, 
               cv::Size kernelSize = cv::Size(9, 9),
               StructuringElementShape shape = MORPH_ELLIPSE);

/**
 * @brief Black-hat (sombrero negro)
 * 
 * Diferencia entre el cierre de la imagen y la imagen original.
 * Resalta estructuras pequeñas más oscuras que su entorno.
 * 
 * @param image Imagen de entrada
 * @param kernelSize Tamaño del elemento estructurante
 * @param shape Forma del elemento estructurante
 * @return Transformada black-hat
 */
cv::Mat blackHat(const cv::Mat& image, 
                 cv::Size kernelSize = cv::Size(9, 9),
                 StructuringElementShape shape = MORPH_ELLIPSE);

/**
 * @brief Hit-or-Miss para detección de patrones
 * 
 * Detecta patrones específicos en imágenes binarias.
 * Útil para detectar esquinas, terminaciones, bifurcaciones.
 * 
 * @param image Imagen binaria de entrada
 * @param kernel1 Elemento estructurante para puntos del objeto
 * @param kernel2 Elemento estructurante para puntos del fondo
 * @return Imagen con patrones detectados
 */
cv::Mat hitOrMiss(const cv::Mat& image, 
                  const cv::Mat& kernel1, 
                  const cv::Mat& kernel2);

// ============================================================================
// ESQUELETIZACIÓN Y ADELGAZAMIENTO
// ============================================================================

/**
 * @brief Esqueletización (thinning)
 * 
 * Reduce objetos a su esqueleto de un píxel de ancho.
 * Preserva la topología del objeto.
 * 
 * @param image Imagen binaria de entrada
 * @return Esqueleto de la imagen
 */
cv::Mat skeletonize(const cv::Mat& image);

/**
 * @brief Adelgazamiento (thinning con Zhang-Suen)
 * 
 * Algoritmo de adelgazamiento que preserva conectividad.
 * 
 * @param image Imagen binaria de entrada
 * @param iterations Número máximo de iteraciones
 * @return Imagen adelgazada
 */
cv::Mat thinning(const cv::Mat& image, int iterations = -1);

// ============================================================================
// RECONSTRUCCIÓN MORFOLÓGICA
// ============================================================================

/**
 * @brief Reconstrucción morfológica por dilatación
 * 
 * Reconstruye objetos de una imagen a partir de marcadores.
 * Útil para eliminar objetos que no están conectados a los marcadores.
 * 
 * @param marker Imagen marcador (objetos de interés)
 * @param mask Imagen máscara (límite de la reconstrucción)
 * @param iterations Número máximo de iteraciones (-1 = hasta convergencia)
 * @return Imagen reconstruida
 */
cv::Mat morphologicalReconstruction(const cv::Mat& marker, 
                                     const cv::Mat& mask,
                                     int iterations = -1);

/**
 * @brief Relleno de huecos
 * 
 * Rellena huecos en objetos binarios.
 * Usa reconstrucción morfológica.
 * 
 * @param image Imagen binaria de entrada
 * @return Imagen con huecos rellenados
 */
cv::Mat fillHoles(const cv::Mat& image);

/**
 * @brief Eliminación de bordes
 * 
 * Elimina objetos que tocan el borde de la imagen.
 * 
 * @param image Imagen binaria de entrada
 * @return Imagen sin objetos en los bordes
 */
cv::Mat clearBorder(const cv::Mat& image);

// ============================================================================
// REFINAMIENTO DE SEGMENTACIONES
// ============================================================================

/**
 * @brief Limpieza de máscara binaria
 * 
 * Aplica una secuencia de operaciones para limpiar una máscara:
 * - Elimina ruido con apertura
 * - Rellena huecos con cierre
 * - Elimina objetos pequeños
 * 
 * @param mask Máscara binaria de entrada
 * @param params Parámetros morfológicos
 * @param minObjectSize Área mínima de objetos a mantener
 * @return Máscara limpia
 */
cv::Mat cleanMask(const cv::Mat& mask, 
                  const MorphParams& params = MorphParams(),
                  int minObjectSize = 100);

/**
 * @brief Suavizado de contornos
 * 
 * Suaviza los bordes de una máscara binaria.
 * Aplica cierre seguido de apertura con elementos grandes.
 * 
 * @param mask Máscara binaria de entrada
 * @param kernelSize Tamaño del elemento estructurante
 * @return Máscara con contornos suavizados
 */
cv::Mat smoothContours(const cv::Mat& mask, 
                       cv::Size kernelSize = cv::Size(7, 7));

/**
 * @brief Separación de objetos conectados
 * 
 * Intenta separar objetos que están conectados incorrectamente.
 * Usa erosión iterativa seguida de reconstrucción.
 * 
 * @param mask Máscara binaria con objetos a separar
 * @param erosionSize Tamaño de la erosión
 * @return Máscara con objetos separados
 */
cv::Mat separateObjects(const cv::Mat& mask, 
                        cv::Size erosionSize = cv::Size(3, 3));

/**
 * @brief Refinamiento completo de segmentación
 * 
 * Pipeline completo de refinamiento morfológico:
 * 1. Limpieza inicial (eliminación de ruido)
 * 2. Relleno de huecos
 * 3. Suavizado de contornos
 * 4. Eliminación de objetos pequeños
 * 5. Eliminación de objetos en bordes (opcional)
 * 
 * @param mask Máscara binaria de entrada
 * @param params Parámetros morfológicos
 * @param minObjectSize Área mínima de objetos a mantener
 * @param removeBorderObjects Si eliminar objetos que tocan los bordes
 * @return Máscara refinada
 */
cv::Mat refineSegmentation(const cv::Mat& mask, 
                           const MorphParams& params = MorphParams(),
                           int minObjectSize = 100,
                           bool removeBorderObjects = false);

// ============================================================================
// OPERACIONES CON MÁSCARAS MÚLTIPLES
// ============================================================================

/**
 * @brief Intersección de múltiples máscaras
 * @param masks Vector de máscaras binarias
 * @return Máscara con la intersección (AND)
 */
cv::Mat intersectMasks(const std::vector<cv::Mat>& masks);

/**
 * @brief Unión de múltiples máscaras
 * @param masks Vector de máscaras binarias
 * @return Máscara con la unión (OR)
 */
cv::Mat unionMasks(const std::vector<cv::Mat>& masks);

/**
 * @brief Diferencia entre dos máscaras
 * @param mask1 Primera máscara
 * @param mask2 Segunda máscara
 * @return mask1 - mask2
 */
cv::Mat subtractMasks(const cv::Mat& mask1, const cv::Mat& mask2);

// ============================================================================
// ANÁLISIS MORFOMÉTRICO
// ============================================================================

/**
 * @brief Calcula el área de objetos en una máscara
 * @param mask Máscara binaria
 * @return Vector con el área de cada objeto
 */
std::vector<double> calculateAreas(const cv::Mat& mask);

/**
 * @brief Calcula el perímetro de objetos en una máscara
 * @param mask Máscara binaria
 * @return Vector con el perímetro de cada objeto
 */
std::vector<double> calculatePerimeters(const cv::Mat& mask);

/**
 * @brief Calcula la circularidad de objetos (4π*área/perímetro²)
 * @param mask Máscara binaria
 * @return Vector con la circularidad de cada objeto (1.0 = círculo perfecto)
 */
std::vector<double> calculateCircularity(const cv::Mat& mask);

// ============================================================================
// UTILIDADES
// ============================================================================

/**
 * @brief Invierte una máscara binaria
 * @param mask Máscara a invertir
 * @return Máscara invertida
 */
cv::Mat invertMask(const cv::Mat& mask);

/**
 * @brief Convierte una máscara a contornos
 * @param mask Máscara binaria
 * @return Vector de contornos
 */
std::vector<std::vector<cv::Point>> maskToContours(const cv::Mat& mask);

/**
 * @brief Visualiza operación morfológica (antes y después)
 * @param original Imagen original
 * @param processed Imagen procesada
 * @param windowName Nombre de la ventana
 */
void visualizeMorphOperation(const cv::Mat& original, 
                              const cv::Mat& processed,
                              const std::string& windowName = "Morfología");

} // namespace Morphology

#endif // MORPHOLOGY_H
