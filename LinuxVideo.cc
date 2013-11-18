
///////////////////
// LinuxVideo.cc //
///////////////////

// API documentation appears to be here:
//
//    http://linuxtv.org/downloads/v4l-dvb-apis/

#include <iostream>

#include <fcntl.h>
#include <libv4l2.h>
#include <linux/videodev2.h>

#include <cassert>
#include <string>
#include <cerrno>
#include <cstring>
#include <cstdlib>

using namespace std;

static string pixelformat_to_string(uint32_t format)
{
    string s;
    for (unsigned i = 0; i < 4; ++i)
    {
        s.push_back(format & 255);
        format >>= 8;
    }
    return s;
}

static void grab_images(const int fd)
{
    const unsigned HPIX = 1920;
    const unsigned VPIX = 1080;

    if (true) // query & set format
    {
        struct v4l2_format fmt;
        int ioctlResult;

        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        ioctlResult = v4l2_ioctl(fd, VIDIOC_G_FMT, &fmt);
        assert(ioctlResult == 0);

        cout << "BEFORE:" << endl << endl;

        cout << "fmt.pix.width ............. : " << fmt.fmt.pix.width                              << endl;
        cout << "fmt.pix.height ............ : " << fmt.fmt.pix.height                             << endl;
        cout << "fmt.pix.pixelformat ....... : " << pixelformat_to_string(fmt.fmt.pix.pixelformat) << endl;
        cout << "fmt.pix.field ............. : " << fmt.fmt.pix.field                              << endl;
        cout << "fmt.pix.bytesperline ...... : " << fmt.fmt.pix.bytesperline                       << endl << endl;

        fmt.fmt.pix.width       = HPIX;
        fmt.fmt.pix.height      = VPIX;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
        fmt.fmt.pix.field       = V4L2_FIELD_NONE;

        ioctlResult = v4l2_ioctl(fd, VIDIOC_S_FMT, &fmt);
        assert(ioctlResult == 0);

        cout << "AFTER:" << endl << endl;

        cout << "fmt.pix.width ............. : " << fmt.fmt.pix.width                              << endl;
        cout << "fmt.pix.height ............ : " << fmt.fmt.pix.height                             << endl;
        cout << "fmt.pix.pixelformat ....... : " << pixelformat_to_string(fmt.fmt.pix.pixelformat) << endl;
        cout << "fmt.pix.field ............. : " << fmt.fmt.pix.field                              << endl;
        cout << "fmt.pix.bytesperline ...... : " << fmt.fmt.pix.bytesperline                       << endl << endl;
    }

    unsigned size = VPIX * HPIX * 3;

    char * buf = new char[size];

    for (int frame = 0; frame < 100; ++frame)
    {
        cout << frame << endl;
        ssize_t readResult = v4l2_read(fd, buf, size);
        assert(readResult == size);
    }

    delete [] buf;
}

static bool try_resolution(const int fd, const unsigned width, const unsigned height)
{
    struct v4l2_format fmt;
    int ioctlResult;

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    ioctlResult = v4l2_ioctl(fd, VIDIOC_G_FMT, &fmt);
    assert(ioctlResult == 0);

    fmt.fmt.pix.width       = width;
    fmt.fmt.pix.height      = height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
    fmt.fmt.pix.field       = V4L2_FIELD_NONE;

    ioctlResult = v4l2_ioctl(fd, VIDIOC_S_FMT, &fmt);
    assert(ioctlResult == 0);

    return (fmt.fmt.pix.width == width) && (fmt.fmt.pix.height == height )&& (fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_RGB24) && (fmt.fmt.pix.field == V4L2_FIELD_NONE);
}

static unsigned gcd(unsigned a, unsigned b)
{
    while (a != 0)
    {
        b %= a;
        swap(a, b);
    }
    return b;
}

static void resolution_scan(int fd)
{
    for (unsigned height = 0; height <= 8000; height += 1)
    {
        if (height % 1 == 0)
        {
            cout << "Scanning: height = " << height << endl;
        }

        for (unsigned width = 0; width <= 8000; width += 1)
        {
            const bool ok = try_resolution(fd, width, height);
            if (ok)
            {
                unsigned g = gcd(width, height);
                cout << "Found supported resolution: " << width << "x" << height << " (" << (width / g) << ":" << (height / g) << " ; " << g << ")" << endl;
            }
        }
    }
}

