#ifndef V4L2CTRLS_H
#define V4L2CTRLS_H

#include <QLabel>
#include <QVector>
#include "v4l2utils.h"
#include <linux/videodev2.h>

#include <QHBoxLayout>

class V4L2Control : public QWidget
{
    Q_OBJECT
public slots:
    virtual void updateStatus();

    virtual void resetToDefault();

    virtual void setValue(int val) = 0;

public:
    virtual int getValue() = 0;

    void updateHardware();
    void setActiveCameraDevice(int fd) {m_fd = fd;}

protected:
    V4L2Control(int fd, const struct v4l2_queryctrl &ctrl, QWidget *parent);
    int m_fd;
    int cid;
    int default_value;
    char name[32];
    QHBoxLayout layout;
    v4l2_queryctrl  m_ctrl;
};

class QSlider;
class QLineEdit;

class V4L2IntegerControl : public V4L2Control
{
    Q_OBJECT
public:
    V4L2IntegerControl(int fd, const struct v4l2_queryctrl &ctrl, QWidget *parent);
public slots:
    void setValue(int val);

public:
    int getValue();

private slots:
    void SetValueFromSlider(void);

    void SetValueFromText(void);

private:
    int minimum;
    int maximum;
    int step;
    QSlider *sl;
    QLineEdit *le;
};

class QCheckBox;
class V4L2BooleanControl : public V4L2Control
{
    Q_OBJECT
public:
    V4L2BooleanControl(int fd, const struct v4l2_queryctrl &ctrl, QWidget *parent);

public slots:
    void setValue(int val);

public:
    int getValue();

private:
    QCheckBox *cb;
};

class QComboBox;
class V4L2MenuControl : public V4L2Control
{
    Q_OBJECT
public:
    V4L2MenuControl(int fd, const struct v4l2_queryctrl &ctrl, QWidget *parent);

public slots:
    void setValue(int val);

public:
    int getValue();

private:
    QComboBox *cb;

private slots:
    void menuActivated(int val);
};

class V4L2ButtonControl : public V4L2Control
{
    Q_OBJECT
public slots:
    void updateStatus();

    void resetToDefault() {};

public:
    V4L2ButtonControl(int fd, const struct v4l2_queryctrl &ctrl, QWidget *parent);

public slots:
    void setValue(int) {};
    int getValue() { return 0; };
};

class MainWindow;
class QWidget;
class QGridLayout;

class v4l2ctrls
{
public:

    v4l2ctrls(MainWindow* mainWin);

    ~v4l2ctrls();

    void add_control(struct v4l2_queryctrl &ctrl);
    void setActiveCameraDevice(int fd);
    void setDefaults();
    void showWindow();
    void closeWindow();

    QVector<V4L2Control*>   m_qctrls;

private:
    v4l2utils       m_utils;
    MainWindow*     mp_mainWin;
    QWidget*        mp_grid;
    QGridLayout*    mp_gridLayout;

};

#endif // V4L2CTRLS_H
