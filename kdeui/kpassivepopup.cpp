/*
 *   copyright            : (C) 2001-2002 by Richard Moore
 *   copyright            : (C) 2004-2005 by Sascha Cunz
 *   License              : This file is released under the terms of the LGPL, version 2.
 *   email                : rich@kde.org
 *   email                : sascha.cunz@tiscali.de
 */

#include <kconfig.h>

#include <qapplication.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtimer.h>

#include <qx11info_x11.h>
#include <QMouseEvent>
#include <qpainter.h>
#include <qtooltip.h>
#include <qbitmap.h>

#include <kdebug.h>
#include <kdialog.h>
#include <kpixmap.h>
#include <kpixmapeffect.h>
#include <kglobalsettings.h>
#include <Q3PointArray>


#include "config.h"
#ifdef Q_WS_X11
#include <netwm.h>
#include <kvbox.h>
#endif

#include "kpassivepopup.h"
#include "kpassivepopup.moc"

class KPassivePopup::Private
{
public:
  int popupStyle;
  Q3PointArray surround;
  QPoint                    anchor;
  QPoint                    fixedPosition;
};

static const int DEFAULT_POPUP_TYPE = KPassivePopup::Boxed;
static const int DEFAULT_POPUP_TIME = 6*1000;
static const Qt::WindowFlags POPUP_FLAGS = Qt::Tool | Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint;

KPassivePopup::KPassivePopup( QWidget *parent, const char *name, Qt::WFlags f )
    : Q3Frame( 0, name, f ? f : POPUP_FLAGS ),
      window( parent ? parent->winId() : 0L ), msgView( 0 ), topLayout( 0 ),
      hideDelay( DEFAULT_POPUP_TIME ), hideTimer( new QTimer( this, "hide_timer" ) ),
      m_autoDelete( false )
{
    init( DEFAULT_POPUP_TYPE );
}

KPassivePopup::KPassivePopup( WId win, const char *name )
    : Q3Frame( 0, name ),
      window( win ), msgView( 0 ), topLayout( 0 ),
      hideDelay( DEFAULT_POPUP_TIME ), hideTimer( new QTimer( this, "hide_timer" ) ),
      m_autoDelete( false )
{
    init( DEFAULT_POPUP_TYPE );
}

KPassivePopup::KPassivePopup( int popupStyle, QWidget *parent, const char *name, Qt::WFlags f )
    : Q3Frame( 0, name, f ? f : POPUP_FLAGS ),
      window( parent ? parent->winId() : 0L ), msgView( 0 ), topLayout( 0 ),
      hideDelay( DEFAULT_POPUP_TIME ), hideTimer( new QTimer( this, "hide_timer" ) ),
      m_autoDelete( false )
{
    init( popupStyle );
}

KPassivePopup::KPassivePopup( int popupStyle, WId win, const char *name, Qt::WFlags f )
    : Q3Frame( 0, name, f ? f : POPUP_FLAGS ),
      window( win ), msgView( 0 ), topLayout( 0 ),
      hideDelay( DEFAULT_POPUP_TIME ), hideTimer( new QTimer( this, "hide_timer" ) ),
      m_autoDelete( false )
{
    init( popupStyle );
}

void KPassivePopup::init( int popupStyle )
{
    setWindowFlags( POPUP_FLAGS );
    setFrameStyle( Q3Frame::Box| Q3Frame::Plain );
    setLineWidth( 2 );

    d = new Private;
    d->popupStyle = popupStyle;
    if( popupStyle == Boxed )
    {
        setFrameStyle( Q3Frame::Box| Q3Frame::Plain );
        setLineWidth( 2 );
    }
    else if( popupStyle == Balloon )
    {
        setPalette(QToolTip::palette());
        //XXX dead ? setAutoMask(true);
    }
    connect( hideTimer, SIGNAL( timeout() ), SLOT( hide() ) );
    connect( this, SIGNAL( clicked() ), SLOT( hide() ) );
}

