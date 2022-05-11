import paho.mqtt.client as mqtt
import subprocess
import time

device_id = "1"


def on_connect(local_client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    client.subscribe("smartinput/#")


def on_message(local_client, userdata, msg):
    if msg.topic == f"smartinput/{device_id}/poti1":
        print("changing volume")
        subprocess.Popen(["amixer", "sset", "Master", str(int(655.36 * int(msg.payload.decode())))])

    if msg.topic == f"smartinput/{device_id}/poti2":
        print("changing volume")
        subprocess.Popen(["amixer", "sset", "spotify", str(int(655.36 * int(msg.payload.decode())))])

    if msg.topic == f"smartinput/{device_id}/button2":
        if msg.payload.decode() == "1":
            print("toggling player")
            subprocess.Popen(["playerctl", "play-pause"])

    if msg.topic == f"smartinput/{device_id}/button1":
        if msg.payload.decode() == "1":
            print("go back")
            subprocess.Popen(["playerctl", "previous"])

    if msg.topic == f"smartinput/{device_id}/button4":

        if msg.payload.decode() == "1":
            print("go back")
            subprocess.Popen(["playerctl", "next"])


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("192.168.178.33", 1883, 60)

client.loop_forever()


# 65536
