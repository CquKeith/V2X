#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //    SyncTimeStamp();//同步时间戳
    /*生成videoSource文件夹，以便存储分享的视野视频文件*/
    checkVideoSourceFolder();

    ui->setupUi(this);
    form_display_tcp_video = new MyTcpVideoDisplayForm;
    defaultCarPicPath = "./images/car.jpg";
    //    pic_num_hasSended = 0;


    timer_timeNow = new QTimer(this);
    connect(timer_timeNow,&QTimer::timeout,[=]{
        ui->label_currentTime->setText(QDateTime::currentDateTime().toString("hh:mm:ss"));
    });
    timer_timeNow->start(1000);


    //    qint64 timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch(); //毫秒级
    //    qDebug()<<timestamp;

    InitStyle();

    InitForm();
    InitBackMenu();

    InitWorkerThread();

    on_pb_getLocalIP_clicked();

    //    timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch(); //毫秒级
    //    qDebug()<<timestamp;

    qDebug()<<__FUNCTION__<<QThread::currentThreadId();

    VIDEOSOURCE = "0";
    deviceType=VideoDeviceType::Camera;



    //    QTimer::singleShot(10,[=]{ui->widgetRealTimeVideo->refreshForm();ui->widgetSightShare->refreshForm();});
    //    qRegisterMetaType<VideoQualityType>("VideoQualityType");

    QTimer::singleShot(10,Qt::VeryCoarseTimer,this,&MainWindow::loadSettings);
}

MainWindow::~MainWindow()
{
    //    saveSettings();

    workerUdpReceiveObj->deleteLater();
    workerUdpSendObj->deleteLater();
    workerTcpObj->deleteLater();
    form_display_tcp_video->close();

    delete ui;
}

/**
 * @brief MainWindow::menuButtonCilck
 * 每个一级菜单点击后 都会进入这个槽函数
 */
void MainWindow::menuButtonCilck()
{
    QToolButton *btn = (QToolButton *)sender();

    QString btn_name = btn->objectName();

    if (btn_name == "tb_changeSkin") {
        changeSkin();
        return;
    }

    ui->stackedWidgetMain->setCurrentIndex(1);
    ui->tb_back->setText(btn->text());
    if(btn_name == "tb_carInfo"){
        ui->stackedWidget_Content->setCurrentIndex(0);
    }else if(btn_name == "tb_video"){
        ui->stackedWidget_Content->setCurrentIndex(1);
    }else if(btn_name == "tb_setting"){
        ui->stackedWidget_Content->setCurrentIndex(4);
        on_pb_getLocalIP_clicked();
    }else if(btn_name == "tb_analyse"){
        ui->stackedWidget_Content->setCurrentIndex(3);
        on_pb_getLocalIP_clicked();
    }else if(btn_name == "tb_shareSight"){
        ui->stackedWidget_Content->setCurrentIndex(2);
        on_pb_getLocalIP_clicked();
    }else if(btn_name == "tb_roadInfo"){
        ui->stackedWidget_Content->setCurrentIndex(5);
        on_pb_getLocalIP_clicked();
    }



}
/**
 * @brief MainWindow::backToMenu
 * 从二级菜单返回一级菜单
 */
void MainWindow::backToMenu()
{
    ui->stackedWidgetMain->setCurrentIndex(0);
}
/**
 * @brief MainWindow::slotRecv 收到消息 在GUI线程显示
 * @param msgtype 消息类型 TextType=1,
                         ImageType=2,
                         VideoType=3
 * @param buf 通过内存共享实现线程之间的通信，读取收到的数据
 * @param len 内容的长度
 */
void MainWindow::slotRecv(int msgtype, char *buf, int len,int quality)
{
    //消息处理 放置界面假死
    //    QCoreApplication::processEvents(QEventLoop::AllEvents,100);

    switch (msgtype)
    {
    case (int)MsgType::TextType:
    {

        //            ui->tabWidget->setCurrentIndex(0);
        //        static quint8 currentIndex = 0;//记录当前是第几次算frame丢包率
        QString str = QString::fromUtf8(buf,len);
        str.fromLatin1(buf,len);
        //        double loss = (1-str.toDouble()/workerUdpReceiveObj->getFrameRcvCount())*100;
        //            ui->textBrowser_recvText->append(str);
        //        QMessageBox::information(this,"消息",QString::number(loss)+"%");
        //        chartPicSize->addData(++currentIndex,loss);
        workerUdpReceiveObj->clearFrameRcvCount();
        videowriter.release();
        label_videoReceivedUdp->setPixmap(QPixmap(defaultCarPicPath));

        break;
    }
    case (int)MsgType::ImageType:
    {
        //        ui->tabWidget->setCurrentIndex(1);
        //        QPixmap pixmap;

        //        //            pixmap.loadFromData((uchar*)buf, len, "bmp");
        //        //            char  imageFormat[4];
        //        //            memcpy(imageFormat,buf,4);
        //        {
        //            //            QMutex mutex;
        //            QMutexLocker locker(&mutex_mBuffer);
        //            pixmap.loadFromData((uchar*)buf, len, "JPG");

        //            QPixmap resultImg = pixmap.scaled(ui->label_SendImg->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
        //            ui->label_RcvImg->setPixmap(resultImg);

        //        }
        break;
    }
    case (int)MsgType::VideoType:
    {
        QPixmap pixmap;

        // 加上{} 互斥的访问buf那段空间 类似于消费者生产者问题中的 “消费者”
        {
            //            QMutexLocker locker(&mutex_mBuffer);
            pixmap.loadFromData((uchar*)buf, len, "JPG");
            //                QPixmap resultImg = pixmap.scaled(ui->label_video->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
            //                ui->label_video->setPixmap(resultImg);
            //            ui->label_video->setPixmap(pixmap);
            //            QImage img((uchar*)buf,ui->widget_RecvVideo->width(),ui->widget_RecvVideo->height(),QImage::Format_Indexed8);
            //            ui->widget_RecvVideo->displayImage(img);
            label_videoReceivedUdp->setPixmap(pixmap);
        }
        receiver_quality->showCurrentVideoQuality((VideoQualityType)quality);

        break;
    }
    }
}
/**
 * @brief MainWindow::slotTcpRecv
 * 显示TCP接收到的图片
 * @param imgdata 图片内容
 * @param num 第几张图片
 */
