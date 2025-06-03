import sys
import os
import subprocess
from PyQt6.QtWidgets import QApplication, QMainWindow, QFileDialog, QMessageBox
from parallelization_ui import (
    Ui_MainWindow,
)

COMMAND = "touch test.txt"
OUTPUT_FOLDER = "output"


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
        carpeta = self.ui.lineEdit_2.text().strip()
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
        print(f"Comando a ejecutar: {comando}")
        try:
            self.ui.label_2.setText("Ejecutando")
            resultado = subprocess.run(
                comando,
                shell=True,
                check=True,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )
            salida = resultado.stdout
            self.ui.label_2.setText("Terminado")
            exec_dir = os.getcwd()
            self.ui.label_output_folder.setText(
                f"Archivos procesados en: {exec_dir + "/" + OUTPUT_FOLDER}"
            )
            QMessageBox.information(
                self,
                "Terminado",
                f"Ejecución terminada con éxito.",
                QMessageBox.StandardButton.Ok,
                QMessageBox.StandardButton.Ok,
            )
        except subprocess.CalledProcessError as e:
            self.ui.label_2.setText("Error")
            QMessageBox.critical(self, "Error al ejecutar el comando", e.stderr)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    ventana = MainApp()
    ventana.show()
    sys.exit(app.exec())
