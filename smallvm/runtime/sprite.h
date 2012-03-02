/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef SPRITE_H
#define SPRITE_H
#include <QImage>
#include <QString>
#include <QRect>
#include <QPixmap>
#include <QBitmap>
class Sprite
{
public:

    QPixmap image;
    QPixmap mask;
    QPoint location;
    bool visible;
public:
    Sprite(QString filename);
    Sprite(QPixmap img);
    void setImage(QPixmap image);
    QRect boundingRect();
private:
    void prepareMask(QPixmap image);
};

#endif // SPRITE_H
