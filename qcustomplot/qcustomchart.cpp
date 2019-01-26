#include "qcustomchart.h"
#include <QToolBar>
#include <QAction>
#include <QStackedWidget>
#include <QTextBrowser>

//////////////////////////////////////////////////////////////////////////////////////////////
/// \brief QCustomChart::QCustomChart
/// \param parent
///
QCustomChart::QCustomChart(QWidget *parent)
    : QWidget(parent)
{
    createWindow();
}

QCustomChart::~QCustomChart()
{

}

void QCustomChart::setTitle(const QString &text)
{
    m_pTitleLabel->setText(text);
}

void QCustomChart::setXaxisLabel(const QString &text)
{
    m_pCustomPlot->xAxis->setLabel(text);
}

void QCustomChart::setYaxisLabel(const QString &text)
{
    m_pCustomPlot->yAxis->setLabel(text);
}

QString QCustomChart::getXaxisLabel()
{
    return m_pCustomPlot->xAxis->label();
}

QString QCustomChart::getYaxisLabel()
{
    return m_pCustomPlot->yAxis->label();
}

void QCustomChart::setYaxisRange(double lower, double upper)
{
    // 自动调整轴上值的显示范围
    m_pCustomPlot->rescaleAxes();

    m_yLower = lower;
    m_yLower = upper;
    // 自定义y轴值的显示范围
    m_pCustomPlot->yAxis->setRange(m_yLower,m_yLower);

    // 重绘
    m_pCustomPlot->replot();
}

void QCustomChart::setData(const QVector<double> &keys, const QVector<double> &values)
{
    m_pCustomPlot->graph(0)->setData(keys, values);
    m_pCustomPlot->rescaleAxes();
    m_pCustomPlot->replot();
}

void QCustomChart::addData(double key, double value)
{
    m_pCustomPlot->graph(0)->addData(key,value);
    m_data.append(m_Data(key,value));

    m_pTextBrowser->append(tr("%1：%2  %3：%4").arg(getXaxisLabel()).arg(key).arg(getYaxisLabel()).arg(value));
    slotRefresh();
}
/**
 * @brief QCustomChart::createWindow
 *
 */
void QCustomChart::createWindow()
{
    QFont font("Segoe UI",10);
    m_pTitleLabel = new QLabel(tr("Title"));
    font.setBold(true);
    font.setPointSize(12);
    m_pTitleLabel->setFont(font);

    // action
    m_pPrimaryAction = new QAction(QIcon(":/chart/data"), tr("原始数据"), this);
    m_pChartAction = new QAction(QIcon(":/chart/line"), tr("图形"), this);
    m_pRefreshAction = new QAction(QIcon(":/chart/refresh"), tr("刷新"), this);
    m_pDownloadAction = new QAction(QIcon(":/chart/download"), tr("下载"), this);
    m_clearContent = new QAction(QIcon(":/chart/images/trash"),tr("清空"),this);

    //Download as picture or excel
    mDownloadMenu=new QMenu(this);
    downloadtoPic=new QAction(QIcon(":/chart/images/image"),"另存为图片",mDownloadMenu);
    downloadtoExcel=new QAction(QIcon(":/chart/images/excel"),"另存为Excel",mDownloadMenu);
    connect(downloadtoPic,&QAction::triggered,this,&QCustomChart::saveAsImage);
    connect(downloadtoExcel,&QAction::triggered,this,&QCustomChart::saveAsExcel);
    mDownloadMenu->addAction(downloadtoPic);
    mDownloadMenu->addAction(downloadtoExcel);
    mDownloadMenu->setStyleSheet("QPushButton::menu-indicator{image:none}");

    m_pDownloadAction->setMenu(mDownloadMenu);
    connect(m_pDownloadAction,&QAction::triggered,mDownloadMenu,&QMenu::setHidden);
    //    m_pDownloadAction->s("QPushButton::menu-indicator{image:none}");

    QActionGroup *actionGroup = new QActionGroup(this);
    m_pPrimaryAction->setCheckable(true);
    m_pChartAction->setCheckable(true);
    actionGroup->addAction(m_pPrimaryAction);
    actionGroup->addAction(m_pChartAction);

    connect(m_pPrimaryAction, SIGNAL(triggered(bool)), this, SLOT(slotPrimaryData()));
    connect(m_pChartAction, SIGNAL(triggered(bool)), this, SLOT(slotChart()));
    connect(m_pRefreshAction, SIGNAL(triggered(bool)), this, SLOT(slotRefresh()));
    //    connect(m_pDownloadAction, SIGNAL(triggered(bool)), this, SLOT(slotDownload()));
    connect(m_clearContent,SIGNAL(triggered(bool)),this,SLOT(slotClear()));

    // 将前面的按钮都放在一起
    m_pToolBar = new QToolBar(this);
    m_pToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_pToolBar->setIconSize(QSize(18,18));

    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_pToolBar->addWidget(m_pTitleLabel);
    m_pToolBar->addWidget(spacer);

    m_pToolBar->addAction(m_clearContent);

    m_pToolBar->addAction(m_pPrimaryAction);
    m_pToolBar->addAction(m_pChartAction);
    m_pToolBar->addAction(m_pRefreshAction);

    m_pToolBar->addAction(m_pDownloadAction);

    m_pStackedWidget = new QStackedWidget();
    m_pCustomPlot = new CustomPlotZoom();
    initCustomPlot();
    m_pTextBrowser = new QTextBrowser();
    m_pStackedWidget->addWidget(m_pCustomPlot);
    m_pStackedWidget->addWidget(m_pTextBrowser);

    m_pRubberBand = new QRubberBand(QRubberBand::Rectangle, m_pCustomPlot);

    connect(m_pCustomPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress(QMouseEvent*)));
    connect(m_pCustomPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMove(QMouseEvent*)));
    connect(m_pCustomPlot, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(mouseRelease(QMouseEvent*)));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(2);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_pToolBar);
    layout->addWidget(m_pStackedWidget);

    setLayout(layout);

    m_pChartAction->setChecked(true);
}

