/* vi: ts=8 sts=4 sw=4
 *
 *
 * This file is part of the KDE project, module kdeui.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * You can Freely distribute this program under the GNU Library
 * General Public License. See the file "COPYING.LIB" for the exact
 * licensing terms.
 */

#include <qwidget.h>
#include <qtimer.h>
#include <qrect.h>
#include <qimage.h>

#ifdef Q_WS_X11
#include <qx11info_x11.h>
#else
#include <QApplication>
#include <QDesktopWidget>
#endif

#include <kapplication.h>
#include <kimageeffect.h>
#include <kpixmapio.h>
#include <kwin.h>
#include <kdebug.h>
#include <netwm.h>
#include <dcopclient.h>
#include <dcopref.h>

#include <ksharedpixmap.h>
#include <krootpixmap.h>


static QString wallpaperForDesktop(int desktop)
{
    return DCOPRef("kdesktop", "KBackgroundIface").call("currentWallpaper", desktop);
}

class KRootPixmapData
{
public:
    QWidget *toplevel;
#ifdef Q_WS_X11
    KWinModule *kwin;
#endif
};


KRootPixmap::KRootPixmap( QWidget *widget )
    : QObject( widget ), m_Desk(0), m_pWidget(widget)
{
    setObjectName( "KRootPixmap" );
    init();
}

KRootPixmap::KRootPixmap( QWidget *widget, QObject *parent )
    : QObject( parent ), m_Desk(0), m_pWidget(widget)
{
    setObjectName( "KRootPixmap" );
    init();
}

void KRootPixmap::init()
{
    d = new KRootPixmapData;
    m_Fade = 0;
    m_pPixmap = new KSharedPixmap; //ordinary KPixmap on win32
    m_pTimer = new QTimer( this );
    m_bInit = false;
    m_bActive = false;
    m_bCustomPaint = false;

    connect(kapp, SIGNAL(backgroundChanged(int)), SLOT(slotBackgroundChanged(int)));
    connect(m_pTimer, SIGNAL(timeout()), SLOT(repaint()));
#ifdef Q_WS_X11
    connect(m_pPixmap, SIGNAL(done(bool)), SLOT(slotDone(bool)));

    d->kwin = new KWinModule( this );
    connect(d->kwin, SIGNAL(windowChanged(WId, unsigned int)), SLOT(desktopChanged(WId, unsigned int)));
    connect(d->kwin, SIGNAL(currentDesktopChanged(int)), SLOT(desktopChanged(int)));
#endif

    d->toplevel = m_pWidget->topLevelWidget();
    d->toplevel->installEventFilter(this);
    m_pWidget->installEventFilter(this);
}

KRootPixmap::~KRootPixmap()
{
    delete m_pPixmap;
    delete d;
}


int KRootPixmap::currentDesktop() const
{
#ifdef Q_WS_X11
    NETRootInfo rinfo( QX11Info::display(), NET::CurrentDesktop );
    rinfo.activate();
    return rinfo.currentDesktop();
#else
    //OK?
    return QApplication::desktop()->screenNumber(m_pWidget);
#endif
}


void KRootPixmap::start()
{
    if (m_bActive)
	return;

    m_bActive = true;
    if ( !isAvailable() )
    {
	// We will get a KIPC message when the shared pixmap is available.
	enableExports();
	return;
    }
    if (m_bInit)
	repaint(true);
}


void KRootPixmap::stop()
{
    m_bActive = false;
    m_pTimer->stop();
}


void KRootPixmap::setFadeEffect(double fade, const QColor &color)
{
    if (fade < 0)
	m_Fade = 0;
    else if (fade > 1)
	m_Fade = 1;
    else
	m_Fade = fade;
    m_FadeColor = color;

    if ( m_bActive && m_bInit ) repaint(true);
}


bool KRootPixmap::eventFilter(QObject *, QEvent *event)
{
    // Initialise after the first show or paint event on the managed widget.
    if (!m_bInit && ((event->type() == QEvent::Show) || (event->type() == QEvent::Paint)))
    {
	m_bInit = true;
	m_Desk = currentDesktop();
    }

    if (!m_bActive)
	return false;

    switch (event->type())
    {
    case QEvent::Resize:
    case QEvent::Move:
        m_pTimer->setSingleShot(true);
	m_pTimer->start(100);
	break;

    case QEvent::Paint:
        m_pTimer->setSingleShot(true);
	m_pTimer->start(0);
	break;

    case QEvent::ParentChange:
        d->toplevel->removeEventFilter(this);
        d->toplevel = m_pWidget->topLevelWidget();
        d->toplevel->installEventFilter(this);
        break;

    default:
	break;
    }

    return false; // always continue processing
}

