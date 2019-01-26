#ifndef WORKERUDPREADOBJECT_H
#define WORKERUDPREADOBJECT_H

#include <QObject>

#include <QThread>
#include <QUdpSocket>
#include <QHostAddress>

#include <QMutex>
#include <QMutexLocker>

#include "msghead.h"
#include <QDateTime>

class WorkerUdpReadObject : public QObject
{
    Q_OBJECT
public:
    explicit WorkerUdpReadObject(QObject *parent = nullptr,QString localIP="192.168.1.83",quint16 localPort=4040);
    ~WorkerUdpReadObject();

    QString getHostIP() const;
    void setHostIP(const QString &value);

    quint16 getHostPort() const;
    void setHostPort(const quint16 &value);

    quint64 getFrameRcvCount() const;
    __inline void clearFrameRcvCount(){frameRcvCount = 0;}



signals:
    void signalUdpSocketStartListion();
    void sigRecvOk(int msgtype, char *buf, int len);
    void signalSinglePicDelay(uint num,qint64 delaytime);
    void signalSingleFrameDelay(uint num,qint64 delaytime);
//    void sigRecvOk(int msgtype, QString, int len);
    void signalWorkerUdpReaderMsgDialog(int,QString);
    void signal_ImageReceiveProcess(int process,int currentFrame,int totalFrame);
public slots:
    void slot_ReadUdpDatagrams();
    void ChangeUdpSocketListion(QString localIP, quint16 localPort);
    bool getCanRecevInfo() const;
    void setCanRecevInfo(bool value);
private slots:
    void udpSocketStartListion();

private:
    QThread *workthread;
    QUdpSocket *udpReceiver;
    char *m_buf;

    //这里的目的地址和端口号都是在老线程中的
    //因此Getter和Setter都不是槽函数，而是可以通过对象访问的public函数
    QString hostIP;
    quint16 hostPort;
    quint64 frameRcvCount;//记录当前收到的是第几个帧（所有图片一起算）
    bool canRecevInfo;
};

#endif // WORKERUDPREADOBJECT_H
