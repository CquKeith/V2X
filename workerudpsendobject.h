#ifndef WORKERUDPSENDOBJECT_H
#define WORKERUDPSENDOBJECT_H

#include <QObject>
#include <QThread>
#include <QUdpSocket>
#include <QHostAddress>

#include <QMutex>
#include <QMutexLocker>
#include <QDebug>
#include <QProcess>
#include <QCoreApplication>
#include <QTime>

#include <QFile>

#include "msghead.h"



class WorkerUdpSendObject : public QObject
{
    Q_OBJECT
public:
    explicit WorkerUdpSendObject(QObject *parent = nullptr);
    ~WorkerUdpSendObject();

    QString getHostIP() const;
    bool setHostIP(const QString &value);

    quint16 getHostPort() const;
    void setHostPort(const quint16 &value);

    void setHostIpPort(const QString & ip,const quint16 &port);

    __inline quint64 getFrameNum() const{return frameNum;}
    __inline void clearFrameNum(){frameNum = 0;}

    __inline quint64 getPicnum() const{return picnum;}
    __inline void clearPicNum(){picnum = 0;}

signals:
    void signal_ImageSendProcess(int process,int currentFrame,int totalFrame);

    void signalWorkerUdpSenderMsgDialog(int,QString);
public slots:
    void udpSendText(QString messge);
    void udpSendImage(QString filepath, int msgtype, QString imageFormat);
private:
    QThread *workthread;

    //这里的目的地址和端口号都是在老线程中的
    //因此Getter和Setter都不是槽函数，而是可以通过对象访问的public函数
    QString hostIP;
    quint16 hostPort;

    quint64 picnum;
    quint64 frameNum;//统计总共发送了多少“帧”
};

#endif // WORKERUDPSENDOBJECT_H