KPassivePopup::~KPassivePopup()
{
    delete d;
}

void KPassivePopup::setView( QWidget *child )
{
    delete msgView;
    msgView = child;

    delete topLayout;
    topLayout = new QVBoxLayout( this, d->popupStyle == Balloon ? 22 : KDialog::marginHint(), KDialog::spacingHint() );
    topLayout->addWidget( msgView );
    topLayout->activate();
}

void KPassivePopup::setView( const QString &caption, const QString &text,
                             const QPixmap &icon )
{
    // kdDebug() << "KPassivePopup::setView " << caption << ", " << text << endl;
    setView( standardView( caption, text, icon, this ) );
}

KVBox * KPassivePopup::standardView( const QString& caption,
                                     const QString& text,
                                     const QPixmap& icon,
                                     QWidget *parent )
{
    KVBox *vb = new KVBox( parent ? parent : this );
    vb->setSpacing( KDialog::spacingHint() );

    KHBox *hb=0;
    if ( !icon.isNull() ) {
	hb = new KHBox( vb );
	hb->setMargin( 0 );
	hb->setSpacing( KDialog::spacingHint() );
	ttlIcon = new QLabel( hb, "title_icon" );
	ttlIcon->setPixmap( icon );
        ttlIcon->setAlignment( Qt::AlignLeft );
    }

    if ( !caption.isEmpty() ) {
	ttl = new QLabel( caption, hb ? hb : vb, "title_label" );
	QFont fnt = ttl->font();
	fnt.setBold( true );
	ttl->setFont( fnt );
	ttl->setAlignment( Qt::AlignHCenter );
        if ( hb )
            hb->setStretchFactor( ttl, 10 ); // enforce centering
    }

    if ( !text.isEmpty() ) {
        msg = new QLabel( text, vb, "msg_label" );
        msg->setAlignment( Qt::AlignLeft );
    }

    return vb;
}

void KPassivePopup::setView( const QString &caption, const QString &text )
{
    setView( caption, text, QPixmap() );
}

void KPassivePopup::setTimeout( int delay )
{
    hideDelay = delay;
    if( hideTimer->isActive() )
    {
        if( delay ) {
            hideTimer->start( delay );
        } else {
            hideTimer->stop();
        }
    }
}

void KPassivePopup::setAutoDelete( bool autoDelete )
{
    m_autoDelete = autoDelete;
}

void KPassivePopup::mouseReleaseEvent( QMouseEvent *e )
{
    emit clicked();
    emit clicked( e->pos() );
}

//
// Main Implementation
//

void KPassivePopup::show()
{
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
    Q3Frame::show();

    int delay = hideDelay;
    if ( delay < 0 ) {
        delay = DEFAULT_POPUP_TIME;
    }

    if ( delay > 0 ) {
        hideTimer->start( delay );
    }
}

void KPassivePopup::show(const QPoint &p)
{
    d->fixedPosition = p;
    show();
}

void KPassivePopup::hideEvent( QHideEvent * )
{
    hideTimer->stop();
    if ( m_autoDelete )
        deleteLater();
}

QRect KPassivePopup::defaultArea() const
{
#ifdef Q_WS_X11
    NETRootInfo info( QX11Info::display(),
                      NET::NumberOfDesktops |
                      NET::CurrentDesktop |
                      NET::WorkArea,
                      -1, false );
    info.activate();
    NETRect workArea = info.workArea( info.currentDesktop() );
    QRect r;
    r.setRect( workArea.pos.x, workArea.pos.y, 0, 0 ); // top left
#else
    // FIX IT
    QRect r;
    r.setRect( 100, 100, 200, 200 ); // top left
#endif
    return r;
}

