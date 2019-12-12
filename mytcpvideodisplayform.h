#ifndef MYTCPVIDEODISPLAYFORM_H
#define MYTCPVIDEODISPLAYFORM_H

#include <QWidget>
#include "myVideoLable/qvideolable.h"

namespace Ui {
class MyTcpVideoDisplayForm;
}
class qVideoLable;

class MyTcpVideoDisplayForm : public QWidget
{
    Q_OBJECT

public:
    explicit MyTcpVideoDisplayForm(QWidget *parent = 0);
    virtual ~MyTcpVideoDisplayForm();

    void showVideoFrame(const QPixmap &pixmap);


private:
    Ui::MyTcpVideoDisplayForm *ui;
};

#endif // MYTCPVIDEODISPLAYFORM_H
