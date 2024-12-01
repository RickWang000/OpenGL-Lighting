#include "Model.h"
#include <QDebug>
#include <QStandardPaths>
#include <QFileInfo>
void Model::Draw(QOpenGLShaderProgram &shader)
{
    for (int i = 0; i < meshes.size(); i++)
        meshes[i]->Draw(shader);
}

void Model::loadModel(const QString &path)
{
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path.toStdString(), aiProcess_Triangulate | aiProcess_FlipUVs);

    qDebug() << "Model loading: " << path;
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        qDebug() << "ERROR::ASSIMP::" << import.GetErrorString();
        return;
    }
    qDebug() << "Model loaded: " << path;
    directory = path.left(path.lastIndexOf('/'));

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene)
{
    // 处理节点所有的网格（如果有的话）
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
        meshes.append(processMesh(mesh, scene));         
    }
    // 接下来对它的子节点重复这一过程
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}


Mesh* Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    QVector<Vertex> vertices;
    QVector<unsigned int> indices;
    QVector<Texture> textures;

    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        QVector3D vector;
        // 处理顶点位置、法线和纹理坐标
        vector.setX(mesh->mVertices[i].x);
        vector.setY(mesh->mVertices[i].y);
        vector.setZ(mesh->mVertices[i].z);
        vertex.Position = vector;

        vector.setX(mesh->mNormals[i].x);
        vector.setY(mesh->mNormals[i].y);
        vector.setZ(mesh->mNormals[i].z);
        vertex.Normal = vector;

        if(mesh->mTextureCoords[0]) // 网格是否有纹理坐标？
        {
            QVector2D vec;
            vec.setX(mesh->mTextureCoords[0][i].x); 
            vec.setY(mesh->mTextureCoords[0][i].y);
            vertex.TexCoords = vec;
        }
        else
            vertex.TexCoords = QVector2D(0.0f, 0.0f);

        vertices.append(vertex);
    }
    // 处理索引
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.append(face.mIndices[j]);
    }
    // 处理材质
    if(mesh->mMaterialIndex >= 0)
    {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        QVector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures += diffuseMaps;
        QVector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures += specularMaps;
    }

    return new Mesh(vertices, indices, textures);
}


QVector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, const QString &typeName)
{
    QVector<Texture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        // 检查纹理是否已被加载，如果是则跳过加载
        bool skip = false;
        for(unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if(std::strcmp(textures_loaded[j].path.C_Str(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true; 
                break;
            }
        }
        if(!skip)
        {   // 如果纹理还没有被加载，则加载它
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);  // 添加到已加载的纹理中
        }
    }
    return textures;
}

unsigned int Model::TextureFromFile(const char *path, const QString &directory)
{
    QString filename = QString(path);
    filename = directory + '/' + filename;

    Texture texture;
    texture.id = 0;
    texture.type = "texture_diffuse";
    texture.path = path;

    QImage img = QImage(filename).convertToFormat(QImage::Format_RGB888);
    if (!img.isNull())
    {
        glGenTextures(1, &texture.id);
        glBindTexture(GL_TEXTURE_2D, texture.id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width(), img.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, img.bits());
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        qDebug() << "Texture failed to load at path: " << path;
    }

    return texture.id;
}

QString Model::extractResource(const QString &resourcePath) {
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString tempFilePath = tempDir + "/" + QFileInfo(resourcePath).fileName();

    QFile file(resourcePath);
    if (file.exists()) {
        if (QFile::exists(tempFilePath)) {
            QFile::remove(tempFilePath);
        }
        file.copy(tempFilePath);
    } else {
        qDebug() << "Resource file not found:" << resourcePath;
    }

    return tempFilePath;
}