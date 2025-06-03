import sys
import os
import subprocess
from PyQt6.QtWidgets import QApplication, QMainWindow, QFileDialog, QMessageBox
from PyQt6.QtGui import QAction
from PyQt6.QtCore import QThread, pyqtSignal
from parallelization_ui import (
    Ui_MainWindow,
)
import time

COMMAND = "mpiexec -np 3 --hostfile mirror/Parallelization/machinefile mirror/Parallelization/reto"
OUTPUT_FOLDER = "output"


class WorkerThread(QThread):
    finished = pyqtSignal(str)
    error = pyqtSignal(str)
    progress = pyqtSignal(int)  # Nueva señal para progreso

    def __init__(self, comando):
        super().__init__()
        self.comando = comando

    def run(self):
        try:
            process = subprocess.Popen(
                self.comando,
                shell=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                bufsize=1,
                universal_newlines=True,
            )
            total = None
            progreso = 0
            salida = ""
            for line in process.stdout:
                salida += line
                if line.startswith("PROGRESS"):
                    # Ejemplo: PROGRESS 3/10
                    try:
                        _, nums = line.strip().split()
                        done, total = map(int, nums.split("/"))
                        progreso = int(done / total * 100)
                        self.progress.emit(progreso)
                    except Exception:
                        pass
            process.wait()
            if process.returncode == 0:
                self.finished.emit(salida)
            else:
                error = process.stderr.read()
                self.error.emit(error)
        except Exception as e:
            self.error.emit(str(e))


class MainApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)
        self.start_time = None

        # Menú de equipo
        menubar = self.menuBar()
        acerca_menu = menubar.addMenu("Acerca de")
        equipo_action = QAction("Equipo desarrollador", self)
        equipo_action.triggered.connect(self.mostrar_equipo)
        acerca_menu.addAction(equipo_action)

        # Conectar botones
        self.ui.pushButton_3.clicked.connect(self.seleccionar_folder)
        self.ui.pushButton.clicked.connect(self.ejecutar_comando)
        self.ui.pushButton_show_ops.clicked.connect(self.mostrar_operations_count)

    def mostrar_equipo(self):
        miembros = [
            "Cruz Daniel Pérez Jiménez",
            "David Alberto Alvarado Cabrero ",
            "Oskar Adolfo Villa López",
        ]
        QMessageBox.information(
            self,
            "Equipo desarrollador",
            "Integrantes:\n" + "\n".join(miembros),
            QMessageBox.StandardButton.Ok,
        )

    def seleccionar_folder(self):
        exec_dir = os.getcwd()
        folder = QFileDialog.getExistingDirectory(
            self, "Selecciona una carpeta", exec_dir
        )
        if folder and os.path.abspath(folder).startswith(os.path.abspath(exec_dir)):
            self.ui.lineEdit_2.setText(folder)
        elif folder:
            QMessageBox.warning(
                self,
                "Error",
                "Por favor selecciona una carpeta dentro del directorio de ejecución o sus subdirectorios.",
            )

    def ejecutar_comando(self):
        carpeta = self.ui.lineEdit_2.text().strip() + "/"
        kernel_size_str = self.ui.lineEdit_kernel.text().strip()
        self.start_time = time.time()

        try:
            kernel_size = int(kernel_size_str)
            if not (55 <= kernel_size <= 150):
                raise ValueError
        except ValueError:
            QMessageBox.warning(
                self, "Error", "Introduce un tamaño de kernel válido (55-150)."
            )
            return

        comando = f"{COMMAND} {carpeta} {kernel_size}"
        self.ui.label_2.setText("Ejecutando")
        self.worker = WorkerThread(comando)
        self.worker.finished.connect(self.comando_terminado)
        self.worker.error.connect(self.comando_error)
        self.worker.progress.connect(
            self.ui.progressBar.setValue
        )  # Conecta progreso real
        self.ui.progressBar.setValue(0)
        self.worker.start()

    def comando_terminado(self, salida):
        print(salida)
        self.ui.label_2.setText("Terminado")
        exec_dir = os.getcwd()
        folder = exec_dir + "/" + OUTPUT_FOLDER
        self.ui.label_output_folder.setText(f"Archivos procesados en: {folder}")

        # Calcular velocidad de procesamiento
        total_pixels = self.obtener_total_pixeles()
        elapsed = time.time() - self.start_time if self.start_time else 1
        if total_pixels > 0:
            speed = total_pixels / elapsed
            self.ui.label_speed.setText(f"Velocidad: {speed:.2f} pixeles/segundo")
        else:
            self.ui.label_speed.setText("Velocidad: N/A")

        QMessageBox.information(
            self,
            "Terminado",
            f"Ejecución terminada con éxito.",
            QMessageBox.StandardButton.Ok,
            QMessageBox.StandardButton.Ok,
        )

    def obtener_total_pixeles(self):
        # Lee el archivo operations_count.txt y suma los pixeles leídos
        try:
            total_pixels_read = 0
            total_pixels_written = 0
            file_path = "output/operations_count.txt"
            with open(file_path, "r") as file:
                for line in file:
                    if line.startswith("Total pixels read:"):
                        total_pixels_read += int(line.split(":")[1].strip())
                    elif line.startswith("Total pixels written:"):
                        total_pixels_written += int(line.split(":")[1].strip())

            # Calculate the total pixels processed
            total_pixels_processed = total_pixels_read + total_pixels_written

            return total_pixels_processed
        except Exception:
            return 0

    def comando_error(self, error):
        self.ui.label_2.setText("Error")
        QMessageBox.critical(self, "Error al ejecutar el comando", error)

    def mostrar_operations_count(self):
        try:
            with open("output/operations_count.txt", "r") as f:
                contenido = f.read()
            self.ui.textEdit_ops.setPlainText(contenido)
        except Exception as e:
            self.ui.textEdit_ops.setPlainText(f"Error: {e}")


if __name__ == "__main__":
    app = QApplication(sys.argv)
    ventana = MainApp()
    ventana.show()
    sys.exit(app.exec())
