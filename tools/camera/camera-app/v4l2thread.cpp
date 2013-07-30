#include "v4l2thread.h"
#include "v4l2utils.h"
#include <QApplication>
#include <QDataStream>
#include <QString>
#include <QDebug>
#include <QBuffer>
#include <QImage>
#include <typeinfo>
#include <QFile>
#include <QDateTime>
//#include <media/v4l2-chip-ident.h> // how do I get this?
#include "v4l2-chip-ident.h" // got a copy
#include "v4l2-dbg-bttv.h"
#include "v4l2-dbg-saa7134.h"
#include "v4l2-dbg-em28xx.h"
#include "v4l2-dbg-ac97.h"
#include "v4l2-dbg-tvp5150.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define ARRAY_SIZE(arr) ((int)(sizeof(arr) / sizeof((arr)[0])))

static const struct V4l2Thread::board_list boards[] = {
#define AC97_BOARD 0
    {				// From ac97-dbg.h
        AC97_IDENT,
        sizeof(AC97_PREFIX) - 1,
        ac97_regs,
        ARRAY_SIZE(ac97_regs),
        NULL,
        0,
    },
    {				// From bttv-dbg.h
        BTTV_IDENT,
        sizeof(BTTV_PREFIX) - 1,
        bt8xx_regs,
        ARRAY_SIZE(bt8xx_regs),
        bt8xx_regs_other,
        ARRAY_SIZE(bt8xx_regs_other),
    },
    {				// From saa7134-dbg.h
        SAA7134_IDENT,
        sizeof(SAA7134_PREFIX) - 1,
        saa7134_regs,
        ARRAY_SIZE(saa7134_regs),
        NULL,
        0,
    },
    {				// From em28xx-dbg.h
        EM28XX_IDENT,
        sizeof(EM28XX_PREFIX) - 1,
        em28xx_regs,
        ARRAY_SIZE(em28xx_regs),
        em28xx_alt_regs,
        ARRAY_SIZE(em28xx_alt_regs),
    },
    {				// From tvp5150-dbg.h
        TVP5150_IDENT,
        sizeof(TVP5150_PREFIX) - 1,
        tvp5150_regs,
        ARRAY_SIZE(tvp5150_regs),
        NULL,
        0,
    },
};

V4l2Thread::V4l2Thread(QObject *parent) : QThread(parent)
{
    runThread=false;
    fd = -1;
    m_capture=false;
    m_frameWidth=640;
    m_frameHeight=480;

}

