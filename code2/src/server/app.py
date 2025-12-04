from flask import Flask, request, jsonify, send_file
import cv2
import numpy as np
import io

app = Flask(__name__)

net = cv2.dnn.readNetFromONNX("../models/dncnn_grayscale.onnx")
net.setPreferableBackend(cv2.dnn.DNN_BACKEND_OPENCV)
net.setPreferableTarget(cv2.dnn.DNN_TARGET_CPU)

@app.route('/denoise', methods=['POST'])
def denoise_image():
    try:
        file = request.files['image'].read()
        npimg = np.frombuffer(file, np.uint8)
        img = cv2.imdecode(npimg, cv2.IMREAD_GRAYSCALE)

        if img is None:
            return jsonify({"error": "No se pudo decodificar la imagen"}), 400

        # preprocesar para la red (Float32, normalizado 0-1)
        img_float = img.astype(np.float32) / 255.0
        blob = cv2.dnn.blobFromImage(img_float)
        net.setInput(blob)

        # inferencia
        output = net.forward()

        clean_img_float = output[0, 0, :, :]
        clean_img = np.clip(clean_img_float * 255, 0, 255).astype(np.uint8)

        # devolver la imagen denoised
        _, img_encoded = cv2.imencode('.png', clean_img)
        return send_file(
            io.BytesIO(img_encoded.tobytes()),
            mimetype='image/png'
        )

    except Exception as e:
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    print("Iniciando servidor de IA en puerto 5000...")
    app.run(host='0.0.0.0', port=5000, debug=False)