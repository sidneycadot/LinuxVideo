
///////////////////
// LinuxVideo.cc //
///////////////////

// API documentation is here:
//
//    http://linuxtv.org/downloads/v4l-dvb-apis/

#include <fcntl.h>
#include <libv4l2.h>
#include <linux/videodev2.h>

#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <cassert>
#include <cerrno>
#include <cstring>
#include <cstdlib>

#include "Targa.h"

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

static void grab_images(const int fd, const unsigned width, const unsigned height, const unsigned num_frames)
{
    if (true) // query & set format
    {
        struct v4l2_format fmt;
        int ioctlResult;

        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        ioctlResult = v4l2_ioctl(fd, VIDIOC_G_FMT, &fmt);
        assert(ioctlResult == 0);

        fmt.fmt.pix.width       = width;
        fmt.fmt.pix.height      = height;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
        fmt.fmt.pix.field       = V4L2_FIELD_NONE;

        ioctlResult = v4l2_ioctl(fd, VIDIOC_S_FMT, &fmt);
        assert(ioctlResult == 0);
    }

    unsigned num_digits = to_string(num_frames - 1).size();

    Targa tga(width, height);
    const unsigned size = height * width * 3;

    for (unsigned frame = 0; frame < num_frames; ++frame)
    {
        ssize_t readResult = v4l2_read(fd, tga.bgr_data(), size);
        if (readResult != size)
        {
            cout << "readResult: " << readResult << endl;
            cout << "size: " << size << endl;
        }
        assert(readResult == size);

        ostringstream ss_filename;

        ss_filename << "frame_" << setw(num_digits) << frame << ".tga";

        cout << "Writing \"" << ss_filename.str() << "\" ..." << endl;
        std::ofstream f(ss_filename.str());
        f.write(reinterpret_cast<const char *>(tga.dvec().data()), tga.dvec().size());
        f.close();
    }
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

static void list_formats(int fd)
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
        // loop over the pixel formats

        for (unsigned format_index = 0; ; ++format_index)
        {
            struct v4l2_fmtdesc format;

            format.type = types[type_index];
            format.index = format_index;

            int rv = v4l2_ioctl(fd, VIDIOC_ENUM_FMT, &format);
            if (rv != 0)
            {
                // rv == -1 indicates we have exhausted all valid pixel formats.

                assert(rv == -1);
                assert(errno == EINVAL);
                break;
            }

            // We have a valid pixel format!

            assert(format.type == types[type_index]);
            assert(format.index == format_index);

            // Print it.

            cout << "format.type ............. : " << format.type << " (" << type_names[type_index] << ")" << endl;
            cout << "format.index ............ : " << format.index << endl;
            cout << "format.flags ............ : " << format.flags << endl;
            cout << "format.description ...... : \"" << format.description << "\"" << endl;
            cout << "format.pixelformat ...... : " << format.pixelformat << " (\"" << pixelformat_to_string(format.pixelformat) << "\")" << endl;
            cout << endl;

            // Now let's enumerate frame sizes for this pixel format...

            for (unsigned framesize_index = 0; ; ++framesize_index)
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

                switch (framesize.type)
                {
                    case V4L2_FRMSIZE_TYPE_DISCRETE:
                    {
                        const unsigned blocksize = gcd(framesize.discrete.width, framesize.discrete.height);
                        cout << "  discrete: " << framesize.discrete.width << "x" << framesize.discrete.height << " [" << (framesize.discrete.width / blocksize) << ":" << (framesize.discrete.height / blocksize) << " with " << blocksize << "x" << blocksize << " blocks]" << endl;
                        break;
                    }
                    case V4L2_FRMSIZE_TYPE_CONTINUOUS:
                    {
                        cout << "  continuous" << endl;
                        break;
                    }
                    case V4L2_FRMSIZE_TYPE_STEPWISE:
                    {
                        cout << "  stepwise" << endl;
                        break;
                    }
                    default:
                    {
                        assert(false);
                    }
                }
            } // loop over frame sizes for the current pixel format

            cout << endl;

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

    cout << "Device \"" << device_name << "\" succesfully opened, file descriptor = " << fd << "." << endl;

    for (int i = 1; i < argc; ++i)
    {
        string arg = argv[i];

        if (arg == "--list-formats")
        {
            cout << "Performing formats scan ..." << endl;
            list_formats(fd);
        }
        else if (arg == "--grab")
        {
            unsigned hpix = 640;
            unsigned vpix = 480;
            unsigned num_frames = 100;

            grab_images(fd, hpix, vpix, num_frames);
        }
        else
        {
            cout << "Unknown command line option \"" << arg << "\" ignored." << endl;
        }
    }

    int closeResult = v4l2_close(fd);
    assert(closeResult == 0);

    return EXIT_SUCCESS;
}
