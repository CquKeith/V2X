#include "workertcpobject.h"

WorkerTcpObject::WorkerTcpObject(QObject *parent) : QObject(parent)
{
    workthread = new QThread;
    moveToThread(workthread);
    workthread->start();

    picnum = 0;

    connect(this,&WorkerTcpObject::signalStartTcp,this,&WorkerTcpObject::slotStartTcp);

//    recvBuf = new char[65536];

    emit signalStartTcp();

    qDebug()<<__FUNCTION__<<QThread::currentThreadId();

}

WorkerTcpObject::~WorkerTcpObject()
{
//    delete recvBuf;
    delete m_sendBuf;
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
    qDebug()<<"TCP init";
    tcpSocket = new QTcpSocket;
    qDebug()<<__FUNCTION__<<QThread::currentThreadId();
    waitForReadyTime = 0;
    connect(tcpSocket,&QTcpSocket::connected,[=]{
        qDebug()<<"连接上服务器";
        emit signalWorkerTcpMsgDialog(0,"成功连上服务器");
        //        QJsonObject json;
        //        json.insert("id",id);
        //        QJsonDocument jsonDocument;
        //        jsonDocument.setObject(json);
        //        QByteArray byteArray = jsonDocument.toJson(QJsonDocument::Compact);
        //        QString strJson(byteArray);
        //        strJson += "\n";
        //        tcpSocket->write(strJson.toUtf8(),strJson.length());
    });
    connect(tcpSocket,&QTcpSocket::disconnected,[=]{
        qDebug()<<"掉线了";
        emit signalWorkerTcpMsgDialog(0,"断开连接...");
    });
    connect(tcpSocket,&QTcpSocket::readyRead,this,&WorkerTcpObject::slotTcpRecvVideo);

    hasRecvedSize = 0;

    m_sendBuf = new char[MAX_ONE_FRAME_SIZE];
}

void WorkerTcpObject::readTcpInfo()
{
    while(tcpSocket->bytesAvailable()){
//        memset(recvBuf, 0, 65536);
//        int size = tcpSocket->bytesAvailable();
//        hasRecvedSize += size;
//        tcpSocket->read(recvBuf, size);
        QByteArray datagram;
        datagram.resize(tcpSocket->bytesAvailable());
        datagram = tcpSocket->readAll();

        char *recvBuf = datagram.data();
        PackageHead *mes = (PackageHead *)datagram.data();

        if (mes->msgType == MsgType::ImageType || mes->msgType == MsgType::VideoType) {
            char * m_buf;
            quint16 key = mes->uPicnum%MEM_CACHE_MAX_SIZE;
            /*memCacheMap中是否有此记录 如果有，则看这片内存是否已经用过。如果已经用过，则可以使用，否则就新建一片内存*/
            if(memCacheMap.contains(key)){
                s_memCache mem_cache = memCacheMap[key];

                //此片内存别人已经用完，可以再次使用
                if(mem_cache.isVisited){
                    m_buf = mem_cache.memStart;
                    mem_cache.memSize = 0;
                }
                //正在使用，则自己新建一片内存空间，并加入到memCacheMap中
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
                        memCacheMap[key] = mem_cache;
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
                memCacheMap.insert(key,mem_cache);
            }
            memcpy(m_buf+mes->uDataInFrameOffset, (recvBuf+ sizeof(PackageHead)), mes->uTransFrameSize);

            mes->uRecDatatime= QDateTime::currentDateTime().toMSecsSinceEpoch();; //获取接收时间戳

            if (hasRecvedSize >= (mes->uDataFrameSize+mes->uDataFrameTotal*mes->uTransFrameHdrSize)) {
                emit signalTcpRecvOK((int)mes->msgType,m_buf, mes->uDataFrameSize);
                emit signalSinglePicDelayAndFrameSize(mes->uPicnum,mes->uRecDatatime-mes->uSendDatatime,((double)(mes->uDataFrameSize+mes->uDataFrameTotal*mes->uTransFrameHdrSize))/1024);
                memCacheMap[key].isVisited = true;
                hasRecvedSize = 0;
            }
        }
    }

}
/**
 * @brief WorkerTcpObject::readTcpInfoDealBug
 * 解决粘包的问题
 */
