#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QPixmap>

#include "v4l2thread.h"

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = 0);


    QPixmap pixmap;
    V4l2Thread thread;

protected:
    void paintEvent(QPaintEvent *event);
    
signals:
    
public slots:
    void setPicture(QImage);


};

#endif // VIDEOWIDGET_H