void MainWindow::slotTcpRecv(int msgtype, char *buf, int len)
{
    //消息处理 放置界面假死
    //    QCoreApplication::processEvents(QEventLoop::AllEvents,100);

    switch (msgtype)
    {
    case (int)MsgType::TextType:
    {

        //            ui->tabWidget->setCurrentIndex(0);
        //            static quint8 currentIndex = 0;//记录当前是第几次算frame丢包率
        QString str = QString::fromUtf8(buf,len);
        str.fromLatin1(buf,len);
        //            double loss = (1-str.toDouble()/workerUdpReceiveObj->getFrameRcvCount())*100;
        //            ui->textBrowser_recvText->append(str);
        //        QMessageBox::information(this,"消息",QString::number(loss)+"%");
        //        chartPicSize->addData(++currentIndex,loss);
        //        workerUdpReceiveObj->clearFrameRcvCount();
        videowriter.release();
        form_display_tcp_video->showVideoFrame(QPixmap(defaultCarPicPath));

        break;
    }

    case (int)MsgType::VideoType:
    {
        QPixmap pixmap;

        pixmap.loadFromData((uchar*)buf, len, "JPG");

        form_display_tcp_video->showVideoFrame(pixmap);

        break;
    }
    }
    //    qint64 delay;

    //    label_video->setPixmap(hexToPixmap(imgdata,delay));

    //    if(delay > 99999) return;

    //    chartPic->addData(num,delay);
}
/**
 * @brief MainWindow::slotPlotSinglePicDelay
 *
 * @param num 该图片是收到的发送出来的第几张的图片
 * @param delaytime 时延，ms为单位
 */
void MainWindow::slotPlotSinglePicDelayAndFrameSize(uint num, qint64 delaytime,double frameSize)
{
    QObject * obj = sender();
    if(obj->inherits("WorkerTcpObject")){
        chartPicTcp->addData(num,delaytime);
        chartPicSizeTcp->addData(num,frameSize);
    }else if(obj->inherits("WorkerUdpReadObject")){
        chartPicUdp->addData(num,delaytime);

        chartPicSizeUdp->addData(num,frameSize);
    }

}
/**
 * @brief MainWindow::slotPlotSingleFrameDelay
 * @param num 该帧是总共收到的第几个帧
 * @param delaytime 时延
 */
void MainWindow::slotPlotSingleFrameDelay(uint num, qint64 delaytime)
{
    //    chartFrame->addData(num,delaytime);
    Q_UNUSED(num)
    Q_UNUSED(delaytime)
}
/**
 * @brief MainWindow::slotGetVideo
 * 捕获摄像头的头像 并立即共享出去
 */
