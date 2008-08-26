/*
    This file is part of the KDE libraries
    Copyright (C) 2007 Laurent Montel (montel@kde.org)
    Copyright (C) 2007 Christian Ehrlicher (ch.ehrlicher@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kwindowsystem.h"

#include <QtGui/QDesktopWidget>
#include <QtGui/QIcon>
#include <QtGui/QBitmap>
#include <QtGui/QPixmap>

#include "kglobal.h"
#include "kdebug.h"
#include "klocalizedstring.h"

#include <windows.h>

class KWindowSystemStaticContainer {
public:
    KWindowSystemStaticContainer() : d(0) {}
    KWindowSystem kwm;
    KWindowSystemPrivate* d;
};

K_GLOBAL_STATIC(KWindowSystemStaticContainer, g_kwmInstanceContainer)

K_GLOBAL_STATIC(QDesktopWidget, s_deskWidget)

int KWindowSystem::currentDesktop()
{
    return 1;
}

int KWindowSystem::numberOfDesktops()
{
    return 1;
}

void KWindowSystem::setMainWindow( QWidget* subwindow, WId mainwindow )
{
    SetForegroundWindow(subwindow->winId());
}

void KWindowSystem::setCurrentDesktop( int desktop )
{
    kDebug() << "KWindowSystem::setCurrentDesktop( int desktop ) isn't yet implemented!";
    //TODO
}

void KWindowSystem::setOnAllDesktops( WId win, bool b )
{
     kDebug() << "KWindowSystem::setOnAllDesktops( WId win, bool b ) isn't yet implemented!";
     //TODO
}

void KWindowSystem::setOnDesktop( WId win, int desktop )
{
     //TODO
     kDebug() << "KWindowSystem::setOnDesktop( WId win, int desktop ) isn't yet implemented!";
}

WId KWindowSystem::activeWindow()
{
    return GetActiveWindow();
}

void KWindowSystem::activateWindow( WId win, long )
{
    SetActiveWindow( win );
}

void KWindowSystem::forceActiveWindow( WId win, long time )
{
    SetActiveWindow( win );
    SetForegroundWindow( win );
}

void KWindowSystem::demandAttention( WId win, bool set )
{
    FLASHWINFO fi;
    fi.cbSize = sizeof( FLASHWINFO );
    fi.hwnd = win;
    fi.dwFlags = set ? FLASHW_ALL : FLASHW_STOP;
    fi.uCount = 5;
    fi.dwTimeout = 0;

    FlashWindowEx( &fi );
}

static HBITMAP QPixmapMask2HBitmap(const QPixmap &pix)
{
    QBitmap bm = pix.mask();
    if( bm.isNull() ) {
        bm = QBitmap( pix.size() );
        bm.fill( Qt::color1 );
    }
    QImage im = bm.toImage().convertToFormat( QImage::Format_Mono );
    im.invertPixels();                  // funny blank'n'white games on windows
    int w = im.width();
    int h = im.height();
    int bpl = (( w + 15 ) / 16 ) * 2;   // bpl, 16 bit alignment
    QByteArray bits( bpl * h, '\0' );
    for (int y=0; y < h; y++)
        memcpy( bits.data() + y * bpl, im.scanLine( y ), bpl );
    return CreateBitmap( w, h, 1, 1, bits );
}

static HICON QPixmap2HIcon(const QPixmap &pix)
{
    if ( pix.isNull() )
        return 0;

    ICONINFO ii;
    ii.fIcon    = true;
    ii.hbmMask  = QPixmapMask2HBitmap( pix );
    ii.hbmColor = pix.toWinHBITMAP( QPixmap::PremultipliedAlpha );
    ii.xHotspot = 0;
    ii.yHotspot = 0;
    HICON result = CreateIconIndirect( &ii );

    DeleteObject( ii.hbmMask );
    DeleteObject( ii.hbmColor );

    return result;
}

static QPixmap HIcon2QPixmap( HICON hIcon )
{
    ICONINFO ii;
    if( GetIconInfo( hIcon, &ii ) == NULL )
        return QPixmap();

    QPixmap pix  = QPixmap::fromWinHBITMAP( ii.hbmColor );
    pix.setMask( QPixmap::fromWinHBITMAP( ii.hbmMask ) );

    return pix;
}

QPixmap KWindowSystem::icon( WId win, int width, int height, bool scale )
{
    UINT size = ICON_BIG;
    if( width < 24 || height < 24 )
        size = ICON_SMALL;
    HICON hIcon = (HICON)SendMessage( win, WM_GETICON, size, 0);
    QPixmap pm = HIcon2QPixmap( hIcon );
    if( scale )
        pm = pm.scaled( width, height );
    return pm;
}

class hIconCache
{
public:
    ~hIconCache()
    {
        Q_FOREACH(const HICON &hIcon, m_iconList)
            DestroyIcon(hIcon);
    }
    QList<HICON> m_iconList;
};
K_GLOBAL_STATIC(hIconCache, s_iconCache)

QPixmap KWindowSystem::icon( WId win, int width, int height, bool scale, int )
{
    return icon( win, width, height, scale );
}

void KWindowSystem::setIcons( WId win, const QPixmap& icon, const QPixmap& miniIcon )
{
    HICON hIconBig = QPixmap2HIcon(icon);
    HICON hIconSmall = QPixmap2HIcon(miniIcon);
    if(hIconBig)
        s_iconCache->m_iconList.append(hIconBig);
    if(hIconSmall)
        s_iconCache->m_iconList.append(hIconSmall);

    hIconBig = (HICON)SendMessage( win, WM_SETICON, ICON_BIG,   (LPARAM)hIconBig );
    hIconSmall = (HICON)SendMessage( win, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall );
    
    s_iconCache->m_iconList.removeAll(hIconBig);
    s_iconCache->m_iconList.removeAll(hIconSmall);
}


void KWindowSystem::setState( WId win, unsigned long state )
{
    bool got = false;
    if (state & NET::SkipTaskbar) {
        got = true;
        LONG_PTR lp = GetWindowLongPtr(win, GWL_EXSTYLE);
        SetWindowLongPtr(win, GWL_EXSTYLE, lp | WS_EX_TOOLWINDOW);
    }
    if (state & NET::KeepAbove) {
        got = true;
        SetWindowPos(win, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    if (!got)
        kDebug() << "KWindowSystem::setState( WId win, unsigned long state ) isn't yet implemented for the state you requested!";
}

void KWindowSystem::clearState( WId win, unsigned long state )
{
    //TODO
    kDebug() << "KWindowSystem::clearState( WId win, unsigned long state ) isn't yet implemented!";
}

void KWindowSystem::minimizeWindow( WId win, bool animation)
{
    Q_UNUSED( animation );
    ShowWindow( win, SW_MINIMIZE );
}

void KWindowSystem::unminimizeWindow( WId win, bool animation )
{
    Q_UNUSED( animation );
    ShowWindow( win, SW_RESTORE );
}

void KWindowSystem::raiseWindow( WId win )
{
    SetWindowPos( win, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE ); // mhhh?
}

void KWindowSystem::lowerWindow( WId win )
{
    SetWindowPos( win, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE ); // mhhh?
}

bool KWindowSystem::compositingActive()
{
    return false;
}

QRect KWindowSystem::workArea( int desktop )
{
    return s_deskWidget->availableGeometry( desktop );
}

QRect KWindowSystem::workArea( const QList<WId>& exclude, int desktop )
{
    //TODO
    kDebug() << "QRect KWindowSystem::workArea( const QList<WId>& exclude, int desktop ) isn't yet implemented!";
    return QRect();
}

QString KWindowSystem::desktopName( int desktop )
{
    return i18n("Desktop %1",  desktop );
}

void KWindowSystem::setDesktopName( int desktop, const QString& name )
{
     kDebug() << "KWindowSystem::setDesktopName( int desktop, const QString& name ) isn't yet implemented!";
    //TODO
}

bool KWindowSystem::showingDesktop()
{
    return false;
}

void KWindowSystem::setUserTime( WId win, long time )
{
    kDebug() << "KWindowSystem::setUserTime( WId win, long time ) isn't yet implemented!";
    //TODO
}

bool KWindowSystem::icccmCompliantMappingState()
{
    return false;
}

void KWindowSystem::connectNotify( const char* signal )
{
  kDebug() << "connectNotify( const char* signal )  isn't yet implemented!";
//TODO
}

void KWindowSystem::setExtendedStrut( WId win, int left_width, int left_start, int left_end,
        int right_width, int right_start, int right_end, int top_width, int top_start, int top_end,
        int bottom_width, int bottom_start, int bottom_end )
{
  kDebug() << "KWindowSystem::setExtendedStrut isn't yet implemented!";
  //TODO
}
void KWindowSystem::setStrut( WId win, int left, int right, int top, int bottom )
{
  kDebug() << "KWindowSystem::setStrut isn't yet implemented!";
  //TODO
}

QString KWindowSystem::readNameProperty( WId window, unsigned long atom )
{
  //TODO
  kDebug() << "QString KWindowSystem::readNameProperty( WId window, unsigned long atom ) isn't yet implemented!";
  return QString();
}

void KWindowSystem::doNotManage( const QString& title )
{
  //TODO
  kDebug() << "KWindowSystem::doNotManage( const QString& title ) isn't yet implemented!";
}

QList<WId> KWindowSystem::stackingOrder()
{
  //TODO
  QList<WId> lst;
  kDebug() << "QList<WId> KWindowSystem::stackingOrder() isn't yet implemented!";
  return lst;
}

const QList<WId>& KWindowSystem::windows()
{
  //TODO
  static QList<WId> lst;
  kDebug() << "const QList<WId>& KWindowSystem::windows()  isn't yet implemented!";
  return lst;
}

void KWindowSystem::setType( WId win, NET::WindowType windowType )
{
 //TODO
 kDebug() << "setType( WId win, NET::WindowType windowType ) isn't yet implemented!";
}

KWindowSystem* KWindowSystem::self()
{
    return &(g_kwmInstanceContainer->kwm);
}

#include "kwindowsystem.moc"

