import cv2
import numpy as np
import time

cap = cv2.VideoCapture('libcamerasrc camera-name=''/base/soc/i2c0mux/i2c@1/imx477@1a'' ! video/x-raw, width=1344, height=990, framerate=120/1 ! videoscale ! appsink',
                       cv2.CAP_GSTREAMER)

fps_count = 0
fps = 0
ts = time.time()

while True:
    ret, image = cap.read()

    fps_count += 1
    if fps_count >= 30:
        t = time.time() - ts
        fps = int(round(fps_count / t, 0))
        fps_count = 0
        ts = time.time()

    image = cv2.resize(
        image, (int(image.shape[1]*0.5), int(image.shape[0]*0.5)))
    image = cv2.putText(image, 'FPS:'+str(fps), (10, 30),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (180, 250, 200), 1)

    cv2.imshow('camera', image)
    key = cv2.waitKey(1)
    if key == ord('q'):
        break
