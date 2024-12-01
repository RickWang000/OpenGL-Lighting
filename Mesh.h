#ifndef MESH_H
#define MESH_H

#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QVector3D>
#include <QVector2D>
#include <QString>
#include <assimp/postprocess.h>

struct Vertex {
    QVector3D Position;
    QVector3D Normal;
    QVector2D TexCoords;
};

struct Texture {
    unsigned int id;
    QString type;
    aiString path;
};

class Mesh: protected QOpenGLFunctions_3_3_Core {
public:
    /*  网格数据  */
    QVector<Vertex> vertices;
    QVector<unsigned int> indices;
    QVector<Texture> textures;

    /*  函数  */
    Mesh(QVector<Vertex> vertices, QVector<unsigned int> indices, QVector<Texture> textures);
    void Draw(QOpenGLShaderProgram &shader);

    // 删除拷贝构造函数和拷贝赋值运算符
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
private:
    /*  渲染数据  */
    unsigned int VAO, VBO, EBO;

    /*  函数  */
    void setupMesh();
};



#endif // MESH_H