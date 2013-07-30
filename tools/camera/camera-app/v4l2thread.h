#ifndef V4l2Thread_H
#define V4l2Thread_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QTextStream>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include "libv4l2.h"
#include "libv4lconvert.h"
#include "v4l2ctrls.h"
#include "v4l2utils.h"

class V4l2Thread : public QThread
{
    Q_OBJECT

public:

    V4l2Thread(QObject *parent = 0);
    ~V4l2Thread();

    void stopCapture();
    void startCapture();
    void resetControls();
    void captureImage();
    void setFrameWidth(int value) { m_frameWidth=value; }
    void setFrameHeight(int value) { m_frameHeight=value; }
    void getRegisters();
    v4l2ctrls* m_v4l2Ctrls;

    struct board_list {
        const char *name;
        int prefix; 		/* Register prefix size */
        const struct board_regs *regs;
        int regs_size;
        const struct board_regs *alt_regs;
        int alt_regs_size;
    };
protected:
    void run();

signals:
    void renderedImage(const QImage &image);

private:

    int setFramerate();
    void dump_regs(QTextStream& out, struct v4l2_dbg_register* reg, unsigned long min, unsigned long max, int stride);
    const char* reg_name(const struct board_list* curr_bd, unsigned long long reg);
    const char* binary(unsigned long long val);

    bool                            runThread;
    struct v4l2_format              fmt;
    struct v4l2_buffer              buf;
    struct v4l2_requestbuffers      req;
    enum v4l2_buf_type              type;
    fd_set                          fds;
    struct timeval                  tv;
    int                             r, fd;
    unsigned int                    n_buffers;
    char                            *dev_name;
    char                            out_name[256];
    FILE                            *fout;

    struct buffer {
            void   *start;
            size_t length;
    };
    struct buffer                   *buffers;
    bool                            m_capture;
    v4l2utils                       m_utils;
    int                             m_frameWidth;
    int                             m_frameHeight;
};

#endif // V4l2Thread_H
