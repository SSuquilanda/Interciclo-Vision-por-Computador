import torch
import torch.nn as nn
import os

# Definir Arquitectura DnCNN-25
class DnCNN(nn.Module):
    def __init__(self, depth=17, n_channels=64, image_channels=1):
        super(DnCNN, self).__init__()
        kernel_size = 3
        padding = 1
        layers = []

        # Primera capa con bias
        layers.append(nn.Conv2d(image_channels, n_channels, kernel_size, padding=padding, bias=True))
        layers.append(nn.ReLU(inplace=True))
        
        # Capas intermedias sin bias, con batch normalization
        for _ in range(depth-2):
            layers.append(nn.Conv2d(n_channels, n_channels, kernel_size, padding=padding, bias=False))
            layers.append(nn.BatchNorm2d(n_channels, eps=0.0001, momentum=0.95))
            layers.append(nn.ReLU(inplace=True))
        
        # Última capa sin bias
        layers.append(nn.Conv2d(n_channels, image_channels, kernel_size, padding=padding, bias=False))
        self.dncnn = nn.Sequential(*layers)
    
    def forward(self, x):
        y = x
        out = self.dncnn(x)
        return y - out

if __name__ == '__main__':
    pth_file = "dncnn_25.pth"
    onnx_file = "dncnn_grayscale.onnx"

    if not os.path.exists(pth_file):
        print(f"ERROR: No se encuentra {pth_file}")
        exit()

    print(f"Cargando {pth_file}...")
    try:
        # Cargar modelo con weights_only=False para compatibilidad
        loaded = torch.load(pth_file, map_location='cpu', weights_only=False)
    except Exception as e:
        print(f"Error al cargar el archivo: {e}")
        exit()

    # Extraer diccionario de pesos
    if isinstance(loaded, nn.Module):
        state_dict = loaded.state_dict()
    elif isinstance(loaded, dict) and 'state_dict' in loaded:
        state_dict = loaded['state_dict']
    else:
        state_dict = loaded

    # Ajustar nombres de las capas
    print("Procesando nombres de capas...")
    
    model = DnCNN()
    new_state_dict = {}
    
    claves_originales = list(state_dict.keys())
    print(f"Primeras claves encontradas: {claves_originales[:3]}")

    for k, v in state_dict.items():
        new_k = k
        # Eliminar prefijos comunes
        new_k = new_k.replace('module.', '')
        
        # Agregar prefijo dncnn si empieza con número
        if new_k[0].isdigit():
            new_k = 'dncnn.' + new_k
            
        new_state_dict[new_k] = v

    # Cargar pesos en el modelo
    missing, unexpected = model.load_state_dict(new_state_dict, strict=False)
    
    print(f"Capas cargadas: {len(state_dict) - len(missing)}")
    if len(missing) > 0:
        print(f"Capas faltantes: {len(missing)}")
        print(f"Ejemplo: {missing[:2]}")
    else:
        print("Todas las capas coinciden correctamente.")

    model.eval()

    # Exportar a ONNX
    print("Exportando a ONNX...")
    dummy = torch.randn(1, 1, 512, 512)
    torch.onnx.export(model, dummy, onnx_file, input_names=['input'], output_names=['output'], opset_version=11)

    print("Aplicando correcciones al modelo ONNX...")
    import onnx
    from onnx import helper, numpy_helper
    m = onnx.load(onnx_file)
    c = 0
    inits = {t.name: t for t in m.graph.initializer}
    for n in m.graph.node:
        if n.op_type == 'Conv' and not any(a.name == 'kernel_shape' for a in n.attribute):
            w = n.input[1]
            if w in inits:
                arr = numpy_helper.to_array(inits[w])
                n.attribute.append(helper.make_attribute('kernel_shape', [arr.shape[2], arr.shape[3]]))
                c += 1
    onnx.save(m, onnx_file)
    print(f"Proceso completado. {c} nodos corregidos. Archivo: {onnx_file}")
