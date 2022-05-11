from servopoti.servopoti.openhab_dimmer import OpenHabDimmer


volume_control = OpenHabDimmer("servopotis/1/state", "servopotis/1/cmnd", "http://192.168.178.33:8080/rest", "AlwinChromecast_Lautstarke")
volume_control.connect("192.168.178.33", 1883, 60)
volume_control.loop_forever()

