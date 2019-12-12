#include "mytcpvideodisplayform.h"
#include "ui_mytcpvideodisplayform.h"

MyTcpVideoDisplayForm::MyTcpVideoDisplayForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MyTcpVideoDisplayForm)
{
    ui->setupUi(this);

//    connect(ui->labelDisplayTcpVideo,&qVideoLable::signalLableClicked,[=]{

//    });
    connect(ui->labelDisplayTcpVideo,&qVideoLable::signalLableDoubleClicked,[=]{
//        qDebug()<<"双击" << ui->labelDisplayTcpVideo->windowFlags();
        static QFlags<Qt::WindowType> flags = ui->labelDisplayTcpVideo->windowFlags();


        if(ui->labelDisplayTcpVideo->isFullScreen()){
            ui->labelDisplayTcpVideo->setWindowFlags(flags);
            ui->labelDisplayTcpVideo->showNormal();
        }else{
            ui->labelDisplayTcpVideo->setWindowFlags(Qt::Dialog);
            ui->labelDisplayTcpVideo->showFullScreen();
        }
    });
}

MyTcpVideoDisplayForm::~MyTcpVideoDisplayForm()
{
    delete ui;
}

void MyTcpVideoDisplayForm::showVideoFrame(const QPixmap &pixmap)
{
    ui->labelDisplayTcpVideo->setPixmap(pixmap);
}

