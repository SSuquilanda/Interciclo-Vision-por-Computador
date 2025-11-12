#!/usr/bin/env python3
"""
Análisis de Resultados del Explorador de Dataset
Genera gráficos y estadísticas a partir del reporte CSV
"""

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import sys
import os

# Configuración de estilo
sns.set_style("whitegrid")
plt.rcParams['figure.figsize'] = (14, 8)

def load_data(csv_path):
    """Carga el archivo CSV generado por ExploreDataset"""
    if not os.path.exists(csv_path):
        print(f"Error: No se encuentra el archivo {csv_path}")
        print("   Ejecuta primero ./run_explorer.sh con opción 1 o 2")
        sys.exit(1)
    
    df = pd.read_csv(csv_path)
    print(f"Datos cargados: {len(df)} slices")
    return df

def plot_psnr_analysis(df):
    """Gráfico de PSNR a lo largo de los slices"""
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 6))
    
    # Gráfico de línea
    ax1.plot(df['SliceNumber'], df['PSNR'], linewidth=2, color='#2E86AB')
    ax1.axhline(y=df['PSNR'].mean(), color='red', linestyle='--', 
                label=f'Media: {df["PSNR"].mean():.2f} dB')
    ax1.axhline(y=40, color='green', linestyle=':', 
                label='Umbral "Excelente" (40 dB)')
    ax1.axhline(y=30, color='orange', linestyle=':', 
                label='Umbral "Bueno" (30 dB)')
    ax1.set_xlabel('Número de Slice', fontsize=12)
    ax1.set_ylabel('PSNR (dB)', fontsize=12)
    ax1.set_title('Peak Signal-to-Noise Ratio: Full Dose vs Quarter Dose', 
                  fontsize=14, fontweight='bold')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # Histograma
    ax2.hist(df['PSNR'], bins=30, color='#A23B72', alpha=0.7, edgecolor='black')
    ax2.axvline(df['PSNR'].mean(), color='red', linestyle='--', linewidth=2,
                label=f'Media: {df["PSNR"].mean():.2f} dB')
    ax2.set_xlabel('PSNR (dB)', fontsize=12)
    ax2.set_ylabel('Frecuencia', fontsize=12)
    ax2.set_title('Distribución de PSNR', fontsize=14, fontweight='bold')
    ax2.legend()
    ax2.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig('psnr_analysis.png', dpi=300, bbox_inches='tight')
    print("Guardado: psnr_analysis.png")
    plt.show()

def plot_snr_comparison(df):
    """Comparación de SNR entre Full Dose y Quarter Dose"""
    fig, axes = plt.subplots(2, 2, figsize=(16, 12))
    
    # SNR Full Dose
    axes[0, 0].plot(df['SliceNumber'], df['FD_SNR'], linewidth=2, color='#06A77D')
    axes[0, 0].axhline(y=df['FD_SNR'].mean(), color='red', linestyle='--',
                       label=f'Media: {df["FD_SNR"].mean():.2f}')
    axes[0, 0].set_xlabel('Número de Slice')
    axes[0, 0].set_ylabel('SNR')
    axes[0, 0].set_title('Signal-to-Noise Ratio - Full Dose', fontweight='bold')
    axes[0, 0].legend()
    axes[0, 0].grid(True, alpha=0.3)
    
    # SNR Quarter Dose
    axes[0, 1].plot(df['SliceNumber'], df['QD_SNR'], linewidth=2, color='#D62246')
    axes[0, 1].axhline(y=df['QD_SNR'].mean(), color='red', linestyle='--',
                       label=f'Media: {df["QD_SNR"].mean():.2f}')
    axes[0, 1].set_xlabel('Número de Slice')
    axes[0, 1].set_ylabel('SNR')
    axes[0, 1].set_title('Signal-to-Noise Ratio - Quarter Dose', fontweight='bold')
    axes[0, 1].legend()
    axes[0, 1].grid(True, alpha=0.3)
    
    # Comparación lado a lado
    x = np.arange(len(df))
    width = 0.35
    axes[1, 0].bar(x - width/2, df['FD_SNR'].head(50), width, 
                   label='Full Dose', alpha=0.8, color='#06A77D')
    axes[1, 0].bar(x + width/2, df['QD_SNR'].head(50), width, 
                   label='Quarter Dose', alpha=0.8, color='#D62246')
    axes[1, 0].set_xlabel('Primeros 50 Slices')
    axes[1, 0].set_ylabel('SNR')
    axes[1, 0].set_title('Comparación SNR (primeros 50 slices)', fontweight='bold')
    axes[1, 0].legend()
    axes[1, 0].grid(True, alpha=0.3, axis='y')
    
    # Degradación de SNR
    snr_degradation = ((df['FD_SNR'] - df['QD_SNR']) / df['FD_SNR'].abs()) * 100
    axes[1, 1].plot(df['SliceNumber'], snr_degradation, linewidth=2, color='#F18F01')
    axes[1, 1].axhline(y=snr_degradation.mean(), color='red', linestyle='--',
                       label=f'Media: {snr_degradation.mean():.2f}%')
    axes[1, 1].set_xlabel('Número de Slice')
    axes[1, 1].set_ylabel('Degradación (%)')
    axes[1, 1].set_title('Degradación de SNR: FD → QD', fontweight='bold')
    axes[1, 1].legend()
    axes[1, 1].grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig('snr_comparison.png', dpi=300, bbox_inches='tight')
    print("Guardado: snr_comparison.png")
    plt.show()

