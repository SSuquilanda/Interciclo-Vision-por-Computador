# Ground Truth - Máscaras de Validación

Esta carpeta contiene las máscaras manuales (Ground Truth) para validar la precisión de las segmentaciones automáticas.

## 📁 Estructura de Archivos

Coloca aquí las imágenes de máscaras manuales creadas por un médico o experto. Las máscaras deben ser:

- **Formato**: PNG, JPG, JPEG, BMP, TIFF
- **Tipo**: Escala de grises (grayscale)
- **Valores**:
  - `0` (negro) = fondo
  - `255` (blanco) = región segmentada
- **Dimensiones**: Deben coincidir exactamente con las dimensiones de la imagen DICOM original

## 🎯 Uso en la Aplicación

1. **Realizar Segmentación Automática**:
   - Ve a la pestaña **Segmentación**
   - Segmenta la estructura deseada (Pulmones, Huesos o Aorta)
   - Opcionalmente, aplica operaciones morfológicas en la pestaña **Morfología**

2. **Cargar Ground Truth**:
   - Ve a la pestaña **Métricas**
   - Haz clic en el botón **📋 Cargar Validación (Ground Truth)**
   - Selecciona la imagen de Ground Truth correspondiente desde esta carpeta
   - El sistema calculará automáticamente el **IoU (Intersection over Union)**

3. **Ver Resultados**:
   - El valor de **Precisión (IoU %)** aparecerá en la tabla de métricas
   - Se mostrará un mensaje con estadísticas detalladas
   - La barra de estado mostrará: "✓ Validación completada: IoU = XX.XX%"

## 📊 Métrica IoU (Intersection over Union)

La métrica IoU mide la similitud entre dos máscaras:

```bash
IoU = (Intersección / Unión) × 100%
```

**Interpretación**:

- **90-100%**: Excelente precisión
- **80-90%**: Muy buena precisión
- **70-80%**: Buena precisión
- **60-70%**: Precisión aceptable
- **< 60%**: Precisión insuficiente

## 📝 Ejemplo de Nombres de Archivos

```bash
pulmones_slice_100.png
huesos_slice_100.png
aorta_slice_100.png
columna_vertebral_ground_truth.png
```

## ⚠️ Notas Importantes

1. **Dimensiones**: Si las dimensiones no coinciden, aparecerá un mensaje de error
2. **Binarización**: Las máscaras se binarizan automáticamente (umbral 127)
3. **Prioridad**: Si existe máscara morfológica, se usa esa; si no, se usa la máscara de segmentación
4. **Formato**: Se recomienda PNG para evitar pérdida de calidad por compresión

## 🔬 Ejemplo de Workflow

```bash
1. Dataset DICOM → Pestaña I/O
2. Preprocesamiento → Aplicar DnCNN, CLAHE, etc.
3. Segmentación → Segmentar estructura
4. Morfología → Refinar máscara (opcional)
5. Métricas → Calcular métricas
6. Validación → Cargar Ground Truth
   → Ver IoU en la tabla
```

## 🛠️ Crear Ground Truth

Para crear máscaras Ground Truth manualmente:

1. Abrir imagen DICOM en un software médico (ITK-SNAP, 3D Slicer, etc.)
2. Segmentar manualmente la estructura deseada
3. Exportar máscara como imagen PNG grayscale
4. Guardar en esta carpeta con nombre descriptivo
5. Usar en la aplicación para validar

---

**Directorio**: `code2/src/core/img/`  
**Acceso desde ejecutable**: `../src/core/img/` (relativo a `build/`)
