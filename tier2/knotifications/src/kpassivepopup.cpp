/*
 *   Copyright (C) 2001-2006 by Richard Moore <rich@kde.org>
 *   Copyright (C) 2004-2005 by Sascha Cunz <sascha.cunz@tiscali.de>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public License
 *   along with this library; see the file COPYING.LIB.  If not, write to
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 */

#include "kpassivepopup.h"

#include <config-knotifications.h>

// Qt
#include <QApplication>
#include <QBitmap>
#include <QDebug>
#include <QDesktopWidget>
#include <QLabel>
#include <QLayout>
#include <QBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QDesktopWidget>
#include <QPolygonF>
#include <QStyle>
#include <QTimer>
#include <QToolTip>
#include <QSystemTrayIcon>

#if HAVE_X11
#include <QX11Info>
#include <netwm.h>
#endif

static const int DEFAULT_POPUP_TYPE = KPassivePopup::Boxed;
static const int DEFAULT_POPUP_TIME = 6*1000;
static const Qt::WindowFlags POPUP_FLAGS = Qt::Tool | Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint;

class KPassivePopup::Private
{
public:
    Private(KPassivePopup *q, WId winId)
    : q(q),
      popupStyle(DEFAULT_POPUP_TYPE),
      window(winId),
      msgView(0),
      topLayout(0),
      hideDelay(DEFAULT_POPUP_TIME),
      hideTimer(new QTimer(q)),
      autoDelete(false)
    {
        q->setWindowFlags(POPUP_FLAGS);
        q->setFrameStyle(QFrame::Box| QFrame::Plain);
        q->setLineWidth(2);

        if (popupStyle == Boxed) {
            q->setFrameStyle(QFrame::Box| QFrame::Plain);
            q->setLineWidth(2);
        } else if (popupStyle == Balloon) {
            q->setPalette(QToolTip::palette());
        }
        connect(hideTimer, SIGNAL(timeout()), q, SLOT(hide()));
        connect(q, SIGNAL(clicked()), q, SLOT(hide()));
    }

    KPassivePopup *q;

    int popupStyle;
    QPolygon surround;
    QPoint anchor;
    QPoint fixedPosition;

    WId window;
    QWidget *msgView;
    QBoxLayout *topLayout;
    int hideDelay;
    QTimer *hideTimer;

    QLabel *ttlIcon;
    QLabel *ttl;
    QLabel *msg;

    bool autoDelete;

    /**
     * Updates the transparency mask. Unused if PopupStyle == Boxed
     */
    void updateMask()
    {
        // get screen-geometry for screen our anchor is on
        // (geometry can differ from screen to screen!
        QRect deskRect = QApplication::desktop()->screenGeometry(anchor);

        const int width = q->width();
        const int height = q->height();

        int xh = 70, xl = 40;
        if (width < 80) {
            xh = xl = 40;
        } else if (width < 110) {
            xh = width - 40;
        }

        bool bottom = (anchor.y() + height) > ((deskRect.y() + deskRect.height()-48));
        bool right = (anchor.x() + width) > ((deskRect.x() + deskRect.width()-48));

        QPoint corners[4] = {
            QPoint(width - 50, 10),
            QPoint(10, 10),
            QPoint(10, height - 50),
            QPoint(width - 50, height - 50)
        };

        QBitmap mask(width, height);
        mask.clear();
        QPainter p(&mask);
        QBrush brush(Qt::color1, Qt::SolidPattern);
        p.setBrush(brush);

        int i = 0, z = 0;
        for (; i < 4; ++i) {
            QPainterPath path;
            path.moveTo(corners[i].x(), corners[i].y());
            path.arcTo(corners[i].x(), corners[i].y(), 40, 40, i * 90, 90);
            QPolygon corner = path.toFillPolygon().toPolygon();

            surround.resize(z + corner.count() - 1);
            for (int s = 1; s < corner.count() - 1; s++, z++) {
                surround.setPoint(z, corner[s]);
            }

            if (bottom && i == 2) {
                if (right) {
                    surround.resize(z + 3);
                    surround.setPoint(z++, QPoint(width - xh, height - 10));
                    surround.setPoint(z++, QPoint(width - 20, height));
                    surround.setPoint(z++, QPoint(width - xl, height - 10));
                } else {
                    surround.resize(z + 3);
                    surround.setPoint(z++, QPoint(xl, height - 10));
                    surround.setPoint(z++, QPoint(20, height));
                    surround.setPoint(z++, QPoint(xh, height - 10));
                }
            } else if (!bottom && i == 0) {
                if (right) {
                    surround.resize(z + 3);
                    surround.setPoint(z++, QPoint(width - xl, 10));
                    surround.setPoint(z++, QPoint(width - 20, 0));
                    surround.setPoint(z++, QPoint(width - xh, 10));
                } else {
                    surround.resize(z + 3);
                    surround.setPoint(z++, QPoint(xh, 10));
                    surround.setPoint(z++, QPoint(20, 0));
                    surround.setPoint(z++, QPoint(xl, 10));
                }
            }
        }

        surround.resize(z + 1);
        surround.setPoint(z, surround[0]);
        p.drawPolygon(surround);
        q->setMask(mask);

        q->move(right ? anchor.x() - width + 20 : (anchor.x() < 11 ? 11 : anchor.x() - 20),
                bottom ? anchor.y() - height : (anchor.y() < 11 ? 11 : anchor.y()));

        q->update();
    }

