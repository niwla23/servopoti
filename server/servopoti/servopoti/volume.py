from servopoti.servopoti.readwrite import ReadWrite
import subprocess


class VolumeController(ReadWrite):
    def read(self):
        return int(subprocess.run(
            ["amixer", "get", "Master"], capture_output=True, shell=True).stdout.decode().split('\n')[5].split(" ")[-2][1:-2])

    def write(self, value):
        subprocess.Popen(["amixer", "sset", "Master",
                         str(int(655.36 * int(value)))])