void MainWindow::slotGetVideo()
{

    Mat mat;

    capture >> mat;

    if(mat.empty()) {

        slotStopSightShare();
        return;
    }

    if(ui->checkBoxStroeSightShare->isChecked()){
        videowriter << mat;
    }
#ifndef DEBUG_SINGLE_PICTURE
    QPixmap pixmap = QPixmap::fromImage(MatToQImage(mat));
#else
    QPixmap pixmap("temp.jpg");
#endif

    pixmap = pixmap.scaled(label_videoRealTimeSight->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    label_videoRealTimeSight->setPixmap(pixmap);

#ifndef DEBUG_SINGLE_PICTURE
    pixmap.save("temp.jpg","JPG");
#endif
    //压缩图片质量

    if(ui->checkBox_UseLTE->isChecked()){
        emit signal_tcpSendImage("temp.jpg",MsgType::VideoType,"JPG");
    }
    if(ui->checkBox_UseDSRC->isChecked()){
        emit signal_udpSendImage("temp.jpg",MsgType::VideoType,"JPG",(int)sender_quality->getCurrentVideoQuality());
    }


}
/**
 * @brief MainWindow::slotStartSightShare
 * 开始共享视野
 */
void MainWindow::slotStartSightShare()
{

    if(capture.isOpened()) {
        ToastString("已经在共享了，请勿重复点击");
        return;
    }

    bool isSuccess = false;
    if(deviceType == VideoDeviceType::Camera){
        isSuccess = capture.open(VIDEOSOURCE.toInt());
        qDebug()<<isSuccess<<VIDEOSOURCE.toInt();
    }else if(deviceType == VideoDeviceType::File){
        //        isSuccess = capture.open(VIDEOSOURCE.toStdString());
        //        VIDEOSOURCE = ".\\videoSource\\test.avi";
        isSuccess = capture.open(VIDEOSOURCE.toStdString());
        qDebug()<<isSuccess<<VIDEOSOURCE<<VIDEOSOURCE2;
    }

    if(!isSuccess){
        qDebug()<<"打开失败!";
        ToastString(tr("打开失败!%1:%2").arg(deviceType).arg(VIDEOSOURCE));
        ui->widgetSightShare->getMenuWidget()->setCurrentIndex(1);
        return;
    }


    timer_get_video = new QTimer(this);
    int videoFrameRate = ui->lineEdit_videoFPS->text().toInt();
    timer_get_video->start(1000/videoFrameRate);//1s采集10张图片
    connect(timer_get_video,&QTimer::timeout,this,&MainWindow::slotGetVideo);
    //将发送出去的视频保存下来
    //    QString date = (QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    QString datetime = (QDateTime::currentDateTime().toString("yyyy-MM-dd hh.mm.ss"));
    QString fileName = tr("./videoSource/%1.avi").arg(datetime);
    if(ui->checkBoxStroeSightShare->isChecked()){
        videowriter.open( fileName.toStdString(),
                          CV_FOURCC('M', 'J', 'P', 'G'),
                          videoFrameRate, // 视频帧率
                          cv::Size(640, 480));
    }


    //定时共享
    timer_sightShareTimeLeft = new QTimer(this);
    time_sightshareSecondLeft = SIGHT_SHARE_TIME;
    qDebug()<<"共享时间："<<time_sightshareSecondLeft;
    label_sightShare->setText(tr("%1 S后结束共享").arg(time_sightshareSecondLeft));
    label_sightShare->adjustSize();
    connect(timer_sightShareTimeLeft,&QTimer::timeout,[=]{

        if(--time_sightshareSecondLeft>0){
            label_sightShare->setText(tr("%1 S后结束共享").arg(time_sightshareSecondLeft));
            label_sightShare->adjustSize();
        }
        else{
            slotStopSightShare();
            ui->widgetSightShare->getMenuWidget()->setCurrentIndex(1);

        }
    });
    timer_sightShareTimeLeft->start(1000);



}
/**
 * @brief MainWindow::slotStopSightShare
 * 停止共享视野
 */
void MainWindow::slotStopSightShare()
{
    //    ui->widgetSightShare->getMenuWidget()->setCurrentIndex(1);

    //发送给对方默认的封面 统计时延时不需要，应用时可以用上
    //    emit signal_tcpSendImage(defaultCarPicPath,MsgType::VideoType,"JPG");

    if(capture.isOpened()){
        capture.release();

        timer_get_video->deleteLater();

        label_videoRealTimeSight->setPixmap(QPixmap(defaultCarPicPath));
        //        workerUdpSendObj->udpSendImage(defaultCarPicPath,MsgType::VideoType,"JPG");

        if(ui->checkBox_UseLTE->isChecked()){
            ToastString(tr("发送完毕，共发送 %1 张图片").arg(workerTcpObj->getPicnum()),Toast::TimeLenth::LONG);
            qDebug()<<tr("发送完毕，共发送 %1 张图片").arg(workerTcpObj->getPicnum());
            workerTcpObj->clearPicNum();
        }if(ui->checkBox_UseDSRC->isChecked()){
            workerUdpSendObj->udpSendText(QString::number(workerUdpSendObj->getFrameNum()));

            ToastString(tr("发送完毕，共发送 %1 张图片，%2帧数据").arg(workerUdpSendObj->getPicnum()).arg(workerUdpSendObj->getFrameNum()),Toast::TimeLenth::LONG);
            qDebug()<<tr("发送完毕，共发送 %1 张图片，%2帧数据").arg(workerUdpSendObj->getPicnum()).arg(workerUdpSendObj->getFrameNum());
            workerUdpSendObj->clearFrameNum();
            workerUdpSendObj->clearPicNum();
        }

        timer_sightShareTimeLeft->stop();
        timer_sightShareTimeLeft->deleteLater();

        label_sightShare->clear();
    }

    if(videowriter.isOpened()){
        videowriter.release();
    }
}
/**
 * @brief MainWindow::InitStyle
 * 设置无边框style
 */
void MainWindow::InitStyle()
{
    this->mousePressed = false;
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
}
/**
 * @brief MainWindow::InitForm
 * 初始化menu按钮的点击事件
 * 为一级菜单添加二级菜单
 */
void MainWindow::InitForm()
{
    QList<QToolButton *> btns = ui->page_index->findChildren<QToolButton*>();
    foreach (QToolButton *btn, btns) {
        connect(btn,&QToolButton::clicked,this,&MainWindow::menuButtonCilck);
    }

    // 设置鼠标可以拖动
    this->setProperty("canMove",true);



    //“系统设置”的二级菜单
    InitSettintMenu();

    //“实时视频”的二级菜单
    InitRealTimeForm();

    //“共享视野”的二级菜单
    InitSightShareForm();

    //数据分析界面
    InitAnalyseForm();

    connect(ui->checkBox_UseLTE,&QCheckBox::toggled,this,[=]{
        if(ui->checkBox_UseLTE->isChecked()){
            form_display_tcp_video->show();
        }else{
            form_display_tcp_video->hide();
        }
    });

    /*选择4G或者LTE配置时，加载之前的设置*/
    connect(ui->rbDSRCSetting,&QRadioButton::toggled,this,&MainWindow::loadLTEAndDSRCPortIPSetting);
    connect(ui->rbLTESetting,&QRadioButton::toggled,this,&MainWindow::loadLTEAndDSRCPortIPSetting);

    // 测试数据
    //    for(int i=1;i<=100;++i){
    //        chartPicUdp->addData(i,i*i);
    //    }

}
/**
 * @brief MainWindow::InitBackMenu
 * 绑定信号和槽：完成从二级菜单向主菜单的返回
 */
void MainWindow::InitBackMenu()
{
    connect(ui->tb_back,&QToolButton::clicked,this,&MainWindow::backToMenu);
}
/**
 * @brief MainWindow::InitSettintMenu
 * 初始化“设置”页的二级菜单
 */
void MainWindow::InitSettintMenu()
{
    ui->widgetSettingMenu->setRowHeight(50);
    ui->widgetSettingMenu->addItem("个人资料");
    ui->widgetSettingMenu->addItem("IP地址");
    ui->widgetSettingMenu->addItem("消息与隐私");
    ui->widgetSettingMenu->addItem("离线地图下载");
    ui->widgetSettingMenu->addItem("关于");
    ui->widgetSettingMenu->setSelectColor("#ffffff");

    connect(ui->widgetSettingMenu,&QNavigationWidget::currentItemChanged,[=](const int &index){
        ui->stackedWidget_settingContent->setCurrentIndex(index);
    });
    ui->widgetSettingMenu->setCurrentIndex(1);
}
/**
 * @brief MainWindow::InitRealTimeForm
 * 初始化 实时视频显示的界面
 */
void MainWindow::InitRealTimeForm()
{

    QNavigationWidget *realtimeSetting = ui->widgetRealTimeVideo->getMenuWidget();

    realtimeSetting->addItem("渝A XXXXX");
    realtimeSetting->addItem("都不看");


    label_videoReceivedUdp = new qVideoLable;
    label_videoReceivedUdp->setScaledContents(true);
    label_videoReceivedUdp->setPixmap(QPixmap(defaultCarPicPath));


    connect(label_videoReceivedUdp,&qVideoLable::signalLableClicked,[=]{
        qDebug()<<"单击";

    });
    connect(label_videoReceivedUdp,&qVideoLable::signalLableDoubleClicked,[=]{
        qDebug()<<"双击" << label_videoReceivedUdp->windowFlags();
        static QFlags<Qt::WindowType> flags = label_videoReceivedUdp->windowFlags();


        if(label_videoReceivedUdp->isFullScreen()){
            label_videoReceivedUdp->setWindowFlags(flags);
            label_videoReceivedUdp->showNormal();
        }else{
            label_videoReceivedUdp->setWindowFlags(Qt::Dialog);
            label_videoReceivedUdp->showFullScreen();
        }
    });


    ui->widgetRealTimeVideo->addContentWidget(label_videoReceivedUdp);
    ui->widgetRealTimeVideo->setMaxListFrameWidthRatio(0.2);

    labelVideoFrom = new QLabel(label_videoReceivedUdp);
    labelVideoFrom->move(20,20);
    labelVideoFrom->setStyleSheet("color: rgb(255, 0, 0);background:transparent;font:12pt bold \"Times New Roman\";");


    connect(realtimeSetting,&QNavigationWidget::currentItemChanged,[=](const int &index){
        int lenth = realtimeSetting->getItemNum();
        if(index == lenth-1){
            labelVideoFrom->clear();
            //            emit signal_SetCanRecevInfo(false);
        }else{
            //            emit signal_SetCanRecevInfo(true);
            labelVideoFrom->setText("视频来源："+realtimeSetting->getCurrentItemText());
        }
        labelVideoFrom->adjustSize();
    });

    realtimeSetting->setCurrentIndex(1);

    //在下方添加 “接口选择”、“视频质量选择”的下拉框

    receiver_interface = new ComboxLabel("DSRC",label_videoReceivedUdp);
    receiver_interface->move(label_videoReceivedUdp->width()*0.85,label_videoReceivedUdp->height()*0.75);
    receiver_interface->setStyleSheet("color: rgb(255, 0, 255);background:transparent;font:12pt bold \"Times New Roman\";");
    receiver_interface->addComboboxText("DSRC");
    receiver_interface->addComboboxText("4G");

    receiver_quality = new ComboxLabel("1080P",label_videoReceivedUdp);
    receiver_quality->move(label_videoReceivedUdp->width()*0.85+100,label_videoReceivedUdp->height()*0.75);
    receiver_quality->setStyleSheet("color: rgb(255, 0, 255);background:transparent;font:12pt bold \"Times New Roman\";");

    receiver_quality->addComboboxText("1080P");
    receiver_quality->addComboboxText("超清");
    receiver_quality->addComboboxText("高清");
    receiver_quality->addComboboxText("标清");


}
/**
 * @brief MainWindow::InitSightShareForm
 */
void MainWindow::InitSightShareForm()
{

    QNavigationWidget *sightShareWidget = ui->widgetSightShare->getMenuWidget();
    sightShareWidget->addItem("开始共享");
    sightShareWidget->addItem("结束共享");
    sightShareWidget->addItem("设备选择");

    sightShareWidget->setCurrentIndex(1);

    label_videoRealTimeSight = new qVideoLable;
    label_videoRealTimeSight->setScaledContents(true);
    label_videoRealTimeSight->setPixmap(QPixmap(defaultCarPicPath));

    connect(label_videoRealTimeSight,&qVideoLable::signalLableClicked,[=]{
        qDebug()<<"单击";

    });
    connect(label_videoRealTimeSight,&qVideoLable::signalLableDoubleClicked,[=]{
        qDebug()<<"双击" << label_videoRealTimeSight->windowFlags();
        static QFlags<Qt::WindowType> flags = label_videoRealTimeSight->windowFlags();

        if(label_videoRealTimeSight->isFullScreen()){
            label_videoRealTimeSight->setWindowFlags(flags);
            label_videoRealTimeSight->showNormal();
        }else{
            label_videoRealTimeSight->setWindowFlags(Qt::Dialog);
            label_videoRealTimeSight->showFullScreen();
        }
    });

    ui->widgetSightShare->addContentWidget(label_videoRealTimeSight);

    label_sightShare = new QLabel(label_videoRealTimeSight);
    label_sightShare->move(20,20);
    //    lable_sightShare->setText("60秒后结束共享");
    label_sightShare->setStyleSheet("color: rgb(255, 0, 0);background:transparent;font:12pt bold \"Times New Roman\";");

    connect(ui->widgetSightShare,&myQSpliter::currentMenuItemChanged,this,[=](int index){

        //        qDebug()<<index;

        if(index == 1){//结束共享
            slotStopSightShare();
        }else if(index == 0){//开始共享
            slotStartSightShare();
        }else if(index == 2){//设备选择
            DialogSelectVideoSourceDevice *dialog = new DialogSelectVideoSourceDevice(this,VIDEOSOURCE);
            connect(dialog,&DialogSelectVideoSourceDevice::signal_selectedDevice,this,[=](int type,QString device){
                VIDEOSOURCE = device;
                deviceType = (VideoDeviceType)type;

                qDebug()<<deviceType<<VIDEOSOURCE;
            });
            dialog->show();

        }
    });


    ui->widgetSightShare->setMaxListFrameWidthRatio(0.2);


    //在下方添加 “接口选择”、“视频质量选择”的下拉框

    sender_interface = new ComboxLabel("DSRC",label_videoRealTimeSight);
    sender_interface->move(label_videoRealTimeSight->width()*0.85,label_videoRealTimeSight->height()*0.75);
    sender_interface->setStyleSheet("color: rgb(255, 0, 255);background:transparent;font:12pt bold \"Times New Roman\";");
    sender_interface->addComboboxText("DSRC");
    sender_interface->addComboboxText("4G");

    sender_quality = new ComboxLabel("1080P",label_videoRealTimeSight);
    sender_quality->move(label_videoRealTimeSight->width()*0.85+100,label_videoRealTimeSight->height()*0.75);
    sender_quality->setStyleSheet("color: rgb(255, 0, 255);background:transparent;font:12pt bold \"Times New Roman\";");

    sender_quality->addComboboxText("1080P");
    sender_quality->addComboboxText("超清");
    sender_quality->addComboboxText("高清");
    sender_quality->addComboboxText("标清");

    qDebug()<<"label_videoRealTimeSight width:"<<label_videoRealTimeSight->width()<<label_videoRealTimeSight->height();



}
/**
 * @brief MainWindow::InitAnalyseForm
 * 初始化 数据分析的界面
 */
void MainWindow::InitAnalyseForm()
{
    QNavigationWidget *analyseWidgetMenu = ui->widget_analyseMenu;
    analyseWidgetMenu->addItem("DSRC单张图片delay");
    analyseWidgetMenu->addItem("DSRC单张图片size");
    analyseWidgetMenu->addItem("LTE单张图片delay");
    analyseWidgetMenu->addItem("LTE单张图片size");
    connect(analyseWidgetMenu,&QNavigationWidget::currentItemChanged,[=](const int &index){
        ui->stackedWidgetAnalyse->setCurrentIndex(index);
    });

    chartPicUdp = new QCustomChart(ui->pageAnalyseDSRCDelay);
    QGridLayout* pageAnalyseDSRCDelayGL = new QGridLayout(ui->pageAnalyseDSRCDelay);
    ui->pageAnalyseDSRCDelay->setLayout(pageAnalyseDSRCDelayGL);
    ui->pageAnalyseDSRCDelay->layout()->addWidget(chartPicUdp);
    connect(chartPicUdp,&QCustomChart::signalQCustomToGUI,[=](QString msg){
        QMessageBox::information(this,tr("tips"),msg);
    });

    chartPicSizeUdp = new QCustomChart(ui->pageAnalyseDSRCSize);
    QGridLayout* pageAnalyseDSRCSizeGL = new QGridLayout(ui->pageAnalyseDSRCSize);
    ui->pageAnalyseDSRCSize->setLayout(pageAnalyseDSRCSizeGL);
    ui->pageAnalyseDSRCSize->layout()->addWidget(chartPicSizeUdp);
    connect(chartPicSizeUdp,&QCustomChart::signalQCustomToGUI,[=](QString msg){
        QMessageBox::information(this,tr("tips"),msg);
    });

    chartPicUdp->setTitle("DSRC Delay");
    chartPicSizeUdp->setTitle("DSRC size");
    chartPicUdp->setXaxisLabel("序号");
    chartPicUdp->setYaxisLabel("时延(ms)");
    chartPicSizeUdp->setXaxisLabel("序号");
    chartPicSizeUdp->setYaxisLabel("大小(KB)");

    chartPicTcp = new QCustomChart(ui->pageAnalyseLTEDelay);
    QGridLayout* glLTEDelay = new QGridLayout(ui->pageAnalyseLTEDelay);
    ui->pageAnalyseLTEDelay->setLayout(glLTEDelay);
    ui->pageAnalyseLTEDelay->layout()->addWidget(chartPicTcp);
    connect(chartPicTcp,&QCustomChart::signalQCustomToGUI,[=](QString msg){
        QMessageBox::information(this,tr("tips"),msg);
    });

    chartPicSizeTcp = new QCustomChart(ui->pageAnalyseLTESize);
    QGridLayout* glLTESize = new QGridLayout(ui->pageAnalyseLTESize);
    ui->pageAnalyseLTESize->setLayout(glLTESize);
    ui->pageAnalyseLTESize->layout()->addWidget(chartPicSizeTcp);
    connect(chartPicSizeTcp,&QCustomChart::signalQCustomToGUI,[=](QString msg){
        QMessageBox::information(this,tr("tips"),msg);
    });

    chartPicTcp->setTitle("Tcp delay");
    chartPicSizeTcp->setTitle("Tcp Size");
    chartPicTcp->setXaxisLabel("序号");
    chartPicTcp->setYaxisLabel("时延(ms)");

    chartPicSizeTcp->setXaxisLabel("序号");
    chartPicSizeTcp->setYaxisLabel("大小(KB)");

}
/**
 * @brief MainWindow::saveSettings
 * 保存设置
 */
void MainWindow::saveSettings()
{
    QSettings setting("config.ini",QSettings::IniFormat);

    /*本机端口号*/
    setting.setValue("local/port",ui->lineEdit_localPort->text());
    /*视频帧率*/
    setting.setValue("local/FPS",ui->lineEdit_videoFPS->text());

    /*是否保存分享的视频*/
    setting.setValue("local/saveSharedVideo",ui->checkBoxStroeSightShare->isChecked());

    /*对方IP地址和端口号*/
    setting.beginGroup("otherSize");
    setting.setValue("port",ui->lineEdit_otherPort->text());
    setting.setValue("ip",ui->comboBoxOtherIP->currentText());

    setting.setValue("radioButtonDSRCSetting",ui->rbDSRCSetting->isChecked());
    setting.setValue("radioButtonLTESetting",ui->rbLTESetting->isChecked());

    /*分别保存DSRC和4G当前的IP地址和端口号*/
    if(ui->rbDSRCSetting->isChecked()){
        setting.setValue("DSRC_IP",ui->comboBoxOtherIP->currentText());
        setting.setValue("DSRC_Port",ui->lineEdit_otherPort->text());
    }else if(ui->rbLTESetting->isChecked()){
        setting.setValue("LTE_IP",ui->comboBoxOtherIP->currentText());
        setting.setValue("LTE_Port",ui->lineEdit_otherPort->text());
    }


    setting.setValue("checkBoxDSRCSetting",ui->checkBox_UseDSRC->isChecked());
    setting.setValue("checkBoxLTESetting",ui->checkBox_UseLTE->isChecked());

    setting.beginWriteArray("IP");
    int count = ui->comboBoxOtherIP->count();
    for(int i=0;i<count;i++)
    {
        setting.setArrayIndex(i);
        setting.setValue("ip",ui->comboBoxOtherIP->itemText(i));
    }
    setting.endArray();

    setting.endGroup();
}

void MainWindow::checkVideoSourceFolder()
{
    // 检查目录是否存在，若不存在则新建
    QString path = QDir::currentPath()+"/videoSource";
    QDir dir(path);
    if(!dir.exists())
    {
        dir.mkdir(path);
    }
}
/**
 * @brief MainWindow::loadSettings
 * 加载设置
 */
void MainWindow::loadSettings()
{
    QSettings setting("config.ini",QSettings::IniFormat);

    /*本机端口号*/
    ui->lineEdit_localPort->setText(setting.value("local/port").toString());
    /*视频帧率*/
    ui->lineEdit_videoFPS->setText(setting.value("local/FPS").toString());
    /*是否保存分享的视频*/
    ui->checkBoxStroeSightShare->setChecked(setting.value("local/saveSharedVideo").toBool());

    /*对方IP地址和端口号*/
    setting.beginGroup("otherSize");
    ui->lineEdit_otherPort->setText(setting.value("port").toString());
    ui->comboBoxOtherIP->setCurrentText(setting.value("ip").toString());ui->checkBox_UseDSRC->setChecked(setting.value("checkBoxDSRCSetting").toBool());
    ui->checkBox_UseLTE->setChecked(setting.value("checkBoxLTESetting").toBool());

    int arraySize = setting.beginReadArray("IP");
    for(int i=0;i<arraySize;i++)
    {
        setting.setArrayIndex(i);
        ui->comboBoxOtherIP->addItem(setting.value("ip").toString());
    }

    setting.endArray();

    ui->rbDSRCSetting->setChecked(setting.value("radioButtonDSRCSetting").toBool());
    ui->rbLTESetting->setChecked(setting.value("radioButtonLTESetting").toBool());

    setting.endGroup();

}
/**
 * @brief MainWindow::loadLTEAndDSRCPortIPSetting
 * 只获取上次的 LTE和 4G 的保存IP和Port信息
 */
void MainWindow::loadLTEAndDSRCPortIPSetting()
{
    QSettings setting("config.ini",QSettings::IniFormat);

    /*对方IP地址和端口号*/
    setting.beginGroup("otherSize");

    /*分别保存DSRC和4G当前的IP地址和端口号*/
    if(ui->rbDSRCSetting->isChecked()){

        ui->comboBoxOtherIP->setCurrentText(setting.value("DSRC_IP").toString());
        ui->lineEdit_otherPort->setText(setting.value("DSRC_Port").toString());

    }else if(ui->rbLTESetting->isChecked()){
        ui->comboBoxOtherIP->setCurrentText(setting.value("LTE_IP").toString());
        ui->lineEdit_otherPort->setText(setting.value("LTE_Port").toString());
    }
    setting.endGroup();
}
/**
 * @brief MainWindow::ToastString
 * @param msg
 */
void MainWindow::ToastString(QString msg,Toast::TimeLenth time_lenth)
{
    Toast *toast = new Toast(this,msg,time_lenth);
    toast->show();
}
/**
 * @brief MainWindow::changeSkin
 * 换肤 更换背景色等
 */
void MainWindow::changeSkin()
{

    static int index = 1;
    if(++index == 6) index = 1;
    QString skin = tr("QWidget#centralWidget{background-image: url(:qss/img/main%1.jpg);font: 12pt \"微软雅黑\";}").arg(index);
    ui->centralWidget->setStyleSheet(skin);

    //    //加载样式表
    //    QFile file(":/qss/blue.css");
    //    if (file.open(QFile::ReadOnly)) {
    //        QString qss1 = QLatin1String(file.readAll());

    //        qApp->setStyleSheet(qss1);
    //        file.close();
    //    }

}
/**
 * @brief MainWindow::InitWorkerThread
 * 初始化 线程 绑定信号 完成UDP的收发
 */
void MainWindow::InitWorkerThread()
{
    //此处一定不能给workerUdpSendObj指定父类，否则 将不能移动到新的线程
    workerUdpSendObj = new WorkerUdpSendObject;
    connect(this,&MainWindow::signal_udpSendText,workerUdpSendObj,&WorkerUdpSendObject::udpSendText);
    connect(this,&MainWindow::signal_udpSendImage,workerUdpSendObj,&WorkerUdpSendObject::udpSendImage);
    connect(workerUdpSendObj,&WorkerUdpSendObject::signal_ImageSendProcess,this,[=](int process,int FrameCurr,int FrameTotal){
        //        ui->progressBar_sendImg->setValue(process);
        //        ui->progressBar_sendImg->setStatusTip(tr("发送第 %1 / %2 个包").arg(FrameCurr).arg(FrameTotal));
        //        qDebug()<<tr("发送第 %1 / %2 个包").arg(FrameCurr).arg(FrameTotal);
        Q_UNUSED(process)
        Q_UNUSED(FrameCurr)
        Q_UNUSED(FrameTotal)
    });
    connect(workerUdpSendObj,&WorkerUdpSendObject::signalWorkerUdpSenderMsgDialog,this,[=](int type,QString msg){
        ToastString(msg,Toast::MEDIUM);
        Q_UNUSED(type)
    });

    workerUdpReceiveObj = new WorkerUdpReadObject;
    connect(workerUdpReceiveObj,&WorkerUdpReadObject::sigRecvOk,this,&MainWindow::slotRecv);
    connect(this,&MainWindow::signal_ChangeUdpListion,workerUdpReceiveObj,&WorkerUdpReadObject::ChangeUdpSocketListion);
    connect(workerUdpReceiveObj,&WorkerUdpReadObject::signalWorkerUdpReaderMsgDialog,this,[=](int type,QString msg){
        Q_UNUSED(type)
        ToastString(msg,Toast::MEDIUM);

    });
    connect(workerUdpReceiveObj,&WorkerUdpReadObject::signal_ImageReceiveProcess,this,[=](int process,int FrameCurr,int FrameTotal){
        //        ui->progressBar_recvImg->setValue(process);
        //        ui->progressBar_recvImg->setStatusTip(tr("接收第 %1 / %2 个包").arg(FrameCurr).arg(FrameTotal));
        //        qDebug()<<tr("接收第 %1 / %2 个包").arg(FrameCurr).arg(FrameTotal);
        Q_UNUSED(process)
        Q_UNUSED(FrameCurr)
        Q_UNUSED(FrameTotal)
    });
    connect(workerUdpReceiveObj,&WorkerUdpReadObject::signalSinglePicDelayAndFrameSize,this,&MainWindow::slotPlotSinglePicDelayAndFrameSize);
    //    connect(workerUdpReceiveObj,&WorkerUdpReadObject::signalSingleFrameDelay,this,&MainWindow::slotPlotSingleFrameDelay);

    workerTcpObj = new WorkerTcpObject;
    connect(this,&MainWindow::signal_ConnectToServer,workerTcpObj,&WorkerTcpObject::slotConnectToServer);
    connect(this,&MainWindow::signal_tcpSendImage,workerTcpObj,&WorkerTcpObject::tcpSendImage,Qt::AutoConnection);
    connect(workerTcpObj,&WorkerTcpObject::signalTcpRecvOK,this,&MainWindow::slotTcpRecv);
    connect(workerTcpObj,&WorkerTcpObject::signalWorkerTcpMsgDialog,this,[=](int type,QString msg){
        Q_UNUSED(type)
        ToastString(msg,Toast::MEDIUM);
    });
    connect(this,&MainWindow::signal_SetCanRecevInfo,workerUdpReceiveObj,&WorkerUdpReadObject::setCanRecevInfo);
    connect(workerTcpObj,&WorkerTcpObject::signalSinglePicDelayAndFrameSize,this,&MainWindow::slotPlotSinglePicDelayAndFrameSize);


}

/**
 * @brief MainWindow::SyncTimeStamp
 * 同步网络时间
 */
void MainWindow::SyncTimeStamp()
{
    //    QString m="date -s \"2007-08-03\" ";

    //    QProcess::startDetached(m); // 0312 表示3月12日，1025表示当前时分
    //    QProcess::startDetached("hwclock -w"); // 同步系统时间
    //    QProcess::startDetached("sync"); // 保存配置

}

void MainWindow::closeEvent(QCloseEvent *e)
{
    saveSettings();

    e->accept();
}



/**
 * @brief MainWindow::on_pb_getLocalIP_clicked
 * 获取本机的ip地址
 */
void MainWindow::on_pb_getLocalIP_clicked()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    ui->comboBoxLocalIP->clear();
    foreach (QHostAddress addr, list) {
        //        if(addr.protocol() == QAbstractSocket::IPv4Protocol && addr.toString().contains("192.")){
        if(addr.protocol() == QAbstractSocket::IPv4Protocol){
            ui->comboBoxLocalIP->addItem(addr.toString());
        }
    }
}
/**
 * @brief MainWindow::on_pb_bindLocal_clicked
 * 绑定本地的IP和端口号
 */
