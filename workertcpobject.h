#ifndef WORKERTCPSENDOBJECT_H
#define WORKERTCPSENDOBJECT_H

#include <QObject>
#include <QThread>
#include <QTcpSocket>
#include <QTimer>
#include <QHostAddress>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QByteArray>
#include <QString>
#include <QDebug>
#include <QFile>
#include <QCoreApplication>
#include <QMapIterator>

#include "imagetool.h"
#include "msghead.h"



class WorkerTcpObject : public QObject
{
    Q_OBJECT
public:
    explicit WorkerTcpObject(QObject *parent = nullptr);
    virtual ~WorkerTcpObject();


    __inline quint64 getPicnum() const{return picnum;}
    __inline void clearPicNum(){picnum = 0;}

    QString getHostIP() const;
    void setHostIP(const QString &value);

    quint16 getHostPort() const;
    void setHostPort(const quint16 &value);

    void setWaitForReadyTime(int value);

signals:
    void finished();

    void signalWorkerTcpMsgDialog(int,QString);

    void signalTcpRecvOK(int msgtype, char *buf, int len);

    //frameSize : KB
    void signalSinglePicDelayAndFrameSize(uint num,qint64 delaytime,double frameSize);

    void signalStartTcp();

public slots:
    void tcpSendText(QString messge);
    void tcpSendImage(QString filepath, int msgtype, QString imageFormat);

    void slotTcpReadInfo();
    void slotTcpRecvVideo();
    void slotConnectToServer(const QString & ip, const quint16 &port);

    void slotStartTcp();


private:
    QThread *workthread;
    QTcpSocket *tcpSocket;

//    char *recvBuf;

    //这里的目的地址和端口号都是在老线程中的
    //因此Getter和Setter都不是槽函数，而是可以通过对象访问的public函数
    //服务器的IP和端口号
    QString hostIP;
    quint16 hostPort;
    QString id;//客户端的id

    //总共发了第几章图片
    quint64 picnum;


    int waitForReadyTime;

    quint32 hasRecvedSize;

    void readTcpInfo();
    void readTcpInfoByMultipleFrames();
    void readTcpInfoOneTime();

    void sendOneImageByMultipleFrames(QString filepath, int msgtype, QString imageFormat);
    void sendOneImageOneTime(QString filepath, int msgtype, QString imageFormat);

    //缓存上一次或多次的未处理的数据
    //这个用来处理，重新粘包
    QByteArray m_buffer;

    char *m_sendBuf;

    //以流的方式接收
     quint64 imageBlockSize;//图片的大小
     quint64 startTimestemp;//发送时候的时间戳
     quint64 imageNumberCurr;//当前接收的图片的id
     quint64 currentInterfaceType;//当前使用的接口是4G还是DSRC
//    char * m_buf;

     // Map的key是当前图片的id%MEM_CACHE_MAX_SIZE,value是这段内存的结构体
//     QMap<quint16,s_memCache> tcpMemCacheMap;
};

#endif // WORKERTCPSENDOBJECT_H
