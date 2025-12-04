# 🧠 DnCNN Flask Server

Servidor Flask para inferencia de red neuronal DnCNN (denoising).

## 📋 Descripción

Este servidor proporciona una API REST para aplicar denoising con DnCNN a imágenes médicas. La aplicación C++ usa este servidor como **método principal**, con fallback automático a OpenCV DNN local si el servidor no está disponible.

## 🏗️ Arquitectura

```bash
┌─────────────────┐         HTTP POST          ┌──────────────────┐
│                 │    /denoise (image.png)     │                  │
│  C++ App        │ ──────────────────────────> │  Flask Server    │
│  (Qt + OpenCV)  │                             │  (Python)        │
│                 │ <────────────────────────── │                  │
└─────────────────┘    PNG denoised image       └──────────────────┘
        │                                                │
        │ (si falla Flask)                             │
        ↓                                                ↓
┌─────────────────┐                            ┌──────────────────┐
│  OpenCV DNN     │                            │  DnCNN Model     │
│  (Fallback)     │                            │  (.onnx)         │
└─────────────────┘                            └──────────────────┘
```

## 🚀 Inicio Rápido

### 1. Instalar dependencias de Python

```bash
pip install flask opencv-python numpy
```

### 2. Iniciar el servidor

```bash
cd src/server
./start_server.sh
```

O manualmente:

```bash
python3 app.py
```

### 3. Ejecutar la aplicación C++

En otra terminal:

```bash
cd ../../build
./MedicalApp ../data/L291_fd/L291_FD_3_1.CT.0005.0100.2015.12.23.17.48.23.868235.130270509.IMA
```

## 📡 API Endpoint

### `POST /denoise`

Aplica DnCNN denoising a una imagen.

**Request:**

- Method: `POST`
- Content-Type: `multipart/form-data`
- Body: `image` (file, PNG/JPG)

**Response:**

- Content-Type: `image/png`
- Body: Imagen procesada en formato PNG

**Ejemplo con curl:**

```bash
curl -X POST \
  http://localhost:5000/denoise \
  -F "image=@test.png" \
  --output denoised.png
```

## 🔧 Configuración

### Cambiar puerto

Editar `app.py`:

```python
if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=False)  # Cambiar 5000
```

Y en `src/ui/mainwindow.cpp`:

```cpp
dncnnDenoiser.setFlaskServer("http://localhost:NUEVO_PUERTO/denoise");
```

### Cambiar modelo

Editar `app.py`:

```python
net = cv2.dnn.readNetFromONNX("ruta/a/tu/modelo.onnx")
```

## 🧪 Testing

### Test manual del servidor

```bash
# Terminal 1: Iniciar servidor
cd src/server
python3 app.py

# Terminal 2: Enviar imagen de prueba
curl -X POST http://localhost:5000/denoise \
  -F "image=@../../data/test_image.png" \
  -o result.png
```

### Verificar que la app C++ usa Flask

Al ejecutar la aplicación, deberías ver en consola:

```bash
[INFO] ========================================
[INFO] Configurando DnCNN via Flask Server
[INFO] ========================================
[INFO] URL: http://localhost:5000/denoise
[✓] Flask server configurado exitosamente

[INFO] Intentando denoising via Flask server...
[✓] Denoising exitoso via Flask
```

## 🔄 Fallback Automático

Si el servidor Flask **no está disponible**, la aplicación automáticamente usa OpenCV DNN local:

```bash
[INFO] Intentando denoising via Flask server...
[!] Flask server no disponible, intentando fallback a OpenCV DNN...
[INFO] Usando OpenCV DNN local como fallback...
[✓] Denoising exitoso via OpenCV DNN
```

## 📊 Rendimiento

| Método | Tiempo promedio | Ventajas |
|--------|-----------------|----------|
| **Flask Server** | ~1500-2000ms | Centralizado, escalable, fácil actualización |
| **OpenCV DNN Local** | ~1800-2200ms | No requiere red, funciona offline |

## 🐛 Troubleshooting

### Error: "Connection refused"

- **Causa**: Servidor Flask no está corriendo
- **Solución**: Iniciar con `./start_server.sh`

### Error: "No module named 'flask'"

- **Causa**: Flask no instalado
- **Solución**: `pip install flask`

### Error: "Modelo no encontrado"

- **Causa**: `dncnn_grayscale.onnx` no está en `../models/`
- **Solución**: Verificar ruta del modelo en `app.py`

### Denoising muy lento

- **Causa**: CPU sin optimizaciones
- **Solución**:
  - Usar OpenCV compilado con optimizaciones Intel MKL/OpenBLAS
  - Considerar usar GPU (modificar `app.py` para usar CUDA)

## 📝 Logs

El servidor muestra información detallada en consola:

```bash
Iniciando servidor de IA en puerto 5000...
 * Serving Flask app 'app'
 * Running on http://0.0.0.0:5000
[INFO] Request recibido para denoising
[INFO] Imagen decodificada: 512x512
[INFO] Inferencia completada en 1.8s
[✓] Respuesta enviada
```

## 🔐 Seguridad

⚠️ **IMPORTANTE**: Este servidor es para desarrollo local. Para producción:

1. Agregar autenticación (JWT, API keys)
2. Validar tamaño y formato de imágenes
3. Usar HTTPS
4. Implementar rate limiting
5. Usar WSGI server (Gunicorn, uWSGI) en lugar de Flask dev server

## 📚 Referencias

- [Flask Documentation](https://flask.palletsprojects.com/)
- [OpenCV DNN Module](https://docs.opencv.org/4.x/d2/d58/tutorial_table_of_content_dnn.html)
- [DnCNN Paper](https://arxiv.org/abs/1608.03981)

---

**Versión**: 1.0  
**Última actualización**: Diciembre 2025  
**Archivos**: `app.py`, `start_server.sh`
