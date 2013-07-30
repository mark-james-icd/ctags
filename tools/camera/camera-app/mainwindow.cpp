#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "v4l2utils.h"
#include <linux/types.h>
#include <linux/videodev2.h>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QtAlgorithms>
#include <QDirIterator>
#include <QMessageBox>

CameraPrefs* CameraPrefs::m_this = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mp_ctrls(0)
{
    ui->setupUi(this);

//    mp_ctrls = new v4l2ctrls(this);
//    ui->videowidget->thread.m_v4l2Ctrls = mp_ctrls;
    connect(&ui->videowidget->thread,SIGNAL(finished()),SLOT(videoDidnotStart()));

    // reset to defaults button
    connect(ui->action_Reset, SIGNAL(triggered()), this, SLOT(handleResetButton()));

    // capture image menu item
    connect(ui->actionCapture_Image, SIGNAL(triggered()), this, SLOT(handleCaptureButton()));

    connect(ui->action_Dump_Sensor_Registers, SIGNAL(triggered()), this, SLOT(handleDumpRegisters()));

    connect(ui->action_Exit, SIGNAL(triggered()), this, SLOT(handleExit()));

    // set up camera device/s
    getCameras();

    // set up resolutions
//    getSupportedResolutions();


//    ui->videowidget->thread.startCapture();
}

MainWindow::~MainWindow()
{
    ui->videowidget->thread.stopCapture();
    delete ui;
    if (mp_ctrls) delete mp_ctrls;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (mp_ctrls) mp_ctrls->closeWindow();
}

void MainWindow::showEvent(QShowEvent *event)
{
    if (mp_ctrls) mp_ctrls->showWindow();
}

void MainWindow::handleResetButton()
{
    ui->videowidget->thread.m_v4l2Ctrls->setDefaults();
}

void MainWindow::handleCaptureButton()
{
    ui->videowidget->thread.captureImage();
}

void MainWindow::handleWindowResolution()
{
    QAction* action = (QAction*)QObject::sender();
    int selected=0;
    // un-check others
    for (int i=0; i<mSupportedResolutions.size(); i++) {
        if (action != mSupportedResolutions[i].menuItem) {
            mSupportedResolutions[i].menuItem->setChecked(false);
        } else {
            selected=i;
        }
    }
    CameraPrefs::getInstance()->setResolution(mSupportedResolutions[selected].width, mSupportedResolutions[selected].height);
    CameraPrefs::getInstance()->setPixelFormat(mSupportedResolutions[selected].pixelFormat);
    //ui->videowidget->thread.setFrameWidth(mSupportedResolutions[selected].width);
    //ui->videowidget->thread.setFrameHeight(mSupportedResolutions[selected].height);
    // stop
    ui->videowidget->thread.stopCapture();
    // resize the windows
    this->resize(mSupportedResolutions[selected].width+20, mSupportedResolutions[selected].height+70);
    ui->videowidget->resize(mSupportedResolutions[selected].width, mSupportedResolutions[selected].height);
    // restart
    ui->videowidget->thread.startCapture();
}

void MainWindow::handleDumpRegisters()
{
    ui->videowidget->thread.getRegisters();
}

void MainWindow::handleCameraSelection()
{
    QAction* action = (QAction*)QObject::sender();
    int selected=0;
    // un-check others
    for (int i=0; i<mCameras.size(); i++) {
        if (action != mCameras[i]) {
            mCameras[i]->setChecked(false);
        } else {
            selected=i;
        }
    }
    initCamera(mCameras[selected]->text());
}

void MainWindow::handleFrameRateSelection()
{
    QAction* action = (QAction*)QObject::sender();
    int selected=0;
    // un-check others
    for (int i=0; i<mSupportedFrameRates.size(); i++) {
        if (action != mSupportedFrameRates[i].menuItem) {
            mSupportedFrameRates[i].menuItem->setChecked(false);
        } else {
            selected=i;
        }
    }
    // stop
    ui->videowidget->thread.stopCapture();
    CameraPrefs::getInstance()->setFrameRate(mSupportedFrameRates[selected].denominator, mSupportedFrameRates[selected].numerator);
    // restart
    ui->videowidget->thread.startCapture();
}

void MainWindow::handleExit()
{
    close();
}

void MainWindow::getCameras()
{
    // check if more than one camera device
    QDirIterator directories("/dev", QDir::Files | QDir::System);
    QString dir;
    QAction* pAction=NULL;

    QMenu *menu = ui->menuSelect_Camera;
    while(directories.hasNext()){
        directories.next();
        dir = directories.filePath();
        if (dir.contains("video")) {
            pAction = menu->addAction(dir, this, SLOT(handleCameraSelection()));
            pAction->setCheckable(true);
            mCameras.push_back(pAction);
        }
    }
    if (mCameras.size()==1) {
        mCameras[0]->setChecked(true);
        initCamera(mCameras[0]->text());
    }
    else if (mCameras.size()>1) {
        // notify user to select the camera device
        QMessageBox msgBox;
        msgBox.setText("Select the camera device from the camera menu.");
        msgBox.exec();
    }
}

