#include "toast.h"
#include "ui_toast.h"

Toast::Toast(QWidget *parent, QString msg, TimeLenth time_lenth) :
    QWidget(parent),
    ui(new Ui::Toast)
{
    ui->setupUi(this);
    QTimer::singleShot(time_lenth,this,&Toast::close);


    ui->label->setText(msg);
    int maxlenth = ui->label->maximumWidth();
    ui->label->setMaximumWidth(10000);
    ui->label->adjustSize();



    if(ui->label->width() > maxlenth){//msg长度超过了最大长度
        ui->label->setWordWrap(true);
        ui->label->setMaximumWidth(maxlenth);
        resize(ui->label->width()+30,ui->label->height()+50);

    }
    else {
        ui->label->setWordWrap(false);
        resize(160, 50);
    }



    ui->label->setStyleSheet("background-color: rgb(116, 116, 116);border-color: rgb(79, 117, 127);\ncolor: rgb(255, 255, 255);border-radius:20px; ");

    this->move((parent->width()-width())/2,(parent->height()-height())/2);



}

Toast::~Toast()
{
    delete ui;
}
