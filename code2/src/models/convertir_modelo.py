import torch
import torch.nn as nn
import os

# 1. Definir Arquitectura DnCNN-25
class DnCNN(nn.Module):
    def __init__(self, depth=17, n_channels=64, image_channels=1):
        super(DnCNN, self).__init__()
        kernel_size = 3
        padding = 1
        layers = []

        # Primera capa (Con Bias)
        layers.append(nn.Conv2d(image_channels, n_channels, kernel_size, padding=padding, bias=True))
        layers.append(nn.ReLU(inplace=True))
        
        # Capas intermedias (Sin Bias, con BN)
        for _ in range(depth-2):
            layers.append(nn.Conv2d(n_channels, n_channels, kernel_size, padding=padding, bias=False))
            layers.append(nn.BatchNorm2d(n_channels, eps=0.0001, momentum=0.95))
            layers.append(nn.ReLU(inplace=True))
        
        # Última capa (Sin Bias)
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
        print(f"❌ ERROR: Falta {pth_file}")
        exit()

    print(f"1. Cargando {pth_file} (Desbloqueando seguridad PyTorch 2.6)...")
    try:
        # AQUÍ ESTÁ EL ARREGLO DE SEGURIDAD:
        loaded = torch.load(pth_file, map_location='cpu', weights_only=False)
    except Exception as e:
        print(f"❌ Error cargando archivo: {e}")
        exit()

    # Extraer diccionario de pesos
    if isinstance(loaded, nn.Module):
        state_dict = loaded.state_dict()
    elif isinstance(loaded, dict) and 'state_dict' in loaded:
        state_dict = loaded['state_dict']
    else:
        state_dict = loaded

    # 2. INTELIGENCIA DE MAPEO DE NOMBRES
    # El archivo puede tener claves como 'features.0.weight' o '0.weight'
    # Nuestro modelo espera 'dncnn.0.weight'
    print("2. Analizando y corrigiendo claves de capas...")
    
    model = DnCNN()
    new_state_dict = {}
    
    # Imprimir muestra de claves originales para depurar
    claves_originales = list(state_dict.keys())
    print(f"   Claves en archivo (ejemplo): {claves_originales[:3]}")

    for k, v in state_dict.items():
        new_k = k
        # Quitar prefijos comunes
        new_k = new_k.replace('module.', '')
        
        # Si la clave empieza directo con número (ej: '0.weight'), agregar 'dncnn.'
        if new_k[0].isdigit():
            new_k = 'dncnn.' + new_k
            
        new_state_dict[new_k] = v

    # Cargar y VERIFICAR
    missing, unexpected = model.load_state_dict(new_state_dict, strict=False)
    
    print(f"   ✅ Capas cargadas correctamente: {len(state_dict) - len(missing)}")
    if len(missing) > 0:
        print(f"   ⚠️ Capas faltantes (esto explica si no ves cambios): {len(missing)}")
        print(f"      Ejemplo faltante: {missing[:2]}")
    else:
        print("   🎉 ¡PERFECTO! Todas las capas coinciden.")

    model.eval()

    # 3. Exportar y Parchado Final
    print("3. Exportando a ONNX...")
    dummy = torch.randn(1, 1, 512, 512)
    torch.onnx.export(model, dummy, onnx_file, input_names=['input'], output_names=['output'], opset_version=11)

    print("4. Parchando Kernel Shape...")
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
    print(f"✅ Listo. {c} nodos parchados. Archivo generado: {onnx_file}")