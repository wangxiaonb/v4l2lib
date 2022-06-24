#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <libv4l2.h>

#include <sys/time.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

using namespace cv;

class clibv4l2
{
private:
    struct buffer
    {
        void *start;
        size_t length;
    };

    struct v4l2_format fmt;
    struct v4l2_buffer buf;
    struct v4l2_requestbuffers req;
    struct buffer *buffers;

    char dev_name[255];
    __u32 width = 1280;
    __u32 height = 800;
    __u32 colorfmt = V4L2_PIX_FMT_GREY;

    int fd = -1;
    unsigned int n_buffers;

    int fps_active = 0;
    int fps_print = 0;
    int fps_count = 0;
    int fps = 0;
    timeval t0;
    timeval t1;

public:
    /****
     * color: videodev2.h
     * V4L2_PIX_FMT_GREY    v4l2_fourcc('G', 'R', 'E', 'Y')
     */
    clibv4l2(const char *dev, __u32 width, __u32 height, const char *colorfmt)
    {
        strcpy(dev_name, dev);
        this->width = width;
        this->height = height;
        this->colorfmt = v4l2_fourcc(colorfmt[0], colorfmt[1], colorfmt[2], colorfmt[3]);
    }

    ~clibv4l2()
    {
    }

    void open(void)
    {
        fd = v4l2_open(dev_name, O_RDWR | O_NONBLOCK, 0);
        if (fd < 0)
        {
            perror("Cannot open device");
            exit(EXIT_FAILURE);
        }

        CLEAR(fmt);
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = width;
        fmt.fmt.pix.height = height;
        fmt.fmt.pix.pixelformat = colorfmt;
        fmt.fmt.pix.field = V4L2_FIELD_ANY;
        xioctl(fd, VIDIOC_S_FMT, &fmt);
        if (fmt.fmt.pix.pixelformat != colorfmt)
        {
            printf("Libv4l didn't accept color format. Can't proceed.\n");
            exit(EXIT_FAILURE);
        }
        if ((fmt.fmt.pix.width != width) || (fmt.fmt.pix.height != height))
            printf("Warning: v4l2 driver is sending image at %dx%d\n", fmt.fmt.pix.width, fmt.fmt.pix.height);

        CLEAR(req);
        req.count = 2;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;
        xioctl(fd, VIDIOC_REQBUFS, &req);

        buffers = (struct buffer *)calloc(req.count, sizeof(*buffers));
        for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
        {
            CLEAR(buf);

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = n_buffers;

            xioctl(fd, VIDIOC_QUERYBUF, &buf);

            buffers[n_buffers].length = buf.length;
            buffers[n_buffers].start = v4l2_mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

            if (MAP_FAILED == buffers[n_buffers].start)
            {
                perror("mmap");
                exit(EXIT_FAILURE);
            }
        }

        for (unsigned int i = 0; i < n_buffers; ++i)
        {
            CLEAR(buf);
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;
            xioctl(fd, VIDIOC_QBUF, &buf);
        }
    }

    void close()
    {
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        xioctl(fd, VIDIOC_STREAMOFF, &type);
        for (u_int i = 0; i < n_buffers; ++i)
            v4l2_munmap(buffers[i].start, buffers[i].length);
        v4l2_close(fd);
    }

    void set_control(int cid, int value)
    {
        struct v4l2_control ctrl;
        ctrl.id = cid;
        ctrl.value = value;
        ioctl(fd, VIDIOC_S_CTRL, &ctrl);

        // v4l2_set_control(fd, cid, value);
    }

    int get_control(int cid)
    {
        struct v4l2_control ctrl;
        ctrl.id = cid;
        ioctl(fd, VIDIOC_G_CTRL, &ctrl);
        return ctrl.value;

        // return v4l2_get_control(fd, cid);
    }

    void start_streaming(void)
    {
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        xioctl(fd, VIDIOC_STREAMON, &type);
        gettimeofday(&t0, NULL);
    }

    void stop_streaming(void)
    {
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        xioctl(fd, VIDIOC_STREAMOFF, &type);
    }

    void get_frame(void **start, size_t *length)
    {
        int r;
        fd_set fds;
        struct timeval tv;
        do
        {
            FD_ZERO(&fds);
            FD_SET(fd, &fds);

            /* Timeout. */
            tv.tv_sec = 2;
            tv.tv_usec = 0;

            r = select(fd + 1, &fds, NULL, NULL, &tv);
        } while ((r == -1 && (errno = EINTR)));
        if (r == -1)
        {
            perror("select");
            // return errno;
            return;
        }

        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        xioctl(fd, VIDIOC_DQBUF, &buf);

        *start = buffers[buf.index].start;
        *length = buf.bytesused;

        xioctl(fd, VIDIOC_QBUF, &buf);

        // calculate fps
        if (fps_active)
        {
            fps_count += 1;
            if (fps_count >= fps_active)
            {
                gettimeofday(&t1, NULL);
                float t = (t1.tv_sec - t0.tv_sec) * 1000 + (t1.tv_usec - t0.tv_usec) / 1000;
                fps = (int)(fps_count * 1000 / t);
                if (fps_print)
                    printf("FPS: %d\n", fps);
                fps_count = 0;
                gettimeofday(&t0, NULL);
            }
        }
    }

    /*****
     * active: the number of frame to calculate fps
     */
    void active_fps(int active)
    {
        fps_active = active;
    }

    void print_fps(int active)
    {
        fps_print = active;
    }

    int get_fps()
    {
        return fps;
    }

    void test(void)
    {
        char text[255];

        open();

        set_control(V4L2_CID_ANALOGUE_GAIN, 16);
        set_control(V4L2_CID_VBLANK, 500);

        gettimeofday(&t0, NULL);
        active_fps(60);
        print_fps(1);

        start_streaming();
        for (int i = 0; i < 2000; i++)
        {
            void *start;
            size_t length;
            get_frame(&start, &length);
            int fps = get_fps();
            sprintf(text, "FPS: %d", fps);
            Mat frame(height, width, CV_8UC1, (unsigned char *)start);
            if (fps_active)
                putText(frame, text, cv::Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(127, 250, 127), 1);
            imshow("image", frame);

            waitKey(1);
        }

        stop_streaming();
        close();
    }

private:
    void xioctl(int fh, int request, void *arg)
    {
        int r;
        do
        {
            r = v4l2_ioctl(fh, request, arg);
        } while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

        if (r == -1)
        {
            fprintf(stderr, "error %d, %s\n", errno, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
};
