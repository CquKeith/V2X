#include "workerudpsendobject.h"
/*
 * 构造函数和在这里new出来的变量都是在老线程中的
*/
WorkerUdpSendObject::WorkerUdpSendObject(QObject *parent) : QObject(parent)
{
    workthread = new QThread;
    moveToThread(workthread);
    workthread->start();

    connect(this,&WorkerUdpSendObject::finished,workthread,&QThread::quit);

    //目的端口和IP的默认值
    hostIP = "127.0.0.1";
    hostPort = 4040;
    picnum = 0;
    frameNum = 0;

//    qDebug()<<sizeof(PackageHead);
}
/*
 * 在主进程中 delete workerobject 的对象时，析构函数是在主线程运行的
 * 在主进程中 workerobject->deleteLater()时，析构函数是在新进程中执行的
 *
*/
WorkerUdpSendObject::~WorkerUdpSendObject()
{
//    workthread->quit();
    emit finished();
//    qDebug()<<tr("in %1() , thread id is:%2").arg(__FUNCTION__).arg((int)QThread::currentThreadId());
}
/*
 * 槽函数如果是直接通过对象调用，则此函数在老线程中执行
 * 如果是通过信号触发调用，则是在新线程中执行
 *
*/
void WorkerUdpSendObject::udpSendText(QString messge)
{
//    qDebug()<<tr("in %1() , thread id is:%2").arg(__FUNCTION__).arg((int)QThread::currentThreadId());

    //这里将目标地址和端口号 声明在当前函数，保证发送数据时 目标地址和端口号是在同一个线程中
    QHostAddress desIP;
    quint16 desPort;
    //这里将这个原子操作放在一个局部代码段中
    //代码段离开时会执行~QMutexLocker，从而unlock信号量
    {
//        QMutex mutex;
//        QMutexLocker locker(&mutex_hostIPandPort);
//         qDebug()<<locker.mutex();
        desIP.setAddress(hostIP);
        desPort = hostPort;
    }

    //UdpSocket在调用此函数时所在的线程中
    QUdpSocket udpsender;

    PackageHead head;
    int lenth_head = sizeof(PackageHead);
    int lenth_content = messge.toUtf8().length();
    char *msg = new char[lenth_content+lenth_head+1];
    memset(msg, 0, lenth_content+lenth_head+1);

    head.msgType = MsgType::TextType;
    head.uTransFrameSize = lenth_content;
    head.uTransFrameHdrSize = lenth_head;
    head.uDataFrameCurr = 1;
    head.uDataFrameSize = lenth_content;
    head.uDataInFrameOffset = 0;
    head.uDataFrameTotal = 1;


    memcpy(msg,(char*)&head,lenth_head);
    memcpy(msg+lenth_head,messge.toStdString().data(),lenth_content);

    udpsender.writeDatagram(msg,head.uTransFrameSize+head.uTransFrameHdrSize,desIP,desPort);

    delete msg;

}
/*
 * 根据文件目录 发送一张图片
 * 类型可以是单纯的图片或者是视频（意味着后面还有很多图片）
*/
void WorkerUdpSendObject::udpSendImage(QString filepath, int msgtype,QString imageFormat,int video_quality)
{
    //这里将目标地址和端口号 声明在当前函数，保证发送数据时 目标地址和端口号是在同一个线程中
    QHostAddress desIP;
    quint16 desPort;
    //这里将这个原子操作放在一个局部代码段中
    //代码段离开时会执行~QMutexLocker，从而unlock信号量
    {
//        QMutex mutex;
//        QMutexLocker locker(&mutex_hostIPandPort);
//        qDebug()<<locker.mutex();
        desIP.setAddress(hostIP);
        desPort = hostPort;
    }

    //UdpSocket在调用此函数时所在的线程中
    QUdpSocket udpsender;

    //下面开始组每一帧
    QFile imgfile(filepath);
    if(!imgfile.open(QIODevice::ReadOnly)){
        emit signalWorkerUdpSenderMsgDialog(WrokerObjectrMsgTypeToDlg::Critical,tr("打开文件失败:%1").arg(imgfile.errorString()));
        return;
    }

    char *m_sendBuf = new char[MAX_ONE_FRAME_SIZE];
    int packageContentSize = MAX_ONE_FRAME_SIZE - sizeof(PackageHead);

    int size = imgfile.size();
    int num = 0;
    int count = 0;
    int endSize = size%packageContentSize;
    if (endSize == 0) {
        num = size/packageContentSize;
    }
    else {
        num = size/packageContentSize+1;
    }

    ++picnum;

    while (count < num) {
        memset(m_sendBuf, 0, MAX_ONE_FRAME_SIZE);

        PackageHead mes;
        mes.msgType = (MsgType)msgtype;

//        mes.interfaceType = InterfaceType::DSRC;
        mes.video_quality_type = (VideoQualityType)video_quality;

        mes.uTransFrameHdrSize = sizeof(PackageHead);
        if ((count+1) != num) {
            mes.uTransFrameSize = packageContentSize;
        }
        else {
            mes.uTransFrameSize = endSize;
        }
        mes.uDataFrameSize = size;
        mes.uDataFrameTotal = num;
        mes.uDataFrameCurr = count+1;
        mes.uDataInFrameOffset = count*packageContentSize;

        mes.uPicnum = picnum;
        mes.uSendDatatime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        mes.uRecDatatime = 0;

        //放入图片的格式
//        memcpy(mes.imageFormat,imageFormat.toStdString().data(),imageFormat.length());

        imgfile.read(m_sendBuf+sizeof(PackageHead), packageContentSize);

        memcpy(m_sendBuf, (char *)&mes, sizeof(PackageHead));

        //放缓发送的速度
//        QTime dieTime = QTime::currentTime().addMSecs(1);
//        while( QTime::currentTime() < dieTime )
//            QCoreApplication::processEvents(QEventLoop::AllEvents,100);
        udpsender.writeDatagram(m_sendBuf, mes.uTransFrameSize+mes.uTransFrameHdrSize, desIP,desPort);
        ++frameNum;

        count++;
//        qDebug()<<tr("in %1():while (count < num),%2 / %3,CurrentSize/totalSize:%4/%5")
//                  .arg(__FUNCTION__).arg(count).arg(num)
//                  .arg(mes.uTransFrameSize).arg(mes.uDataFrameSize);
//        if(msgtype == MsgType::ImageType){
//            //告诉GUI进程 发送的进度
//            emit signal_ImageSendProcess((int)(count*100/num),count,num);
//        }
    }
    imgfile.close();
    delete m_sendBuf;
}



