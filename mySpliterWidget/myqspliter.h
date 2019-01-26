#ifndef MYQSPLITER_H
#define MYQSPLITER_H

#include <QWidget>
#include <QResizeEvent>
#include <QPushButton>
#include <QTimer>
#include <QTime>
#include <QDebug>
#include <QFrame>
#include "myNavigationWidget/qnavigationwidget.h"
namespace Ui {
class myQSpliter;
}

class myQSpliter : public QWidget
{
    Q_OBJECT

public:
    explicit myQSpliter(QWidget *parent = 0);
    ~myQSpliter();
    float getMaxListFrameWidthRatio() const;
    void setMaxListFrameWidthRatio(float value);

    void addListWidget(QWidget * list);
    void addContentWidget(QWidget *content);
    QNavigationWidget *getMenuWidget();
    QFrame *getContentFrame();
    QLayout* getContentLayout();

    void refreshForm();

signals:
    void currentMenuItemChanged(const int &index);

protected:
    void resizeEvent(QResizeEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
//    void mouseReleaseEvent(QMouseEvent *e);
//    void mousePressEvent(QMouseEvent *e);

private slots:
    void slotClickedBtn();
    void slotSplitterMoved(int pos, int index);
private:
    Ui::myQSpliter *ui;

    int width,height;
    float maxListFrameWidthRatio;

    bool isWidthChanged;

    QPushButton *btn;
    QIcon leftIcon;
    QIcon rightIcon;

    void InitWidget();
    void setBtnPos();
    void setBtnIcon();

};

#endif // MYQSPLITER_H
