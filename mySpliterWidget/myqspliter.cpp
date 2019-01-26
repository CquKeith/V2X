#include "myqspliter.h"
#include "ui_myqspliter.h"


myQSpliter::myQSpliter(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::myQSpliter)
{
    ui->setupUi(this);
    maxListFrameWidthRatio = 0.2;
    isWidthChanged = false;

    width = this->size().width();
    height = this->size().height();

//    resize(width,height);

    InitWidget();
    //使用槽函数进行设置箭头按钮的位置，避免刚开始界面尺寸不确定带来的按钮位置不准现象
    //    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    //    qDebug()<<(int)qrand()%10;
    QTimer::singleShot(10,[=]{
//        qDebug()<<"singleShot";
//        slotClickedBtn();
        slotClickedBtn();
    });


}

myQSpliter::~myQSpliter()
{
    delete ui;
}
/**
 * @brief myQSpliter::resizeEvent
 * @param event
 * 窗口大小发生变化时，更新位置
 */
void myQSpliter::resizeEvent(QResizeEvent *event)
{

    isWidthChanged = true;


    width = this->size().width();
    height = this->size().height();

    ui->frameList->setMaximumWidth(width*maxListFrameWidthRatio);
    ui->frameContent->setMinimumWidth(width*(1-maxListFrameWidthRatio));

    ui->navigationWidget->setWidth(ui->frameList->width());

    setBtnPos();
    setBtnIcon();

    return QWidget::resizeEvent(event);;
}
/**
 * @brief myQSpliter::eventFilter 鼠标经过按钮时 显示按钮，否则隐藏按钮
 * @param obj
 * @param event
 * @return
 */
bool myQSpliter::eventFilter(QObject *obj, QEvent *event)
{

    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseMove = static_cast<QMouseEvent*>(event);
        QRect rect = btn->frameGeometry();

        if (rect.contains(mouseMove->pos())) {
            btn->show();
        }
        else {
            btn->hide();
        }
    }
    return QWidget::eventFilter(obj, event);
}

/**
 * @brief myQSpliter::slotClickedBtn 按钮点击时，更改菜单的大小
 */
void myQSpliter::slotClickedBtn()
{
//    qDebug()<<tr("in %1()").arg(__FUNCTION__);
    QList <int> sizeList;
    sizeList.clear();
    if (ui->frameList->width() != 0) {
        sizeList.append(0);
        sizeList.append(width);
    }
    else {
        sizeList.append(ui->frameList->maximumWidth());
        sizeList.append(ui->frameContent->minimumWidth());
    }
    ui->splitter->setSizes(sizeList);


    setBtnPos();
    setBtnIcon();
    btn->show();

    ui->navigationWidget->setWidth(ui->frameList->width());
}
/**
 * @brief myQSpliter::slotSplitterMoved 鼠标手动滑动spliter时刷新按钮位置
 * @param pos
 * @param index
 */
void myQSpliter::slotSplitterMoved(int pos, int index)
{
    Q_UNUSED(pos)
    Q_UNUSED(index)


    setBtnIcon();
    setBtnPos();


//    qDebug()<<"index:"<<index<<" pos:"<<pos;

    ui->navigationWidget->setWidth(ui->frameList->width());



}
/**
 * @brief myQSpliter::getMaxListFrameWidthRatio
 * @return
 */
float myQSpliter::getMaxListFrameWidthRatio() const
{
    return maxListFrameWidthRatio;
}
/**
 * @brief myQSpliter::setMaxListFrameWidthRatio
 * @param value listFrame所占的比例 0~1的一个数
 */
void myQSpliter::setMaxListFrameWidthRatio(float value)
{
    maxListFrameWidthRatio = value;

}
/**
 * @brief myQSpliter::addListWidget 增加一个菜单控件
 * @param list
 */
void myQSpliter::addListWidget(QWidget *list)
{
    ui->gridLayout_list->addWidget(list);
}
/**
 * @brief myQSpliter::addContentWidget 增加一个内容
 * @param content
 */
void myQSpliter::addContentWidget(QWidget* content)
{
    ui->gridLayout_content->addWidget(content);

}

QNavigationWidget *myQSpliter::getMenuWidget()
{
    return ui->navigationWidget;
}

QFrame *myQSpliter::getContentFrame()
{
    return ui->frameContent;
}

QLayout *myQSpliter::getContentLayout()
{
    return ui->gridLayout_content;
}
/**
 * @brief myQSpliter::refreshForm
 */
void myQSpliter::refreshForm()
{
    qDebug()<<"refreshForm()";
//    setBtnIcon();
//    setBtnPos();
//    ui->navigationWidget->setWidth(width*getMaxListFrameWidthRatio());
    slotClickedBtn();
    slotClickedBtn();
}

/**
 * @brief myQSpliter::InitWidget 初始化控件
 */
void myQSpliter::InitWidget()
{
    leftIcon = QIcon(":img/navigate-left.ico");
    rightIcon = QIcon(":img/navigate-right.ico");
    btn = new QPushButton(this);
    btn->setFocusPolicy(Qt::NoFocus);
//    btn->hide();
        btn->show();
    btn->setFixedSize(13, 42);
    btn->setIconSize(btn->size());
    btn->setStyleSheet("border:none;");
    btn->setIcon(leftIcon);
    btn->move(width*maxListFrameWidthRatio, (height - btn->height())/2);
    connect(btn,&QPushButton::clicked,this,&myQSpliter::slotClickedBtn);

    connect(ui->splitter,&QSplitter::splitterMoved,this,&myQSpliter::slotSplitterMoved);


    ui->frameList->setMouseTracking(true);
    ui->navigationWidget->setMouseTracking(true);

    ui->frameList->installEventFilter(this);
    ui->navigationWidget->installEventFilter(this);

    connect(ui->navigationWidget,&QNavigationWidget::currentItemChanged,this,[=](int index){
        emit currentMenuItemChanged(index);
    });

    ui->frameList->setMaximumWidth(width*maxListFrameWidthRatio);
    ui->frameContent->setMinimumWidth(width*(1-maxListFrameWidthRatio));



}
/**
 * @brief myQSpliter::setBtnPos
 * 设置箭头按钮的位置
 */
void myQSpliter::setBtnPos()
{

    if (ui->frameList->width() > 0){
        btn->move(ui->frameList->width() - btn->width(), (height - btn->height())/2);
    }else{
        btn->move(0, (height - btn->height())/2);
    }
}
/**
 * @brief myQSpliter::setBtnIcon
 * 根据位置设置箭头按钮的图标
 */
void myQSpliter::setBtnIcon()
{

    if (ui->frameList->width() > 0) {
        btn->setIcon(leftIcon);
    }
    else {
        btn->setIcon(rightIcon);
    }
}
