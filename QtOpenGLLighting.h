#ifndef QTOPENGLLIGHTING_H
#define QTOPENGLLIGHTING_H

#include <QWidget>
#include "OpenGLWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class QtOpenGLLightingClass; }
QT_END_NAMESPACE

class QtOpenGLLighting : public QWidget
{
    Q_OBJECT

public:
    QtOpenGLLighting(QWidget *parent = nullptr);
    ~QtOpenGLLighting();

public slots:
    void set_ortho();
    void set_persective();
    void set_projection_button();

private:
    Ui::QtOpenGLLightingClass *ui;
    CoreFunctionWidget *core_widget;
};
#endif // QTOPENGLLIGHTING_H
