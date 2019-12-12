#ifndef QNAVIGATIONWIDGET_H
#define QNAVIGATIONWIDGET_H

#include <QWidget>
#include <QMouseEvent>

class QNavigationWidget : public QWidget
{
    Q_OBJECT
    
public:
    QNavigationWidget(QWidget *parent=0);
    virtual ~QNavigationWidget();

    void addItem(const QString &title);
    void setWidth(const int &width);
    void setBackgroundColor(const QString &color);
    void setSelectColor(const QString &color);
    void setRowHeight(const int &height);

    int getCurrentIndex() const{return currentIndex;}
    void setCurrentIndex(int value){currentIndex = value;  emit currentItemChanged(value); update();}
    int getItemNum(){return listItems.size();}
    QString getCurrentItemText(){return listItems.at(currentIndex);}
    void setCurrentItemText(const QString &value){listItems[currentIndex] = value;}


protected:
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
signals:

private:
    QList<QString> listItems;
    QString backgroundColor;
    QString selectedColor;
    int rowHeight;
    int currentIndex;

signals:
    void currentItemChanged(const int &index);
};

#endif
