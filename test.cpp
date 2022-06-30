#include <sys/time.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <opencv2/videoio.hpp>

#include <time.h>

// #include "libv4l2.hpp"

#include "v4l2lib.hpp"

// #define WIDTH 640
// #define HEIGHT 400

// #define WIDTH 640
// #define HEIGHT 480

// #define WIDTH 1280
// #define HEIGHT 960

// #define WIDTH 640
// #define HEIGHT 480

// #define WIDTH 1600
// #define HEIGHT 650

#define WIDTH 1280
#define HEIGHT 800


using namespace cv;

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    clock_t ts, te;
    double duration;

    int r;
    char text[255];

    clibv4l2 *v4l2 = new clibv4l2("/dev/video0", WIDTH, HEIGHT, "GREY");

    v4l2->open();

    v4l2->set_control(V4L2_CID_EXPOSURE, 800);

    v4l2->set_control(V4L2_CID_ANALOGUE_GAIN, 20);
    r = v4l2->get_control(V4L2_CID_ANALOGUE_GAIN);
    printf("get_control, V4L2_CID_ANALOGUE_GAIN %d\n", r);

    v4l2->set_control(V4L2_CID_VBLANK, 600);
    r = v4l2->get_control(V4L2_CID_VBLANK);
    printf("get_control, V4L2_CID_VBLANK %d\n", r);

    v4l2->active_fps(120);
    // v4l2->print_fps(1);

    v4l2->start_streaming();
    // for (int i = 0; i < 2000; i++)
    Mat resized;
    Mat image;
    Mat view;
    for (;;)
    {
        void *start;
        size_t length;
        v4l2->get_frame(&start, &length);
        // printf("length: %ld\r\n", length);
        int fps = v4l2->get_fps();
        sprintf(text, "FPS: %d", fps);
        Mat frame(HEIGHT, WIDTH, CV_8UC1, (unsigned char *)start);

        // ts = clock();
        // resize(frame, resized, Size(640, 480));
        // te = clock();
        // duration = (double)(te - ts) / CLOCKS_PER_SEC;
        // printf("%f seconds\n", duration);

        // flip(frame, image, 0);
        cvtColor(frame, view, COLOR_GRAY2BGR);
        putText(view, text, cv::Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(127, 250, 127), 1);
        imshow("view", view);

        waitKey(1);
    }

    v4l2->stop_streaming();
    v4l2->close();
    delete v4l2;
    return 0;
}
