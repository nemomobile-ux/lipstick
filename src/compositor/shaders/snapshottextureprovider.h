#ifndef SNAPSHOTTEXTUREPROVIDER_H
#define SNAPSHOTTEXTUREPROVIDER_H

#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QSGTextureProvider>


struct SnapshotProgram
{
    QOpenGLShaderProgram program;
    int vertexLocation;
    int textureLocation;
};

class SnapshotTextureProvider : public QSGTextureProvider
{
    Q_OBJECT
public:
    SnapshotTextureProvider();
    ~SnapshotTextureProvider();
    QSGTexture *texture() const Q_DECL_OVERRIDE;
    QSGTexture *t;
    QOpenGLFramebufferObject *fbo;
};

#endif // SNAPSHOTTEXTUREPROVIDER_H
