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
    mask = image.createMaskFromColor(image.toImage().pixel(0,0));
    image.setMask(mask);

}
QRect Sprite::boundingRect()
{
    return QRect(location, image.size());
}
