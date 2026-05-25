#include "snapshottextureprovider.h"

SnapshotTextureProvider::SnapshotTextureProvider()
    : t(nullptr)
    , fbo(nullptr)
{
}

SnapshotTextureProvider::~SnapshotTextureProvider()
{
    delete fbo;
    delete t;
}

QSGTexture *SnapshotTextureProvider::texture() const
{
    return t;
}