void V4l2Thread::run(){

    // init device ////////////////////////////////////////////
    if (m_utils.openDevice()) {
        fd = m_utils.getDeviceId();
        m_v4l2Ctrls->setActiveCameraDevice(fd);
    } else {
        qDebug("Cannot open device");
        quit();
        return;
    }

    static struct v4lconvert_data *v4lconvert_data;
    static struct v4l2_format src_fmt;
    static unsigned char *dst_buf;

    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    CameraPrefs::getInstance()->getResolution(m_frameWidth, m_frameHeight);
    long pixelFormat = CameraPrefs::getInstance()->getPixelFormat();
    fmt.fmt.pix.width       = m_frameWidth;
    fmt.fmt.pix.height      = m_frameHeight;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24; //pixelFormat; //V4L2_PIX_FMT_RGB24;
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
    xioctl(fd, VIDIOC_S_FMT, &fmt);
/*    if (pixelFormat != V4L2_PIX_FMT_RGB24) {
           printf("Libv4l didn't accept RGB24 format. Can't proceed.\n");           
           return;
    }*/
/*    if ((fmt.fmt.pix.width != 640) || (fmt.fmt.pix.height != 480)) {
           printf("Warning: driver is sending image at %dx%d\n",
                   fmt.fmt.pix.width, fmt.fmt.pix.height);
    }*/

    v4lconvert_data = v4lconvert_create(fd);
    if (v4lconvert_data == NULL)
        qDebug("v4lconvert_create");
    if (v4lconvert_try_format(v4lconvert_data, &fmt, &src_fmt) != 0)
        qDebug("v4lconvert_try_format");
    xioctl(fd, VIDIOC_S_FMT, &src_fmt);
    char header [50];
    sprintf(header,"P6\n%d %d 255\n",fmt.fmt.pix.width,fmt.fmt.pix.height);
    int headerSz=qstrlen(header);
    dst_buf = (unsigned char*)malloc(fmt.fmt.pix.sizeimage+headerSz);
    memcpy(dst_buf,header,headerSz);

    //  init_mmap /////////////////////////////////////////////////
    CLEAR(req);
    req.count = 2;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    xioctl(fd, VIDIOC_REQBUFS, &req);

    buffers = (buffer*)calloc(req.count, sizeof(*buffers));
    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
           CLEAR(buf);

           buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
           buf.memory      = V4L2_MEMORY_MMAP;
           buf.index       = n_buffers;

           xioctl(fd, VIDIOC_QUERYBUF, &buf);

           buffers[n_buffers].length = buf.length;
           buffers[n_buffers].start = v4l2_mmap(NULL, buf.length,
                         PROT_READ | PROT_WRITE, MAP_SHARED,
                         fd, buf.m.offset);

           if (MAP_FAILED == buffers[n_buffers].start) {
                   qDebug("mmap");
                   //exit(EXIT_FAILURE);
                   return;
           }
    }

    for (unsigned int i = 0; i < n_buffers; ++i) {

        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        xioctl(fd, VIDIOC_QBUF, &buf);
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl(fd, VIDIOC_STREAMON, &type);
    qDebug()<<"buf.timecode.type:"<< buf.timecode.type;

    // main loop ///////////////////////////////////////////////////////
    int di=0;

    runThread=true;

    // For image capture
    QImage *qq;
    // Setting the frame rate
    setFramerate();
    // Timeout.
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    while(runThread) {

        // get the frame
        do {
            FD_ZERO(&fds);
            FD_SET(fd,&fds);

            r = select(fd + 1, &fds, NULL, NULL, &tv);
        } while ((r == -1 && (errno = EINTR)));

        if (r == -1) {
            qDebug("select");
            return;
        }

        // convert the image to be displayed
        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        xioctl(fd, VIDIOC_DQBUF, &buf);
            
        if (v4lconvert_convert(v4lconvert_data,
                                &src_fmt,
                                &fmt,
                                (unsigned char*)buffers[buf.index].start, buf.bytesused,
                                dst_buf+headerSz, fmt.fmt.pix.sizeimage) < 0) {
                if (errno != EAGAIN)
                        qDebug("v4l_convert");
        }

/*        unsigned char* asil=(unsigned char*)malloc(fmt.fmt.pix.sizeimage+qstrlen(header));
        memmove(asil, dst_buf, fmt.fmt.pix.sizeimage);
        memmove(asil+qstrlen(header), asil, fmt.fmt.pix.sizeimage);
        memcpy(asil,header,qstrlen(header));*/

        qq=new QImage();
        if(qq->loadFromData(/*asil*/dst_buf,fmt.fmt.pix.sizeimage+headerSz,"PPM")){
            emit renderedImage(*qq);            
        }
//        free(asil);
        if (m_capture) {
            qDebug("Capturing image...");
            QDateTime dateTime = QDateTime::currentDateTime();
            QString fileName = dateTime.toString() + ".png";
            qq->save(fileName);
            m_capture=false;
        }
        delete qq;

        xioctl(fd, VIDIOC_QBUF, &buf);
        di++;
    }    

    // clean up
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl(fd, VIDIOC_STREAMOFF, &type);
    for (unsigned int i = 0; i < n_buffers; ++i) {
        v4l2_munmap(buffers[i].start, buffers[i].length);
    }

    m_utils.closeDevice();
    fd = -1;
}

V4l2Thread::~V4l2Thread()
{    
    stopCapture();
}

void V4l2Thread::stopCapture()
{
    runThread=false;
    wait();
}

void V4l2Thread::startCapture()
{    
    start();
}

void V4l2Thread::resetControls()
{
    m_v4l2Ctrls->setDefaults();
}

void V4l2Thread::captureImage()
{
    m_capture=true;
}