quint16 WorkerUdpSendObject::getHostPort() const
{
    return hostPort;
}

void WorkerUdpSendObject::setHostPort(const quint16 &value)
{
//    QMutex mutex;
//    QMutexLocker locker(&mutex_hostIPandPort);
//    qDebug()<<locker.mutex();
    hostPort = value;
//    emit signalWorkerUdpSenderMsgDialog(WrokerObjectrMsgTypeToDlg::Information,tr("设置接收方端口号 %1 成功").arg(value));
}
/**
 * @brief WorkerUdpSendObject::setHostIpPort 设置接收方的IP和端口号
 * @param ip
 * @param port
 */
void WorkerUdpSendObject::setHostIpPort(const QString &ip, const quint16 &port)
{
    if(setHostIP(ip)){
        setHostPort(port);
        emit signalWorkerUdpSenderMsgDialog(WrokerObjectrMsgTypeToDlg::Information,tr("设置接收方%1：%2成功").arg(ip).arg(port));
    }else{
        emit signalWorkerUdpSenderMsgDialog(WrokerObjectrMsgTypeToDlg::Warning,tr("未与%1ping通，请检查网络连接！").arg(ip));
    }
}

QString WorkerUdpSendObject::getHostIP() const
{
    return hostIP;
}

bool WorkerUdpSendObject::setHostIP(const QString &value)
{
//    QMutex mutex;
//    QMutexLocker locker(&mutex_hostIPandPort);
//    qDebug()<<locker.mutex();
//    hostIP = value;
    //检测该主机是否能够ping通
    QString cmdstr = QString("ping %1 -n 1 -w %2")
                .arg(value).arg(1000);
    QProcess cmd;
    cmd.start(cmdstr);
    cmd.waitForReadyRead(1000);
    cmd.waitForFinished(1000);
    QString response = cmd.readAll();
    if(response.indexOf("TTL") == -1)
//        emit signalWorkerUdpSenderMsgDialog(WrokerObjectrMsgTypeToDlg::Warning,tr("未与IP地址 %1 ping通，请检查网络连接").arg(value));
        return false;
    else
        {
//        emit signalWorkerUdpSenderMsgDialog(WrokerObjectrMsgTypeToDlg::Information,tr("设置接收方IP地址 %1 成功").arg(value));
        hostIP = value;
        return true;
    }

}
