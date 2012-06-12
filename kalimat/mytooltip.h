// Directly Copied from Qt source (4.8)
// of qtooltip.h and qtooltip.cpp
// and slightly modified to allow disabling
// autohide timer

#ifndef MYTOOLTIP_H
#define MYTOOLTIP_H

#include "mytiplabel.h"

class MyToolTip
{
    MyToolTip();
public:
    static void showText(const QPoint &pos, const QString &text, QWidget *w = 0, bool autoExpire=true);
    static void showText(const QPoint &pos, const QString &text, QWidget *w, const QRect &rect,bool autoExpire=true);
    static inline void hideText() { showText(QPoint(), QString()); }

    static bool isVisible();
    static QString text();

    static QPalette palette();
    static void setPalette(const QPalette &);
    static QFont font();
    static void setFont(const QFont &);
};


#endif // MYTOOLTIP_H