void KRootPixmap::desktopChanged(int desktop)
{
    if (wallpaperForDesktop(m_Desk) == wallpaperForDesktop(desktop) &&
	!wallpaperForDesktop(m_Desk).isNull())
	return;

#ifdef Q_WS_X11
    if (KWin::windowInfo(m_pWidget->topLevelWidget()->winId()).desktop() == NET::OnAllDesktops &&
	pixmapName(m_Desk) != pixmapName(desktop))
#endif
	repaint(true);
}

void KRootPixmap::desktopChanged( WId window, unsigned int properties )
{
#ifdef Q_WS_X11
    if( !(properties & NET::WMDesktop) ||
	(window != m_pWidget->topLevelWidget()->winId()))
	return;
#endif

    kdDebug() << k_funcinfo << endl;
    repaint(true);
}

void KRootPixmap::repaint()
{
    repaint(false);
}


void KRootPixmap::repaint(bool force)
{
    QPoint p1 = m_pWidget->mapToGlobal(m_pWidget->rect().topLeft());
    QPoint p2 = m_pWidget->mapToGlobal(m_pWidget->rect().bottomRight());
    if (!force && (m_Rect == QRect(p1, p2)))
	return;

    // Due to northwest bit gravity, we don't need to do anything if the
    // bottom right corner of the widget is moved inward.
    // That said, konsole clears the background when it is resized, so
    // we have to reset the background pixmap.
    if ((p1 == m_Rect.topLeft()) && (m_pWidget->width() < m_Rect.width()) &&
	(m_pWidget->height() < m_Rect.height())
       )
    {
        m_Rect = QRect(p1, p2);
 	updateBackground( m_pPixmap );
	return;
    }
    m_Rect = QRect(p1, p2);
#ifdef Q_WS_X11
    m_Desk = KWin::windowInfo(m_pWidget->topLevelWidget()->winId()).desktop();
    if (m_Desk == NET::OnAllDesktops)
	m_Desk = currentDesktop();

    // KSharedPixmap will correctly generate a tile for us.
    m_pPixmap->loadFromShared(pixmapName(m_Desk), m_Rect);
#else
	m_Desk = currentDesktop();
    // !x11 note: tile is not generated!
    // TODO: pixmapName() is a nonsense now!
    m_pPixmap->load( pixmapName(m_Desk) );
    if (!m_pPixmap->isNull()) {
        m_pPixmap->resize( m_Rect.size() );
        slotDone(true);
    }
#endif
}

bool KRootPixmap::isAvailable() const
{
#ifdef Q_WS_X11
    return m_pPixmap->isAvailable(pixmapName(m_Desk));
#else
    return m_pPixmap->isNull();
#endif
}

QString KRootPixmap::pixmapName(int desk) {
    QString pattern = QString("DESKTOP%1");
#ifdef Q_WS_X11
    int screen_number = DefaultScreen(QX11Info::display());
    if (screen_number) {
        pattern = QString("SCREEN%1-DESKTOP").arg(screen_number) + "%1";
    }
#endif
    return pattern.arg( desk );
}


void KRootPixmap::enableExports()
{
#ifdef Q_WS_X11
    kdDebug(270) << k_lineinfo << "activating background exports.\n";
    DCOPClient *client = KApplication::dcopClient();
    if (!client->isAttached())
	client->attach();
    QByteArray data;
    QDataStream args( &data, QIODevice::WriteOnly );
    args << 1;

    DCOPCString appname( "kdesktop" );
    int screen_number = DefaultScreen(QX11Info::display());
    if ( screen_number )
        appname = DCOPCString("kdesktop-screen-") + QByteArray::number( screen_number );

    client->send( appname, "KBackgroundIface", "setExport(int)", data );
#endif
}


void KRootPixmap::slotDone(bool success)
{
    if (!success)
    {
	kdWarning(270) << k_lineinfo << "loading of desktop background failed.\n";
	return;
    }

    // We need to test active as the pixmap might become available
    // after the widget has been destroyed.
    if ( m_bActive )
	updateBackground( m_pPixmap );
}

void KRootPixmap::updateBackground( KSharedPixmap *spm )
{
    QPixmap pm = spm->pixmap();

    if (m_Fade > 1e-6)
    {
	KPixmapIO io;
	QImage img = io.convertToImage(pm);
	img = KImageEffect::fade(img, m_Fade, m_FadeColor);
	pm = io.convertToPixmap(img);
    }

    if ( !m_bCustomPaint ) {
        QBrush brush( pm );
        QPalette pal = m_pWidget->palette();
        pal.setBrush( QPalette::Window, brush );
	m_pWidget->setPalette( pal );
    } else {
	emit backgroundUpdated( pm );
    }
}


void KRootPixmap::slotBackgroundChanged(int desk)
{
    if (!m_bInit || !m_bActive)
	return;

    if (desk == m_Desk)
	repaint(true);
}

#include "krootpixmap.moc"
