#include <sys/time.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

// #include "libv4l2.hpp"

#include "v4l2lib.hpp"

// #define WIDTH 640
// #define HEIGHT 400

#define WIDTH 1280
#define HEIGHT 800

using namespace cv;

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int r;
    char text[255];

    clibv4l2 *v4l2 = new clibv4l2("/dev/video1", WIDTH, HEIGHT, "GREY");

    v4l2->open();

    v4l2->set_control(V4L2_CID_ANALOGUE_GAIN, 17);
    r = v4l2->get_control(V4L2_CID_ANALOGUE_GAIN);
    printf("get_control, V4L2_CID_ANALOGUE_GAIN %d\n",r);

    v4l2->set_control(V4L2_CID_VBLANK, 500);
    r = v4l2->get_control(V4L2_CID_VBLANK);
    printf("get_control, V4L2_CID_VBLANK %d\n",r);

    v4l2->active_fps(120);
    v4l2->print_fps(1);

    v4l2->start_streaming();
    for (int i = 0; i < 2000; i++)
    {
        void *start;
        size_t length;
        v4l2->get_frame(&start, &length);
        int fps = v4l2->get_fps();
        sprintf(text, "FPS: %d", fps);
        Mat frame(HEIGHT, WIDTH, CV_8UC1, (unsigned char *)start);
        putText(frame, text, cv::Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(127, 250, 127), 1);
        imshow("image", frame);

        waitKey(1);
    }

    v4l2->stop_streaming();
    v4l2->close();
    delete v4l2;
    return 0;
}