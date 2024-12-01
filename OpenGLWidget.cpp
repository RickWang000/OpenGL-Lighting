#include "OpenGLWidget.h"
#include <QDebug>
#include <QTimer>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

CoreFunctionWidget::CoreFunctionWidget(QWidget* parent) : QOpenGLWidget(parent)
{
    this->setFocusPolicy(Qt::StrongFocus);
}

CoreFunctionWidget::~CoreFunctionWidget()
{
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteTextures(1, &skyboxTexture);
    skyboxShaderProgram.removeAllShaders();
}


void CoreFunctionWidget::initializeGL() {
    this->initializeOpenGLFunctions();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND); // 启用混合
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 设置混合函数

    this->cam.set_initial_distance_ratio(10.0);
    
    loadConfig(); // 加载配置文件

    modelA = new Model(modelPaths[0].toStdString().c_str());
    qDebug() << "modelA loaded!";

    modelB = new Model(modelPaths[1].toStdString().c_str());
    qDebug() << "modelB loaded!";
    
    modelC = new Model(modelPaths[2].toStdString().c_str());
    qDebug() << "modelC loaded!";

    setupShaders();
    setupTextures();
    setupVertices();
    setupLighting();
}


void CoreFunctionWidget::loadConfig() {
    QFile file(":/config.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open config file!";
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc(QJsonDocument::fromJson(data));
    QJsonObject json = doc.object();

    // 读取
    // 读取光源盒配置
    QJsonObject lightbox = json["lightbox"].toObject();
    QVector3D lightColor(lightbox["color"].toArray()[0].toDouble(),
                         lightbox["color"].toArray()[1].toDouble(),
                         lightbox["color"].toArray()[2].toDouble());
    float lightSize = lightbox["size"].toDouble();
    float lightSpeed = lightbox["speed"].toDouble();
    QVector3D lightStart(lightbox["start"].toArray()[0].toDouble(),
                         lightbox["start"].toArray()[1].toDouble(),
                         lightbox["start"].toArray()[2].toDouble());
    QVector3D lightEnd(lightbox["end"].toArray()[0].toDouble(),
                       lightbox["end"].toArray()[1].toDouble(),
                       lightbox["end"].toArray()[2].toDouble());

    // 设置光源盒属性
    this->lightColor = lightColor;
    this->lightSize = lightSize;
    this->lightSpeed = lightSpeed;
    this->lightStart = lightStart;
    this->lightEnd = lightEnd;
    this->lightPos = lightStart;
    this->lightVelocity = (lightEnd - lightStart).normalized() * lightSpeed;

    // 读取模型配置
    QJsonArray models = json["models"].toArray();
    for (int i = 0; i < models.size(); ++i) {
        QJsonObject model = models[i].toObject();
        QString path = model["path"].toString();
        QVector3D position(model["position"].toArray()[0].toDouble(),
                           model["position"].toArray()[1].toDouble(),
                           model["position"].toArray()[2].toDouble());
        QVector4D rotation(model["rotation"].toArray()[0].toDouble(),
                           model["rotation"].toArray()[1].toDouble(),
                           model["rotation"].toArray()[2].toDouble(),
                           model["rotation"].toArray()[3].toDouble());
        QVector3D scale(model["scale"].toArray()[0].toDouble(),
                        model["scale"].toArray()[1].toDouble(),
                        model["scale"].toArray()[2].toDouble());
        float shininess = model["shininess"].toDouble();
        float opacity = model["opacity"].toDouble();

        // 设置模型属性
        modelPaths.append(path);
        modelsPosition.append(position);
        modelsRotation.append(rotation);
        modelsScale.append(scale);
        modelsShininess.append(shininess);
        modelsOpacity.append(opacity);
    }

}


void CoreFunctionWidget::setupShaders() {
    // 初始化天空盒着色器
    bool success = skyboxShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/skybox.vert");
    if (!success) {
        qDebug() << "skyboxShaderProgram addShaderFromSourceFile failed!" << skyboxShaderProgram.log();
        return;
    }

    success = skyboxShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/skybox.frag");
    if (!success) {
        qDebug() << "skyboxShaderProgram addShaderFromSourceFile failed!" << skyboxShaderProgram.log();
        return;
    }

    success = skyboxShaderProgram.link();
    if (!success) {
        qDebug() << "skyboxShaderProgram link failed!" << skyboxShaderProgram.log();
    }

    // 初始化光源着色器
    success = lightboxShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/lightbox.vert");
    if (!success) {
        qDebug() << "lightboxShaderProgram addShaderFromSourceFile failed!" << lightboxShaderProgram.log();
        return;
    }

    success = lightboxShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/lightbox.frag");
    if (!success) {
        qDebug() << "lightboxShaderProgram addShaderFromSourceFile failed!" << lightboxShaderProgram.log();
        return;
    }

    success = lightboxShaderProgram.link();
    if (!success) {
        qDebug() << "lightboxShaderProgram link failed!" << lightboxShaderProgram.log();
    }


    // 初始化模型A着色器
    success = modelAShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/model.vert");
    if (!success) {
        qDebug() << "modelAShaderProgram addShaderFromSourceFile failed!" << modelAShaderProgram.log();
        return;
    }

    success = modelAShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/model.frag");
    if (!success) {
        qDebug() << "modelAShaderProgram addShaderFromSourceFile failed!" << modelAShaderProgram.log();
        return;
    }

    success = modelAShaderProgram.link();
    if (!success) {
        qDebug() << "modelAShaderProgram link failed!" << modelAShaderProgram.log();
    }

    // 初始化模型B着色器
    success = modelBShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/model.vert");
    if (!success) {
        qDebug() << "modelBShaderProgram addShaderFromSourceFile failed!" << modelBShaderProgram.log();
        return;
    }

    success = modelBShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/model.frag");
    if (!success) {
        qDebug() << "modelBShaderProgram addShaderFromSourceFile failed!" << modelBShaderProgram.log();
        return;
    }

    success = modelBShaderProgram.link();
    if (!success) {
        qDebug() << "modelBShaderProgram link failed!" << modelBShaderProgram.log();
    }


    // 初始化模型C着色器
    success = modelCShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/model.vert");
    if (!success) {
        qDebug() << "modelCShaderProgram addShaderFromSourceFile failed!" << modelCShaderProgram.log();
        return;
    }

    success = modelCShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/model.frag");
    if (!success) {
        qDebug() << "modelCShaderProgram addShaderFromSourceFile failed!" << modelCShaderProgram.log();
        return;
    }

    success = modelCShaderProgram.link();
    if (!success) {
        qDebug() << "modelCShaderProgram link failed!" << modelCShaderProgram.log();
    }


}

void CoreFunctionWidget::setupTextures() {
    // 加载天空盒纹理
    std::vector<std::string> faces
    {
        ":/res/skybox/right.jpg",
        ":/res/skybox/left.jpg",
        ":/res/skybox/top.jpg",
        ":/res/skybox/bottom.jpg",
        ":/res/skybox/front.jpg",
        ":/res/skybox/back.jpg"
    };
    skyboxTexture = loadCubemap(faces);

}

GLuint CoreFunctionWidget::loadCubemap(std::vector<std::string> faces) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        QImage img = QImage(faces[i].c_str()).convertToFormat(QImage::Format_RGB888);
        if (!img.isNull())
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, img.width(), img.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, img.bits());
        }
        else
        {
            qDebug() << "Cubemap texture failed to load at path: " << faces[i].c_str();
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void CoreFunctionWidget::setupVertices() {
    // 设置天空盒 VAO 和 VBO
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);


    // 设置光源 VAO 和 VBO
    float lightboxVertices[] = {
        // positions          
        -0.1f,  0.1f, -0.1f,
        -0.1f, -0.1f, -0.1f,
         0.1f, -0.1f, -0.1f,
         0.1f, -0.1f, -0.1f,
         0.1f,  0.1f, -0.1f,
        -0.1f,  0.1f, -0.1f,

        -0.1f, -0.1f,  0.1f,
        -0.1f, -0.1f, -0.1f,
        -0.1f,  0.1f, -0.1f,
        -0.1f,  0.1f, -0.1f,
        -0.1f,  0.1f,  0.1f,
        -0.1f, -0.1f,  0.1f,

         0.1f, -0.1f, -0.1f,
         0.1f, -0.1f,  0.1f,
         0.1f,  0.1f,  0.1f,
         0.1f,  0.1f,  0.1f,
         0.1f,  0.1f, -0.1f,
         0.1f, -0.1f, -0.1f,

        -0.1f, -0.1f,  0.1f,
        -0.1f,  0.1f,  0.1f,
         0.1f,  0.1f,  0.1f,
         0.1f,  0.1f,  0.1f,
         0.1f, -0.1f,  0.1f,
        -0.1f, -0.1f,  0.1f,

        -0.1f,  0.1f, -0.1f,
         0.1f,  0.1f, -0.1f,
         0.1f,  0.1f,  0.1f,
         0.1f,  0.1f,  0.1f,
        -0.1f,  0.1f,  0.1f,
        -0.1f,  0.1f, -0.1f,

        -0.1f, -0.1f, -0.1f,
        -0.1f, -0.1f,  0.1f,
         0.1f, -0.1f, -0.1f,
         0.1f, -0.1f, -0.1f,
        -0.1f, -0.1f,  0.1f,
        0.1f, -0.1f,  0.1f
    };

    glGenVertexArrays(1, &lightboxVAO);
    glGenBuffers(1, &lightboxVBO);
    glBindVertexArray(lightboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lightboxVertices), &lightboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);


}

