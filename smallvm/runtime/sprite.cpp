/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "sprite.h"

Sprite::Sprite(QString filename)
    :image(filename)
{
    visible = false;
    collides = true;
    prepareMask(image);

}

Sprite::Sprite(QPixmap img)
    :image(img)
{
    visible = false;
    collides = true;
    prepareMask(image);
}

void Sprite::setImage(QPixmap image)
{
    this->image = image;
    prepareMask(image);
}

void Sprite::prepareMask(QPixmap image)
{
    //mask = image.createHeuristicMask(true);
    mask = image.createMaskFromColor(image.toImage().pixel(0, 0), Qt::MaskInColor);
    this->image.setMask(mask);
}

QRect Sprite::boundingRect()
{
    return QRect(location, image.size());
}

void Sprite::enableColission(bool enable)
{
    collides = enable;
}