    /**
     * Calculates the position to place the popup near the specified rectangle.
     */
    QPoint calculateNearbyPoint(const QRect &target) {
        QPoint pos = target.topLeft();
        int x = pos.x();
        int y = pos.y();
        int w = q->minimumSizeHint().width();
        int h = q->minimumSizeHint().height();

        QRect r = QApplication::desktop()->screenGeometry(QPoint(x+w/2,y+h/2));

        if (popupStyle == Balloon) {
            // find a point to anchor to
            if (x + w > r.width()) {
                x = x + target.width();
            }

            if (y + h > r.height()) {
                y = y + target.height();
            }
        } else {
            if (x < r.center().x()) {
                x = x + target.width();
            } else {
                x = x - w;
            }

            // It's apparently trying to go off screen, so display it ALL at the bottom.
            if ((y + h) > r.bottom()) {
                y = r.bottom() - h;
            }

            if ((x + w) > r.right()) {
                x = r.right() - w;
            }
        }
        if (y < r.top()) {
            y = r.top();
        }

        if (x < r.left()) {
            x = r.left();
        }

        return QPoint(x, y);
    }
};

KPassivePopup::KPassivePopup(QWidget *parent, Qt::WindowFlags f)
    : QFrame(0, f ? f : POPUP_FLAGS),
      d(new Private(this, parent ? parent->effectiveWinId() : 0L))
{
}

KPassivePopup::KPassivePopup(WId win)
    : QFrame(0),
      d(new Private(this, win))
{
}

KPassivePopup::~KPassivePopup()
{
    delete d;
}

void KPassivePopup::setPopupStyle( int popupstyle )
{
    if ( d->popupStyle == popupstyle )
	return;

    d->popupStyle = popupstyle;
    if( d->popupStyle == Boxed )
    {
        setFrameStyle( QFrame::Box| QFrame::Plain );
        setLineWidth( 2 );
    }
    else if( d->popupStyle == Balloon )
    {
        setPalette(QToolTip::palette());
    }
}

void KPassivePopup::setView( QWidget *child )
{
    delete d->msgView;
    d->msgView = child;

    delete d->topLayout;
    d->topLayout = new QVBoxLayout( this );
    if ( d->popupStyle == Balloon ) {
        const int marginHint = style()->pixelMetric(QStyle::PM_DefaultChildMargin);
        d->topLayout->setMargin( 2 * marginHint );
    }
    d->topLayout->addWidget( d->msgView );
    d->topLayout->activate();
}

void KPassivePopup::setView( const QString &caption, const QString &text,
                             const QPixmap &icon )
{
    // qDebug() << "KPassivePopup::setView " << caption << ", " << text;
    setView( standardView( caption, text, icon, this ) );
}


QWidget * KPassivePopup::standardView( const QString& caption,
                                     const QString& text,
                                     const QPixmap& icon,
                                     QWidget *parent )
{
    QWidget *top = new QWidget(parent ? parent : this);
    QVBoxLayout *vb = new QVBoxLayout(top);
    vb->setMargin(0);
    top->setLayout(vb);

    QHBoxLayout *hb = 0;
    if ( !icon.isNull() ) {
	hb = new QHBoxLayout( top );
	hb->setMargin( 0 );
        vb->addLayout( hb );
	d->ttlIcon = new QLabel( top );
	d->ttlIcon->setPixmap( icon );
        d->ttlIcon->setAlignment( Qt::AlignLeft );
        hb->addWidget( d->ttlIcon );
    }

    if ( !caption.isEmpty() ) {
	d->ttl = new QLabel( caption, top );
	QFont fnt = d->ttl->font();
	fnt.setBold( true );
	d->ttl->setFont( fnt );
	d->ttl->setAlignment( Qt::AlignHCenter );

        if ( hb ) {
            hb->addWidget(d->ttl);
            hb->setStretchFactor( d->ttl, 10 ); // enforce centering
        } else {
            vb->addWidget(d->ttl);
        }
    }

    if ( !text.isEmpty() ) {
        d->msg = new QLabel( text, top );
        d->msg->setAlignment( Qt::AlignLeft );
        d->msg->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        d->msg->setOpenExternalLinks(true);
        vb->addWidget(d->msg);
    }

    return top;
}

