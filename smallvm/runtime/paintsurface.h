/**************************************************************************
**   The Kalimat programming language
**   Copyright 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef PAINTSURFACE_H
#define PAINTSURFACE_H

#include <QPainter>

#ifndef TEXTLAYER_H
    #include "textlayer.h"
#endif

#ifndef SPRITELAYER_H
    #include "spritelayer.h"
#endif

class PaintSurface
{
    QImage image;
    QImage finalImg;
    QFont textFont;
    QColor textColor;
public:
    // We have a specia 'demo coordinates' mode in Kalimat
    // to teach children about the x/y coordinate system
    // by drawing the axes and showing mouse position if this
    // mode was activated

    bool showCoordinates;
    QPoint mouseLocationForDemo;

public:
    PaintSurface(QSize size, QFont font);
    void paint(QPainter &painter, TextLayer &textLayer, SpriteLayer &spriteLayer);
    void TX(int &);

    void zoom(int x1, int y1, int x2, int y2);
    void clearImage();
    QImage *GetImage();
    QColor GetColor(int color);
    void setTextColor(QColor);
    void resize(int width, int height);
    void resizeImage(QImage *image, const QSize &newSize);

    QRect textCursor(TextLayer &layer);
    void drawTextLayer(QPainter &imgPainter, TextLayer &layer);
    void drawSpriteLayer(QPainter &imgPainter, SpriteLayer &layer);
};

#endif // PAINTSURFACE_H
