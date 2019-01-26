#ifndef TOAST_H
#define TOAST_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class Toast;
}

class Toast : public QWidget
{
    Q_OBJECT

public:

    enum TimeLenth{
      SHORT=1000,
      MEDIUM=2000,
      LONG=3000,
    };

    explicit Toast(QWidget *parent = 0, QString msg="Toast",TimeLenth time_lenth=SHORT);
    ~Toast();


private:
    Ui::Toast *ui;
    quint16 timeLent;//显示的时长 单位ms
};

#endif // TOAST_H
