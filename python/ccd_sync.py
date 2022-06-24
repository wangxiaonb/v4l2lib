import v4l2lib as v4l2
import cv2
import numpy as np
import sys
import time
import argparse
import os

# User Controls
exposure = 0x00980911  # (int) : min=4 max=906 step=1 default=800 value=800
horizontal_flip = 0x00980914  # (bool): default=0 value=0
vertical_flip = 0x00980915  # (bool): default=0 value=0

# Camera Controls
# (menu):min=0 max=2 default=2 value=2 flags=read-only
camera_orientation = 0x009a0922
# (int):min=0 max=0 step=1 default=0 value=0 flags=read-only
camera_sensor_rotation = 0x009a0923

# Image Source Controls
# (int) : min=21 max=32367 step=1 default=21 value=21
vertical_blanking = 0x009e0901
# (int) : min=816 max=816 step=1 default=816 value=816 flags=read-only
horizontal_blanking = 0x009e0902
analogue_gain = 0x009e0903  # (int) : min=16 max=248 step=1 default=16 value=16

# Image Processing Controls
# (intmenu): min=0 max=0 default=0 value=0 flags=read-only
link_frequency = 0x009f0901
# (int64)  : min=200000000 max=200000000 step=1 default=200000000 value=2000000
pixel_rate = 0x009f0902

##########################################################################
width, height = 1280, 800
# width, height = 1280, 720
# width, height = 640, 480
# width, height = 640, 400

##############################################################################
parser = argparse.ArgumentParser(description='camera display')
parser.add_argument('-n', '--number', type=str,
                    default='0', help='camera number')
args = parser.parse_args()

camera_num = args.number

camera = v4l2.open('/dev/video'+camera_num, width, height, 'GREY')
# v4l2.setformat(camera,width,height,'GREY')

v4l2.set_control(camera, exposure, 800)
value = v4l2.get_control(camera, exposure)

v4l2.set_control(camera, analogue_gain, 16)
value = v4l2.get_control(camera, analogue_gain)

v4l2.set_control(camera, vertical_blanking, 400)
value = v4l2.get_control(camera, vertical_blanking)

v4l2.active_fps(camera, 80)
# v4l2.print_fps(camera, 1)

v4l2.start(camera)

# os.system("i2cset -y -f 0 0x60 0x4f 0x00 0x01 i")
# os.system("i2cset -y -f 0 0x60 0x30 0x30 0x04 i")
# os.system("i2cset -y -f 0 0x60 0x30 0x3F 0x01 i")
# os.system("i2cset -y -f 0 0x60 0x30 0x2C 0x00 i")
# os.system("i2cset -y -f 0 0x60 0x30 0x2F 0x7F i")
# os.system("i2cset -y -f 0 0x60 0x38 0x23 0x30 i")
os.system("i2cset -y -f 0 0x60 0x01 0x00 0x00 i")

while True:
    data = v4l2.read(camera)
    # data = bytearray(data)

    image_array = np.frombuffer(data, dtype=np.uint8)
    image = image_array.reshape(height, width)

    # image = cv2.resize(image, (640, 400))
    # image = cv2.flip(image, 0)

    # image = cv2.cvtColor(image,cv2.COLOR_GRAY2BGR)

    fps = v4l2.get_fps(camera)
    image = cv2.putText(image, 'FPS:'+str(fps), (10, 30),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (127, 250, 127), 1)
    cv2.imshow('camera'+camera_num, image)
    key = cv2.waitKey(1)
    if key == ord('q'):
        break

v4l2.stop(camera)
v4l2.close(camera)
