/********************************************************************************
** Form generated from reading UI file 'toast.ui'
**
** Created by: Qt User Interface Compiler version 5.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TOAST_H
#define UI_TOAST_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Toast
{
public:
    QGridLayout *gridLayout;
    QLabel *label;

    void setupUi(QWidget *Toast)
    {
        if (Toast->objectName().isEmpty())
            Toast->setObjectName(QStringLiteral("Toast"));
        Toast->resize(160, 50);
        gridLayout = new QGridLayout(Toast);
        gridLayout->setSpacing(0);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(Toast);
        label->setObjectName(QStringLiteral("label"));
        QFont font;
        font.setPointSize(10);
        label->setFont(font);
        label->setStyleSheet(QLatin1String("background-color: rgb(116, 116, 116);\n"
"border-color: rgb(79, 117, 127);\n"
"color: rgb(255, 255, 255);border-radius:20px; "));
        label->setFrameShape(QFrame::NoFrame);
        label->setTextFormat(Qt::PlainText);
        label->setScaledContents(true);
        label->setAlignment(Qt::AlignCenter);
        label->setWordWrap(false);
        label->setMaximumSize(QSize(180, 16777215));

        gridLayout->addWidget(label, 0, 0, 1, 1);


        retranslateUi(Toast);

        QMetaObject::connectSlotsByName(Toast);
    } // setupUi

    void retranslateUi(QWidget *Toast)
    {
        Toast->setWindowTitle(QApplication::translate("Toast", "Form", Q_NULLPTR));
        label->setText(QApplication::translate("Toast", "TextLabel", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class Toast: public Ui_Toast {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TOAST_H
