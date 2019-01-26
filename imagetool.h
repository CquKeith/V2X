#ifndef IMAGETOOL_H
#define IMAGETOOL_H

#include <QObject>
#include <QPixmap>
#include <QBuffer>
#include <QDebug>
#include <QDateTime>

QString pixmapToHex(const QPixmap &pix);
QPixmap hexToPixmap(QString hexStr, qint64 &delay);



#endif // IMAGETOOL_H
