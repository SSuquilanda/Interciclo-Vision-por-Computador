import onnxruntime as ort
import numpy as np

try:
    session = ort.InferenceSession("dncnn_grayscale.onnx")
    print("lee")
except Exception as e:
    print("no lee:", e)
    exit()

dummy_input = np.random.randn(1, 1, 512, 512).astype(np.float32)
outputs = session.run(None, {"input": dummy_input})
result = outputs[0]

diff = np.max(np.abs(dummy_input - result))
print(f"Diferencia Máxima: {diff:.5f}")

if diff > 0.00001:
    print("hay red")
else:
    print("no hace nada la red")