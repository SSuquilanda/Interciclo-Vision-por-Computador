import torch
import torch.nn as nn
import os
import numpy as np
import onnx
from onnx import helper, numpy_helper

# Arquitectura DnCNN compatible con dncnn_25
class DnCNN(nn.Module):
    def __init__(self, depth=17, n_channels=64, image_channels=1):
        super(DnCNN, self).__init__()
        layers = []
        
        # Primera capa
        layers.append(nn.Conv2d(image_channels, n_channels, 3, padding=1, bias=True))
        layers.append(nn.ReLU(inplace=True))
        
        # Capas intermedias
        for _ in range(depth-2):
            layers.append(nn.Conv2d(n_channels, n_channels, 3, padding=1, bias=False))
            layers.append(nn.BatchNorm2d(n_channels, eps=0.0001, momentum=0.95))
            layers.append(nn.ReLU(inplace=True))
        
        # Última capa
        layers.append(nn.Conv2d(n_channels, image_channels, 3, padding=1, bias=False))
        self.dncnn = nn.Sequential(*layers)
    
    def forward(self, x):
        y = x
        out = self.dncnn(x)
        return y - out

if __name__ == '__main__':
    pth_file = "dncnn_25.pth"
    onnx_file = "dncnn_grayscale.onnx"

    print("=" * 80)
    print("   CONVERSION DnCNN A ONNX (OPSET 12)")
    print("=" * 80)

    # Verificar existencia del archivo
    if not os.path.exists(pth_file):
        print("Error: No se encuentra el archivo .pth")
        exit()
        
    print("\n1. Cargando pesos del modelo...")
    
    # Cargar archivo
    loaded = torch.load(pth_file, map_location='cpu', weights_only=False)
    
    # Extraer state_dict según el formato
    if isinstance(loaded, nn.Module):
        print("   Formato: Modelo completo")
        state_dict = loaded.state_dict()
    elif isinstance(loaded, dict):
        if 'state_dict' in loaded:
            print("   Formato: Diccionario con clave 'state_dict'")
            state_dict = loaded['state_dict']
        else:
            print("   Formato: Diccionario directo")
            state_dict = loaded
    else:
        print(f"   Error: Formato desconocido - {type(loaded)}")
        exit()
    
    print(f"   Parametros encontrados: {len(state_dict)}")
    
    # Mostrar primeras claves
    print("\n   Primeras 5 claves:")
    for i, key in enumerate(list(state_dict.keys())[:5]):
        print(f"      {i+1}. {key}")
    
    # Limpiar nombres de claves
    new_state_dict = {}
    for k, v in state_dict.items():
        name = k.replace('module.', '')
        new_state_dict[name] = v

    # Crear modelo y cargar pesos
    model = DnCNN()
    
    try:
        missing, unexpected = model.load_state_dict(new_state_dict, strict=False)
        
        if len(missing) == 0 and len(unexpected) == 0:
            print("   Pesos cargados correctamente")
        else:
            print(f"   Advertencia: {len(missing)} claves faltantes, {len(unexpected)} claves extra")
            if len(missing) > 0:
                print(f"      Ejemplo: {missing[:3]}")
    except Exception as e:
        print(f"   Error al cargar pesos: {e}")
        exit()

    model.eval()

    # Prueba con PyTorch
    print("\n2. Probando modelo en PyTorch...")
    
    test_input = torch.randn(1, 1, 512, 512)
    with torch.no_grad():
        test_output = model(test_input)
    
    diff_pytorch = (test_input - test_output).abs().max().item()
    print(f"   Diferencia maxima: {diff_pytorch:.6f}")
    
    if diff_pytorch < 1e-5:
        print("   Advertencia: El modelo devuelve casi la misma entrada")
        print("   Posibles pesos mal cargados")
    else:
        print("   Modelo funciona correctamente")

    # Exportar a ONNX
    print("\n3. Exportando a ONNX (Opset 12)...")
    
    dummy = torch.randn(1, 1, 512, 512)
    
    torch.onnx.export(
        model, 
        dummy, 
        onnx_file, 
        input_names=['input'], 
        output_names=['output'],
        opset_version=12,
        do_constant_folding=True,
        verbose=False
    )
    
    print("   Exportacion completa")

    # Aplicar parche para OpenCV
    print("\n4. Aplicando parche kernel_shape...")
    
    m = onnx.load(onnx_file)
    patched = 0
    
    for node in m.graph.node:
        if node.op_type == 'Conv':
            has_k = any(a.name == 'kernel_shape' for a in node.attribute)
            if not has_k:
                w_name = node.input[1] if len(node.input) > 1 else None
                if w_name:
                    for init in m.graph.initializer:
                        if init.name == w_name:
                            w_arr = numpy_helper.to_array(init)
                            attr = helper.make_attribute('kernel_shape', [int(w_arr.shape[2]), int(w_arr.shape[3])])
                            node.attribute.append(attr)
                            patched += 1
                            break
    
    onnx.save(m, onnx_file)
    print(f"   Capas Conv parcheadas: {patched}")

    # Verificar modelo ONNX
    print("\n5. Verificando modelo ONNX...")
    
    try:
        onnx.checker.check_model(m)
        size_mb = os.path.getsize(onnx_file) / (1024 * 1024)
        print(f"   Modelo valido")
        print(f"   Tamaño: {size_mb:.2f} MB")
    except Exception as e:
        print(f"   Error en validacion: {e}")

    # Prueba con ONNX Runtime
    print("\n6. Prueba final con ONNX Runtime...")
    
    try:
        import onnxruntime as ort
        
        sess = ort.InferenceSession(onnx_file, providers=['CPUExecutionProvider'])
        
        # Generar entrada de prueba
        ruido_input = np.random.randn(1, 1, 512, 512).astype(np.float32)
        res = sess.run(None, {'input': ruido_input})[0]
        
        diff_onnx = np.max(np.abs(ruido_input - res))
        
        print(f"   Diferencia maxima: {diff_onnx:.6f}")
        
        if diff_onnx < 1e-5:
            print("red no modifica la imagen")
        else:
            print("   CONVERSION EXITOSA")
            
    except ImportError:
        print("   onnxruntime no instalado")
        print("   Ejecutar: pip install onnxruntime")
    except Exception as e:
        print(f"   Error en prueba: {e}")
