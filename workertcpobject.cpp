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
    workthread->quit();
    delete m_sendBuf;

}

void WorkerTcpObject::slotConnectToServer(const QString &ip, const quint16 &port)
{
    tcpSocket->disconnectFromHost();
    //    tcpSocket->waitForDisconnected(3000);
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
    imageBlockSize = 0;
    imageNumberCurr = 0;
    startTimestemp = 0;
    connect(tcpSocket,&QTcpSocket::connected,[=]{
        qDebug()<<"连接上服务器";
        emit signalWorkerTcpMsgDialog(0,"成功连上服务器");
    });
    connect(tcpSocket,&QTcpSocket::disconnected,[=]{
        qDebug()<<"掉线了";
        emit signalWorkerTcpMsgDialog(0,"连接断开...");
    });
    connect(tcpSocket,&QTcpSocket::readyRead,this,&WorkerTcpObject::slotTcpRecvVideo);

    hasRecvedSize = 0;

    m_sendBuf = new char[MAX_ONE_FRAME_SIZE];
}
/**
 * @brief WorkerTcpObject::readTcpInfo
 * 弃用
 */
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
            if(memCacheMapTcp.contains(key)){
                s_memCache mem_cache = memCacheMapTcp[key];

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
                        memCacheMapTcp[key] = mem_cache;
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
                memCacheMapTcp.insert(key,mem_cache);
            }
            memcpy(m_buf+mes->uDataInFrameOffset, (recvBuf+ sizeof(PackageHead)), mes->uTransFrameSize);

            mes->uRecDatatime= QDateTime::currentMSecsSinceEpoch();; //获取接收时间戳

            if (hasRecvedSize >= (mes->uDataFrameSize+mes->uDataFrameTotal*mes->uTransFrameHdrSize)) {
                emit signalTcpRecvOK((int)mes->msgType,m_buf, mes->uDataFrameSize);
                emit signalSinglePicDelayAndFrameSize(mes->uPicnum,mes->uRecDatatime-mes->uSendDatatime,((double)(mes->uDataFrameSize+mes->uDataFrameTotal*mes->uTransFrameHdrSize))/1024);
                memCacheMapTcp[key].isVisited = true;
                hasRecvedSize = 0;
            }
        }
    }

}
/**
 * @brief WorkerTcpObject::readTcpInfoDealBug
 * 解决粘包的问题
 * 弃用
 */
