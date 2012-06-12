// Directly Copied from Qt source (4.8)
// of qtooltip.h and qtooltip.cpp
// and slightly modified to allow disabling
// autohide timer

#include "mytooltip.h"

/*!
    Shows \a text as a tool tip, with the global position \a pos as
    the point of interest. The tool tip will be shown with a platform
    specific offset from this point of interest.

    If you specify a non-empty rect the tip will be hidden as soon
    as you move your cursor out of this area.

    The \a rect is in the coordinates of the widget you specify with
    \a w. If the \a rect is not empty you must specify a widget.
    Otherwise this argument can be 0 but it is used to determine the
    appropriate screen on multi-head systems.

    If \a text is empty the tool tip is hidden. If the text is the
    same as the currently shown tooltip, the tip will \e not move.
    You can force moving by first hiding the tip with an empty text,
    and then showing the new tip at the new position.
*/

void MyToolTip::showText(const QPoint &pos, const QString &text, QWidget *w, const QRect &rect, bool autoExpire)
{
    if (MyTipLabel::instance && MyTipLabel::instance->isVisible()){ // a tip does already exist
        if (text.isEmpty()){ // empty text means hide current tip
            MyTipLabel::instance->hideTip();
            return;
        }
        else if (!MyTipLabel::instance->fadingOut){
            // If the tip has changed, reuse the one
            // that is showing (removes flickering)
            QPoint localPos = pos;
            if (w)
                localPos = w->mapFromGlobal(pos);
            if (MyTipLabel::instance->tipChanged(localPos, text, w)){
                MyTipLabel::instance->reuseTip(text, autoExpire);
                MyTipLabel::instance->setTipRect(w, rect);
                MyTipLabel::instance->placeTip(pos, w);
            }
            return;
        }
    }

    if (!text.isEmpty()){ // no tip can be reused, create new tip:
#ifndef Q_WS_WIN
        new MyTipLabel(text, w); // sets MyTipLabel::instance to itself
#else
        // On windows, we can't use the widget as parent otherwise the window will be
        // raised when the tooltip will be shown
        new MyTipLabel(text, QApplication::desktop()->screen(MyTipLabel::getTipScreen(pos, w)), autoExpire);
#endif
        MyTipLabel::instance->setTipRect(w, rect);
        MyTipLabel::instance->placeTip(pos, w);
        MyTipLabel::instance->setObjectName(QLatin1String("MyToolTip_label"));



        MyTipLabel::instance->show();
    }
}

/*!
    \overload

    This is analogous to calling MyToolTip::showText(\a pos, \a text, \a w, QRect())
*/

void MyToolTip::showText(const QPoint &pos, const QString &text, QWidget *w, bool autoExpire)
{
    MyToolTip::showText(pos, text, w, QRect(), autoExpire);
}


/*!
    \fn void MyToolTip::hideText()
    \since 4.2

    Hides the tool tip. This is the same as calling showText() with an
    empty string.

    \sa showText()
*/


/*!
  \since 4.4

  Returns true if this tooltip is currently shown.

  \sa showText()
 */
bool MyToolTip::isVisible()
{
    return (MyTipLabel::instance != 0 && MyTipLabel::instance->isVisible());
}

/*!
  \since 4.4

  Returns the tooltip text, if a tooltip is visible, or an
  empty string if a tooltip is not visible.
 */
QString MyToolTip::text()
{
    if (MyTipLabel::instance)
        return MyTipLabel::instance->text();
    return QString();
}


Q_GLOBAL_STATIC(QPalette, tooltip_palette)

/*!
    Returns the palette used to render tooltips.

    \note Tool tips use the inactive color group of QPalette, because tool
    tips are not active windows.
*/
QPalette MyToolTip::palette()
{
    return *tooltip_palette();
}

/*!
    \since 4.2

    Returns the font used to render tooltips.
*/
QFont MyToolTip::font()
{
    return QApplication::font("MyTipLabel");
}

/*!
    \since 4.2

    Sets the \a palette used to render tooltips.

    \note Tool tips use the inactive color group of QPalette, because tool
    tips are not active windows.
*/
void MyToolTip::setPalette(const QPalette &palette)
{
    *tooltip_palette() = palette;
    if (MyTipLabel::instance)
        MyTipLabel::instance->setPalette(palette);
}

/*!
    \since 4.2

    Sets the \a font used to render tooltips.
*/
void MyToolTip::setFont(const QFont &font)
{
    QApplication::setFont(font, "MyTipLabel");
}