void KPassivePopup::setView( const QString &caption, const QString &text )
{
    setView( caption, text, QPixmap() );
}

QWidget *KPassivePopup::view() const
{
    return d->msgView;
}

int KPassivePopup::timeout() const
{
    return d->hideDelay;
}

void KPassivePopup::setTimeout( int delay )
{
    d->hideDelay = delay;
    if( d->hideTimer->isActive() )
    {
        if( delay ) {
            d->hideTimer->start( delay );
        } else {
            d->hideTimer->stop();
        }
    }
}

bool KPassivePopup::autoDelete() const
{
    return d->autoDelete;
}

void KPassivePopup::setAutoDelete( bool autoDelete )
{
    d->autoDelete = autoDelete;
}

void KPassivePopup::mouseReleaseEvent( QMouseEvent *e )
{
    emit clicked();
    emit clicked( e->pos() );
}

//
// Main Implementation
//

void KPassivePopup::setVisible( bool visible )
{
    if (! visible ) {
        QFrame::setVisible( visible );
        return;
    }

    if ( size() != sizeHint() )
        resize( sizeHint() );

    if ( d->fixedPosition.isNull() )
        positionSelf();
    else {
        if( d->popupStyle == Balloon )
            setAnchor( d->fixedPosition );
        else
            move( d->fixedPosition );
    }
    QFrame::setVisible( /*visible=*/ true );

    int delay = d->hideDelay;
    if ( delay < 0 ) {
        delay = DEFAULT_POPUP_TIME;
    }

    if ( delay > 0 ) {
        d->hideTimer->start( delay );
    }
}

void KPassivePopup::show(const QPoint &p)
{
    d->fixedPosition = p;
    show();
}

void KPassivePopup::hideEvent( QHideEvent * )
{
    d->hideTimer->stop();
    if ( d->autoDelete )
        deleteLater();
}

QPoint KPassivePopup::defaultLocation() const
{
    const QRect r = QApplication::desktop()->availableGeometry();
    return QPoint(r.left(), r.top());
}

void KPassivePopup::positionSelf()
{
    QRect target;

    if ( !d->window ) {
        target = QRect(defaultLocation(), QSize(0, 0));
#if HAVE_X11
    } else if ( QX11Info::isPlatformX11() ) {
        NETWinInfo ni( QX11Info::display(), d->window, QX11Info::appRootWindow(),
                       NET::WMIconGeometry | NET::WMState );

        // Try to put the popup by the taskbar entry
        if ( !(ni.state() & NET::SkipTaskbar) ) {
            NETRect r = ni.iconGeometry();
            target.setRect( r.pos.x, r.pos.y, r.size.width, r.size.height );
        }
        // If that failed, put it by the window itself
        if ( target.isNull() ) {
            NETRect dummy;
            NETRect r;
            ni.kdeGeometry( dummy, r );
            target.setRect( r.pos.x, r.pos.y,
                            r.size.width, r.size.height);
        }
#else
    } else {
        // Our only choice is to put it by the window

        // Easy case: we were passed one of our own widgets
        QWidget *widget = QWidget::find( d->winId );
        if ( widget ) {
            target = widget->geometry();
            if ( target.isNull() ) {
                const QRect d = QApplication::desktop()->availableGeometry( widget );
                target.setRect( d.x(), d.y(), 0, 0 );
            }
        } else {
            // NB: this will not work on X11 (hence the fallback in the NET
            //     code above) as the XCB platform plugin does not collect
            //     information about foreign windows
            QWindow *window = QWindow::fromWinId( d->winId );
            target = window->geometry();
            if ( target.isNull() ) {
                const QRect d = window->screen()->availableGeometry();
                target.setRect( d.x(), d.y(), 0, 0 );
            }
        }
#endif
    }
    moveNear( target );
}

void KPassivePopup::moveNear( const QRect &target )
{
    QPoint pos = d->calculateNearbyPoint(target);
    if( d->popupStyle == Balloon )
        setAnchor( pos );
    else
        move( pos.x(), pos.y() );
}


QPoint KPassivePopup::anchor() const
{
    return d->anchor;
}

