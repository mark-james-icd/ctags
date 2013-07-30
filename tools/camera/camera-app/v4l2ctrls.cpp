
#include <QLabel>
#include <QVector>
#include "v4l2utils.h"
#include <linux/videodev2.h>

#include <QHBoxLayout>
#include <QCheckBox>
#include <QSlider>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>

#include <sys/ioctl.h>

#include "v4l2ctrls.h"
#include "mainwindow.h"

V4L2Control::V4L2Control(int fd, const struct v4l2_queryctrl &ctrl, QWidget *parent) :
    QWidget(parent), cid(ctrl.id), default_value(ctrl.default_value)
{
    m_fd = fd;
    strncpy(name, (const char *)ctrl.name, sizeof(name));
    name[sizeof(name)-1] = '\0';
    this->setLayout(&layout);
    memcpy(&m_ctrl, &ctrl, sizeof(v4l2_queryctrl));
}

void V4L2Control::updateHardware()
{
    struct v4l2_control ctrl;
    ctrl.id = m_ctrl.id;
    ctrl.value = getValue();
    ioctl(m_fd, VIDIOC_S_CTRL, &ctrl);
}

void V4L2Control::updateStatus() {

    struct v4l2_control c;
    c.id = cid;
    if(ioctl(m_fd, VIDIOC_G_CTRL, &c) == -1) {
        QString msg;
        msg.sprintf("Unable to get %s\n%s", name, strerror(errno));
        //QMessageBox::warning(this, "Unable to get control", msg, "OK");
        qDebug("Unable to get control %s\n%s", name, strerror(errno));
    } else {
        if(c.value != getValue())
        setValue(c.value);
    }
    struct v4l2_queryctrl ctrl;
    ctrl.id = cid;
    if(ioctl(m_fd, VIDIOC_QUERYCTRL, &ctrl) == -1) {
        QString msg;
        msg.sprintf("Unable to get the status of %s\n%s", name, strerror(errno));
        //QMessageBox::warning(this, "Unable to get control status", msg, "OK");
        qDebug("Unable to get the status of %s\n%s", name, strerror(errno));
    } else {
        setEnabled(ctrl.flags == 0);
    }
}

void V4L2Control::resetToDefault() {
    if(isEnabled()) {
        setValue(default_value);
        updateHardware();
    }
}

V4L2IntegerControl::V4L2IntegerControl(int fd, const struct v4l2_queryctrl &ctrl, QWidget *parent):
    V4L2Control(fd, ctrl, parent), minimum(ctrl.minimum), maximum(ctrl.maximum), step(ctrl.step)
{
    int pageStep = (maximum-minimum)/10;
    if(step > pageStep)
        pageStep = step;
    sl = new QSlider(Qt::Horizontal, this);
    sl->setMinimum(minimum);
    sl->setMaximum(maximum);
    sl->setPageStep(pageStep);
    sl->setValue(default_value);
    //sl->setLineStep(step);
    sl->setVisible(true);
    this->layout.addWidget(sl);

    QString maxStr, minStr, defStr, inputMask;
    maxStr.setNum(maximum);
    minStr.setNum(minimum);
    defStr.setNum(default_value);
    int maxlen = maxStr.length() > minStr.length() ?
        maxStr.length() : minStr.length();
    inputMask.fill('0',maxlen);
    if(minimum < 0)
        inputMask.replace(0,1,'#');
    if(maximum < 0)
        inputMask.replace(0,1,'-');
    le = new QLineEdit(this);
    le->setText(defStr);
    le->setInputMask(inputMask);
    le->setMaxLength(maxlen);
    le->setValidator(new QIntValidator(minimum, maximum, this));
    this->layout.addWidget(le);

    QPushButton *pb;
    pb = new QPushButton("Reset", this);
    this->layout.addWidget(pb);
    QObject::connect(pb, SIGNAL(clicked()), this, SLOT(resetToDefault()) );

    QObject::connect( sl, SIGNAL(valueChanged(int)),
                      this, SLOT(SetValueFromSlider()) );
    QObject::connect( sl, SIGNAL(sliderReleased()),
                      this, SLOT(SetValueFromSlider()) );
    QObject::connect( le, SIGNAL(returnPressed()),
                      this, SLOT(SetValueFromText()) );
    updateStatus();
}

