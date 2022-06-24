import v4l2lib as v4l2
import cv2
import numpy as np
import sys
import time
import argparse

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

test_pattern = 0x009f0903
digital_gain =0x009f0905

##########################################################################
# width, height = 2028, 1520
width, height = 1332, 990
# width, height = 1280, 720
# width, height = 640, 480
# width, height = 640, 400

##############################################################################
parser = argparse.ArgumentParser(description='camera display')
parser.add_argument('-n', '--number', type=str,
                    default='0', help='camera number')
args = parser.parse_args()

camera_num = args.number

camera = v4l2.open('/dev/video'+camera_num, width, height, 'RG10')
# v4l2.setformat(camera,width,height,'GREY')

v4l2.set_control(camera, exposure, 2000)
# v4l2.set_control(camera, horizontal_flip, 0)
# v4l2.set_control(camera, vertical_flip, 0)

v4l2.set_control(camera, analogue_gain, 800)
v4l2.set_control(camera, digital_gain, 1600)

# v4l2.set_control(camera, test_pattern, 0)
# value = v4l2.get_control(camera, test_pattern)

v4l2.set_control(camera, vertical_blanking, 677)

value = v4l2.get_control(camera, vertical_blanking)
value = v4l2.get_control(camera, exposure)
value = v4l2.get_control(camera, analogue_gain)


v4l2.active_fps(camera, 30)
# v4l2.print_fps(camera, 1)

v4l2.start(camera)

# width, height = 2032, 1520
width, height = 1344, 990

while True:
    data = v4l2.read(camera)
    # data = bytearray(data)

    image_array = np.frombuffer(data, dtype=np.uint16)
    image_array = image_array >> 2
    image_array = np.array(image_array, dtype=np.uint8)
    image = image_array.reshape(height, width)
    # image = image >>4
    # image = np.array(image,dtype=np.uint8)

    image = cv2.cvtColor(image, cv2.COLOR_BayerRG2GRAY)
    # image = cv2.cvtColor(image, cv2.COLOR_BayerRG2BGR)

    image = cv2.resize(image, (int(width*0.6), int(height*0.6)))

    fps = v4l2.get_fps(camera)
    image = cv2.putText(image, 'FPS:'+str(fps), (10, 30),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (127, 250, 127), 1)
    cv2.imshow('camera'+camera_num, image)
    key = cv2.waitKey(1)
    if key == ord('q'):
        break

v4l2.stop(camera)
v4l2.close(camera)
