from sys import stdout
import paho.mqtt.client as mqtt
import subprocess
import time

device_id = "1"


def on_connect(local_client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    client.subscribe("servopotis/1/#")


block_for = 0
last_local_change = 0

def on_message(local_client, userdata, msg):
    global last_local_change
    if msg.topic == f"servopotis/1/state":
        print(time.time() - last_local_change)
        last_local_change = time.time()

        subprocess.Popen(["amixer", "sset", "Master", str(
            int(655.36 * int(msg.payload.decode())))])


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("192.168.178.33", 1883, 60)


last_volume = 0
while True:
    # get system master volume
    volume = int(subprocess.run(
        ["amixer", "get", "Master"], capture_output=True, shell=True).stdout.decode().split('\n')[5].split(" ")[-2][1:-2])

    if abs(volume - last_volume) > 5:
        if time.time() - last_local_change > 0.1:
            client.publish(f"servopotis/1/cmnd", volume)
    last_volume = volume

    time.sleep(0.05)
    client.loop()

# 65536