void WorkerTcpObject::readTcpInfoByMultipleFrames()
{
    //缓冲区没有数据，直接无视
    if(tcpSocket->bytesAvailable()<=0 || tcpSocket->bytesAvailable() < sizeof(PackageHead)){
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
            if(memCacheMapTcp.contains(key)){
                s_memCache mem_cache = memCacheMapTcp[key];

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
                        memCacheMapTcp[key] = mem_cache;
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
                memCacheMapTcp.insert(key,mem_cache);
            }
            memcpy(m_buf+mes->uDataInFrameOffset, (recvBuf+ sizeof(PackageHead)), mes->uTransFrameSize);

            mes->uRecDatatime = QDateTime::currentMSecsSinceEpoch(); //获取接收时间戳

            if (mes->uDataFrameCurr == mes->uDataFrameTotal) {
                emit signalTcpRecvOK((int)mes->msgType,m_buf, mes->uDataFrameSize);
                emit signalSinglePicDelayAndFrameSize(mes->uPicnum,mes->uRecDatatime-mes->uSendDatatime,((double)(mes->uDataFrameSize+mes->uDataFrameTotal*mes->uTransFrameHdrSize))/1024);
                memCacheMapTcp[key].isVisited = true;
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
/**
 * @brief WorkerTcpObject::readTcpInfoOneTime
 * 以流的形式 每次接收一张图片
 */

void WorkerTcpObject::readTcpInfoOneTime()
{
    forever{

        QByteArray message;//存放从服务器接收到的字节流数据
        QDataStream in(tcpSocket);	//将客户端套接字与输入数据流对象in绑定

        in.setVersion(QDataStream::Qt_5_9);//设置数据流的版本


        /*接收端的 这部分控制逻辑很重要*/
        if (imageBlockSize == 0)
        {
            //如果imageBlockSize == 0 则说明,一幅图像的大小信息还未传输过来

            //uint64是8字节的8 Bytes  64bit
            //判断接收的数据是否有8字节（文件大小信息）
            //如果有则保存到basize变量中，没有则返回，继续接收数据
            if (tcpSocket->bytesAvailable() < (int)sizeof(quint64))
            {
                //一幅图像的大小信息还未传输过来
//                QCoreApplication::processEvents(QEventLoop::AllEvents,10);
                return;
            }
            in >> imageBlockSize;//一幅图像的大小信息


            //        if (imageBlockSize == (quint64)0xFFFFFFFFFFFFFFFF)//视频结束的标注符
            //        {
            //            tcpSocket->close();
            //            QMessageBox::information(this, tr("warning"), tr("the video is end!"));
            //            return;
            //        }

            //        qDebug() << "imageBlockSize  is " << imageBlockSize;
            //        QString imageBlockS = "imageBlockSize  is " + QString::number(imageBlockSize) + "Bytes!";
            //        ui->info->append(imageBlockS);
            message.resize(imageBlockSize);

        }
        //如果没有得到一幅图像的全部数据，则返回继续接收数据
        if (tcpSocket->bytesAvailable() < imageBlockSize)
        {
            //            QCoreApplication::processEvents(QEventLoop::AllEvents,10);
            return;
        }

        in >> imageNumberCurr;
        in >> startTimestemp;
        in >> currentInterfaceType;


        in >> message;//一幅图像所有像素的完整字节流

        qint64 endTimestamp = QDateTime::currentMSecsSinceEpoch();

        quint16 key = imageNumberCurr % MEM_CACHE_MAX_SIZE;
        /*memCacheMap中是否有此记录 如果有，则看这片内存是否已经用过。如果已经用过，则可以使用，否则就新建一片内存*/
        if(memCacheMapTcp.contains(key)){
            if(memCacheMapTcp[key].isVisited){
                //重复使用此key对应的内存
                memcpy(memCacheMapTcp[key].memStart,message.data(),imageBlockSize);
                memCacheMapTcp[key].memSize = imageBlockSize;
                memCacheMapTcp[key].picNum = imageNumberCurr;
//                qDebug()<<tr("reuse key %1").arg(key);

            }else{
                qDebug()<<tr("is going to reuse key %1 ,but it being used......").arg(key);
                continue;
            }
        }else{
            //第一次使用此key对应的空间 此时内存还没申请
            s_memCache s;
            s.isVisited = false;
            s.memSize = imageBlockSize;
            s.picNum = imageNumberCurr;
            s.memStart = new char[MAX_IMAGE_SIZE];
            memcpy(s.memStart,message.data(),imageBlockSize);
            memCacheMapTcp.insert(key,s);
//            qDebug()<<"first use mem cache";
        }


        emit signalTcpRecvOK((int)MsgType::VideoType,memCacheMapTcp[key].memStart, imageBlockSize);
        emit signalSinglePicDelayAndFrameSize(imageNumberCurr,endTimestamp - startTimestemp,(double)(imageBlockSize+sizeof(qint64)*4)/1024);


        imageBlockSize = 0;//已经收到一幅完整的图像，将imageBlockSize置0，等待接收下一幅图像
        startTimestemp = 0;

        memCacheMapTcp[key].isVisited = true;


    }
    //    QImage img;
    //    //img.loadFromData(rdc,"JPEG");//解释为jpg格式的图像
    //    img.loadFromData(message);//解释为jpg格式的图像

    //    QPixmap pixmap = QPixmap::fromImage(img);
    //    pixmap.scaled(ui->label_imageShow->size());
    //    ui->label_imageShow->setPixmap(pixmap);
}
/**
 * @brief WorkerTcpObject::sendOneImageByMultipleFrames
 * 一张图片分为多帧发送
 */
void WorkerTcpObject::sendOneImageByMultipleFrames(QString filepath, int msgtype, QString imageFormat)
{
    //下面开始组每一帧
    QFile imgfile(filepath);
    if(!imgfile.open(QIODevice::ReadOnly)){
        emit signalWorkerTcpMsgDialog(WrokerObjectrMsgTypeToDlg::Critical,tr("打开文件失败:%1").arg(imgfile.errorString()));
        return;
    }

    qDebug()<<__FUNCTION__<<QThread::currentThreadId();

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
        mes.uSendDatatime = QDateTime::currentMSecsSinceEpoch();
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
/**
 * @brief WorkerTcpObject::sendOneImageOneTime
 * 通过流的方式 一次发送一张图片
 * @param filepath
 * @param msgtype
 * @param imageFormat
 */
void WorkerTcpObject::sendOneImageOneTime(QString filepath, int msgtype, QString imageFormat)
{
    QByteArray byte;	//The QByteArray class provides an array of bytes.
    QBuffer buf(&byte);		//缓存区域

    QImage image(filepath);
    image.save(&buf, imageFormat.toStdString().data());	//将图像以jpg的压缩方式压缩了以后保存在 buf当中

    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);	//二进制只写输出流
    out.setVersion(QDataStream::Qt_5_9);	//输出流的版本
    /*当操作复杂数据类型时，我们就要确保读取和写入时的QDataStream版本是一样的，简单类型，比如char，short，int，char* 等不需要指定版本也行*/


    ++picnum;

    out << (quint64)0;	//写入套接字图像数据的大小
    out << (quint64)0;	//写入套接字 当前是第几张图片
    out << (quint64)0;	//写入套接字 当前时间戳

    out << (quint64)0;	//写入套接字 当前使用的接口(LTE or DSRC)

    out << byte;			//写入套接字的经压缩-编码后的图像数据

    out.device()->seek(0);
    out << (quint64)(ba.size() - sizeof(quint64));//写入套接字的经压缩-编码后的图像数据的大小

    out << picnum;
    out << QDateTime::currentMSecsSinceEpoch();

    out << InterfaceType::LTE;

    tcpSocket->write(ba);	//将整块数据写入套接字

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
    //        readTcpInfoByMultipleFrames();
    readTcpInfoOneTime();
}
void WorkerTcpObject::tcpSendText(QString messge){

}
void WorkerTcpObject::tcpSendImage(QString filepath, int msgtype, QString imageFormat)
{

    sendOneImageOneTime(filepath,msgtype,imageFormat);
    //    sendOneImageByMultipleFrames(filepath,msgtype,imageFormat);

}