void V4L2IntegerControl::setValue(int val) {
    if(val < minimum)
        val = minimum;
    if(val > maximum)
        val = maximum;
    if(step > 1) {
        int mod = (val-minimum)%step;
        if(mod > step/2) {
            val += step-mod;
        } else {
            val -= mod;
        }
    }
    QString str;
    str.setNum(val);
    le->setText(str);

    /* FIXME: find clean solution to prevent infinite loop */
    sl->blockSignals(true);
    sl->setValue(val);
    sl->blockSignals(false);
}

int V4L2IntegerControl::getValue() {
    return sl->value();
}

void V4L2IntegerControl::SetValueFromSlider(void) {
    setValue(sl->value());
    updateHardware();
}

void V4L2IntegerControl::SetValueFromText(void) {
    if(le->hasAcceptableInput()) {
        setValue(le->text().toInt());
        updateHardware();
    } else {
        SetValueFromSlider();
    }
}


V4L2BooleanControl::V4L2BooleanControl(int fd, const struct v4l2_queryctrl &ctrl, QWidget *parent):
    V4L2Control(fd, ctrl, parent), cb(new QCheckBox(this))
{
    this->layout.addWidget(cb);
    QObject::connect( cb, SIGNAL(clicked()), this, SLOT(updateHardware()) );
    updateStatus();
}

void V4L2BooleanControl::setValue(int val) {
    cb->setChecked(val != 0);
}

int V4L2BooleanControl::getValue() {
    return cb->isChecked();
}


V4L2MenuControl::V4L2MenuControl(int fd, const struct v4l2_queryctrl &ctrl, QWidget *parent) :
    V4L2Control(fd, ctrl, parent)
{
    cb = new QComboBox(this);
    this->layout.addWidget(cb);

    for(int i=ctrl.minimum; i<=ctrl.maximum; i++) {
        struct v4l2_querymenu qm;
        qm.id = ctrl.id;
        qm.index = i;
        if(ioctl(fd, VIDIOC_QUERYMENU, &qm) == 0) {
            cb->insertItem(i, (const char *)qm.name);
        } else {
            QString msg;
            msg.sprintf("Unable to get menu item for %s, index=%d\n"
                    "Will use Unknown", name, qm.index);
            //QMessageBox::warning(this, "Unable to get menu item", msg, "OK");
            qDebug(msg.toStdString().c_str());
            cb->insertItem(i, "Unknown");
        }
    }
    cb->setCurrentIndex(default_value);
    QObject::connect( cb, SIGNAL(activated(int)),
                      this, SLOT(menuActivated(int)) );
    updateStatus();
}

void V4L2MenuControl::setValue(int val) {
    cb->setCurrentIndex(val);
}

int V4L2MenuControl::getValue() {
    return cb->currentIndex();
}

void V4L2MenuControl::menuActivated(int val) {
    setValue(val);
    updateHardware();
}


V4L2ButtonControl::V4L2ButtonControl(int fd, const struct v4l2_queryctrl &ctrl, QWidget *parent) :
    V4L2Control(fd, ctrl, parent)
{
    QPushButton *pb = new QPushButton((const char *)ctrl.name, this);
    this->layout.addWidget(pb);
    QObject::connect( pb, SIGNAL(clicked()), this, SLOT(updateHardware()) );
}

void V4L2ButtonControl::updateStatus() {
    struct v4l2_queryctrl ctrl;
    ctrl.id = cid;
    if(ioctl(m_fd, VIDIOC_QUERYCTRL, &ctrl) == -1) {
        QString msg;
        msg.sprintf("Unable to get the status of %s\n%s", name,
                    strerror(errno));
        //QMessageBox::warning(this, "Unable to get control status", msg, "OK");
        qDebug(msg.toStdString().c_str());
    } else {
        setEnabled(ctrl.flags == 0);
    }
}