void KPassivePopup::positionSelf()
{
    QRect target;

#ifdef Q_WS_X11
    if ( !window ) {
        target = defaultArea();
    }

    else {
        NETWinInfo ni( QX11Info::display(), window, QX11Info::appRootWindow(),
                       NET::WMIconGeometry | NET::WMKDESystemTrayWinFor );

        // Figure out where to put the popup. Note that we must handle
        // windows that skip the taskbar cleanly
        if ( ni.kdeSystemTrayWinFor() ) {
            NETRect frame, win;
            ni.kdeGeometry( frame, win );
            target.setRect( win.pos.x, win.pos.y,
                            win.size.width, win.size.height );
        }
        else if ( ni.state() & NET::SkipTaskbar ) {
            target = defaultArea();
        }
        else {
            NETRect r = ni.iconGeometry();
            target.setRect( r.pos.x, r.pos.y, r.size.width, r.size.height );
                if ( target.isNull() ) { // bogus value, use the exact position
                    NETRect dummy;
                    ni.kdeGeometry( dummy, r );
                    target.setRect( r.pos.x, r.pos.y, 
                                    r.size.width, r.size.height);
                }
        }
    }
#else
        target = defaultArea();
#endif
    moveNear( target );
}

void KPassivePopup::moveNear( const QRect &target )
{
    QPoint pos = target.topLeft();
    int x = pos.x();
    int y = pos.y();
    int w = width();
    int h = height();

    QRect r = KGlobalSettings::desktopGeometry(QPoint(x+w/2,y+h/2));

    if( d->popupStyle == Balloon )
    {
        // find a point to anchor to
        if( x + w > r.width() ){
            x = x + target.width();
        }

        if( y + h > r.height() ){
            y = y + target.height();
        }
    } else
    {
        if ( x < r.center().x() )
            x = x + target.width();
        else
            x = x - w;

        // It's apparently trying to go off screen, so display it ALL at the bottom.
        if ( (y + h) > r.bottom() )
            y = r.bottom() - h;

        if ( (x + w) > r.right() )
            x = r.right() - w;
    }
    if ( y < r.top() )
        y = r.top();

    if ( x < r.left() )
        x = r.left();

    if( d->popupStyle == Balloon )
        setAnchor( QPoint( x, y ) );
    else
        move( x, y );
}

void KPassivePopup::setAnchor(const QPoint &anchor)
{
    d->anchor = anchor;
    updateMask();
}

void KPassivePopup::paintEvent( QPaintEvent* pe )
{
    if( d->popupStyle == Balloon )
    {
        QPainter p;
        p.begin( this );
        p.drawPolygon( d->surround );
    } else
        Q3Frame::paintEvent( pe );
}

