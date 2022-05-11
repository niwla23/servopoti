from servopoti.servopoti.volume import VolumeController

volume_control = VolumeController("servopotis/1/state", "servopotis/1/cmnd")
volume_control.connect("192.168.178.33", 1883, 60)
volume_control.loop_forever()
