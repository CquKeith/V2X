#include "workerudpreadobject.h"

WorkerUdpReadObject::WorkerUdpReadObject(QObject *parent, QString localIP, quint16 localPort) : QObject(parent),hostIP(localIP),hostPort(localPort)
{
    workthread = new QThread;
    moveToThread(workthread);
    workthread->start();

    frameRcvCount = 0;


    //使用信号触发槽函数，在槽函数中实例化UdpSocket 让其存在于新线程中
    connect(this,&WorkerUdpReadObject::signalUdpSocketStartListion,this,&WorkerUdpReadObject::udpSocketStartListion);
    emit signalUdpSocketStartListion();

}

WorkerUdpReadObject::~WorkerUdpReadObject()
{
    workthread->quit();

//    qDebug()<<tr("in %1() , thread id is:%2").arg(__FUNCTION__).arg((int)QThread::currentThreadId());
    //此处要小心，m_buf是在新进程中的，因此若是析构函数是由信号触发而执行的话，则不会有问题。否则，会出问题的
//    delete m_buf;
}

/*
 * 由信号触发，此函数运行在新的线程中
*/
void WorkerUdpReadObject::slot_ReadUdpDatagrams()
{
//    qDebug()<<tr("in %1() , thread id is:%2").arg(__FUNCTION__).arg((int)QThread::currentThreadId());

//    if(!canRecevInfo)return;


    char *recvBuf = new char[MAX_ONE_FRAME_SIZE];
//    char *m_buf;

//    memset(recvBuf, 0, MAX_ONE_FRAME_SIZE);

    //    qDebug()<<tr("in %1() , thread id is:%2").arg(__FUNCTION__).arg((int)QThread::currentThreadId());
    //此处需要注意，udpReceiver若是在构造函数中实例化，则其是在老线程中的，而当前函数是在新线程中运行的
    //因此 udpReceiver的实例化我们也要把它放在槽函数中
    while(udpReceiver->hasPendingDatagrams()){
        memset(recvBuf, 0, MAX_ONE_FRAME_SIZE);
        int size = udpReceiver->pendingDatagramSize();

        udpReceiver->readDatagram(recvBuf, size);
        //        qDebug()<<tr("in %1():while(udpReceiver->hasPendingDatagrams()) , thread id is:%2,read size is %3")
        //                  .arg(__FUNCTION__).arg((int)QThread::currentThreadId()).arg(size);


        PackageHead *mes = (PackageHead *)recvBuf;

        if (mes->msgType == MsgType::ImageType || mes->msgType == MsgType::VideoType) {

            //与GUI线程通过共享内存（m_buf这片空间）进行通信，所以这里要互斥的访问缓冲区
            {
//                QMutex mutex;
//                QMutexLocker locker(&mutex_mBuffer);

                char * m_buf;
                quint16 key = mes->uPicnum%MEM_CACHE_MAX_SIZE;
                /*memCacheMap中是否有此记录 如果有，则看这片内存是否已经用过。如果已经用过，则可以使用，否则就新建一片内存*/
                if(memCacheMapUdp.contains(key)){
                    s_memCache mem_cache = memCacheMapUdp[key];
//                    m_buf = mem_cache.memStart;
                    //此片内存别人已经用完，可以再次使用
                    if(mem_cache.isVisited){
                        m_buf = mem_cache.memStart;
                        mem_cache.memSize = 0;
                    }
//                    //别人正在使用，则自己新建一片内存空间，并加入到memCacheMap中
                    else{
                        //如果这片内存存的是本张图片的剩余部分
                        if(mem_cache.picNum == mes->uPicnum){
                            m_buf = mem_cache.memStart;
                        }
                        //如果这片内存存的是其他的图片
                        else{
                            m_buf = new char[MAX_IMAGE_SIZE];
                            mem_cache.isVisited = false;
                            mem_cache.memStart = m_buf;
                            mem_cache.memSize = 0;
                            memCacheMapUdp[key] = mem_cache;
                        }

                    }
                }
                //不包含，则是第一次来此key（picNum%MEM_CACHE_MAX_SIZE）
                else{
                    m_buf = new char[MAX_IMAGE_SIZE];
                    s_memCache mem_cache;
                    mem_cache.isVisited = false;
                    mem_cache.memStart = m_buf;
                    mem_cache.memSize = 0;
                    mem_cache.picNum = mes->uPicnum;
                    memCacheMapUdp.insert(key,mem_cache);
                }

                memcpy(m_buf+mes->uDataInFrameOffset, (recvBuf+ sizeof(PackageHead)), mes->uTransFrameSize);


//                qDebug()<<tr("receiving Image:%1/%2,CurrentSize/totalSize:%3/%4")
//                          .arg(mes->uDataFrameCurr).arg(mes->uDataFrameTotal)
//                          .arg(mes->uTransFrameSize).arg(mes->uDataFrameSize);
                mes->uRecDatatime= QDateTime::currentDateTime().toMSecsSinceEpoch();; //获取接收时间戳
                ++frameRcvCount;
//                emit signalSingleFrameDelay(++frame_count,mes->uRecDatatime - mes->uSendDatatime);
//                emit signal_ImageReceiveProcess((int)mes->uDataFrameCurr*100/mes->uDataFrameTotal,mes->uDataFrameCurr,mes->uDataFrameTotal);
                if (mes->uDataFrameCurr == mes->uDataFrameTotal) {
//                    qDebug()<<"received an image,then emit signal to GUI Thread";
                    emit sigRecvOk((int)mes->msgType,m_buf, mes->uDataFrameSize,(int)mes->video_quality_type);
                    emit signalSinglePicDelayAndFrameSize(mes->uPicnum,mes->uRecDatatime-mes->uSendDatatime,((double)(mes->uDataFrameSize+mes->uDataFrameTotal*mes->uTransFrameHdrSize))/1024);
                    memCacheMapUdp[key].isVisited = true;
                }
            }
        }else if(mes->msgType == MsgType::TextType ){
//            {
////                QMutex mutex;
//                QMutexLocker locker(&mutex_mBuffer);
//                memcpy(m_buf,(recvBuf+ sizeof(PackageHead)),mes->uTransFrameSize);
//            }
//            emit sigRecvOk((int)mes->msgType,m_buf, mes->uDataFrameSize);

        }

    }

    delete recvBuf;
}

