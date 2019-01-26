/********************************************************************************
** Form generated from reading UI file 'myqspliter.ui'
**
** Created by: Qt User Interface Compiler version 5.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MYQSPLITER_H
#define UI_MYQSPLITER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "myNavigationWidget/qnavigationwidget.h"

QT_BEGIN_NAMESPACE

class Ui_myQSpliter
{
public:
    QGridLayout *gridLayout;
    QSplitter *splitter;
    QFrame *frameList;
    QGridLayout *gridLayout_list;
    QNavigationWidget *navigationWidget;
    QFrame *frameContent;
    QGridLayout *gridLayout_content;

    void setupUi(QWidget *myQSpliter)
    {
        if (myQSpliter->objectName().isEmpty())
            myQSpliter->setObjectName(QStringLiteral("myQSpliter"));
        myQSpliter->resize(400, 300);
        gridLayout = new QGridLayout(myQSpliter);
        gridLayout->setSpacing(0);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        splitter = new QSplitter(myQSpliter);
        splitter->setObjectName(QStringLiteral("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        splitter->setHandleWidth(1);
        frameList = new QFrame(splitter);
        frameList->setObjectName(QStringLiteral("frameList"));
        frameList->setStyleSheet(QStringLiteral("background-color: rgb(255, 255, 255);"));
        frameList->setFrameShape(QFrame::StyledPanel);
        gridLayout_list = new QGridLayout(frameList);
        gridLayout_list->setSpacing(0);
        gridLayout_list->setObjectName(QStringLiteral("gridLayout_list"));
        gridLayout_list->setContentsMargins(0, 0, 0, 0);

        navigationWidget = new QNavigationWidget(frameList);
        navigationWidget->setObjectName(QStringLiteral("widget"));
        navigationWidget->setStyleSheet(QStringLiteral("background-color: rgb(255, 0, 0);"));
        gridLayout_list->addWidget(navigationWidget);

        splitter->addWidget(frameList);
        frameContent = new QFrame(splitter);
        frameContent->setObjectName(QStringLiteral("frameContent"));
        frameContent->setStyleSheet(QStringLiteral("background-color: rgb(170, 170, 255);"));
        frameContent->setFrameShape(QFrame::StyledPanel);
        frameContent->setFrameShadow(QFrame::Raised);
        gridLayout_content = new QGridLayout(frameContent);
        gridLayout_content->setSpacing(0);
        gridLayout_content->setObjectName(QStringLiteral("gridLayout_content"));
        gridLayout_content->setContentsMargins(0, 0, 0, 0);

        splitter->addWidget(frameContent);

        gridLayout->addWidget(splitter, 0, 0, 1, 1);


        retranslateUi(myQSpliter);

        QMetaObject::connectSlotsByName(myQSpliter);
    } // setupUi

    void retranslateUi(QWidget *myQSpliter)
    {
        myQSpliter->setWindowTitle(QApplication::translate("myQSpliter", "Form", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class myQSpliter: public Ui_myQSpliter {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MYQSPLITER_H
