from servopoti.servopoti.readwrite import ReadWrite
from openhab import OpenHAB
print("hi")

class OpenHabHue(ReadWrite):
    def __init__(self, mqtt_state_topic: str, mqtt_command_topic: str, oh_server_url: str, item_name: str):
        self.oh_server_url = oh_server_url
        self.item_name = item_name
        self.openhab = OpenHAB(oh_server_url)
        self.oh_item = self.openhab.get_item(self.item_name)
        super().__init__(mqtt_state_topic, mqtt_command_topic)

    def read(self):
        return self.oh_item.state[0] // 3.59

    def write(self, value):
        print(f"{round(value * 3.59)},100,100")
        self.oh_item.command(f"{round(value * 3.59)},100,100")