/*
 * 此槽函数只能由信号触发，因为要delete新线程中的udpReceiver，并重新监听
*/
void WorkerUdpReadObject::ChangeUdpSocketListion(QString localIP,quint16 localPort)
{
    setHostIP(localIP);
    setHostPort(localPort);
//    delete m_buf;
    delete udpReceiver;
    emit signalUdpSocketStartListion();
}
/*
 * 私有槽函数，用于实例化udpSocket
*/
void WorkerUdpReadObject::udpSocketStartListion()
{
    udpReceiver = new QUdpSocket;
    canRecevInfo = true;
    QString ip;
    quint16 port;
    //加锁，访问hostIP和hostPort，因其在老进程，而此函数在新进程
    {
//        QMutex mutex;
//        QMutexLocker locker(&mutex_hostIPandPort);
        ip = hostIP;
        port = hostPort;
    }
    QString str;
    if(udpReceiver->bind(QHostAddress(ip),port)){
        str = tr("绑定%1：%2成功").arg(ip).arg(port);
    }else str = tr("绑定失败:%1").arg(udpReceiver->errorString());

//    m_buf = new char[MAX_IMAGE_SIZE];
    connect(udpReceiver,&QUdpSocket::readyRead,this,&WorkerUdpReadObject::slot_ReadUdpDatagrams);

//    qDebug()<<str;
    emit signalWorkerUdpReaderMsgDialog(WrokerObjectrMsgTypeToDlg::Information,str);

}

bool WorkerUdpReadObject::getCanRecevInfo() const
{
    return canRecevInfo;
}

void WorkerUdpReadObject::setCanRecevInfo(bool value)
{
    canRecevInfo = value;
}

quint64 WorkerUdpReadObject::getFrameRcvCount() const
{
    return frameRcvCount;
}

quint16 WorkerUdpReadObject::getHostPort() const
{
    //    QMutex mutex;
//    QMutexLocker locker(&mutex_hostIPandPort);
    return hostPort;
}

void WorkerUdpReadObject::setHostPort(const quint16 &value)
{
//    QMutex mutex;
//    QMutexLocker locker(&mutex_hostIPandPort);
    hostPort = value;
}

QString WorkerUdpReadObject::getHostIP() const
{
//    QMutex mutex;
//    QMutexLocker locker(&mutex_hostIPandPort);
    return hostIP;
}

void WorkerUdpReadObject::setHostIP(const QString &value)
{
//    QMutex mutex;
//    QMutexLocker locker(&mutex_hostIPandPort);
    hostIP = value;
}
