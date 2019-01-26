#include "imagetool.h"


QString pixmapToHex(const QPixmap &pix)
{
    QByteArray ba;
    QBuffer buf(&ba);
    buf.open(QIODevice::WriteOnly);
    pix.save(&buf,"JPG");

    QString str;

    int len = buf.data().size();
    str.append(len);

    str.append(ba.toHex());

     QString time = QString::number(QDateTime::currentMSecsSinceEpoch());
    str.append(time);

    return str;
}

QPixmap hexToPixmap(QString hexStr,qint64 &delay)
{
    //16进制字符串  转为uchar
    int num = hexStr.length() - 1; //除去 第一个 放置图片大小的 那个数

    num -= 13;//13个字节 存13位的时间戳 ms


    num /= 2;
    uchar date[num] = {0};

    bool ok;
    ushort len = hexStr.at(0).unicode();
    quint64 startTimeStamp = hexStr.right(13).toLongLong(&ok,10);
    quint64 stopTimeStamp = QDateTime::currentMSecsSinceEpoch();

    delay = stopTimeStamp - startTimeStamp;
    qDebug()<<"delay"<<delay;



//    ulong startTime = hexStr.mid(1,10).unicode();
//    qint64 startTimeStamp = hexStr.mid(1,13).unicode();
//    qint64 stopTimeStamp = QDateTime::currentMSecsSinceEpoch();

//    qDebug()<<"delay:"<<stopTimeStamp-startTimeStamp<<"ms";


    //    qDebug()<<"pix recv size:"<<len;
    int i=0;
    while(i<num)
    {
        date[i]=hexStr.mid(i*2+1,2).toInt(&ok,16);
        i++;
    }

    QPixmap pix;
    pix.loadFromData(date,num);
    return pix;
}
