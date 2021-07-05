/**************************************************************************
**   The Kalimat programming language
**   Copyright 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "spritelayer.h"
#include <QMap>

SpriteLayer::SpriteLayer()
{
    dirtyState = false;
}

bool SpriteLayer::dirty()
{
    return dirtyState;
}

void SpriteLayer::AddSprite(Sprite *s)
{
    _sprites.append(s);
}

void SpriteLayer::showSprite(Sprite *s)
{
    _visibleSprites.insert(s);
    dirtyState = true;
}
void SpriteLayer::hideSprite(Sprite *s)
{
    _visibleSprites.remove(s);
    dirtyState = true;
}

void SpriteLayer::checkCollision(Sprite *s, CollisionListener *callback)
{
    QMap<Sprite *, Sprite *> collides;
    for(QSet<Sprite *>::iterator i = _visibleSprites.begin(); i!=_visibleSprites.end(); ++i)
    {
        Sprite *s2 = *i;
        if(!s2->collides)
            continue;
        if(s != s2)
        {
            if(s->boundingRect().intersects(s2->boundingRect()))
            {
                collides[s] = s2;
            }
        }
    }
    for(QMap<Sprite *, Sprite *>::iterator i = collides.begin(); i!= collides.end(); ++i)
    {
        Sprite *s1 = i.key();
        Sprite *s2 = i.value();

        (*callback)(s1, s2);
    }
}
