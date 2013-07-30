#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "v4l2ctrls.h"

namespace Ui {

class MainWindow;

}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
protected:
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);

private slots:
    void handleResetButton();
    void handleCaptureButton();
    void handleWindowResolution();
    void handleDumpRegisters();
    void handleCameraSelection();
    void handleFrameRateSelection();
    void handleExit();

private:
    void getSupportedResolutions();
    void getSupportedFrameRates();
    void getCameras();
    void initCamera(QString name);
struct Resolution
{
    int width;
    int height;
    long pixelFormat;
    QAction* menuItem;
};
struct FrameRate
{
    long numerator;
    long denominator;
    QAction* menuItem;
};
    Ui::MainWindow *ui;
    v4l2ctrls* mp_ctrls;
    QVector<Resolution> mSupportedResolutions;
    QVector<FrameRate> mSupportedFrameRates;
    QVector<QAction*> mCameras;
};

#endif // MAINWINDOW_H