static void show_formats(int fd)
{
    const unsigned num_types = 5;

    const unsigned types[num_types] = {
        V4L2_BUF_TYPE_VIDEO_CAPTURE,
        V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
        V4L2_BUF_TYPE_VIDEO_OUTPUT,
        V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE,
        V4L2_BUF_TYPE_VIDEO_OVERLAY
    };

    const char * type_names[num_types] = {
        "V4L2_BUF_TYPE_VIDEO_CAPTURE",
        "V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE",
        "V4L2_BUF_TYPE_VIDEO_OUTPUT",
        "V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE",
        "V4L2_BUF_TYPE_VIDEO_OVERLAY"
    };

    for (unsigned type_index = 0; type_index < num_types; ++type_index)
    {
        unsigned format_index = 0;

        while (1)
        {
            struct v4l2_fmtdesc format;

            format.type = types[type_index];
            format.index = format_index;

            int rv = v4l2_ioctl(fd, VIDIOC_ENUM_FMT, &format);
            if (rv != 0)
            {
                assert(rv == -1);
                assert(errno == EINVAL);
                break;
            }

            // We have a valid format!
            assert(format.type == types[type_index]);
            assert(format.index == format_index);

            cout << "format.type ............. : " << format.type << "(" << type_names[type_index] << ")" << endl;
            cout << "format.index ............ : " << format.index << endl;
            cout << "format.flags ............ : " << format.flags << endl;
            cout << "format.description ...... : \"" << format.description << "\"" << endl;
            cout << "format.pixelformat ...... : " << format.pixelformat << " (\"" << pixelformat_to_string(format.pixelformat) << "\")" << endl;
            cout << endl;

            // Now let's enumerate frame sizes for this pixel format...
            {
                unsigned framesize_index = 0;

                while (1)
                {
                    struct v4l2_frmsizeenum framesize;

                    framesize.index = framesize_index;
                    framesize.pixel_format = format.pixelformat;

                    int rv = v4l2_ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &framesize);

                    if (rv != 0)
                    {
                        assert(rv == -1);
                        assert(errno == EINVAL);
                        break;
                    }

                    assert(framesize.index == framesize_index);
                    assert(framesize.pixel_format == format.pixelformat);
                    assert(framesize.type == V4L2_FRMSIZE_TYPE_DISCRETE || framesize.type == V4L2_FRMSIZE_TYPE_CONTINUOUS || framesize.type == V4L2_FRMSIZE_TYPE_STEPWISE);

                    if (framesize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
                    {
                        cout << "  discrete: " << framesize.discrete.width << "x" << framesize.discrete.height << endl;
                    }

                    ++framesize_index;

                } // loop over frame sizes for the current pixel format
            }

            cout << endl;

            ++format_index;

        } // loop over pixel formats

    } // loop over the allowed types.
}

int main(int argc, char ** argv)
{
    const char * device_name = "/dev/video0";

    // Note: docs for open() state that the "flags" parameter has to use O_RDWR.
    int fd = v4l2_open(device_name, O_RDWR);

    if (fd < 0)
    {
        cout << "Error while opening device \"" << device_name << "\": " << strerror(errno) << endl;
        return EXIT_FAILURE;
    }

    assert(fd >= 0);

    for (int i = 1; i < argc; ++i)
    {
        string arg = argv[i];

        if (arg == "scan")
        {
            cout << "Performing resolution scan ..." << endl;
            resolution_scan(fd);
        }
        else if (arg == "formats")
        {
            cout << "Performing formats scan ..." << endl;
            show_formats(fd);
        }
        else if (arg == "grab")
        {
            cout << "Grab 100 images ..." << endl;
            grab_images(fd);
        }
    }

    int closeResult = v4l2_close(fd);
    assert(closeResult == 0);

    return EXIT_SUCCESS;
}