/* sets video device frame rate
 * args:
 * vd: pointer to a VdIn struct ( must be allready initiated)
 *
 * returns: VIDIOC_S_PARM ioctl result value
*/
int V4l2Thread::setFramerate()
{
    long fps=0;
    long fps_num=0;
    int ret=0;
    struct v4l2_streamparm streamparm;
    memset(&streamparm,0,sizeof(streamparm));

    streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(fd, VIDIOC_G_PARM, &streamparm);
    if (ret < 0)
        return ret;

    if (!(streamparm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME))
        return -ENOTSUP;

    CameraPrefs::getInstance()->getFrameRate(fps, fps_num);
    streamparm.parm.capture.timeperframe.numerator = fps_num;
    streamparm.parm.capture.timeperframe.denominator = fps;

    ret = ioctl(fd,VIDIOC_S_PARM,&streamparm);
    if (ret < 0)
    {
        qDebug("Unable to set %ld/%ld fps\n", fps_num, fps);
    }

    /*make sure we now have the correct fps*/
//	input_get_framerate (device, fps, fps_num);

    return ret;
}
void V4l2Thread::getRegisters()
{
//#define V4L2_CHIP_MATCH_HOST       0  /* Match against chip ID on host (0 for the host) */
//#define V4L2_CHIP_MATCH_I2C_DRIVER 1  /* Match against I2C driver name */
//#define V4L2_CHIP_MATCH_I2C_ADDR   2  /* Match against I2C 7-bit address */
//#define V4L2_CHIP_MATCH_AC97       3  /* Match against anciliary AC97 chip */
    // test code below to get register values doesn't seem to work
    // believe v4l2 code to get/set registers is wrapped in CONFIG_VIDEO_ADV_DEBUG, and looks to not be set causing this to fail
    // this is the same for the cmd line v4l2-dbg -d /dev/video0 -l --verbose, which also fails
//#ifdef CONFIG_VIDEO_ADV_DEBUG
    struct v4l2_capability vcap;
    struct v4l2_dbg_register reg;
    struct v4l2_dbg_chip_ident chip_id;
    struct v4l2_dbg_match match;
    const struct board_list *curr_bd = NULL;
    memset(&reg, 0, sizeof(reg));
    memset(&chip_id, 0, sizeof(chip_id));
    match.type = V4L2_CHIP_MATCH_HOST;
    match.addr = 0;
    reg.match = match;
    unsigned long long reg_min = 0;
    unsigned long long reg_max = reg_max = 1<<31 - 1;
    int stride = 4;
    char buf[256];
    QFile file("reg_dump.log");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);

    try {
    ioctl(fd, VIDIOC_QUERYCAP, &vcap);
    sprintf(buf,"Video Driver-%s\n",(char *)vcap.driver);
    out << buf;
    for (int board = ARRAY_SIZE(boards) - 1; board >= 0; board--) {
        if (!strcasecmp((char *)vcap.driver, boards[board].name)) {
            curr_bd = &boards[board];
            break;
        }
    }
    if (curr_bd) {
        for (int i = 0; i < curr_bd->regs_size; i++) {
            if (((curr_bd->regs[i].reg >= reg_min) && curr_bd->regs[i].reg <= reg_max)) {
                reg.reg = curr_bd->regs[i].reg;

                if (ioctl(fd, VIDIOC_DBG_G_REGISTER, &reg) < 0)  {
                    sprintf(buf, "ioctl: VIDIOC_DBG_G_REGISTER failed for 0x%llx\n", reg.reg);
                    out << buf;
                } else {
                    const char *name = reg_name(curr_bd, reg.reg);

                    out << "Register ";

                    if (name) {
                        sprintf(buf, "%s (0x%08llx)", name, reg.reg);
                    } else {
                        sprintf(buf, "0x%08llx", reg.reg);
                    }
                    out << buf;

                    sprintf(buf, " = %llxh (%lldd  %sb)\n",reg.val, reg.val, binary(reg.val));
                    out << buf;
                }
            }
        }
    }
    /* try to figure out which chip it is */
    std::string name;
    chip_id.match = match;
    if (ioctl(fd, VIDIOC_DBG_G_CHIP_IDENT, &chip_id) != 0) {
        chip_id.ident = V4L2_IDENT_NONE;
    }
    switch (chip_id.ident) {
    case V4L2_IDENT_CX23415:
    case V4L2_IDENT_CX23416:
        name = "cx23416";
        break;
    case V4L2_IDENT_CX23418:
        name = "cx23418";
        break;
    case V4L2_IDENT_CAFE:
        name = "cafe";
        break;
    default:
        if (reg.match.type == V4L2_CHIP_MATCH_I2C_DRIVER)
            name = reg.match.name;
        break;
    }

    if (name == "saa7115") {
        dump_regs(out, &reg, 0, 0xff, stride);
    } else if (name == "saa717x") {
        // FIXME: use correct reg regions
        dump_regs(out, &reg, 0, 0xff, stride);
    } else if (name == "saa7127") {
        dump_regs(out, &reg, 0, 0x7f, stride);
    } else if (name == "ov7670") {
        dump_regs(out, &reg, 0, 0x89, stride);
    } else if (name == "cx25840") {
        dump_regs(out, &reg, 0, 2, stride);
        dump_regs(out, &reg, 0x100, 0x15f, stride);
        dump_regs(out, &reg, 0x200, 0x23f, stride);
        dump_regs(out, &reg, 0x400, 0x4bf, stride);
        dump_regs(out, &reg, 0x800, 0x9af, stride);
    } else if (name == "cs5345") {
        dump_regs(out, &reg, 1, 0x10, stride);
    } else if (name == "cx23416") {
        dump_regs(out, &reg, 0x02000000, 0x020000ff, stride);
    } else if (name == "cx23418") {
        dump_regs(out, &reg, 0x02c40000, 0x02c409c7, stride);
    } else if (name == "cafe") {
        dump_regs(out, &reg, 0, 0x43, stride);
        dump_regs(out, &reg, 0x88, 0x8f, stride);
        dump_regs(out, &reg, 0xb4, 0xbb, stride);
        dump_regs(out, &reg, 0x3000, 0x300c, stride);
    } else {
        /* unknown chip, dump 0-0xff by default */
        dump_regs(out, &reg, 0, 0xff, stride);
    }
    } catch(...) {
        qDebug("getRegisters Failed");
    }

    file.close();
