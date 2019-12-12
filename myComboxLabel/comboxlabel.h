#ifndef COMBOXLABEL_H
#define COMBOXLABEL_H

#include <QObject>
#include <QLabel>
#include <QComboBox>
#include <QMouseEvent>
#include <QFocusEvent>

#include <QDebug>

#include "msghead.h"

class ComboxLabel : public QLabel
{
    Q_OBJECT
public:
    ComboxLabel(const QString text="", QWidget *parent=Q_NULLPTR);
    ~ComboxLabel();

    void addComboboxText(QString text);

    VideoQualityType getCurrentVideoQuality(){return (VideoQualityType)combox.currentIndex();}

    void showCurrentVideoQuality(VideoQualityType q){combox.setCurrentIndex((int)q);}

protected:
    void mousePressEvent(QMouseEvent *ev);

private:
    QComboBox combox;

private slots:
    void setLabelText(QString t);

};

#endif // COMBOXLABEL_H
