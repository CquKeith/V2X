#include "qvideolable.h"

qVideoLable::qVideoLable(QWidget *parent)
{
    Q_UNUSED(parent)
    _mousePressed = false;
    _clickedNum = 0;
    timer = new QTimer(this);
    connect(timer,&QTimer::timeout,this,&qVideoLable::slotTimeout);
}

qVideoLable::~qVideoLable()
{
    timer->deleteLater();
}
/**
 * @brief qVideoLable::slotTimeout
 */
void qVideoLable::slotTimeout()
{
    if(_clickedNum == 1){
        emit signalLableClicked();
    }
    else if(_clickedNum >= 2){
        emit signalLableDoubleClicked();
    }
    _clickedNum = 0;
    timer->stop();
}

void qVideoLable::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
       _mousePressed = true;
    }
}

void qVideoLable::mouseReleaseEvent(QMouseEvent *event)
{
    if(_mousePressed && this->rect().contains(event->pos())){
        _clickedNum++;
        _mousePressed = false;
        timer->start(400);
    }
}
/**
 * @brief qVideoLable::keyReleaseEvent
 * Esc 退出全屏
 * @param event
 */
void qVideoLable::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape && this->isFullScreen()){
//        this->setWindowFlags(this->windowFlags()-3);
//        this->showNormal();
    }
}
