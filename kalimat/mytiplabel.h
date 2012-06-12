// Directly Copied from Qt source (4.8)
// of qtooltip.h and qtooltip.cpp
// and slightly modified to allow disabling
// autohide timer

#ifndef MYTIPLABEL_H
#define MYTIPLABEL_H


#include <QPoint>
#include <QWidget>
#include <QString>
#include <QPalette>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qevent.h>
#include <qhash.h>
#include <qlabel.h>
#include <qpointer.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qstylepainter.h>
#include <qtimer.h>
//#include <private/qeffects_p.h>
#include <qtextdocument.h>
#include <qdebug.h>
//#include <private/qstylesheetstyle_p.h>


class MyTipLabel : public QLabel
{
    Q_OBJECT
public:
    MyTipLabel(const QString &text, QWidget *w,  bool autoExpire);
    ~MyTipLabel();
    static MyTipLabel *instance;

    bool eventFilter(QObject *, QEvent *);

    QBasicTimer hideTimer, expireTimer;

    bool fadingOut;

    void reuseTip(const QString &text, bool autoExpire);
    void hideTip();
    void hideTipImmediately();
    void setTipRect(QWidget *w, const QRect &r);
    void restartExpireTimer();
    bool tipChanged(const QPoint &pos, const QString &text, QObject *o);
    void placeTip(const QPoint &pos, QWidget *w);

    static int getTipScreen(const QPoint &pos, QWidget *w);
protected:
    void timerEvent(QTimerEvent *e);
    void paintEvent(QPaintEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *e);

#ifndef QT_NO_STYLE_STYLESHEET
public slots:
    /** \internal
      Cleanup the _q_stylesheet_parent propery.
     */
    void styleSheetParentDestroyed() {
        setProperty("_q_stylesheet_parent", QVariant());
        styleSheetParent = 0;
    }

private:
    QWidget *styleSheetParent;
#endif

private:
    QWidget *widget;
    QRect rect;
};

#endif // MYTIPLABEL_H