void KPassivePopup::updateMask()
{
    // get screen-geometry for screen our anchor is on
    // (geometry can differ from screen to screen!
    QRect deskRect = KGlobalSettings::desktopGeometry(d->anchor);

    int xh = 70, xl = 40;
    if( width() < 80 )
        xh = xl = 40;
    else if( width() < 110 )
        xh = width() - 40;

    bool bottom = (d->anchor.y() + height()) > ((deskRect.y() + deskRect.height()-48));
    bool right = (d->anchor.x() + width()) > ((deskRect.x() + deskRect.width()-48));

    QPoint corners[4] = {
        QPoint( width() - 50, 10 ),
        QPoint( 10, 10 ),
        QPoint( 10, height() - 50 ),
        QPoint( width() - 50, height() - 50 )
    };

    QBitmap mask( width(), height(), true );
    QPainter p( &mask );
    QBrush brush( Qt::white, Qt::SolidPattern );
    p.setBrush( brush );

    int i = 0, z = 0;
    for (; i < 4; ++i) {
        Q3PointArray corner;
        corner.makeArc(corners[i].x(), corners[i].y(), 40, 40, i * 16 * 90, 16 * 90);

        d->surround.resize( z + corner.count() );
        for (int s = 0; s < corner.count() - 1; s++) {
            d->surround.setPoint( z++, corner[s] );
        }
		
        if (bottom && i == 2) {
            if (right) {
                d->surround.resize( z + 3 );
                d->surround.setPoint( z++, QPoint( width() - xh, height() - 11 ) );
                d->surround.setPoint( z++, QPoint( width() - 20, height() ) );
                d->surround.setPoint( z++, QPoint( width() - xl, height() - 11 ) );
            } else {
                d->surround.resize( z + 3 );
                d->surround.setPoint( z++, QPoint( xl, height() - 11 ) );
                d->surround.setPoint( z++, QPoint( 20, height() ) );
                d->surround.setPoint( z++, QPoint( xh, height() - 11 ) );
            }
        } else if (!bottom && i == 0) {
            if (right) {
                d->surround.resize( z + 3 );
                d->surround.setPoint( z++, QPoint( width() - xl, 10 ) );
                d->surround.setPoint( z++, QPoint( width() - 20, 0 ) );
                d->surround.setPoint( z++, QPoint( width() - xh, 10 ) );
            } else {
                d->surround.resize( z + 3 );
                d->surround.setPoint( z++, QPoint( xh, 10 ) );
                d->surround.setPoint( z++, QPoint( 20, 0 ) );
                d->surround.setPoint( z++, QPoint( xl, 10 ) );
            }
        }
    }

    d->surround.resize( z + 1 );
    d->surround.setPoint( z, d->surround[0] );
    p.drawPolygon( d->surround );
    setMask(mask);

    move( right ? d->anchor.x() - width() + 20 : ( d->anchor.x() < 11 ? 11 : d->anchor.x() - 20 ),
          bottom ? d->anchor.y() - height() : ( d->anchor.y() < 11 ? 11 : d->anchor.y() ) );

    update();
}

//
// Convenience Methods
//

KPassivePopup *KPassivePopup::message( const QString &caption, const QString &text,
				       const QPixmap &icon,
				       QWidget *parent, const char *name, int timeout )
{
    return message( DEFAULT_POPUP_TYPE, caption, text, icon, parent, name, timeout );
}

KPassivePopup *KPassivePopup::message( const QString &text, QWidget *parent, const char *name )
{
    return message( DEFAULT_POPUP_TYPE, QString::null, text, QPixmap(), parent, name );
}

KPassivePopup *KPassivePopup::message( const QString &caption, const QString &text,
				       QWidget *parent, const char *name )
{
    return message( DEFAULT_POPUP_TYPE, caption, text, QPixmap(), parent, name );
}

KPassivePopup *KPassivePopup::message( const QString &caption, const QString &text,
				       const QPixmap &icon, WId parent, const char *name, int timeout )
{
    return message( DEFAULT_POPUP_TYPE, caption, text, icon, parent, name, timeout );
}

KPassivePopup *KPassivePopup::message( int popupStyle, const QString &caption, const QString &text,
				       const QPixmap &icon,
				       QWidget *parent, const char *name, int timeout )
{
    KPassivePopup *pop = new KPassivePopup( popupStyle, parent, name );
    pop->setAutoDelete( true );
    pop->setView( caption, text, icon );
    pop->hideDelay = timeout;
    pop->show();

    return pop;
}

KPassivePopup *KPassivePopup::message( int popupStyle, const QString &text, QWidget *parent, const char *name )
{
    return message( popupStyle, QString::null, text, QPixmap(), parent, name );
}

KPassivePopup *KPassivePopup::message( int popupStyle, const QString &caption, const QString &text,
				       QWidget *parent, const char *name )
{
    return message( popupStyle, caption, text, QPixmap(), parent, name );
}

KPassivePopup *KPassivePopup::message( int popupStyle, const QString &caption, const QString &text,
				       const QPixmap &icon, WId parent, const char *name, int timeout )
{
    KPassivePopup *pop = new KPassivePopup( popupStyle, parent, name );
    pop->setAutoDelete( true );
    pop->setView( caption, text, icon );
    pop->hideDelay = timeout;
    pop->show();

    return pop;
}

// Local Variables:
// c-basic-offset: 4
// End:
