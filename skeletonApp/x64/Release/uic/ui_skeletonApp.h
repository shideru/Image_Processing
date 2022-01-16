/********************************************************************************
** Form generated from reading UI file 'skeletonApp.ui'
**
** Created by: Qt User Interface Compiler version 6.1.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SKELETONAPP_H
#define UI_SKELETONAPP_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_skeletonAppClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *skeletonAppClass)
    {
        if (skeletonAppClass->objectName().isEmpty())
            skeletonAppClass->setObjectName(QString::fromUtf8("skeletonAppClass"));
        skeletonAppClass->resize(600, 400);
        menuBar = new QMenuBar(skeletonAppClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        skeletonAppClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(skeletonAppClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        skeletonAppClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(skeletonAppClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        skeletonAppClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(skeletonAppClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        skeletonAppClass->setStatusBar(statusBar);

        retranslateUi(skeletonAppClass);

        QMetaObject::connectSlotsByName(skeletonAppClass);
    } // setupUi

    void retranslateUi(QMainWindow *skeletonAppClass)
    {
        skeletonAppClass->setWindowTitle(QCoreApplication::translate("skeletonAppClass", "skeletonApp", nullptr));
    } // retranslateUi

};

namespace Ui {
    class skeletonAppClass: public Ui_skeletonAppClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SKELETONAPP_H
