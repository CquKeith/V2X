#ifndef MSGHEAD_H
#define MSGHEAD_H
#include <QObject>
#include <QMutex>
#include <QMap>

// 一张图片 最大1M
#define MAX_IMAGE_SIZE 1024*1024*1
//一帧数据的最大字节数
#define MAX_ONE_FRAME_SIZE 1300

//最多可以同时缓冲多少张图片
#define MEM_CACHE_MAX_SIZE 100

static QMutex mutex_hostIPandPort;
static QMutex mutex_mBuffer;

/*内存缓冲的结构体*/
typedef struct memCacheStructure
{
    quint32 memSize;    //从起始地址开始存了多少的数据
    quint16 picNum;     //这是存的哪张图片的内容
    bool isVisited;     //如果 为true，说明此段内存空间可以被使用
    char* memStart;   //内存的起始地址

}s_memCache;

/*Map的key是当前图片的id%MEM_CACHE_MAX_SIZE,value是这段内存的结构体*/
static QMap<quint16,s_memCache> memCacheMap;

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
//    char imageFormat[4]={0};
    unsigned int uPicnum;               //当前发送的是第几张图片
    long long  uSendDatatime;          //数据帧发送时间戳
    long long  uRecDatatime;           //数据帧接收时间戳
//    double picSize;                    //如果是发送的视频或者图片，这里标记图片文件的大小
}PackageHead;

enum WrokerObjectrMsgTypeToDlg{
    Information,
    Warning,
    Critical
};
enum VideoDeviceType{
    Camera,
    File
};




#endif // MSGHEAD_H
