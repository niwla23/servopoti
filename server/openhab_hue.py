from servopoti.servopoti.openhab_hue import OpenHabHue
print("x")

volume_control = OpenHabHue("servopotis/1/state", "servopotis/1/cmnd", "http://192.168.178.33:8080/rest", "alwin_rgb_desk_color")
volume_control.connect("192.168.178.33", 1883, 60)
volume_control.loop_forever()