v4l2ctrls::v4l2ctrls(MainWindow* mainWin) {

    mp_mainWin = mainWin;

    if (m_utils.openDevice()) {
        mp_grid = new QWidget(/*mp_mainWin*/);
        mp_gridLayout = new QGridLayout();
        mp_grid->setLayout(mp_gridLayout);

        struct v4l2_queryctrl ctrl;
        int device=m_utils.getDeviceId();
#ifdef V4L2_CTRL_FLAG_NEXT_CTRL
        /* Try the extended control API first */
        ctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
        if(0 == ioctl (device, VIDIOC_QUERYCTRL, &ctrl)) {
            do {
                add_control(ctrl);
                ctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
            } while(0 == ioctl (device, VIDIOC_QUERYCTRL, &ctrl));
        } else
#endif
        {
            /* Fall back on the standard API */
            /* Check all the standard controls */
            for(int i=V4L2_CID_BASE; i<V4L2_CID_LASTP1; i++) {
                ctrl.id = i;
                if(ioctl(device, VIDIOC_QUERYCTRL, &ctrl) == 0) {
                    add_control(ctrl);
                }
            }
        }

        /* Check any custom controls */
        for(int i=V4L2_CID_PRIVATE_BASE; ; i++) {
            ctrl.id = i;
            if(ioctl(device, VIDIOC_QUERYCTRL, &ctrl) == 0) {
                add_control(ctrl);
            } else {
                break;
            }
        }
        m_utils.closeDevice();

    }

}

v4l2ctrls::~v4l2ctrls() {

    delete mp_gridLayout;
    delete mp_grid;
}

void v4l2ctrls::add_control(struct v4l2_queryctrl &ctrl)
{
    QWidget *w = NULL;

    if(ctrl.flags & V4L2_CTRL_FLAG_DISABLED)
        return;

    QLabel *l = new QLabel((const char *)ctrl.name, mp_grid);
    mp_gridLayout->addWidget(l);

    int device=m_utils.getDeviceId();
    switch(ctrl.type) {
        case V4L2_CTRL_TYPE_INTEGER:
            w = new V4L2IntegerControl(device, ctrl, mp_grid);
            break;
        case V4L2_CTRL_TYPE_BOOLEAN:
            w = new V4L2BooleanControl(device, ctrl, mp_grid);
            break;
        case V4L2_CTRL_TYPE_MENU:
            w = new V4L2MenuControl(device, ctrl, mp_grid);
            break;
        case V4L2_CTRL_TYPE_BUTTON:
            w = new V4L2ButtonControl(device, ctrl, mp_grid);
            break;
        case V4L2_CTRL_TYPE_INTEGER64:
        case V4L2_CTRL_TYPE_CTRL_CLASS:
        default:
            break;
    }

    if(!w) {
        new QLabel("Unknown control", mp_grid);
        new QLabel(mp_grid);
        new QLabel(mp_grid);
        return;
    } else {
        m_qctrls.push_back((V4L2Control*)w);
    }

    mp_gridLayout->addWidget(w);
    if(ctrl.flags & V4L2_CTRL_FLAG_GRABBED) {
        w->setEnabled(false);
    }

    QPushButton *pb;
/*    pb = new QPushButton("Update", mp_grid);
    mp_gridLayout->addWidget(pb);
    QObject::connect( pb, SIGNAL(clicked()), w, SLOT(updateStatus()) );
    QObject::connect( mp_mainWin, SIGNAL(updateNow()), w, SLOT(updateStatus()) );*/

    if(ctrl.type == V4L2_CTRL_TYPE_BUTTON) {
        l = new QLabel(mp_grid);
        mp_gridLayout->addWidget(l);
    } else {
/*        pb = new QPushButton("Reset", mp_grid);
        mp_gridLayout->addWidget(pb);
        QObject::connect(pb, SIGNAL(clicked()), w, SLOT(resetToDefault()) );*/
//        QObject::connect(resetAllId, SIGNAL(triggered(bool)), w, SLOT(resetToDefault()) );
    }
}

void v4l2ctrls::setActiveCameraDevice(int fd)
{
    // set the active camera device id
    for (int i = 0; i < m_qctrls.size(); i++) {
        m_qctrls[i]->setActiveCameraDevice(fd);
    }
}

void v4l2ctrls::setDefaults()
{
    // set the defaults for all the controls
    for (int i = 0; i < m_qctrls.size(); i++) {
        m_qctrls[i]->resetToDefault();
    }
}

void v4l2ctrls::showWindow()
{
    mp_grid->setWindowFlags(Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint);
    mp_grid->setWindowTitle("Camera Diagnostics Controls");
    mp_grid->setVisible(true);
    // move to right side of camera window (** TO FINISH - NOT WORKING**)

    int x=mp_mainWin->pos().x();
    int y=mp_mainWin->y();
    mp_grid->setGeometry(x,y,mp_grid->geometry().width(),mp_grid->geometry().height());

}

void v4l2ctrls::closeWindow()
{
    mp_grid->close();
}