bool resolutionLessThan(const QString &s1, const QString &s2)
{
    return s1.left(s1.toStdString().find(" ")).toInt() < s2.left(s2.toStdString().find(" ")).toInt();
}

void MainWindow::getSupportedResolutions()
{
    QMenu *menu = ui->menuResolution;

    v4l2utils utils;
    if (!utils.openDevice()) return;

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    struct v4l2_fmtdesc fmt;
    struct v4l2_frmsizeenum frmsize;
    struct v4l2_frmivalenum frmival;

    int fd=utils.getDeviceId();
    fmt.index = 0;
    fmt.type = type;
    QString str;
    QStringList resolutions;
    Resolution res;
    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmt) >= 0) {
    //if (ioctl(fd, VIDIOC_ENUM_FMT, &fmt) >= 0) {
        qDebug("Devices supporting formats: %s\n",fmt.description);
        if (V4L2_PIX_FMT_MJPEG == fmt.pixelformat) { // only supporting V4L2_PIX_FMT_MJPEG, V4L2_PIX_FMT_YUYV
            frmsize.pixel_format = fmt.pixelformat;
            frmsize.index = 0;
            while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) >= 0) {
                if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE || frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
                    str.sprintf("%d x %d",frmsize.discrete.width,frmsize.discrete.height);
                    res.width = frmsize.discrete.width;
                    res.height = frmsize.discrete.height;
                    res.pixelFormat = frmsize.pixel_format;
                    resolutions.push_back(str);
                    mSupportedResolutions.push_back(res);
                }
                frmsize.index++;
            }
        }
        fmt.index++;
    }
    // create a sorted resolution menu
    bool idealSz=false;
    qSort(resolutions.begin(),resolutions.end(),resolutionLessThan);
    QAction* pAction=NULL;
    int pos;
    int width;
    int height;
    for (int i=0; i<resolutions.size(); i++) {
        pAction = menu->addAction(resolutions[i], this, SLOT(handleWindowResolution()));
        pAction->setCheckable(true);
        pos=resolutions[i].toStdString().find(" ");
        width=resolutions[i].left(pos).toInt();
        height = resolutions[i].mid(pos+3).toInt();
        if (640 == width) {
            idealSz=true;
            pAction->setChecked(true);
            CameraPrefs::getInstance()->setResolution(width, height);
        }
        for (int j=0; j<mSupportedResolutions.size(); j++)
        {
            if (mSupportedResolutions[j].width==width && mSupportedResolutions[j].height==height) {
                // add the action to the supported resolutions list item
                mSupportedResolutions[j].menuItem = pAction;
                if (idealSz) CameraPrefs::getInstance()->setPixelFormat(mSupportedResolutions[j].pixelFormat);
                break;
            }
        }
    }
    utils.closeDevice();
    if (!idealSz) { // if 640x480 not found, use the first
        mSupportedResolutions[0].menuItem->setChecked(true);
        //ui->videowidget->thread.setFrameWidth(mSupportedResolutions[0].width);
        //ui->videowidget->thread.setFrameHeight(mSupportedResolutions[0].height);
        CameraPrefs::getInstance()->setResolution(mSupportedResolutions[0].width, mSupportedResolutions[0].height);
        CameraPrefs::getInstance()->setPixelFormat(mSupportedResolutions[0].pixelFormat);
    }
}

void MainWindow::initCamera(QString name)
{
    if (mp_ctrls) {
        mp_ctrls->closeWindow();
        delete mp_ctrls;
    }
    CameraPrefs::getInstance()->setCameraName(name);
    mp_ctrls = new v4l2ctrls(this);
    ui->videowidget->thread.m_v4l2Ctrls = mp_ctrls;

    // stop
    ui->videowidget->thread.stopCapture();

    // set up resolutions
    getSupportedResolutions();
    
    // set up frame rates
    getSupportedFrameRates();

    ui->videowidget->thread.startCapture();

    mp_ctrls->showWindow();
}

void MainWindow::getSupportedFrameRates()
{
    QMenu *menu = ui->menuSelect_Frame_Rate;

    v4l2utils utils;
    if (!utils.openDevice()) return;
    
    int fd=utils.getDeviceId();

    mSupportedFrameRates.clear();
    menu->clear();

    struct v4l2_fmtdesc fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.index = 0;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//	LFormats *listFormats = NULL;
//	listFormats = g_new0 ( LFormats, 1);
//	listFormats->listVidFormats = NULL;

/*    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmt) == 0) {

        fmt.index++;
        pixfmt = fmt.pixelformat;

    }*/

    int pixfmt=CameraPrefs::getInstance()->getPixelFormat();
    int width;
    int height;
    CameraPrefs::getInstance()->getResolution(width, height);

    // get the default fps
    struct v4l2_streamparm streamparm;
    int defFps=0;
    int defFps_num=0;

    streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd,VIDIOC_G_PARM,&streamparm) < 0)
    {
        perror("VIDIOC_G_PARM - Unable to get timeperframe");
    }
    else
    {
        if (streamparm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME) {
            // it seems numerator is allways 1 but we don't do assumptions here :-)
            defFps = streamparm.parm.capture.timeperframe.denominator;
            defFps_num = streamparm.parm.capture.timeperframe.numerator;
        }
    }
    if(defFps == 0 ) defFps = 1;
    if(defFps_num == 0) defFps_num = 1;
    CameraPrefs::getInstance()->setFrameRate(defFps, defFps_num);