void MainWindow::on_pb_bindLocal_clicked()
{
    //    if(ui->rbDSRCSetting->isChecked()){
    //        ToastString("使用TCP无需设置此选项");
    //        return;
    //    }

    QString ip = ui->comboBoxLocalIP->currentText();
    if(ip.isEmpty()){
        ToastString("请选择ip地址！");
    }else{
        emit signal_ChangeUdpListion(ip,ui->lineEdit_localPort->text().toInt());
    }

}
/**
 * @brief MainWindow::on_tb_sightShare_clicked
 * 打开摄像头抓取图片 并实时广播出去
 *  开始共享自己的视野
 */
//void MainWindow::on_tb_sightShare_clicked()
//{

//    if(capture.isOpened()){
//        capture.release();
//        timer_get_video->deleteLater();
////        ui->tb_sightShare->setText("开始共享");
//        label_videoMy->setPixmap(QPixmap("./images/car.jpg"));
//        workerUdpSendObj->udpSendImage("./images/car.jpg",MsgType::VideoType,"JPG");
//    }else{
//        if(!capture.open(0)){
//            qDebug()<<"打开摄像头失败!";
//            return;
//        }
//        timer_get_video = new QTimer;

//        timer_get_video->start(100);//1s采集10张图片
//        connect(timer_get_video,&QTimer::timeout,this,&MainWindow::slotGetVideo);
//        //        timer_get_video->moveToThread(new QThread);
////        ui->tb_sightShare->setText("停止共享");
//    }
//}
/**
 * @brief MatToQImage opencv捕获到的是Mat格式，将Mat转为QImage格式 方便显示
 * @param mat 得到的mat的引用
 * @return 转换后的QImage
 */
