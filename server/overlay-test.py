import sys
import threading

from PyQt5 import QtWidgets, QtCore
import time
import multiprocessing


class VolumeStateOverlay(QtWidgets.QWidget):
    def __init__(self, volume: int):
        super().__init__()

        self.bar = QtWidgets.QProgressBar(self)
        self.bar.setGeometry(0, 0, 200, 50)
        self.bar.setMaximum(100)
        self.bar.setValue(volume)

        self.setGeometry(10, 10, 200, 50)
        self.setWindowTitle("PyQt45")
        self.setAttribute(QtCore.Qt.WidgetAttribute.WA_TranslucentBackground)
        self.setAttribute(QtCore.Qt.WidgetAttribute.WA_TransparentForMouseEvents)
        self.setWindowFlag(QtCore.Qt.FramelessWindowHint)
        self.setWindowFlag(QtCore.Qt.WindowStaysOnTopHint)
        center_point = QtWidgets.QDesktopWidget().availableGeometry().center()
        center_point.setX(int(center_point.x() - 200 / 2))
        center_point.setY(int(center_point.y() - 50 / 2))

        self.move(center_point)

        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.close)
        self.timer.start(2000)

        self.show()

    def set_volume(self, volume: int):
        self.bar.setValue(volume)


def show_volume(volume: int):
    app = QtWidgets.QApplication(sys.argv)
    ex = VolumeStateOverlay(volume)

    ex.set_volume(99)
    t = threading.Thread(target=app.exec_)
    t.daemon = True
    t.start()
    ex.set_volume(9)

show_volume(55)

