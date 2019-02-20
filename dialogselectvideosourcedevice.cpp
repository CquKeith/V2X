#include "dialogselectvideosourcedevice.h"
#include "ui_dialogselectvideosourcedevice.h"

DialogSelectVideoSourceDevice::DialogSelectVideoSourceDevice(QWidget *parent, QString device) :
    QDialog(parent),
    ui(new Ui::DialogSelectVideoSourceDevice)
{
    ui->setupUi(this);
    this->device = device;
    ui->label->setWordWrap(true);
    ui->label_deviceCurr->setText(device);
    ui->label_deviceCurr->setToolTip(device);
    ui->label_deviceCurr->setWordWrap(true);
    ui->label_deviceCurr->adjustSize();
}

DialogSelectVideoSourceDevice::~DialogSelectVideoSourceDevice()
{
    delete ui;
}

void DialogSelectVideoSourceDevice::on_pushButton_clicked()
{
    if(ui->rbDevice->isChecked()){
        device = ui->comboBox_deviceID->currentText();
        deviceType = VideoDeviceType::Camera;
    }else if(ui->rbFile->isChecked()){
        device = ui->label->text();
        deviceType = VideoDeviceType::File;
    }
    emit signal_selectedDevice((int)deviceType,device);

    close();
    deleteLater();
}
/**
 * @brief DialogSelectVideoSourceDevice::on_rbFile_clicked
 * 选择文件
 */
void DialogSelectVideoSourceDevice::on_rbFile_clicked()
{
    static QString defaultContents = "./";
    QString fileName = QFileDialog::getOpenFileName(this,tr("Choose File"),defaultContents,tr("Video File(*.mp4 *.avi *.mkv)"));
    if(fileName.isEmpty()){
        return;
    }
    ui->label->setText(fileName);
    ui->label->setToolTip(fileName);
    ui->label->adjustSize();
}
