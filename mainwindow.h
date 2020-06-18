#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QDebug>
#include <QFile>
#include <QImageReader>
#include <QLabel>
#include <QMap>
//自定义的左侧导航栏
#include <myNavigationWidget/qnavigationwidget.h>
//Socket相关
#include <QHostAddress>
#include <QUdpSocket>
#include <QNetworkInterface>
//工作线程
#include "workerudpreadobject.h"
#include "workerudpsendobject.h"
#include "workertcpobject.h"

//摄像头
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
//算法相关
//#include <opencv2/nonfree/nonfree.hpp>
//#include <opencv2/calib3d/calib3d.hpp>
#include <QTimer>

//Toast
#include <myToastWidget/toast.h>

//画图
#include "qcustomplot/qcustomchart.h"

//可以双击 label
#include "myVideoLable/qvideolable.h"
//结合combobox的label
#include "myComboxLabel/comboxlabel.h"

#include "dialogselectvideosourcedevice.h"
#include "mytcpvideodisplayform.h"
//#include <QMetaType>

#define SIGHT_SHARE_TIME 2000

//#define VIDEOSOURCE "./videoSource/2018-11-24 10.18.51.avi"

#define VIDEOSOURCE2 "./videoSource/2018-11-24 10.18.51.avi"
//#define VIDEOSOURCE "./videoSource/2018-11-24 10.47.43.avi"
//#define VIDEOSOURCE 0

#include <QSettings>

#ifndef DEBUG_SINGLE_PICTURE
#define DEBUG_SINGLE_PICTURE false
#endif

namespace Ui {
class MainWindow;
}
class ComboxLabel;
class MyTcpVideoDisplayForm;
using namespace cv;
QImage MatToQImage(const cv::Mat& mat);
cv::Mat QImage2cvMat(QImage &image);
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

//    __inline quint64 getPic_num_hasSended() const;
//    __inline void clearPicNum(){pic_num_hasSended = 0;}

    Q_INVOKABLE void showRecvImage(QPixmap &pixmap){label_videoReceivedUdp->setPixmap(pixmap);}

signals:
    void signal_udpSendText(QString);   //使用线程发送字符串
    void signal_udpSendImage(QString,int,QString,int);  //使用线程发送一个图片
    void signal_ChangeUdpListion(QString,int);      //通知udp发送的线程，更改本地监听的IP和端口号

    void signal_tcpSendText(QString);
    void signal_tcpSendImage(QString,int,QString);
    void signal_ConnectToServer(QString,int);

    void signal_SetCanRecevInfo(bool can);

private slots:
    void menuButtonCilck(); //各个一级菜单的点击信号
    void backToMenu();  //返回主菜单
    void slotRecv(int msgtype, char *buf, int len, int quality); //收到消息 在GUI线程显示
    void slotTcpRecv(int msgtype, char *buf, int len);
    void slotPlotSinglePicDelayAndFrameSize(uint num, qint64 delaytime, double frameSize);//单个图片的传输时延
    void slotPlotSingleFrameDelay(uint num, qint64 delaytime);//MAC帧的传输时延
    void slotGetVideo();                            //捕获摄像头的图像
    void slotStartSightShare();
    void slotStopSightShare();

    void on_pb_getLocalIP_clicked();

    void on_pb_bindLocal_clicked();

//    void on_tb_sightShare_clicked();


    void on_pb_setOtherSocket_clicked();

    void loadSettings();

    void loadLTEAndDSRCPortIPSetting();

private:
    Ui::MainWindow *ui;
    QPoint mousePoint;  //鼠标拖动自定义标题栏时的坐标
    bool mousePressed;  //鼠标是否按下

    QMap<QString,int> menuMap;

    void InitStyle();   //初始化无边框窗体
    void InitForm();    //初始化菜单按钮的点击事件
    void InitBackMenu();//初始化返回主菜单
    void InitSettintMenu();//初始化“设置”页的二级菜单
    void InitRealTimeForm();//初始化 接收 实时视频显示的界面
    void InitSightShareForm();//初始化 分享 实时视频显示的界面
    void InitAnalyseForm(); //初始化 图像分析的界面

    /* 保存和加载设置 */
    void saveSettings();
//    void loadSettings();

    /*./videoSource文件夹不存在就创建*/
    void checkVideoSourceFolder();

    inline void ToastString(QString msg="", Toast::TimeLenth time_lenth=Toast::TimeLenth::SHORT);//toast

    void changeSkin();  //换肤

    //线程实现UDP的RSRC收和发 4G的方式通过Tcp实现
    WorkerUdpSendObject *workerUdpSendObj;
    WorkerUdpReadObject *workerUdpReceiveObj;
    WorkerTcpObject *workerTcpObj;
    void InitWorkerThread();    //初始化发送个接收的线程

    //通过opencv捕获摄像头图片
    VideoCapture capture;
    VideoWriter videowriter;
    QTimer *timer_get_video;
    QTimer *timer_sightShareTimeLeft;
    qint16 time_sightshareSecondLeft;//定时视野共享的时间
    QLabel *label_sightShare;//显示还剩多少秒结束共享

    //接收实时视频显示
//    QNavigationWidget * realtimeSetting;
    qVideoLable * label_videoReceivedUdp;
    // 显示视频来源
    QLabel *labelVideoFrom;

    //分享实时视频显示
    qVideoLable *label_videoRealTimeSight;
    //默认的汽车的一个图片
    QString defaultCarPicPath;

    // 图像分析
    QCustomChart *chartPicUdp,*chartPicSizeUdp,*chartPicTcp,*chartPicSizeTcp;
//    quint64 pic_num_hasSended;

    //系统时间显示
    QTimer *timer_timeNow;
    void SyncTimeStamp();//同步网络时间

    QString VIDEOSOURCE;
    VideoDeviceType deviceType;


    /*显示当前使用的网络接口和视频是质量*/
    ComboxLabel *sender_interface,*sender_quality,*receiver_interface,*receiver_quality;

    MyTcpVideoDisplayForm * form_display_tcp_video;
protected:
    void closeEvent(QCloseEvent *e);
};

#endif // MAINWINDOW_H