QImage MatToQImage(const Mat &mat)
{
    // 8-bits unsigned, NO. OF CHANNELS = 1
    if (mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        // Set the color table (used to translate colour indexes to qRgb values)
        image.setColorCount(256);
        for (int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar *pSrc = mat.data;
        for (int row = 0; row < mat.rows; row++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }
    // 8-bits unsigned, NO. OF CHANNELS = 3
    else if (mat.type() == CV_8UC3)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    else if (mat.type() == CV_8UC4)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
    else
    {
        return QImage();
    }
}
/**
 * @brief QImage2cvMat
 * 将QImage转换为Mat格式
 * @param image
 * @return
 */
cv::Mat QImage2cvMat(QImage &image)
{
    cv::Mat mat;
    switch (image.format())
    {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        break;
    case QImage::Format_RGB888:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        cv::cvtColor(mat, mat, CV_BGR2RGB);
        break;
    case QImage::Format_Grayscale8:
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        break;
    default:break;
    }
    return mat;
}

/**
 * @brief MainWindow::on_pb_setOtherSocket_clicked
 * 设置接收方的IP地址和端口号
 */
void MainWindow::on_pb_setOtherSocket_clicked()
{
    //    workerUdpSendObj->setHostIP(ui->comboBoxOtherIP->currentText());
    //    workerUdpSendObj->setHostPort(ui->lineEdit_otherPort->text().toInt());
    QString ip = ui->comboBoxOtherIP->currentText();
    int port = ui->lineEdit_otherPort->text().toInt();
    if(ui->rbDSRCSetting->isChecked()){
        workerUdpSendObj->setHostIpPort(ip,port);
    }
    //当前设置4G
    else if(ui->rbLTESetting->isChecked()){
        emit signal_ConnectToServer(ip,port);

    }
    saveSettings();
}