void WorkerTcpObject::readTcpInfoDealBug()
{
    //缓冲区没有数据，直接无视
    if(tcpSocket->bytesAvailable()<=0){
        return;
    }
    //临时获得从缓存区取出来的数据，但是不确定每次取出来的是多少。
    QByteArray buffer;
    //如果是信号readyRead触发的，使用readAll时会一次把这一次可用的数据全总读取出来
    //所以使用while(m_tcpClient->bytesAvailable())意义不大，其实只执行一次。
    buffer = tcpSocket->readAll();


    //上次缓存加上这次数据
    /**
        上面有讲到混包的三种情况，数据A、B，他们过来时有可能是A+B、B表示A包+B包中一部分数据，
        然后是B包剩下的数据，或者是A、A+B表示A包一部分数据，然后是A包剩下的数据与B包组合。
        这个时候，我们解析时肯定会残留下一部分数据，并且这部分数据对于下一包会有效，所以我们
        要和下一包组合起来。
    */
    m_buffer.append(buffer);

    int totalLen = m_buffer.size();
    while( totalLen ){
        //不够包头的数据直接就不处理。
        if( totalLen < sizeof(PackageHead) )
        {
            break;
        }
        char *recvBuf = buffer.data();
        PackageHead *mes = (PackageHead *)recvBuf;
        //如果不够长度，等够了再解析
        if(totalLen < mes->uTransFrameSize)break;

        //数据足够多，且满足我们定义的包头的几种类型
        if (mes->msgType == MsgType::ImageType || mes->msgType == MsgType::VideoType) {
            char * m_buf;
            quint16 key = mes->uPicnum%MEM_CACHE_MAX_SIZE;
            /*memCacheMap中是否有此记录 如果有，则看这片内存是否已经用过。如果已经用过，则可以使用，否则就新建一片内存*/
            if(memCacheMap.contains(key)){
                s_memCache mem_cache = memCacheMap[key];

                //此片内存别人已经用完，可以再次使用
                if(mem_cache.isVisited){
                    m_buf = mem_cache.memStart;
                    mem_cache.memSize = 0;
                }
                //正在使用，则自己新建一片内存空间，并加入到memCacheMap中
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
                        memCacheMap[key] = mem_cache;
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
                memCacheMap.insert(key,mem_cache);
            }
            memcpy(m_buf+mes->uDataInFrameOffset, (recvBuf+ sizeof(PackageHead)), mes->uTransFrameSize);

            mes->uRecDatatime= QDateTime::currentDateTime().toMSecsSinceEpoch();; //获取接收时间戳

            if (mes->uDataFrameCurr == mes->uDataFrameTotal) {
                emit signalTcpRecvOK((int)mes->msgType,m_buf, mes->uDataFrameSize);
                emit signalSinglePicDelayAndFrameSize(mes->uPicnum,mes->uRecDatatime-mes->uSendDatatime,((double)(mes->uDataFrameSize+mes->uDataFrameTotal*mes->uTransFrameHdrSize))/1024);
                memCacheMap[key].isVisited = true;
//                hasRecvedSize = 0;
            }
        }

        //缓存多余的数据
        buffer = m_buffer.right(totalLen - mes->uTransFrameSize - sizeof(PackageHead));

        //更新长度
        totalLen = buffer.size();

        //更新多余数据
        m_buffer = buffer;
    }


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
 * 接收服务器发来的消息(unused)
 */
void WorkerTcpObject::slotTcpReadInfo()
{
    //    static quint32 dataSize = 0;
    //    static quint32 dataRecvLen = 0;
    //    static QString msg = "";
    ////    static quint64 startTimeStamp = 0;

    //    while(tcpSocket->bytesAvailable()>0)
    //    {

    //        QByteArray datagram;

    //        datagram = tcpSocket->readAll();
    //        //        qDebug()<<"this time receive datagram size:" << datagram.size();

    //        msg += datagram;
    //        msg = msg.trimmed();
    //        dataRecvLen += msg.size();

    //        tcpSocket->waitForReadyRead(waitForReadyTime);

    //        if(msg.length() < 14) return;

    //        if(dataSize == 0){

    //            dataSize = msg.at(0).unicode();
    ////            dataSize *= 2;
    //            dataRecvLen -= 14;
    //        }

    //        if( dataSize > dataRecvLen){
    ////            qDebug()<< "dataSize:" << dataSize  <<"dataRecvLenAll:"<< dataRecvLen;
    //            return;
    //        }
    //    }

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

    //    emit signalTcpRecvOK(msg,++picnum);
    //    msg.clear();
    //    dataRecvLen = 0;
    //    dataSize = 0;
    //    startTimeStamp = 0;

}
/**
 * @brief WorkerTcpObject::slotTcpRecvVideo
 * Tcp接收服务器发来的数据（带有缓冲）
 */
void WorkerTcpObject::slotTcpRecvVideo()
{
    //    memset(recvBuf, 0, MAX_ONE_FRAME_SIZE);
//    readTcpInfo();
    readTcpInfoDealBug();
}
void WorkerTcpObject::tcpSendText(QString messge){

}
void WorkerTcpObject::tcpSendImage(QString filepath, int msgtype, QString imageFormat)
{

    //下面开始组每一帧
    QFile imgfile(filepath);
    if(!imgfile.open(QIODevice::ReadOnly)){
        emit signalWorkerTcpMsgDialog(WrokerObjectrMsgTypeToDlg::Critical,tr("打开文件失败:%1").arg(imgfile.errorString()));
        return;
    }

    qDebug()<<"tcpSendImageFile";

//    char *m_sendBuf = new char[MAX_ONE_FRAME_SIZE];
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
        tcpSocket->write(m_sendBuf, mes.uTransFrameSize+mes.uTransFrameHdrSize);
        //        tcpSocket->write("\n");
        tcpSocket->flush();

        tcpSocket->waitForBytesWritten();

        count++;

    }
    imgfile.close();

//    delete m_sendBuf;

}
