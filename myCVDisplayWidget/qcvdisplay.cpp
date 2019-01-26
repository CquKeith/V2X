#include "qcvdisplay.h"

QCVDisplay::QCVDisplay(QWidget *parent) : QWidget(parent)
{

}

cv::Mat QCVDisplay::getBuffer()
{
    return buffer;
}
/**
 * @brief QCVDisplay::displayImage
 * @param img
 * 在控件上显示图片，使用 opencv 自带的 resize 使其缩放到和控件大小一致
 */
void QCVDisplay::displayImage(const cv::Mat &img)
{
    QSize sz = data.size();
    if (img.channels() == 3) {
        buffer = img.clone();
    } else if (img.channels() == 1) {
        cv::cvtColor(img, buffer, CV_GRAY2RGB);
    } else {
        throw UnsupportedFormatError();
    }
    cv::Mat resized;
    if (!sz.isEmpty()) {
        cv::resize(buffer, resized, cv::Size(sz.width(), sz.height()));
        matToQImage(resized, data);
        update();
    }

}
/**
 * @brief QCVDisplay::displayImage
 * @param img
 * 通过update的方法 直接展示QImage
 */
void QCVDisplay::displayImage(const QImage &img)
{
    data = img.copy(img.rect());
    update();

}
/**
 * @brief QCVDisplay::matToQImage
 * @param src
 * @param dest
 * 将 Mat 转换为 QImage，由于 QImage 的每一行有多余对齐字节
 * 故采用 RBG32 来消除多余字节
 */
void QCVDisplay::matToQImage(const cv::Mat3b &src, QImage &dest)
{
    for (int y = 0; y < src.rows; ++y) {
        const cv::Vec3b *srcrow = src[y];
        QRgb *destrow = (QRgb*)dest.scanLine(y);
        for (int x = 0; x < src.cols; ++x) {
            destrow[x] = qRgba(srcrow[x][2], srcrow[x][1], srcrow[x][0], 255);
        }
    }
}
/**
 * @brief QCVDisplay::paintEvent
 * @param event
 * 绘图事件处理函数
 */
void QCVDisplay::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawImage(event->rect(), data, event->rect());
}
/**
 * @brief QCVDisplay::resizeEvent
 * @param event
 * 缩放事件也采用 opencv 自带的 resize
 */
void QCVDisplay::resizeEvent(QResizeEvent *event)
{
    if (data.size() != event->size()) {
        cv::Mat resized;
        data = QImage(event->size(), QImage::Format_RGB32);
        if (!buffer.empty() && !event->size().isEmpty()) {
            cv::resize(buffer, resized, cv::Size(event->size().width(),
                                                 event->size().height()));
            matToQImage(resized, data);
        }
    }
    QWidget::resizeEvent(event);
}
