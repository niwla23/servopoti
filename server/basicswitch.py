import paho.mqtt.client as mqtt
import time

device_id = "1"


SNAP_POINT_TOP = 95
SNAP_POINT_BOTTOM = 5

def on_connect(local_client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    client.subscribe("servopotis/1/state")
    

pause_till = 0

def on_message(local_client, userdata, msg):
    global pause_till
    if msg.topic == f"servopotis/1/state":
        if time.time() < pause_till:
            return
        value = int(msg.payload.decode())
        if value > SNAP_POINT_BOTTOM and value < 50:
            client.publish(f"servopotis/1/cmnd", 100)
        elif value < SNAP_POINT_TOP and value > 50:
            client.publish(f"servopotis/1/cmnd", 0)
        else:
            return
        pause_till = time.time() + 0.5
        
            


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("192.168.178.33", 1883, 60)


last_volume = 0
while True:
    # get system master volume

    # if abs(volume - last_volume) > 2:
    #     if time.time() - last_local_change > 0.3:
    #         client.publish(f"servopotis/1/cmnd", volume)
    # last_volume = volume

    time.sleep(0.05)
    client.loop()

# 65536
