/********************************************************************************
** Form generated from reading UI file 'QtOpenGLLighting.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTOPENGLLIGHTING_H
#define UI_QTOPENGLLIGHTING_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QtOpenGLLightingClass
{
public:
    QGroupBox *groupBox;
    QRadioButton *perspectiveButton;
    QRadioButton *orthoButton;

    void setupUi(QWidget *QtOpenGLLightingClass)
    {
        if (QtOpenGLLightingClass->objectName().isEmpty())
            QtOpenGLLightingClass->setObjectName("QtOpenGLLightingClass");
        QtOpenGLLightingClass->resize(900, 700);
        groupBox = new QGroupBox(QtOpenGLLightingClass);
        groupBox->setObjectName("groupBox");
        groupBox->setGeometry(QRect(720, 30, 161, 101));
        perspectiveButton = new QRadioButton(groupBox);
        perspectiveButton->setObjectName("perspectiveButton");
        perspectiveButton->setGeometry(QRect(10, 30, 132, 22));
        perspectiveButton->setChecked(true);
        orthoButton = new QRadioButton(groupBox);
        orthoButton->setObjectName("orthoButton");
        orthoButton->setGeometry(QRect(10, 60, 132, 22));

        retranslateUi(QtOpenGLLightingClass);

        QMetaObject::connectSlotsByName(QtOpenGLLightingClass);
    } // setupUi

    void retranslateUi(QWidget *QtOpenGLLightingClass)
    {
        QtOpenGLLightingClass->setWindowTitle(QCoreApplication::translate("QtOpenGLLightingClass", "QtOpenGLLighting", nullptr));
        groupBox->setTitle(QCoreApplication::translate("QtOpenGLLightingClass", "Projection", nullptr));
        perspectiveButton->setText(QCoreApplication::translate("QtOpenGLLightingClass", "Perspective", nullptr));
        orthoButton->setText(QCoreApplication::translate("QtOpenGLLightingClass", "Ortho", nullptr));
    } // retranslateUi

};

namespace Ui {
    class QtOpenGLLightingClass: public Ui_QtOpenGLLightingClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTOPENGLLIGHTING_H
