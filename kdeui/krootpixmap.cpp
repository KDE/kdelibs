/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdeui.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * You can Freely distribute this program under the GNU Library
 * General Public License. See the file "COPYING.LIB" for the exact
 * licensing terms.
 */

#include <math.h>

#include <qwidget.h>
#include <qtimer.h>
#include <qrect.h>
#include <qpoint.h>
#include <qevent.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qcstring.h>

#include <kapp.h>
#include <klocale.h>
#include <kwin.h>
#include <kimageeffect.h>
#include <kpixmapeffect.h>
#include <kmessagebox.h>
#include <kpixmapio.h>
#include <kdebug.h>
#include <netwm.h>
#include <dcopclient.h>

#include <ksharedpixmap.h>
#include <krootpixmap.h>

#include <X11/X.h>
#include <X11/Xlib.h>


KRootPixmap::KRootPixmap(QWidget *widget)
    : QObject(widget)
{
    m_pWidget = widget;
    m_pPixmap = new KSharedPixmap;
    m_pTimer = new QTimer;
    m_bInit = false;
    m_bActive = false;

    connect(kapp, SIGNAL(backgroundChanged(int)), SLOT(slotBackgroundChanged(int)));
    connect(m_pPixmap, SIGNAL(done(bool)), SLOT(slotDone(bool)));
    connect(m_pTimer, SIGNAL(timeout()), SLOT(repaint()));

    QObject *obj = m_pWidget;
    while (obj->parent())
	obj = obj->parent();
    obj->installEventFilter(this);
}


KRootPixmap::~KRootPixmap()
{
    delete m_pPixmap;
    delete m_pTimer;
}


int KRootPixmap::currentDesktop()
{
    NETRootInfo rinfo( qt_xdisplay(), NET::CurrentDesktop );
    rinfo.activate();
    return rinfo.currentDesktop();
}

    
void KRootPixmap::start()
{
    if (m_bActive)
	return;

    m_bActive = true;
    if (!checkAvailable( false ))
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


void KRootPixmap::setFadeEffect(double fade, QColor color)
{
    if (fade < 0)
	m_Fade = 0;
    else if (fade > 1)
	m_Fade = 1;
    else
	m_Fade = fade;
    m_FadeColor = color;
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
	m_pTimer->start(100, true);
	break;

    case QEvent::Paint:
	m_pTimer->start(0, true);
	break;

    default:
	break;
    }

    return false; // always continue processing
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
        slotDone( true );
	return;
    }
    m_Rect = QRect(p1, p2);
    m_Desk = currentDesktop();

    // KSharedPixmap will correctly generate a tile for us.
    m_pPixmap->loadFromShared(QString("DESKTOP%1").arg(m_Desk), m_Rect);
}


bool KRootPixmap::checkAvailable( bool /* ignored */ )
{
    QString name = QString("DESKTOP%1").arg( currentDesktop() );
    return m_pPixmap->isAvailable(name);
}


void KRootPixmap::enableExports()
{
    kdDebug(270) << k_lineinfo << "activating background exports.\n";
    DCOPClient *client = kapp->dcopClient();
    if (!client->isAttached())
	client->attach();
    QByteArray data;
    QDataStream args( data, IO_WriteOnly );
    args << 1;
    client->send( "kdesktop", "KBackgroundIface", "setExport(int)", data );
}
    

void KRootPixmap::slotDone(bool success)
{
    if (!success)
    {
	kdWarning(270) << k_lineinfo << "loading of desktop background failed.\n";
	return;
    }

    QPixmap pm = *m_pPixmap;

    if (m_Fade > 1e-6)
    {
	KPixmapIO io;
	QImage img = io.convertToImage(pm);
	img = KImageEffect::fade(img, m_Fade, m_FadeColor);
	pm = io.convertToPixmap(img);
    }

    m_pWidget->setBackgroundPixmap(pm);
}


void KRootPixmap::slotBackgroundChanged(int desk)
{
    if (!m_bInit || !m_bActive)
	return;

    if (desk == m_Desk)
	repaint(true);
}


#include "krootpixmap.moc"
