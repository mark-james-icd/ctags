#include "videowidget.h"
#include <QPainter>
#include <QApplication>

VideoWidget::VideoWidget(QWidget *parent) :
    QWidget(parent)
{
    connect(&thread, SIGNAL(renderedImage(QImage)),
                    this, SLOT(setPicture(QImage)));

    setAutoFillBackground(true);
}

void VideoWidget::paintEvent(QPaintEvent */*event*/) {
    QPainter painter(this);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 30));
    painter.drawText(rect(), Qt::AlignCenter, "Qt");

    painter.drawPixmap(this->rect(),pixmap);
    //painter.drawPixmap(((QWidget*)parent())->rect(),pixmap);
}

void VideoWidget::setPicture(QImage i){
    pixmap=QPixmap::fromImage(i);
    update();
    qApp->processEvents();
}
