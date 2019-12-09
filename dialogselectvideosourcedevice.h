#ifndef DIALOGSELECTVIDEOSOURCEDEVICE_H
#define DIALOGSELECTVIDEOSOURCEDEVICE_H

#include <QDialog>
#include <QFileDialog>

#include "msghead.h"

namespace Ui {
class DialogSelectVideoSourceDevice;
}

class DialogSelectVideoSourceDevice : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSelectVideoSourceDevice(QWidget *parent = 0,QString device="");
    virtual ~DialogSelectVideoSourceDevice();

private slots:
    void on_pushButton_clicked();
    void on_rbFile_clicked();

signals:
    void signal_selectedDevice(int type,QString device);
private:
    Ui::DialogSelectVideoSourceDevice *ui;
    QString device;

    VideoDeviceType deviceType=VideoDeviceType::Camera;
};

#endif // DIALOGSELECTVIDEOSOURCEDEVICE_H
