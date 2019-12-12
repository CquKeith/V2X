#include "comboxlabel.h"

ComboxLabel::ComboxLabel(const QString text, QWidget *parent)
{
    setText(text);
    setParent(parent);

    connect(&combox,&QComboBox::currentTextChanged,this,&ComboxLabel::setLabelText);


    combox.setParent(parent);
    combox.hide();

    combox.setStyleSheet(
            "QComboBox {background: transparent;color: rgb(255, 0, 255);}"
            "QComboBox::drop-down {background: transparent;}"
            "QComboBox::drop-down:hover {background:transparent;}"
            "QComboBox QAbstractItemView { background: transparent;}"
            "QComboBox QAbstractItemView::item:selected {background:transparent;}"
            );


    //将下拉框放置在label的下方
//    combox.move(this->x(),this->y()+this->height()+10);
//    combox.move(200,200);
//    combox.move(0,0);

}

ComboxLabel::~ComboxLabel()
{

}

void ComboxLabel::addComboboxText(QString text)
{
    combox.addItem(text);
}

/**
 * @brief ComboxLabel::mouseMoveEvent
 * @param ev
 * 鼠标划过，出现下拉框，失去焦点隐藏下拉框
 */
void ComboxLabel::mousePressEvent(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton){
//        qDebug()<<"mousePress";

        if(combox.isHidden()){
            combox.move(this->x(),this->y());
            this->hide();
            combox.show();
            //下拉栏显示的时候 自动产生下拉
            combox.showPopup();
        }
        else {
            combox.hide();
            this->show();
        }
    }
}

void ComboxLabel::setLabelText(QString t)
{
    this->setText(t);
    combox.hide();
    this->show();
}
