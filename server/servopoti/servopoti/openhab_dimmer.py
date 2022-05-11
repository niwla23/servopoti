from servopoti.servopoti.readwrite import ReadWrite
from openhab import OpenHAB
import subprocess


class OpenHabDimmer(ReadWrite):
    def __init__(self, mqtt_state_topic: str, mqtt_command_topic: str, oh_server_url: str, item_name: str):
        self.oh_server_url = oh_server_url
        self.item_name = item_name
        self.openhab = OpenHAB(oh_server_url)
        self.oh_item = self.openhab.get_item(self.item_name)
        super().__init__(mqtt_state_topic, mqtt_command_topic)

    def read(self):
        return int(self.oh_item.state)

    def write(self, value):
        self.oh_item.command(int(value))

