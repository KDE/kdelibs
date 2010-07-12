/*
    This file is part of the KDE libraries
    Copyright (C) 2008 Carlo Segato (brandon.ml@gmail.com)

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

#include "kwindowinfo.h"
#include "kwindowsystem.h"
#include <windows.h>

class KWindowInfo::Private
{
    public:
	Private()
	: properties(0),properties2(0)
	{}
    
	~Private() { }
   
	WId win_;
	int ref;
    unsigned long properties;
    unsigned long properties2;
    private:
	Private( const Private& );
	void operator=( const Private& );
};

#include <QRect>

KWindowInfo::KWindowInfo( WId win, unsigned long properties, unsigned long properties2) : d ( new Private ) 
{
    d->ref = 1;
    d->win_ = win;
    d->properties  = properties;
    d->properties2 = properties2;
}

KWindowInfo::KWindowInfo()
    : d( NULL )
{

}

KWindowInfo::~KWindowInfo()
{
    if( d != NULL ) {
	if( --d->ref == 0 ) {
	    delete d;
	}
    }
}

KWindowInfo::KWindowInfo( const KWindowInfo& wininfo )
    : d( wininfo.d )
{
    if( d != NULL )
	++d->ref;
}

KWindowInfo& KWindowInfo::operator=( const KWindowInfo& wininfo )
{
    if( d != wininfo.d ) {
	if( d != NULL )
	    if( --d->ref == 0 )
		delete d;
	d = wininfo.d;
	if( d != NULL )
	    ++d->ref;
    }
    return *this;
}


bool KWindowInfo::valid( bool withdrawn_is_valid ) const
{
    return true;
}

WId KWindowInfo::win() const
{
    return d->win_;
}

unsigned long KWindowInfo::state() const
{
    unsigned long state = 0;
#ifndef _WIN32_WCE
     if(IsZoomed(d->win_))
        state |= NET::Max;
#endif
     if(!IsWindowVisible(d->win_))
        state |= NET::Hidden;
     
#ifndef _WIN32_WCE     
    LONG_PTR lp = GetWindowLongPtr(d->win_, GWL_EXSTYLE);
    if(lp & WS_EX_TOOLWINDOW)
        state |= NET::SkipTaskbar;
#endif
        
    return state;
}

bool KWindowInfo::hasState( unsigned long s ) const
{
    return (state() & s);
}

bool KWindowInfo::isMinimized() const
{
#ifndef _WIN32_WCE
    return IsIconic(d->win_);
#else
    return false;
#endif
}

NET::MappingState KWindowInfo::mappingState() const
{    
#ifndef _WIN32_WCE
    if(IsIconic(d->win_))
        return NET::Iconic;  
#endif
    if(!IsWindowVisible(d->win_)) 
        return NET::Withdrawn;
    return NET::Visible;
}

NETExtendedStrut KWindowInfo::extendedStrut() const
{
    return NETExtendedStrut();
}

NET::WindowType KWindowInfo::windowType( int supported_types ) const
{
    NET::WindowType wt(NET::Normal);
    
    
    long windowStyle   = GetWindowLong(d->win_,GWL_STYLE);
    long windowStyleEx = GetWindowLong(d->win_,GWL_EXSTYLE);  

    if(windowStyle & WS_POPUP && supported_types & NET::PopupMenuMask)
        return NET::PopupMenu;
    else if(windowStyleEx & WS_EX_TOOLWINDOW && supported_types & NET::TooltipMask)
        return NET::Tooltip;
    else if(!(windowStyle & WS_CHILD) && supported_types & NET::NormalMask)
        return NET::Normal;
        
    return wt;
}

QString KWindowInfo::visibleNameWithState() const
{
    return name();
}

QString KWindowInfo::visibleName() const
{
    return name();
}

QString KWindowInfo::name() const
{
    QByteArray windowText = QByteArray ( (GetWindowTextLength(d->win_)+1) * sizeof(wchar_t), 0 ) ;
    GetWindowTextW(d->win_, (LPWSTR)windowText.data(), windowText.size());
    return QString::fromWCharArray((wchar_t*)windowText.data());
}

QString KWindowInfo::visibleIconNameWithState() const
{
    return QString();
}

QString KWindowInfo::visibleIconName() const
{
    return QString();
}

QString KWindowInfo::iconName() const
{
    return QString();
}

bool KWindowInfo::isOnCurrentDesktop() const
{
    return true;
}

bool KWindowInfo::isOnDesktop( int desk ) const
{
    if(desk == desktop())
        return true;
    return false;
}

bool KWindowInfo::onAllDesktops() const
{
    return false;
}

int KWindowInfo::desktop() const
{
    return 0;
}

QRect KWindowInfo::geometry() const
{
    RECT wndRect;
    memset(&wndRect,0,sizeof(wndRect));
    
    //fetch the geometry INCLUDING the frames
    if(GetWindowRect(d->win_,&wndRect)){
    QRect result;
    result.setCoords ( wndRect.left, wndRect.top, wndRect.right, wndRect.bottom );
    return result;
    }
    
    return QRect();
}

QRect KWindowInfo::frameGeometry() const
{
    RECT wndRect;
    memset(&wndRect,0,sizeof(wndRect));
    
    //fetch only client area geometries ... i hope thats right
    if(GetClientRect(d->win_,&wndRect)){
    QRect result;
    result.setCoords ( wndRect.left, wndRect.top, wndRect.right, wndRect.bottom );
    return result;
    }
    
    return QRect();
}

bool KWindowInfo::actionSupported( NET::Action action ) const
{
    return true; // no idea if it's supported or not -> pretend it is
}

#if 0
WId KWindowInfo::transientFor() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2TransientFor ) == 0, 176 )
        << "Pass NET::WM2TransientFor to KWindowInfo";
    return d->info->transientFor();
}

WId KWindowInfo::groupLeader() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2GroupLeader ) == 0, 176 )
        << "Pass NET::WM2GroupLeader to KWindowInfo";
    return d->info->groupLeader();
}

QByteArray KWindowInfo::windowClassClass() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2WindowClass ) == 0, 176 )
        << "Pass NET::WM2WindowClass to KWindowInfo";
    return d->info->windowClassClass();
}

QByteArray KWindowInfo::windowClassName() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2WindowClass ) == 0, 176 )
        << "Pass NET::WM2WindowClass to KWindowInfo";
    return d->info->windowClassName();
}

QByteArray KWindowInfo::windowRole() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2WindowRole ) == 0, 176 )
        << "Pass NET::WM2WindowRole to KWindowInfo";
    return d->info->windowRole();
}

QByteArray KWindowInfo::clientMachine() const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2ClientMachine ) == 0, 176 )
        << "Pass NET::WM2ClientMachine to KWindowInfo";
    return d->info->clientMachine();
}

bool KWindowInfo::actionSupported( NET::Action action ) const
{
    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2AllowedActions ) == 0, 176 )
        << "Pass NET::WM2AllowedActions to KWindowInfo";
    if( KWindowSystem::allowedActionsSupported())
        return d->info->allowedActions() & action;
    else
        return true; // no idea if it's supported or not -> pretend it is
}

// see NETWM spec section 7.6
bool KWindowInfo::isMinimized() const
{
    if( mappingState() != NET::Iconic )
        return false;
    // NETWM 1.2 compliant WM - uses NET::Hidden for minimized windows
    if(( state() & NET::Hidden ) != 0
	&& ( state() & NET::Shaded ) == 0 ) // shaded may have NET::Hidden too
        return true;
    // older WMs use WithdrawnState for other virtual desktops
    // and IconicState only for minimized
    return KWindowSystem::icccmCompliantMappingState() ? false : true;
}
#endif
