import sys
import os
import subprocess
from PyQt6.QtWidgets import QApplication, QMainWindow, QFileDialog, QMessageBox
from parallelization_ui import (
    Ui_MainWindow,
)

COMMAND = "touch test.txt"


class MainApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)

        # Conectar botones
        self.ui.pushButton_3.clicked.connect(self.seleccionar_folder)
        self.ui.pushButton.clicked.connect(self.ejecutar_comando)

    def seleccionar_folder(self):
        folder = QFileDialog.getExistingDirectory(self, "Selecciona una carpeta")
        if folder:
            self.ui.lineEdit_2.setText(folder)

    def ejecutar_comando(self):
        carpeta = self.ui.lineEdit_2.text().strip()
        if not carpeta or not os.path.isdir(carpeta):
            QMessageBox.warning(
                self, "Error", "Por favor selecciona una carpeta válida."
            )
            return

        comando = COMMAND
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
            QMessageBox.information(
                self,
                "Terminado",
                f"Ejecución terminada con éxito.",
                QMessageBox.StandardButton.Ok,
                QMessageBox.StandardButton.Ok,
            )
        except subprocess.CalledProcessError as e:
            self.ui.label_2.setText("Error, por favor revisa la carpeta")
            QMessageBox.critical(self, "Error al ejecutar el comando", e.stderr)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    ventana = MainApp()
    ventana.show()
    sys.exit(app.exec())
