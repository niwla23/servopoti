import paho.mqtt.client as mqtt
import time
import queue

device_id = "1"

snappoints = [0, 33, 66, 100]
TOLERANCE = 2

def on_connect(local_client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    client.subscribe("servopotis/1/state")
    

pause_till = 0
last_positions = []
current_snappoint = 0

def find_nearest_snap_point(position: int, upwards: bool = True):
    global current_snappoint
    # overwrite upwards if we are at the first or last snappoint
    if current_snappoint == 0:
        upwards = True
    elif current_snappoint == len(snappoints) - 1:
        upwards = False
    
    # if upwards, search the next snappoint from current position going up
    if upwards:
        point = next((i for i in snappoints if i > position), None)

    # if downwards, search the next snappoint from current position going down
    else:
        try:
            point = list(i for i in snappoints if i < position)[-1]
        except IndexError:
            point = snappoints[0]

    if point is None:
        return


    if abs(point - position) < TOLERANCE:
        return None

    current_snappoint = point
    return point

def is_list_increasing(l: list):
    """checks whether a list is strictly increasing"""

    return all(l[i] < l[i+1] for i in range(len(l)-1))

def on_message(local_client, userdata, msg):
    global pause_till
    global last_positions
    if msg.topic == f"servopotis/1/state":
        if time.time() < pause_till:
            return

        value = int(msg.payload.decode())

        last_positions.append(value)
        if len(last_positions) >= 3:
            upwards = is_list_increasing(last_positions)
        else:
            return
        
        point = find_nearest_snap_point(value, upwards)
        if point is not None:
            client.publish(f"servopotis/1/cmnd", point)
            last_positions = []


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
