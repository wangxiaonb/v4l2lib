import cv2
import time


cap = cv2.VideoCapture()

cap.open(0, apiPreference=cv2.CAP_V4L2)

cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('G', 'R', 'E', 'Y'))

cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 400)

cap.set(cv2.CAP_PROP_EXPOSURE, 801)

cap.set(cv2.CAP_PROP_ROLL, 1)
cap.set(cv2.CAP_PROP_GAIN, 16)
cap.set(cv2.CAP_PROP_AUTO_EXPOSURE, 0)
# cap.set(cv2.CAP_PROP_FPS, 30.0)


fps_count = 0
fps = 0
ts = time.time()
while(True):
    ret, frame = cap.read()

    fps_count += 1
    if fps_count >= 60:
        t = time.time() - ts
        fps = int(round(fps_count / t, 0))
        # print("fps:%d" % (fps))
        fps_count = 0
        ts = time.time()

    frame = cv2.putText(frame, 'FPS:'+str(fps), (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (127, 127, 127), 1)

    cv2.imshow('frame', frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# After the loop release the cap object
cap.release()
# Destroy all the windows
cv2.destroyAllWindows()
