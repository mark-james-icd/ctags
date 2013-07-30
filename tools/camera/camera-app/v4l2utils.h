#ifndef V4L2UTILS_H
#define V4L2UTILS_H

#include <fcntl.h>
#include <linux/videodev2.h>
#include "libv4l2.h"
#include <errno.h>
#include <QString>


static void xioctl(int fh, int request, void *arg)
{
    int r;

    do {
            r = v4l2_ioctl(fh, request, arg);
    } while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

    if (r == -1) {
            fprintf(stderr, "error %d, %s\n", errno, strerror(errno));
            return;
    }
}


class CameraPrefs
{
public:
    CameraPrefs() {
    }

    static CameraPrefs* getInstance() {
        if (!m_this) {
            m_this = new CameraPrefs();
        }
        return m_this;
    }

    void setCameraName(QString cameraName) { mCameraName=cameraName; }
    QString getCameraName() { return mCameraName; }
    void setResolution(int x, int y) { mResX=x; mResY=y; }
    void getResolution(int &x, int &y) { x=mResX; y=mResY; }
    void setFrameRate(long fps, long fps_num) { mCameraFps=fps; mCameraFps_num=fps_num; }
    void getFrameRate(long &fps, long &fps_num) { fps=mCameraFps; fps_num=mCameraFps_num; }
    void setPixelFormat(long pxlFmt) { mPixelFmt=pxlFmt; }
    long getPixelFormat() { return mPixelFmt; }

private:
    static CameraPrefs* m_this;

    QString             mCameraName;
    int                 mResX;
    int                 mResY;
    long                mCameraFps;
    long                mCameraFps_num;
    long                mPixelFmt;
};

class v4l2utils
{
public:
    v4l2utils()
    {
        m_device=-1;
    }

    bool openDevice()
    {
        const char* name = CameraPrefs::getInstance()->getCameraName().toStdString().c_str(); //"/dev/video0"
        m_device = v4l2_open(name, O_RDWR | O_NONBLOCK, 0);
        return m_device >= 0;
    }

    void closeDevice()
    {
        v4l2_close(m_device);
        m_device = -1;
    }

    int getDeviceId() {return m_device; }
private:
    int m_device;
};
#endif // V4L2UTILS_H
