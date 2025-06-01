# 🖥️ Procesamiento Paralelo de Imágenes con MPI y OpenMP

Este proyecto implementa un sistema distribuido de procesamiento de imágenes usando un clúster de tres máquinas virtuales interconectadas mediante red local (NFS + SSH). Aprovechando `MPI` para la distribución de tareas entre nodos y `OpenMP` para la paralelización dentro de cada nodo, el sistema procesa archivos BMP aplicando transformaciones como:

- Escala de grises
- Desenfoque (Blur)
- Espejado horizontal y vertical (color y B/N)

El sistema incluye una interfaz gráfica en **PyQt** para facilitar su uso por usuarios no técnicos.

---

## 🛠️ Tecnologías utilizadas

- Ubuntu 22.04 LTS
- VirtualBox 7.1.8
- GCC + OpenMP
- MPI (MPICH)
- NFS (Network File System)
- PyQt6 para interfaz gráfica
- Script de métricas (MIPS, tiempo de ejecución, bytes procesados)

---

## 🔗 Documentación completa en la Wiki

Toda la información técnica del proyecto está organizada en la **Wiki del repositorio**. Incluye:

- Descripción del problema
- Metodología
- Resultados con capturas
- Conclusiones
- Códigos fuente y estructura de carpetas
- Métricas de rendimiento

👉 **[Ir a la Wiki del Proyecto](../../wiki)**

---

## 👨‍💻 Equipo de trabajo

- Oskar Adolfo Villa López | A01275287
- Cruz Daniel Pérez Jiménez | A01736214
- David Alberto Alvarado Cabrero | A01736390
- Tecnológico de Monterrey — Campus Puebla  
- Curso: Implementación de Redes de Área Amplia y Servicios Distribuidos (Gpo 501)