//    int enum_frame_intervals(VidFormats *listVidFormats, __u32 pixfmt, __u32 width, __u32 height, int fmtind, int fsizeind, int fd)

    int ret=0;
    struct v4l2_frmivalenum frmrts;
    int list_fps=0;
    memset(&frmrts, 0, sizeof(frmrts));
    frmrts.index = 0;
    frmrts.pixel_format = pixfmt;
    frmrts.width = width;
    frmrts.height = height;
    //listVidFormats[fmtind-1].listVidCap[fsizeind-1].framerate_num=NULL;
    //listVidFormats[fmtind-1].listVidCap[fsizeind-1].framerate_denom=NULL;
    QAction* pAction=NULL;
    FrameRate frameRate;
    QString str;

    //g_print("\tTime interval between frame: ");
    while (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmrts) == 0)
    {
        frmrts.index++;
        if (frmrts.type == V4L2_FRMIVAL_TYPE_DISCRETE)
        {
            //g_print("%u/%u, ", fival.discrete.numerator, fival.discrete.denominator);

            list_fps++;
            /*listVidFormats[fmtind-1].listVidCap[fsizeind-1].framerate_num = g_renew(
                int, listVidFormats[fmtind-1].listVidCap[fsizeind-1].framerate_num, list_fps);
            listVidFormats[fmtind-1].listVidCap[fsizeind-1].framerate_denom = g_renew(
                int, listVidFormats[fmtind-1].listVidCap[fsizeind-1].framerate_denom, list_fps);

            listVidFormats[fmtind-1].listVidCap[fsizeind-1].framerate_num[list_fps-1] = fival.discrete.numerator;
            listVidFormats[fmtind-1].listVidCap[fsizeind-1].framerate_denom[list_fps-1] = fival.discrete.denominator;*/
            frameRate.numerator = frmrts.discrete.numerator;
            frameRate.denominator = frmrts.discrete.denominator;
            str.sprintf("%d/%d",frameRate.numerator,frameRate.denominator);
            pAction = menu->addAction(str, this, SLOT(handleFrameRateSelection()));
            pAction->setCheckable(true);
            if (frameRate.denominator==defFps) pAction->setChecked(true);
            frameRate.menuItem = pAction;
            mSupportedFrameRates.push_back(frameRate);
        }
        else if (frmrts.type == V4L2_FRMIVAL_TYPE_CONTINUOUS)
        {
            /*g_print("{min { %u/%u } .. max { %u/%u } }, ",
                fival.stepwise.min.numerator, fival.stepwise.min.numerator,
                fival.stepwise.max.denominator, fival.stepwise.max.denominator);*/
            break;
        }
        else if (frmrts.type == V4L2_FRMIVAL_TYPE_STEPWISE)
        {
            /*g_print("{min { %u/%u } .. max { %u/%u } / "
                "stepsize { %u/%u } }, ",
                fival.stepwise.min.numerator, fival.stepwise.min.denominator,
                fival.stepwise.max.numerator, fival.stepwise.max.denominator,
                fival.stepwise.step.numerator, fival.stepwise.step.denominator);*/
            break;
        }
    }

/*    if (list_fps==0)
    {
        listVidFormats[fmtind-1].listVidCap[fsizeind-1].numb_frates = 1;
        listVidFormats[fmtind-1].listVidCap[fsizeind-1].framerate_num = g_renew(
                int, listVidFormats[fmtind-1].listVidCap[fsizeind-1].framerate_num, 1);
        listVidFormats[fmtind-1].listVidCap[fsizeind-1].framerate_denom = g_renew(
                int, listVidFormats[fmtind-1].listVidCap[fsizeind-1].framerate_denom, 1);

        listVidFormats[fmtind-1].listVidCap[fsizeind-1].framerate_num[0] = 1;
        listVidFormats[fmtind-1].listVidCap[fsizeind-1].framerate_denom[0] = 1;
    }
    else
        listVidFormats[fmtind-1].listVidCap[fsizeind-1].numb_frates = list_fps;

    g_print("\n");
    if (ret != 0 && errno != EINVAL)
    {
        perror("VIDIOC_ENUM_FRAMEINTERVALS - Error enumerating frame intervals");
        return errno;
    }
    return 0;*/

    utils.closeDevice();
}