void KPassivePopup::setAnchor(const QPoint &anchor)
{
    d->anchor = anchor;
    d->updateMask();
}

void KPassivePopup::paintEvent( QPaintEvent* pe )
{
    if( d->popupStyle == Balloon )
    {
        QPainter p;
        p.begin( this );
        p.drawPolygon( d->surround );
    } else
        QFrame::paintEvent( pe );
}


//
// Convenience Methods
//

KPassivePopup *KPassivePopup::message( const QString &caption, const QString &text,
                        const QPixmap &icon, QWidget *parent, int timeout, const QPoint &p )
{
    return message( DEFAULT_POPUP_TYPE, caption, text, icon, parent, timeout, p );
}

KPassivePopup *KPassivePopup::message( const QString &text, QWidget *parent, const QPoint &p )
{
    return message( DEFAULT_POPUP_TYPE, QString(), text, QPixmap(), parent, -1, p );
}

KPassivePopup *KPassivePopup::message( const QString &caption, const QString &text,
				       QWidget *parent, const QPoint &p )
{
    return message( DEFAULT_POPUP_TYPE, caption, text, QPixmap(), parent, -1, p );
}

KPassivePopup *KPassivePopup::message( const QString &caption, const QString &text,
				       const QPixmap &icon, WId parent, int timeout, const QPoint &p )
{
    return message( DEFAULT_POPUP_TYPE, caption, text, icon, parent, timeout, p );
}

KPassivePopup *KPassivePopup::message( const QString &caption, const QString &text,
                       const QPixmap &icon, QSystemTrayIcon *parent, int timeout )
{
    return message( DEFAULT_POPUP_TYPE, caption, text, icon, parent, timeout );
}

KPassivePopup *KPassivePopup::message( const QString &text, QSystemTrayIcon *parent )
{
    return message( DEFAULT_POPUP_TYPE, QString(), text, QPixmap(), parent);
}

KPassivePopup *KPassivePopup::message( const QString &caption, const QString &text,
                       QSystemTrayIcon *parent )
{
    return message( DEFAULT_POPUP_TYPE, caption, text, QPixmap(), parent );
}


KPassivePopup *KPassivePopup::message( int popupStyle, const QString &caption, const QString &text,
                        const QPixmap &icon, QWidget *parent, int timeout, const QPoint &p )
{
    KPassivePopup *pop = new KPassivePopup( parent );
    pop->setPopupStyle( popupStyle );
    pop->setAutoDelete( true );
    pop->setView( caption, text, icon );
    pop->d->hideDelay = timeout;
    if(p.isNull())
        pop->show();
    else
        pop->show(p);

    return pop;
}

KPassivePopup *KPassivePopup::message( int popupStyle, const QString &text, QWidget *parent, const QPoint& p )
{
    return message( popupStyle, QString(), text, QPixmap(), parent, -1, p );
}

KPassivePopup *KPassivePopup::message( int popupStyle, const QString &caption, const QString &text,
                            QWidget *parent, const QPoint& p )
{
    return message( popupStyle, caption, text, QPixmap(), parent, -1, p );
}

KPassivePopup *KPassivePopup::message( int popupStyle, const QString &caption, const QString &text,
                                       const QPixmap &icon, WId parent, int timeout, const QPoint &p )
{
    KPassivePopup *pop = new KPassivePopup( parent );
    pop->setPopupStyle( popupStyle );
    pop->setAutoDelete( true );
    pop->setView( caption, text, icon );
    pop->d->hideDelay = timeout;
    if(p.isNull())
        pop->show();
    else
        pop->show(p);

    return pop;
}

KPassivePopup *KPassivePopup::message( int popupStyle, const QString &caption, const QString &text,
                       const QPixmap &icon, QSystemTrayIcon *parent, int timeout )
{
    KPassivePopup *pop = new KPassivePopup( );
    pop->setPopupStyle( popupStyle );
    pop->setAutoDelete( true );
    pop->setView( caption, text, icon );
    pop->d->hideDelay = timeout;
    QPoint pos = pop->d->calculateNearbyPoint(parent->geometry());
    pop->show(pos);
    pop->moveNear(parent->geometry());

    return pop;
}

KPassivePopup *KPassivePopup::message( int popupStyle, const QString &text, QSystemTrayIcon *parent )
{
    return message( popupStyle, QString(), text, QPixmap(), parent );
}

KPassivePopup *KPassivePopup::message( int popupStyle, const QString &caption, const QString &text,
                       QSystemTrayIcon *parent )
{
    return message( popupStyle, caption, text, QPixmap(), parent );
}


// Local Variables:
// c-basic-offset: 4
// End:

