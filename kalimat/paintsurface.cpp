/**************************************************************************
**   The Kalimat programming language
**   Copyright 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "paintsurface.h"
#include <QFontDatabase>

PaintSurface::PaintSurface(QSize size, QFont font)
{
    textColor = Qt::black;
    QImage im(size, QImage::Format_RGB32);
    im.fill(qRgb(255,255,255));
    image = im;
    textFont = font;
}

void PaintSurface::paint(QPainter &painter, TextLayer &textLayer, SpriteLayer &spriteLayer)
{
    QImage finalImg(image.size(), image.format());
    QPainter imgPainter(&finalImg);

    imgPainter.drawImage(QPoint(0,0), image);
    drawTextLayer(imgPainter, textLayer);
    drawSpriteLayer(imgPainter, spriteLayer);

    painter.drawImage(QPoint(0, 0), finalImg);
}

void PaintSurface::drawTextLayer(QPainter &imgPainter, TextLayer &textLayer)
{
    imgPainter.setLayoutDirection(Qt::RightToLeft);

    QTextOption options;
    options.setAlignment(Qt::AlignLeft);
    options.setTextDirection(Qt::RightToLeft);

    textFont.setPixelSize(image.height()/25);
    //textFont.setRawName("Simplified Arabic Fixed");
    imgPainter.setFont(textFont);

    QPen oldPen = imgPainter.pen();
    imgPainter.setPen(textColor);
    for(int i=0; i<textLayer.lines().count(); i++)
    {
        QRect rct(0, i* textFont.pixelSize(), image.width()-1, image.height());
        imgPainter.drawText(rct, textLayer.lines()[i], options);
    }
    imgPainter.setPen(oldPen);
    if(textLayer.inputState())
    {
        QRect cur = textCursor(textLayer);
        imgPainter.drawLine(cur.topRight(), cur.bottomRight());
    }
}

void PaintSurface::drawSpriteLayer(QPainter &imgPainter, SpriteLayer &spriteLayer)
{
    QPainter::CompositionMode oldMode = imgPainter.compositionMode();
    for(int i=0; i < spriteLayer.sprites().count(); ++i)
    {
        Sprite *s = spriteLayer.sprites()[i];
        if(s->visible)
        {
            imgPainter.setCompositionMode(QPainter::RasterOp_SourceAndDestination);
            imgPainter.drawPixmap(s->location, s->mask);
            imgPainter.setCompositionMode(QPainter::RasterOp_SourceOrDestination);
            imgPainter.drawPixmap(s->location, s->image);
        }
    }
    imgPainter.setCompositionMode(oldMode);
}

QImage *PaintSurface::GetImage()
{
    return &this->image;
}

QColor PaintSurface::GetColor(int color)
{
    switch(color)
    {
    case 0:
        return QColor::fromRgb(0,0,0);
    case 1:
        return QColor::fromRgb(255,145,72);
    case 2:
        return QColor::fromRgb(36,218,36);
    case 3:
        return QColor::fromRgb(109,255,109);
    case 4:
        return QColor::fromRgb(36,35,255);
    case 5:
        return QColor::fromRgb(72,109,255);
    case 6:
        return QColor::fromRgb(182,36,36);
    case 7:
        return QColor::fromRgb(72,182,255);
    case 8:
        return QColor::fromRgb(255,36,36);
    case 9:
        return QColor::fromRgb(255,109,109);
    case 10:
        return QColor::fromRgb(218,218,36);
    case 11:
        return QColor::fromRgb(218,218,145);
    case 12:
        return QColor::fromRgb(36,145,36);
    case 13:
        return QColor::fromRgb(218,72,182);
    case 14:
        return QColor::fromRgb(182,182,182);
    case 15:
        return QColor::fromRgb(255,255,255);
    default:
        return Qt::white;
    }
}

void PaintSurface::zoom(int x1, int y1, int x2, int y2)
{

}

void PaintSurface::clearImage()
{
    QPainter p(GetImage());
    p.fillRect(0,0,GetImage()->width(), GetImage()->height(), Qt::white);

}

void PaintSurface::TX(int &x)
{
    x = (this->image.width()-1)-x;
}

void PaintSurface::setTextColor(QColor color)
{
    textColor = color;
}


QRect PaintSurface::textCursor(TextLayer &textLayer)
{
    QPainter p(GetImage());

    p.setFont(textFont);
    QString line = textLayer.currentLine().left(textLayer.cursorColumn());
    int w = p.fontMetrics().tightBoundingRect(line).width();
    int h = textFont.pixelSize();

    int x = image.width() - w;
    int y = textLayer.cursorLine() * h;

    return QRect(x-2, y, 2, h);
}

void PaintSurface::resize(int width, int height)
{
    if (width > image.width() || height > image.height())
    {
        int newWidth = qMax(width + 128, image.width());
        int newHeight = qMax(height + 128, image.height());
        resizeImage(&image, QSize(newWidth, newHeight));
    }
}

void PaintSurface::resizeImage(QImage *image, const QSize &newSize)
{
     if (image->size() == newSize)
         return;
     QImage newImage(newSize, QImage::Format_RGB32);
     newImage.fill(qRgb(255, 255, 255));
     QPainter painter(&newImage);
     painter.drawImage(QPoint(0, 0), *image);
     *image = newImage;
}
