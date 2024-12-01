#ifndef MODEL_H
#define MODEL_H

#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <QVector>
#include <QString>
#include "Mesh.h"

class Model: protected QOpenGLFunctions_3_3_Core
{
public:
    /*  函数   */
    Model(const QString &path) {
        initializeOpenGLFunctions();
        QString extractedPath = extractResource(path);
        loadModel(path);
    }
    ~Model() {
        for (Mesh* mesh : meshes) {
            delete mesh;
        }
    }
    void Draw(QOpenGLShaderProgram &shader);

private:
    /*  模型数据  */
    QVector<Mesh*> meshes;
    QString directory;
    QVector<Texture> textures_loaded;

    /*  函数   */
    void loadModel(const QString &path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh* processMesh(aiMesh *mesh, const aiScene *scene);
    QVector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, const QString &typeName);
    unsigned int TextureFromFile(const char *path, const QString &directory);
    QString extractResource(const QString &resourcePath);
};

#endif // MODEL_H