//    xioctl(fd, VIDIOC_DBG_G_REGISTER, &reg);
//    qDebug("Register value=%d\n",reg.val);
//#endif
}

void V4l2Thread::dump_regs(QTextStream& out, struct v4l2_dbg_register *reg, unsigned long min, unsigned long max, int stride)
{
    unsigned long mask = stride > 1 ? 0x1f : 0x0f;
    unsigned long i;
    int line = 0;

    for (i = min & ~mask; i <= max; i += stride) {
        if ((i & mask) == 0 && line % 32 == 0) {
            if (stride == 4) {
                out << "\n                00       04       08       0C       10       14       18       1C";
            } else {
                out << "\n          00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F";
            }
        }
        char buf[256];
        if ((i & mask) == 0) {
            sprintf(buf, "\n%08lx: ", i);
            out << buf;
            line++;
        }
        if (i < min) {
            sprintf(buf, "%*s ", 2 * stride, "");
            out << buf;
            continue;
        }
        reg->reg = i;
        if (ioctl(fd, VIDIOC_DBG_G_REGISTER, reg) < 0) {
            sprintf(buf, "ioctl: VIDIOC_DBG_G_REGISTER failed for 0x%llx\n", reg->reg);
        } else {
            sprintf(buf, "%0*llx ", 2 * stride, reg->val);
        }
        out << buf;
        usleep(1);
    }
}

const char* V4l2Thread::reg_name(const struct board_list *curr_bd, unsigned long long reg)
{
    if (curr_bd) {
        for (int i = 0; i < curr_bd->regs_size; i++) {
            if (reg == curr_bd->regs[i].reg)
                return curr_bd->regs[i].name;
        }
        for (int i = 0; i < curr_bd->alt_regs_size; i++) {
            if (reg == curr_bd->regs[i].reg)
                return curr_bd->regs[i].name;
        }
    }
    return NULL;
}

const char* V4l2Thread::binary(unsigned long long val)
{
    static char bin[80];
    char *p = bin;
    int i, j;
    int bits = 64;

    if ((val & 0xffffffff00000000LL) == 0) {
        if ((val & 0xffff0000) == 0) {
            if ((val & 0xff00) == 0)
                bits = 8;
            else
                bits= 16;
        }
        else
            bits = 32;
    }

    for (i = bits - 1; i >= 0; i -= 8) {
        for (j = i; j >= i - 7; j--) {
            if (val & (1LL << j))
                *p++ = '1';
            else
                *p++ = '0';
        }
        *p++ = ' ';
    }
    p[-1] = 0;
    return bin;
}
