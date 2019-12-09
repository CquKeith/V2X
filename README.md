# V2X
DSRC与LTE对比的上位机软件。通过UDP和DSRC设备通信，TCP来模拟LTE的传输。使用Qt编写



## 2019.12.9

Issues：

- Tcp接收的时候，发出readyRead从而进入读取的时机是不确定的；另外，读取tcp数据的槽函数在退出之前，tcpSocket->bytesAvailable()是不会变化的，所以在槽函数中使用while(tcpSocket->bytesAvailable()>0)是没有用的，一直在槽函数中循环也是没有用的。具体参考[这里]( https://blog.csdn.net/dengdew/article/details/79065608 )
- 发送图片的时候可以不将图片存成文件，然后通知其他线程来读取图片文件，以达到通信的目的，可以将图片写入内存，然后告诉其他线程从内存哪里读，读取多少即可



Solutions：

- [ ] Tcp接收的槽函数，每次进来（有readyRead信号触发）就读取所有的数据，存入buffer中，这样可以清空Tcp自带的buffer，bytesAvailable就会清空，也就不存在挤在一块，时延不断上升的情况了。所有数据存入buffer中之后，尽可能读取所有的数据，也就是说一次可能读取到多张图片；
- [ ] 发送方捕获到图片之后，将图片数据存入Redis中，然后通知其他进程读取Redis
- [ ] 增加保存和加载软件设置的功能，方便调试