def plot_statistics_comparison(df):
    """Comparación de estadísticas básicas"""
    fig, axes = plt.subplots(2, 2, figsize=(16, 12))
    
    # Media
    axes[0, 0].plot(df['SliceNumber'], df['FD_Mean'], label='Full Dose', 
                    linewidth=2, color='#06A77D')
    axes[0, 0].plot(df['SliceNumber'], df['QD_Mean'], label='Quarter Dose', 
                    linewidth=2, color='#D62246')
    axes[0, 0].set_xlabel('Número de Slice')
    axes[0, 0].set_ylabel('Media (HU)')
    axes[0, 0].set_title('Intensidad Media de Píxeles', fontweight='bold')
    axes[0, 0].legend()
    axes[0, 0].grid(True, alpha=0.3)
    
    # Desviación Estándar
    axes[0, 1].plot(df['SliceNumber'], df['FD_StdDev'], label='Full Dose', 
                    linewidth=2, color='#06A77D')
    axes[0, 1].plot(df['SliceNumber'], df['QD_StdDev'], label='Quarter Dose', 
                    linewidth=2, color='#D62246')
    axes[0, 1].set_xlabel('Número de Slice')
    axes[0, 1].set_ylabel('Desviación Estándar (HU)')
    axes[0, 1].set_title('Ruido (Desviación Estándar)', fontweight='bold')
    axes[0, 1].legend()
    axes[0, 1].grid(True, alpha=0.3)
    
    # Diferencia en Media
    axes[1, 0].plot(df['SliceNumber'], df['MeanDiff'], linewidth=2, color='#F18F01')
    axes[1, 0].axhline(y=df['MeanDiff'].mean(), color='red', linestyle='--',
                       label=f'Media: {df["MeanDiff"].mean():.2f} HU')
    axes[1, 0].set_xlabel('Número de Slice')
    axes[1, 0].set_ylabel('Diferencia Absoluta (HU)')
    axes[1, 0].set_title('Diferencia en Media: |FD - QD|', fontweight='bold')
    axes[1, 0].legend()
    axes[1, 0].grid(True, alpha=0.3)
    
    # Diferencia en Desviación Estándar
    axes[1, 1].plot(df['SliceNumber'], df['StdDevDiff'], linewidth=2, color='#A23B72')
    axes[1, 1].axhline(y=df['StdDevDiff'].mean(), color='red', linestyle='--',
                       label=f'Media: {df["StdDevDiff"].mean():.2f} HU')
    axes[1, 1].set_xlabel('Número de Slice')
    axes[1, 1].set_ylabel('Diferencia Absoluta (HU)')
    axes[1, 1].set_title('Diferencia en Ruido: |FD - QD|', fontweight='bold')
    axes[1, 1].legend()
    axes[1, 1].grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig('statistics_comparison.png', dpi=300, bbox_inches='tight')
    print("Guardado: statistics_comparison.png")
    plt.show()