void QCustomChart::initCustomPlot()
{

    //    // 添加一个图表
    m_pCustomPlot->addGraph();
    //    // 设置关键点的显示效果
    m_pCustomPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle,
                                                             QPen(QColor(41,138,220), 2), QBrush(QColor(5,189,251)), 5));

    //    // 设置x轴显示时间
    //    // configure bottom axis to show date instead of number:
    //    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
    //    dateTicker->setDateTimeFormat("MM-dd hh:mm");
    //    m_pCustomPlot->xAxis->setTicker(dateTicker);

    //    // make range draggable and zoomable:
    m_pCustomPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    //    // make top right axes clones of bottom left axes:
    m_pCustomPlot->axisRect()->setupFullAxesBox();
    //    // connect signals so top and right axes move in sync with bottom and left axes:
    connect(m_pCustomPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), m_pCustomPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(m_pCustomPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), m_pCustomPlot->yAxis2, SLOT(setRange(QCPRange)));

    //    // 设置轴的标题
    //    m_pCustomPlot->xAxis->setLabel(tr("- Time -"));
    //        m_pCustomPlot->graph(0)->setName("delay");

    // set title of plot:
    /*
     * m_pCustomPlot->legend->setFont(QFont(font().family(),15));
    m_pCustomPlot->legend->setIconSize(50,20);
    m_pCustomPlot->legend->setVisible(true);
    m_pCustomPlot->axisRect(0)->insetLayout()->setInsetAlignment(0,Qt::AlignLeft | Qt::AlignTop);
    */



}

void QCustomChart::slotRefresh()
{
    m_pCustomPlot->rescaleAxes();
    m_pCustomPlot->replot();
}

