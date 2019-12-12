#ifndef QVIDEOLABLE_H
#define QVIDEOLABLE_H

#include <QObject>
#include <QLabel>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTimer>
class qVideoLable : public QLabel
{
    Q_OBJECT
public:
    qVideoLable(QWidget *parent = 0);
    virtual ~qVideoLable();
signals:
    void signalLableClicked();
    void signalLableDoubleClicked();
private slots:
    void slotTimeout();
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private:
    bool _mousePressed;
    int _clickedNum;
    QTimer *timer;
};

#endif // QVIDEOLABLE_H
