/**************************************************************************
**   The Kalimat programming language
**   Copyright 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef SPRITELAYER_H
#define SPRITELAYER_H

#include <QVector>
#include <QSet>
#include "sprite.h"

class CollisionListener
{
public:
    virtual void operator ()(Sprite *s1, Sprite *s2) = 0;
};

class SpriteLayer
{
    QVector<Sprite *> _sprites;
    QSet<Sprite *> _visibleSprites;

public:
    SpriteLayer();
    void showSprite(Sprite *s);
    void hideSprite(Sprite *s);
    void AddSprite(Sprite *);
    void checkCollision(Sprite *s, CollisionListener *callback);
    QSet<Sprite *> visibleSprites() { return _visibleSprites; }
    QVector<Sprite *> &sprites() { return _sprites; }
};

#endif // SPRITELAYER_H
