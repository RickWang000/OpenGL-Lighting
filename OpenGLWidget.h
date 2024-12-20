#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QKeyEvent>
#include <QTimer>
#include "Camera.h"
#include "Model.h"

class CoreFunctionWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    explicit CoreFunctionWidget(QWidget* parent = nullptr);
    ~CoreFunctionWidget();

signals:
    void projection_change();
protected:
    virtual void initializeGL();
    virtual void resizeGL(int w, int h);
    virtual void paintGL();
    void keyPressEvent(QKeyEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    int mouse_x, mouse_y;

private:
    void setupShaders();
    void setupTextures();
    void setupVertices();
    void setupLighting();

    GLuint loadCubemap(std::vector<std::string> faces);
    void loadConfig();


    GLuint EBO;
    GLuint skyboxVAO, skyboxVBO, skyboxTexture;
    QOpenGLShaderProgram skyboxShaderProgram;

    GLuint lightboxVAO, lightboxVBO;
    QOpenGLShaderProgram lightboxShaderProgram;

    QVector3D lightPos;
    QVector3D lightVelocity;
    QVector3D lightColor;
    float lightSize;
    float lightSpeed;
    QVector3D lightStart;
    QVector3D lightEnd;
    QTimer* timer;

    QVector<QString> modelPaths;
    QVector<QVector3D> modelsPosition;
    QVector<QVector4D> modelsRotation;
    QVector<QVector3D> modelsScale;
    QVector<float> modelsShininess;
    QVector<float> modelsOpacity;

    Model* modelA;
    Model* modelB;
    Model* modelC;
    QOpenGLShaderProgram modelAShaderProgram;
    QOpenGLShaderProgram modelBShaderProgram;
    QOpenGLShaderProgram modelCShaderProgram;

    Camera cam;
public:
    bool use_perspective = true;
};


#endif // OPENGLWIDGET_H
