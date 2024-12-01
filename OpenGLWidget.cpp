#include "OpenGLWidget.h"
#include <QDebug>
#include <QTimer>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

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
    

}

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
    this->cam.set_initial_distance_ratio(8.0);
    
    loadConfig(); // 加载配置文件

    setupShaders();
    setupTextures();
    setupVertices();
    
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

