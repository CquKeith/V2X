#ifndef MSGHEAD_H
#define MSGHEAD_H
#include <QObject>
#include <QMutex>

// 一张图片 最大1M
#define MAX_IMAGE_SIZE 1024*1024*1
//一帧数据的最大字节数
#define MAX_ONE_FRAME_SIZE 1300

static QMutex mutex_hostIPandPort;
static QMutex mutex_mBuffer;

enum MsgType{
    TextType=1,
    ImageType,
    VideoType
};

typedef struct  {
    MsgType msgType;                        //!消息类型
    unsigned int uTransFrameHdrSize;    //!sizeof(WIFI_FRAME_HEADER)
    unsigned int uTransFrameSize;       //!sizeof(WIFI_FRAME_HEADER) + Data Size

    //数据帧变量
    unsigned int uDataFrameSize;        //数据帧的总大小
    unsigned int uDataFrameTotal;       //一帧数据被分成传输帧的个数
    unsigned int uDataFrameCurr;        //数据帧当前的帧号
    unsigned int uDataInFrameOffset;    //数据帧在整帧的偏移
    char imageFormat[4]={0};
    unsigned int uPicnum;               //当前发送的是第几张图片
    long long  uSendDatatime;          //数据帧发送时间戳
    long long  uRecDatatime;           //数据帧接收时间戳
}PackageHead;

enum WrokerObjectrMsgTypeToDlg{
    Information,
    Warning,
    Critical
};



#endif // MSGHEAD_H
