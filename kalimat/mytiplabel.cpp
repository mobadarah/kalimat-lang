// Directly Copied from Qt source (4.8)
// of qtooltip.h and qtooltip.cpp
// and slightly modified to allow disabling
// autohide timer

#include "mytiplabel.h"
#include "mytooltip.h"
MyTipLabel *MyTipLabel::instance = 0;

MyTipLabel::MyTipLabel(const QString &text, QWidget *w, bool autoExpire)
#ifndef QT_NO_STYLE_STYLESHEET
    : QLabel(w, Qt::ToolTip | Qt::BypassGraphicsProxyWidget), styleSheetParent(0), widget(0)
#else
    : QLabel(w, Qt::ToolTip | Qt::BypassGraphicsProxyWidget), widget(0)
#endif
{
    delete instance;
    instance = this;
    setForegroundRole(QPalette::ToolTipText);
    setBackgroundRole(QPalette::ToolTipBase);
    setPalette(MyToolTip::palette());
    ensurePolished();
    setMargin(1 + style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth, 0, this));
    setFrameStyle(QFrame::NoFrame);
    setAlignment(Qt::AlignLeft);
    setIndent(1);
    qApp->installEventFilter(this);
    setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, 0, this) / qreal(255.0));
    setMouseTracking(true);
    fadingOut = false;
    reuseTip(text, autoExpire);
}

void MyTipLabel::restartExpireTimer()
{
    int time = 10000 + 40 * qMax(0, text().length()-100);
    expireTimer.start(time, this);
    hideTimer.stop();
}

void MyTipLabel::reuseTip(const QString &text, bool autoExpire)
{
#ifndef QT_NO_STYLE_STYLESHEET
    if (styleSheetParent){
        disconnect(styleSheetParent, SIGNAL(destroyed()),
                   MyTipLabel::instance, SLOT(styleSheetParentDestroyed()));
        styleSheetParent = 0;
    }
#endif

    setWordWrap(Qt::mightBeRichText(text));
    setText(text);
    QFontMetrics fm(font());
    QSize extra(1, 0);
    // Make it look good with the default ToolTip font on Mac, which has a small descent.
    if (fm.descent() == 2 && fm.ascent() >= 11)
        ++extra.rheight();
    resize(sizeHint() + extra);
    if(autoExpire)
        restartExpireTimer();
}

void MyTipLabel::paintEvent(QPaintEvent *ev)
{
    QStylePainter p(this);
    QStyleOptionFrame opt;
    opt.init(this);
    p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
    p.end();

    QLabel::paintEvent(ev);
}

void MyTipLabel::resizeEvent(QResizeEvent *e)
{
    QStyleHintReturnMask frameMask;
    QStyleOption option;
    option.init(this);
    if (style()->styleHint(QStyle::SH_ToolTip_Mask, &option, this, &frameMask))
        setMask(frameMask.region);

    QLabel::resizeEvent(e);
}

void MyTipLabel::mouseMoveEvent(QMouseEvent *e)
{
    if (rect.isNull())
        return;
    QPoint pos = e->globalPos();
    if (widget)
        pos = widget->mapFromGlobal(pos);
    if (!rect.contains(pos))
        hideTip();
    QLabel::mouseMoveEvent(e);
}

MyTipLabel::~MyTipLabel()
{
    instance = 0;
}

void MyTipLabel::hideTip()
{
    if (!hideTimer.isActive())
        hideTimer.start(300, this);
}

void MyTipLabel::hideTipImmediately()
{
    close(); // to trigger QEvent::Close which stops the animation
    deleteLater();
}

void MyTipLabel::setTipRect(QWidget *w, const QRect &r)
{
    if (!rect.isNull() && !w)
        qWarning("MyToolTip::setTipRect: Cannot pass null widget if rect is set");
    else{
        widget = w;
        rect = r;
    }
}

void MyTipLabel::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == hideTimer.timerId()
        || e->timerId() == expireTimer.timerId()){
        hideTimer.stop();
        expireTimer.stop();
#if defined(Q_WS_MAC) && !defined(QT_NO_EFFECTS)
        if (QApplication::isEffectEnabled(Qt::UI_FadeTooltip)){
            // Fade out tip on mac (makes it invisible).
            // The tip will not be deleted until a new tip is shown.

                        // DRSWAT - Cocoa
                        macWindowFade(qt_mac_window_for(this));
            MyTipLabel::instance->fadingOut = true; // will never be false again.
        }
        else
            hideTipImmediately();
#else
        hideTipImmediately();
#endif
    }
}

bool MyTipLabel::eventFilter(QObject *o, QEvent *e)
{
    switch (e->type()) {
#ifdef Q_WS_MAC
    case QEvent::KeyPress:
    case QEvent::KeyRelease: {
        int key = static_cast<QKeyEvent *>(e)->key();
        Qt::KeyboardModifiers mody = static_cast<QKeyEvent *>(e)->modifiers();
        if (!(mody & Qt::KeyboardModifierMask)
            && key != Qt::Key_Shift && key != Qt::Key_Control
            && key != Qt::Key_Alt && key != Qt::Key_Meta)
            hideTip();
        break;
    }
#endif
    case QEvent::Leave:
        hideTip();
        break;
    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::FocusIn:
    case QEvent::FocusOut:
    case QEvent::Wheel:
        hideTipImmediately();
        break;

    case QEvent::MouseMove:
        if (o == widget && !rect.isNull() && !rect.contains(static_cast<QMouseEvent*>(e)->pos()))
            hideTip();
    default:
        break;
    }
    return false;
}

int MyTipLabel::getTipScreen(const QPoint &pos, QWidget *w)
{
    if (QApplication::desktop()->isVirtualDesktop())
        return QApplication::desktop()->screenNumber(pos);
    else
        return QApplication::desktop()->screenNumber(w);
}

void MyTipLabel::placeTip(const QPoint &pos, QWidget *w)
{


    QRect screen = QApplication::desktop()->screenGeometry(getTipScreen(pos, w));

    QPoint p = pos;
    p += QPoint(2,
#ifdef Q_WS_WIN
                21
#else
                16
#endif
        );
    if (p.x() + this->width() > screen.x() + screen.width())
        p.rx() -= 4 + this->width();
    if (p.y() + this->height() > screen.y() + screen.height())
        p.ry() -= 24 + this->height();
    if (p.y() < screen.y())
        p.setY(screen.y());
    if (p.x() + this->width() > screen.x() + screen.width())
        p.setX(screen.x() + screen.width() - this->width());
    if (p.x() < screen.x())
        p.setX(screen.x());
    if (p.y() + this->height() > screen.y() + screen.height())
        p.setY(screen.y() + screen.height() - this->height());
    this->move(p);
}

bool MyTipLabel::tipChanged(const QPoint &pos, const QString &text, QObject *o)
{
    if (MyTipLabel::instance->text() != text)
        return true;

    if (o != widget)
        return true;

    if (!rect.isNull())
        return !rect.contains(pos);
    else
       return false;
}
