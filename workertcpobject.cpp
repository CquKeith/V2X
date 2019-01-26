#include "workertcpobject.h"

WorkerTcpObject::WorkerTcpObject(QObject *parent) : QObject(parent)
{
    workthread = new QThread;
    moveToThread(workthread);
    workthread->start();

    picnum = 0;

    connect(this,&WorkerTcpObject::signalStartTcp,this,&WorkerTcpObject::slotStartTcp);

    emit signalStartTcp();

}

void WorkerTcpObject::slotConnectToServer(const QString &ip, const quint16 &port,const QString & id)
{
    tcpSocket->disconnectFromHost();
    tcpSocket->waitForDisconnected(3000);
    tcpSocket->connectToHost(QHostAddress(ip),port);
    tcpSocket->waitForConnected(3000);
    this->id = id;
}
/**
 * @brief WorkerTcpObject::slotStartTcp
 * 开始TCP线程的初始化
 */
void WorkerTcpObject::slotStartTcp()
{
    qDebug()<<"TCP初始化";
    tcpSocket = new QTcpSocket(this);
    waitForReadyTime = 0;
    connect(tcpSocket,&QTcpSocket::connected,[=]{
        qDebug()<<"连接上服务器";
        emit signalWorkerTcpMsgDialog(0,"成功连上服务器");
        QJsonObject json;
        json.insert("id",id);
        QJsonDocument jsonDocument;
        jsonDocument.setObject(json);
        QByteArray byteArray = jsonDocument.toJson(QJsonDocument::Compact);
        QString strJson(byteArray);
        strJson += "\n";
        tcpSocket->write(strJson.toUtf8(),strJson.length());
    });
    connect(tcpSocket,&QTcpSocket::disconnected,[=]{
       qDebug()<<"掉线了";
       emit signalWorkerTcpMsgDialog(0,"断开连接...");
    });
    connect(tcpSocket,&QTcpSocket::readyRead,this,&WorkerTcpObject::slotTcpReadInfo);
}

void WorkerTcpObject::setWaitForReadyTime(int value)
{
    waitForReadyTime = value;
}

QString WorkerTcpObject::getHostIP() const
{
    return hostIP;
}

void WorkerTcpObject::setHostIP(const QString &value)
{
    hostIP = value;
}

quint16 WorkerTcpObject::getHostPort() const
{
    return hostPort;
}

void WorkerTcpObject::setHostPort(const quint16 &value)
{
    hostPort = value;
}
/**
 * @brief WorkerTcpObject::tcpReadInfo
 * 接收服务器发来的消息
 */
void WorkerTcpObject::slotTcpReadInfo()
{
    static quint32 dataSize = 0;
    static quint32 dataRecvLen = 0;
    static QString msg = "";
//    static quint64 startTimeStamp = 0;

    while(tcpSocket->bytesAvailable()>0)
    {

        QByteArray datagram;

        datagram = tcpSocket->readAll();
        //        qDebug()<<"this time receive datagram size:" << datagram.size();

        msg += datagram;
        msg = msg.trimmed();
        dataRecvLen += msg.size();

        tcpSocket->waitForReadyRead(waitForReadyTime);

        if(msg.length() < 14) return;

        if(dataSize == 0){

            dataSize = msg.at(0).unicode();
//            dataSize *= 2;
            dataRecvLen -= 14;
        }

        if( dataSize > dataRecvLen){
//            qDebug()<< "dataSize:" << dataSize  <<"dataRecvLenAll:"<< dataRecvLen;
            return;
        }
    }

//    qDebug()<<"接收完毕："<<dataRecvLen;
//    picnum++;
//    qint64 stopTime = QDateTime::currentMSecsSinceEpoch();
//    int delay = stopTime - startTimeStamp;
//    if(delay > 99999){
//        qDebug()<<tr("delay=%1 = %2 - %3").arg(delay).arg(stopTime).arg(startTimeStamp);
//        emit signalWorkerTcpMsgDialog(0,tr("delay=%1 = %2 - %3").arg(delay).arg(stopTime).arg(startTimeStamp));
//        delay = 0;
//    }
//    if(startTimeStamp > 0){

//    }

    emit signalTcpRecvOK(msg,++picnum);
    msg.clear();
    dataRecvLen = 0;
    dataSize = 0;
//    startTimeStamp = 0;

}
void WorkerTcpObject::tcpSendText(QString messge){

}
void WorkerTcpObject::tcpSendImage(QString filepath, int msgtype, QString imageFormat){

    QPixmap pix(filepath);

    QJsonObject json;
    json.insert("id",id.toInt());

    QJsonArray desArray;
//    desArray.insert(0,id);
    desArray.insert(0,"002");
    desArray.insert(1,"003");
    json.insert("desID",desArray);

    QString content = pixmapToHex(pix);

    json.insert("content",content);

    QJsonDocument jsonDocument;
    jsonDocument.setObject(json);
    QByteArray byteArray = jsonDocument.toJson(QJsonDocument::Compact);
    byteArray.append('\n');

    tcpSocket->write(byteArray);

    ++picnum;

}
