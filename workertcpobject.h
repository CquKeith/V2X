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

#include "imagetool.h"

class WorkerTcpObject : public QObject
{
    Q_OBJECT
public:
    explicit WorkerTcpObject(QObject *parent = nullptr);



    __inline quint64 getPicnum() const{return picnum;}
    __inline void clearPicNum(){picnum = 0;}

    QString getHostIP() const;
    void setHostIP(const QString &value);

    quint16 getHostPort() const;
    void setHostPort(const quint16 &value);

    void setWaitForReadyTime(int value);

signals:
    void signalWorkerTcpMsgDialog(int,QString);

    void signalTcpRecvOK(QString imgdata,uint num);

    void signalStartTcp();

public slots:
    void tcpSendText(QString messge);
    void tcpSendImage(QString filepath, int msgtype, QString imageFormat);

    void slotTcpReadInfo();
    void slotConnectToServer(const QString & ip, const quint16 &port, const QString &id);

    void slotStartTcp();


private:
    QThread *workthread;
    QTcpSocket *tcpSocket;

    //这里的目的地址和端口号都是在老线程中的
    //因此Getter和Setter都不是槽函数，而是可以通过对象访问的public函数
    //服务器的IP和端口号
    QString hostIP;
    quint16 hostPort;
    QString id;//客户端的id

    //总共发了第几章图片
    quint64 picnum;

    int waitForReadyTime;
};

#endif // WORKERTCPSENDOBJECT_H
