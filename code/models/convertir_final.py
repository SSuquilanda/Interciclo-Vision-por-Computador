import torch
import torch.nn as nn
import os
import numpy as np
import onnx
from onnx import helper, numpy_helper

# 1. ARQUITECTURA DNCNN (Asegurando compatibilidad con dncnn_25)
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
    print("   CONVERSIÓN DnCNN → ONNX (OPSET 12)")
    print("=" * 80)

    # 1. CARGA INTELIGENTE DE PESOS
    if not os.path.exists(pth_file):
        print("❌ Error: Falta el archivo .pth")
        exit()
        
    print("\n1. Cargando pesos...")
    
    # Cargar con weights_only=False
    loaded = torch.load(pth_file, map_location='cpu', weights_only=False)
    
    # EXTRAER el state_dict dependiendo del formato
    if isinstance(loaded, nn.Module):
        # Si es un modelo completo
        print("   📦 Formato: Modelo completo (nn.Module)")
        state_dict = loaded.state_dict()
    elif isinstance(loaded, dict):
        # Si es un diccionario
        if 'state_dict' in loaded:
            print("   📦 Formato: Diccionario con clave 'state_dict'")
            state_dict = loaded['state_dict']
        else:
            print("   📦 Formato: Diccionario directo (state_dict)")
            state_dict = loaded
    else:
        print(f"   ❌ Formato desconocido: {type(loaded)}")
        exit()
    
    print(f"   ✅ {len(state_dict)} parámetros encontrados")
    
    # Mostrar primeras claves para debug
    print("\n   📋 Primeras 5 claves:")
    for i, key in enumerate(list(state_dict.keys())[:5]):
        print(f"      {i+1}. {key}")
    
    # Arreglar claves (quitar module. si existe)
    new_state_dict = {}
    for k, v in state_dict.items():
        name = k.replace('module.', '')
        new_state_dict[name] = v

    # Crear modelo vacío y cargar pesos
    model = DnCNN()
    
    try:
        missing, unexpected = model.load_state_dict(new_state_dict, strict=False)
        
        if len(missing) == 0 and len(unexpected) == 0:
            print("   ✅ Todos los pesos cargados correctamente")
        else:
            print(f"   ⚠️  Claves faltantes: {len(missing)}")
            print(f"   ⚠️  Claves extra: {len(unexpected)}")
            if len(missing) > 0:
                print(f"      Ejemplo faltante: {missing[:3]}")
    except Exception as e:
        print(f"   ❌ Error al cargar: {e}")
        exit()

    model.eval()

    # 2. PRUEBA EN PYTORCH ANTES DE EXPORTAR
    print("\n2. 🧪 Probando modelo en PyTorch...")
    
    test_input = torch.randn(1, 1, 512, 512)
    with torch.no_grad():
        test_output = model(test_input)
    
    diff_pytorch = (test_input - test_output).abs().max().item()
    print(f"   Diferencia máx PyTorch: {diff_pytorch:.6f}")
    
    if diff_pytorch < 1e-5:
        print("   ⚠️  ADVERTENCIA: Modelo devuelve casi identidad en PyTorch")
        print("   Esto puede indicar pesos mal cargados")
    else:
        print("   ✅ Modelo funciona en PyTorch")

    # 3. EXPORTAR A OPSET 12
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
    
    print("   ✅ Exportado a ONNX")

    # 4. PARCHE DE KERNEL
    print("\n4. Aplicando parche kernel_shape para OpenCV...")
    
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
    print(f"   ✅ {patched} capas Conv parcheadas")

    # 5. VERIFICAR ONNX
    print("\n5. Verificando modelo ONNX...")
    
    try:
        onnx.checker.check_model(m)
        size_mb = os.path.getsize(onnx_file) / (1024 * 1024)
        print(f"   ✅ Modelo válido")
        print(f"   📊 Tamaño: {size_mb:.2f} MB")
    except Exception as e:
        print(f"   ❌ Error en validación: {e}")

    # 6. PRUEBA CON ONNX RUNTIME
    print("\n6. 🧪 PRUEBA FINAL con ONNX Runtime...")
    
    try:
        import onnxruntime as ort
        
        sess = ort.InferenceSession(onnx_file, providers=['CPUExecutionProvider'])
        
        # Generar ruido de prueba
        ruido_input = np.random.randn(1, 1, 512, 512).astype(np.float32)
        res = sess.run(None, {'input': ruido_input})[0]
        
        diff_onnx = np.max(np.abs(ruido_input - res))
        
        print(f"   Diferencia máx ONNX: {diff_onnx:.6f}")
        
        if diff_onnx < 1e-5:
            print("\n" + "=" * 80)
            print("   ❌ PROBLEMA CRÍTICO: La red NO modifica la imagen")
            print("   Posibles causas:")
            print("   1. El archivo dncnn_25.pth no contiene pesos entrenados")
            print("   2. Incompatibilidad entre arquitectura y pesos")
            print("   3. Modelo fue guardado de forma incorrecta")
            print("=" * 80)
        else:
            print("\n" + "=" * 80)
            print("   ✅✅✅ ÉXITO COMPLETO")
            print(f"   La red SÍ procesa imágenes (diferencia: {diff_onnx:.6f})")
            print(f"   📂 Archivo listo: {onnx_file}")
            print("\n   🎯 Siguiente paso:")
            print(f"      copy {onnx_file} ..\\build\\Release\\models\\")
            print("=" * 80)
            
    except ImportError:
        print("   ⚠️ onnxruntime no instalado, no se puede probar")
        print("   Ejecuta: pip install onnxruntime")
    except Exception as e:
        print(f"   ❌ Error en prueba: {e}")