void QCustomChart::slotDownload()
{



    //    QMessageBox msgBox;
    //    msgBox.setWindowTitle(tr("数据导出"));
    //    //    msgBox.setWindowIcon(QIcon(":/UIPic/img/ui/系统图标.png"));
    //    msgBox.setText(tr("\n请选择要导出的类型:\n图片或者Excel？"));
    //    msgBox.addButton(tr("图片"),QMessageBox::AcceptRole);
    //    msgBox.addButton(tr("Excel"),QMessageBox::AcceptRole);
    //    msgBox.addButton(tr("取消"),QMessageBox::RejectRole);


    //    switch (msgBox.exec()) {
    //    case 0:
    //        saveAsImage();
    //        break;
    //    case 1:
    //        saveAsExcel();
    //        break;
    //    default:
    //        break;
    //    }


}
/*
 * 将数据保存为图片
*/
void QCustomChart::saveAsImage()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save picture"),"untitled.png",
                                                    tr("png file(*.png);;jpg file(*.jpg);;pdf file(*.pdf)"));
    if(fileName.isEmpty()){
        return;
    }
    QString suffix = QFileInfo(fileName).suffix().toLower();
    if(suffix == "png"){
        m_pCustomPlot->savePng(fileName,m_pCustomPlot->width(), m_pCustomPlot->height());
    }else if(suffix == "jpg"){
        m_pCustomPlot->saveJpg(fileName,m_pCustomPlot->width(), m_pCustomPlot->height());
    }else if(suffix == "pdf"){
        m_pCustomPlot->savePdf(fileName,m_pCustomPlot->width(), m_pCustomPlot->height());
    }
}
/*
 * 将数据保存为Excel
*/
void QCustomChart::saveAsExcel()
{
    QString filepath=QFileDialog::getSaveFileName(this,tr("保存至Excel"),".",tr("Microsoft Office 2007 (*.xlsx)"));//获取保存路径
    if(!filepath.isEmpty()){
        QAxObject *excel = new QAxObject(this);
        excel->setControl("Excel.Application");//连接Excel控件
        excel->dynamicCall("SetVisible (bool Visible)","false");//不显示窗体
        excel->setProperty("DisplayAlerts", false);//不显示任何警告信息。如果为true那么在关闭是会出现类似“文件已修改，是否保存”的提示

        QAxObject *workbooks = excel->querySubObject("WorkBooks");//获取工作簿集合
        workbooks->dynamicCall("Add");//新建一个工作簿
        QAxObject *workbook = excel->querySubObject("ActiveWorkBook");//获取当前工作簿
        QAxObject *worksheets = workbook->querySubObject("Sheets");//获取工作表集合
        QAxObject *worksheet = worksheets->querySubObject("Item(int)",1);//获取工作表集合的工作表1，即sheet1

        QAxObject *cellA,*cellB;

        //设置标题
        int cellrow=1;
        QString A="A"+QString::number(cellrow);//设置要操作的单元格，如A1
        QString B="B"+QString::number(cellrow);
        //        QString C="C"+QString::number(cellrow);
        //        QString D="D"+QString::number(cellrow);
        cellA = worksheet->querySubObject("Range(QVariant, QVariant)",A);//获取单元格
        cellB = worksheet->querySubObject("Range(QVariant, QVariant)",B);
        //        cellC=worksheet->querySubObject("Range(QVariant, QVariant)",C);
        //        cellD=worksheet->querySubObject("Range(QVariant, QVariant)",D);
        cellA->dynamicCall("SetValue(const QVariant&)",QVariant(getXaxisLabel()));//设置单元格的值
        cellB->dynamicCall("SetValue(const QVariant&)",QVariant(getYaxisLabel()));
        //        cellC->dynamicCall("SetValue(const QVariant&)",QVariant("金额"));
        //        cellD->dynamicCall("SetValue(const QVariant&)",QVariant("日期"));
        cellrow++;

        //        int rows=this->model->rowCount();
        int rows = m_data.size();
        for(int i=0;i<rows;i++){
            QString A="A"+QString::number(cellrow);//设置要操作的单元格，如A1
            QString B="B"+QString::number(cellrow);
            //            QString C="C"+QString::number(cellrow);
            //            QString D="D"+QString::number(cellrow);
            cellA = worksheet->querySubObject("Range(QVariant, QVariant)",A);//获取单元格
            cellB = worksheet->querySubObject("Range(QVariant, QVariant)",B);
            //            cellC=worksheet->querySubObject("Range(QVariant, QVariant)",C);
            //            cellD=worksheet->querySubObject("Range(QVariant, QVariant)",D);
            cellA->dynamicCall("SetValue(const QVariant&)",QVariant(m_data.at(cellrow-2).V));//设置单元格的值
            cellB->dynamicCall("SetValue(const QVariant&)",QVariant(m_data.at(cellrow-2).I));
            //            cellC->dynamicCall("SetValue(const QVariant&)",QVariant("this->model->item(i,2)->data(Qt::DisplayRole).toString()"));
            //            cellD->dynamicCall("SetValue(const QVariant&)",QVariant("this->model->item(i,3)->data(Qt::DisplayRole).toString()"));
            cellrow++;
        }

        emit signalExportExcelDone();

        workbook->dynamicCall("SaveAs(const QString&)",QDir::toNativeSeparators(filepath));//保存至filepath，注意一定要用QDir::toNativeSeparators将路径中的"/"转换为"\"，不然一定保存不了。
        workbook->dynamicCall("Close()");//关闭工作簿
        excel->dynamicCall("Quit()");//关闭excel
        delete excel;
        excel=NULL;
    }
}