def print_summary_statistics(df):
    """Imprime estadísticas resumidas"""
    print("ESTADÍSTICAS RESUMIDAS DEL ANÁLISIS\n")
    print(f"Total de slices analizados: {len(df)}\n")
    
    print("--- PSNR (Peak Signal-to-Noise Ratio) ---")
    print(f"  Media:    {df['PSNR'].mean():.2f} dB")
    print(f"  Mediana:  {df['PSNR'].median():.2f} dB")
    print(f"  Mínimo:   {df['PSNR'].min():.2f} dB (Slice {df.loc[df['PSNR'].idxmin(), 'SliceNumber']:.0f})")
    print(f"  Máximo:   {df['PSNR'].max():.2f} dB (Slice {df.loc[df['PSNR'].idxmax(), 'SliceNumber']:.0f})")
    print(f"  Desv.Est: {df['PSNR'].std():.2f} dB\n")
    
    print("--- SNR (Signal-to-Noise Ratio) ---")
    print(f"  Full Dose promedio:     {df['FD_SNR'].mean():.2f}")
    print(f"  Quarter Dose promedio:  {df['QD_SNR'].mean():.2f}")
    degradation = ((df['FD_SNR'].mean() - df['QD_SNR'].mean()) / abs(df['FD_SNR'].mean())) * 100
    print(f"  Degradación:            {degradation:.2f}%\n")
    
    print("--- Diferencias ---")
    print(f"  Diferencia media en intensidad:  {df['MeanDiff'].mean():.2f} HU")
    print(f"  Diferencia media en ruido:       {df['StdDevDiff'].mean():.2f} HU\n")
    
    # Clasificación de calidad
    excellent = (df['PSNR'] > 40).sum()
    good = ((df['PSNR'] >= 30) & (df['PSNR'] <= 40)).sum()
    poor = (df['PSNR'] < 30).sum()
    
    print("--- Clasificación de Calidad ---")
    print(f"  Excelente (>40 dB):  {excellent:3d} slices ({excellent/len(df)*100:.1f}%)")
    print(f"  Buena (30-40 dB):    {good:3d} slices ({good/len(df)*100:.1f}%)")
    print(f"  Regular (<30 dB):    {poor:3d} slices ({poor/len(df)*100:.1f}%)\n")
    
    # Slices más representativos
    print("--- Top 5 Slices con Menor PSNR (Mayor Diferencia) ---")
    worst = df.nsmallest(5, 'PSNR')
    for idx, row in worst.iterrows():
        print(f"  Slice {row['SliceNumber']:.0f}: PSNR = {row['PSNR']:.2f} dB")
    
    print("\n--- Top 5 Slices con Mayor PSNR (Más Similares) ---")
    best = df.nlargest(5, 'PSNR')
    for idx, row in best.iterrows():
        print(f"  Slice {row['SliceNumber']:.0f}: PSNR = {row['PSNR']:.2f} dB")
    
    print()

def main():
    print("ANÁLISIS DE RESULTADOS - Dataset Explorer")
    
    # Buscar el archivo CSV
    csv_path = '../data/dataset_comparison_report.csv'
    if not os.path.exists(csv_path):
        csv_path = 'data/dataset_comparison_report.csv'
    
    # Cargar datos
    df = load_data(csv_path)
    
    # Imprimir estadísticas
    print_summary_statistics(df)
    
    # Generar gráficos
    print("Generando gráficos...\n")
    plot_psnr_analysis(df)
    plot_snr_comparison(df)
    plot_statistics_comparison(df)

    print("\nAnálisis completado!")
    print("   Gráficos guardados:")
    print("   - psnr_analysis.png")
    print("   - snr_comparison.png")
    print("   - statistics_comparison.png\n")

if __name__ == '__main__':
    main()
