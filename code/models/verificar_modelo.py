import onnxruntime as ort
import numpy as np

# 1. Cargar el modelo que creaste
try:
    session = ort.InferenceSession("dncnn_grayscale.onnx")
    print("✅ El archivo ONNX se lee correctamente.")
except Exception as e:
    print("❌ Error leyendo ONNX:", e)
    exit()

# 2. Probar con ruido falso (para obligar a la red a trabajar)
dummy_input = np.random.randn(1, 1, 512, 512).astype(np.float32)
outputs = session.run(None, {"input": dummy_input})
result = outputs[0]

# 3. Verificar si hizo algo
diff = np.max(np.abs(dummy_input - result))
print(f"Diferencia Máxima (Input vs Output): {diff:.5f}")

if diff > 0.00001:
    print("🎉 CONCLUSIÓN: ¡La red está VIVA! (Está procesando los píxeles activamente)")
else:
    print("⚠️ CONCLUSIÓN: La red es identidad (No está haciendo nada).")