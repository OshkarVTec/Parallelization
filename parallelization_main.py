import sys
import os
import subprocess
from PyQt6.QtWidgets import QApplication, QMainWindow, QFileDialog, QMessageBox
from PyQt6.QtCore import QThread, pyqtSignal
from parallelization_ui import (
    Ui_MainWindow,
)

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

        # Conectar botones
        self.ui.pushButton_3.clicked.connect(self.seleccionar_folder)
        self.ui.pushButton.clicked.connect(self.ejecutar_comando)

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
        QMessageBox.information(
            self,
            "Terminado",
            f"Ejecución terminada con éxito.",
            QMessageBox.StandardButton.Ok,
            QMessageBox.StandardButton.Ok,
        )

    def comando_error(self, error):
        self.ui.label_2.setText("Error")
        QMessageBox.critical(self, "Error al ejecutar el comando", error)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    ventana = MainApp()
    ventana.show()
    sys.exit(app.exec())
