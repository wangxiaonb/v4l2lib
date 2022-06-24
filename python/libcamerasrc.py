import cv2
import numpy as np
import time

# use cv2 to open camera
cap = cv2.VideoCapture('libcamerasrc camera-name=''/base/soc/i2c0mux/i2c@0/ov2311@60'' ! video/x-raw, width=640, height=480, framerate=60/1 ! videoconvert ! videoscale ! appsink',
                       cv2.CAP_GSTREAMER)
# cap = cv2.VideoCapture('libcamerasrc camera-name=''/base/soc/i2c0mux/i2c@0/ov9281@60'' ! video/x-raw, width=1280, height=800, framerate=60/1 ! videoconvert ! videoscale ! appsink',
#                        cv2.CAP_GSTREAMER)

count = 0
t0 = time.time()
fps = 0
while True:
    ret, frame = cap.read()
    frame = cv2.flip(frame, 0)

    count += 1
    if count >= 30:
        t1 = time.time()
        fps = int(count/(t1-t0))
        t0 = time.time()
        count = 0

    frame = cv2.putText(frame, 'FPS:'+str(fps), (10, 30),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (127, 250, 127), 1)

    cv2.imshow('frame', frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