void CoreFunctionWidget::setupLighting() {
    // 设置光源运动
    lightPos = lightStart;
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [=]() {
        lightPos += lightVelocity;
        if ((lightPos - lightStart).length() > (lightEnd - lightStart).length()) {
            lightVelocity = -lightVelocity;
        }
        update();
    });

    timer->start(1000 / 60); // 60fps
}

void CoreFunctionWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

void CoreFunctionWidget::paintGL() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // 渲染天空盒
    glDepthFunc(GL_LEQUAL);  // 更改深度函数，以便天空盒能在最远处绘制
    skyboxShaderProgram.bind();
    {
        QMatrix4x4 view = this->cam.get_camera_matrix();
        view.setColumn(3, QVector4D(0, 0, 0, 1));
        QMatrix4x4 projection;
        if (this->use_perspective)
            projection.perspective(90, 1.0, 0.01, 50.0);
        else
            projection.ortho(-2, 2, -2, 2, 0.01, 50.0);

        glUniformMatrix4fv(skyboxShaderProgram.uniformLocation("view"), 1, GL_FALSE, view.data());
        glUniformMatrix4fv(skyboxShaderProgram.uniformLocation("projection"), 1, GL_FALSE, projection.data());
        // 绘制天空盒
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }
    skyboxShaderProgram.release();
    glDepthFunc(GL_LESS); // 重置深度函数

    // 渲染光源
    lightboxShaderProgram.bind();
    {
        QMatrix4x4 view = this->cam.get_camera_matrix();
        QMatrix4x4 projection;
        if (this->use_perspective)
            projection.perspective(90, 1.0, 0.01, 50.0);
        else
            projection.ortho(-2, 2, -2, 2, 0.01, 50.0);

        glUniformMatrix4fv(lightboxShaderProgram.uniformLocation("view"), 1, GL_FALSE, view.data());
        glUniformMatrix4fv(lightboxShaderProgram.uniformLocation("projection"), 1, GL_FALSE, projection.data());
        QMatrix4x4 modelMatrix;

        modelMatrix.translate(lightPos);
        modelMatrix.scale(lightSize, lightSize, lightSize);
        lightboxShaderProgram.setUniformValue("model", modelMatrix);

        glBindVertexArray(lightboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }
    lightboxShaderProgram.release();

    // 渲染模型A
    modelAShaderProgram.bind();
    {
        QMatrix4x4 view = this->cam.get_camera_matrix();
        QMatrix4x4 projection;
        if (this->use_perspective)
            projection.perspective(90, 1.0, 0.01, 50.0);
        else
            projection.ortho(-2, 2, -2, 2, 0.01, 50.0);

        glUniformMatrix4fv(modelAShaderProgram.uniformLocation("view"), 1, GL_FALSE, view.data());
        glUniformMatrix4fv(modelAShaderProgram.uniformLocation("projection"), 1, GL_FALSE, projection.data());
        QMatrix4x4 modelMatrix;

        modelMatrix.translate(modelsPosition[0]);
        modelMatrix.scale(modelsScale[0]);
        modelMatrix.rotate(modelsRotation[0].x(), modelsRotation[0].y(), modelsRotation[0].z(), modelsRotation[0].w());
        modelAShaderProgram.setUniformValue("model", modelMatrix);

        // 设置光源属性
        modelAShaderProgram.setUniformValue("light.position", lightPos);
        modelAShaderProgram.setUniformValue("light.ambient", QVector3D(0.5f, 0.5f, 0.5f));
        modelAShaderProgram.setUniformValue("light.diffuse", QVector3D(0.5f, 0.5f, 0.5f));
        modelAShaderProgram.setUniformValue("light.specular", QVector3D(1.0f, 1.0f, 1.0f));
        modelAShaderProgram.setUniformValue("viewPos", this->cam.eye);

        // 设置材质属性
        modelAShaderProgram.setUniformValue("material.shininess", modelsShininess[0]);
        modelAShaderProgram.setUniformValue("material.opacity", modelsOpacity[0]);

        
        modelA->Draw(modelAShaderProgram);
    }
    modelAShaderProgram.release();

    // 渲染模型B
    modelBShaderProgram.bind();
    {
        QMatrix4x4 view = this->cam.get_camera_matrix();
        QMatrix4x4 projection;
        if (this->use_perspective)
            projection.perspective(90, 1.0, 0.01, 50.0);
        else
            projection.ortho(-2, 2, -2, 2, 0.01, 50.0);

        glUniformMatrix4fv(modelBShaderProgram.uniformLocation("view"), 1, GL_FALSE, view.data());
        glUniformMatrix4fv(modelBShaderProgram.uniformLocation("projection"), 1, GL_FALSE, projection.data());
        QMatrix4x4 modelMatrix;

        modelMatrix.translate(modelsPosition[1]);
        modelMatrix.scale(modelsScale[1]);
        modelMatrix.rotate(180.0f, QVector3D(0.0f, 1.0f, 0.0f));
        modelBShaderProgram.setUniformValue("model", modelMatrix);

        // 设置光源属性
        modelBShaderProgram.setUniformValue("light.position", lightPos);
        modelBShaderProgram.setUniformValue("light.ambient", QVector3D(0.5f, 0.5f, 0.5f));
        modelBShaderProgram.setUniformValue("light.diffuse", QVector3D(0.5f, 0.5f, 0.5f));
        modelBShaderProgram.setUniformValue("light.specular", QVector3D(1.0f, 1.0f, 1.0f));
        modelBShaderProgram.setUniformValue("viewPos", this->cam.eye);

        // 设置材质属性
        modelAShaderProgram.setUniformValue("material.shininess", modelsShininess[1]);
        modelAShaderProgram.setUniformValue("material.opacity", modelsOpacity[1]);


        modelB->Draw(modelBShaderProgram);
    }

    modelBShaderProgram.release();

    // 渲染模型C
    modelCShaderProgram.bind();
    {
        QMatrix4x4 view = this->cam.get_camera_matrix();
        QMatrix4x4 projection;
        if (this->use_perspective)
            projection.perspective(90, 1.0, 0.01, 50.0);
        else
            projection.ortho(-2, 2, -2, 2, 0.01, 50.0);

        glUniformMatrix4fv(modelCShaderProgram.uniformLocation("view"), 1, GL_FALSE, view.data());
        glUniformMatrix4fv(modelCShaderProgram.uniformLocation("projection"), 1, GL_FALSE, projection.data());
        QMatrix4x4 modelMatrix;

        modelMatrix.translate(modelsPosition[2]);
        modelMatrix.scale(modelsScale[2]);
        modelMatrix.rotate(180.0f, QVector3D(0.0f, 1.0f, 0.0f));
        modelCShaderProgram.setUniformValue("model", modelMatrix);

        // 设置光源属性
        modelCShaderProgram.setUniformValue("light.position", lightPos);
        modelCShaderProgram.setUniformValue("light.ambient", QVector3D(0.5f, 0.5f, 0.5f));
        modelCShaderProgram.setUniformValue("light.diffuse", QVector3D(0.5f, 0.5f, 0.5f));
        modelCShaderProgram.setUniformValue("light.specular", QVector3D(1.0f, 1.0f, 1.0f));
        modelCShaderProgram.setUniformValue("viewPos", this->cam.eye);

        // 设置材质属性
        modelAShaderProgram.setUniformValue("material.shininess", modelsShininess[2]);
        modelAShaderProgram.setUniformValue("material.opacity", modelsOpacity[2]);


        modelC->Draw(modelCShaderProgram);
    }

    modelCShaderProgram.release();

}