void QCustomChart::slotPrimaryData()
{
    m_pStackedWidget->setCurrentWidget(m_pTextBrowser);
}

void QCustomChart::slotChart()
{
    m_pStackedWidget->setCurrentWidget(m_pCustomPlot);
}

void QCustomChart::slotClear()
{
    m_data.clear();
    m_pTextBrowser->clear();

    // 清空现有图形
    m_pCustomPlot->graph(0)->data().data()->clear();
    //    m_pCustomPlot->clearItems();
    slotRefresh();
}

void QCustomChart::mousePress(QMouseEvent *mevent)
{
    if(mevent->button() == Qt::RightButton)
    {
        m_rubberOrigin = mevent->pos();
        m_pRubberBand->setGeometry(QRect(m_rubberOrigin, QSize()));
        m_pRubberBand->show();
    }
}

void QCustomChart::mouseMove(QMouseEvent *mevent)
{
    if(m_pRubberBand->isVisible())
        m_pRubberBand->setGeometry(QRect(m_rubberOrigin, mevent->pos()).normalized());
}

void QCustomChart::mouseRelease(QMouseEvent *mevent)
{
    Q_UNUSED(mevent);
    if (m_pRubberBand->isVisible())
    {
        const QRect zoomRect = m_pRubberBand->geometry();
        int xp1, yp1, xp2, yp2;
        zoomRect.getCoords(&xp1, &yp1, &xp2, &yp2);
        double x1 = m_pCustomPlot->xAxis->pixelToCoord(xp1);
        double x2 = m_pCustomPlot->xAxis->pixelToCoord(xp2);
        double y1 = m_pCustomPlot->yAxis->pixelToCoord(yp1);
        double y2 = m_pCustomPlot->yAxis->pixelToCoord(yp2);

        m_pCustomPlot->xAxis->setRange(x1, x2);
        m_pCustomPlot->yAxis->setRange(y1, y2);

        m_pRubberBand->hide();
        m_pCustomPlot->replot();
    }
}

//////////////////////////////////////////////////////////////////////////
/// \brief CustomPlotZoom::CustomPlotZoom
/// \param parent
///
CustomPlotZoom::CustomPlotZoom(QWidget *parent)
    : QCustomPlot(parent)
    , m_bZoomMode(false)
    , m_pRubberBand(new QRubberBand(QRubberBand::Rectangle, this))
{

}

CustomPlotZoom::~CustomPlotZoom()
{

}

void CustomPlotZoom::setZoomMode(bool mode)
{
    m_bZoomMode = mode;
}

void CustomPlotZoom::mousePressEvent(QMouseEvent *event)
{
    if (m_bZoomMode)
    {
        if (event->button() == Qt::RightButton)
        {
            m_origin = event->pos();
            m_pRubberBand->setGeometry(QRect(m_origin, QSize()));
            m_pRubberBand->show();
        }
    }
    QCustomPlot::mousePressEvent(event);
}

void CustomPlotZoom::mouseMoveEvent(QMouseEvent *event)
{
    if (m_pRubberBand->isVisible())
    {
        m_pRubberBand->setGeometry(QRect(m_origin, event->pos()).normalized());
    }
    QCustomPlot::mouseMoveEvent(event);
}

void CustomPlotZoom::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_pRubberBand->isVisible())
    {
        const QRect zoomRect = m_pRubberBand->geometry();
        int xp1, yp1, xp2, yp2;
        zoomRect.getCoords(&xp1, &yp1, &xp2, &yp2);
        double x1 = xAxis->pixelToCoord(xp1);
        double x2 = xAxis->pixelToCoord(xp2);
        double y1 = yAxis->pixelToCoord(yp1);
        double y2 = yAxis->pixelToCoord(yp2);

        xAxis->setRange(x1, x2);
        yAxis->setRange(y1, y2);

        m_pRubberBand->hide();
        replot();
    }
    QCustomPlot::mouseReleaseEvent(event);
}

