#include "QtOpenGLLighting.h"
#include "ui_QtOpenGLLighting.h"

QtOpenGLLighting::QtOpenGLLighting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QtOpenGLLightingClass)
{
    ui->setupUi(this);
    core_widget = new CoreFunctionWidget(this);
    core_widget->setObjectName(QStringLiteral("OpenGLWidget"));
    core_widget->setGeometry(QRect(0, 0, 700, 700));

    connect(this->ui->perspectiveButton, SIGNAL(clicked()), this, SLOT(set_persective()));
    connect(this->ui->orthoButton, SIGNAL(clicked()), this, SLOT(set_ortho()));
    connect(this->core_widget, SIGNAL(projection_change()), this, SLOT(set_projection_button()));
}

QtOpenGLLighting::~QtOpenGLLighting()
{
    delete ui;
}

void QtOpenGLLighting::set_projection_button() {
    if (this->core_widget->use_perspective) {
        this->ui->perspectiveButton->setChecked(true);
    }
    else {
        this->ui->orthoButton->setChecked(true);
    }
}

void QtOpenGLLighting::set_ortho() {
    this->core_widget->use_perspective = false;
    this->core_widget->update();
}

void QtOpenGLLighting::set_persective() {
    this->core_widget->use_perspective = true;
    this->core_widget->update();
}