void CoreFunctionWidget::keyPressEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_A) {
        this->cam.translate_left(0.2);
    }
    else if (e->key() == Qt::Key_D) {
        this->cam.translate_left(-0.2);
    }
    else if (e->key() == Qt::Key_W) {
        this->cam.translate_up(0.2);
    }
    else if (e->key() == Qt::Key_S) {
        this->cam.translate_up(-0.2);
    }
    else if (e->key() == Qt::Key_F) {
        this->cam.translate_forward(0.2);
    }
    else if (e->key() == Qt::Key_B) {
        this->cam.translate_forward(-0.2);;
    }
    else if (e->key() == Qt::Key_Z) {
        this->cam.zoom_near(0.1);
    }
    else if (e->key() == Qt::Key_X) {
        this->cam.zoom_near(-0.1);
    }
    else if (e->key() == Qt::Key_T) {
        this->use_perspective = !this->use_perspective;
    }

    emit projection_change();

    update();
}

void CoreFunctionWidget::mousePressEvent(QMouseEvent* e) {
    mouse_x = e->position().x();
    mouse_y = e->position().y();
}

void CoreFunctionWidget::mouseMoveEvent(QMouseEvent* e) {
    int x = e->position().x();
    int y = e->position().y();

    int threshold = 2;
    int delta_angle = 1;

    if (abs(x - mouse_x) >= threshold) {
        if (x > mouse_x) {
            this->cam.rotate_left(delta_angle);
        }
        else {
            this->cam.rotate_left(-delta_angle);
        }
        mouse_x = x;
    }

    if (abs(y - mouse_y) >= threshold) {
        if (y > mouse_y) {
            this->cam.rotate_up(delta_angle);
        }
        else {
            this->cam.rotate_up(-delta_angle);
        }

        mouse_y = y;
    }

    update();
}

