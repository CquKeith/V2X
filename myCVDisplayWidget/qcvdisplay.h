#ifndef QCVDISPLAY_H
#define QCVDISPLAY_H

#include <QWidget>
#include <QException>
#include <QPainter>
#include <QResizeEvent>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

// 当图片不是灰度图或者 BGR 图像时抛出此异常
class UnsupportedFormatError : public QException
{
public:
    void raise() const { throw *this; }
    UnsupportedFormatError *clone() const { return new UnsupportedFormatError(*this); }
};

class QCVDisplay : public QWidget
{
    Q_OBJECT
public:
    explicit QCVDisplay(QWidget *parent = nullptr);
    cv::Mat getBuffer();
signals:
public slots:
    void displayImage(const cv::Mat& img);
    void displayImage(const QImage &img);

protected:
    void matToQImage(const cv::Mat3b &src, QImage &dest);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    QImage data;
    cv::Mat buffer;
public slots:
};

#endif // QCVDISPLAY_H
