import time
import paho.mqtt.client as mqtt
import subprocess
print("hhh")


class ReadWrite(mqtt.Client):
    """
    base class for applications where the control target needs to be written to 
    but the controller also needs the current value of the control target
    if changed by other means.
    """

    def __init__(self, mqtt_state_topic: str, mqtt_command_topic: str):
        super().__init__()
        self.mqtt_state_topic = mqtt_state_topic
        self.mqtt_command_topic = mqtt_command_topic
        self.last_control_target_change = 0
        self.last_value = None

    def connect(self, host, port=1883, keepalive=60, bind_address="", bind_port=0,
                clean_start=mqtt.MQTT_CLEAN_START_FIRST_ONLY, properties=None):
        super().connect(host, port, keepalive, bind_address,
                        bind_port, clean_start, properties)
        self.on_message = self.message_handler
        self.on_disconnect = self.disconnect_handler
        self.subscribe(self.mqtt_state_topic)

    def read(self):
        """returns the current value of the control target"""
        raise NotImplementedError

    def write(self, value: int):
        """sets the control target to the given value"""
        raise NotImplementedError

    def message_handler(self, local_client, userdata, msg):
        if msg.topic == f"servopotis/1/state":
            self.last_control_target_change = time.time()
            self.write(int(msg.payload.decode()))

    def disconnect_handler(self, local_client, userdata, rc):
        if rc != 0:
            print("Unexpected MQTT disconnection. Attempting to reconnect.")
            while not self.is_connected:
                self.reconnect()
                time.sleep(5)

    def loop(self):
        value = self.read()

        if self.last_value is None or abs(value - self.last_value) > 5:
            if time.time() - self.last_control_target_change > 0.1:
                self.publish(self.mqtt_command_topic, value)
        self.last_value = value

        super().loop()

    def loop_forever(self):
        while True:
            try:
                self.loop()
            except ConnectionRefusedError:
                print("error")
                continue
            time.sleep(0.05)


