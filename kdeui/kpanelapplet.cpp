/*****************************************************************

Copyright (c) 2000 Matthias Elter

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <kconfig.h>

#include "kpanelapplet.h"
#include "kpanelapplet.moc"

KPanelApplet::KPanelApplet(const QString& configFile, Type type,
                           int actions, QWidget *parent, const char *name, WFlags f)
  : QFrame(parent, name, f)
  , _type(type)
  , _position( pBottom )
  , _alignment( LeftTop )
  , _config(0)
  , _actions(actions)
{
  setFrameStyle(NoFrame);
  QPalette pal(palette());
  if(pal.active().mid() != pal.inactive().mid()){
    pal.setInactive(pal.active());
    setPalette(pal);
  }

  _config = new KConfig(configFile);
}

KPanelApplet::~KPanelApplet()
{
  delete _config;
}

void KPanelApplet::setPosition( Position p )
{
  if( _position == p ) return;
  _position = p;
  positionChange( p );
}

void KPanelApplet::setAlignment( Alignment a )
{
  if( _alignment == a ) return;
  _alignment = a;
  alignmentChange( a );
}

// FIXME: Remove implementation for KDE 4
void KPanelApplet::positionChange( Position )
{
  orientationChange( orientation() );
  QResizeEvent e( size(), size() );
  resizeEvent( &e );
  popupDirectionChange( popupDirection() );
}

Qt::Orientation KPanelApplet::orientation() const
{
  if( _position == pTop || _position == pBottom ) {
    return Horizontal;
  } else {
    return Vertical;
  }
}

// FIXME: Remove for KDE 4
KPanelApplet::Direction KPanelApplet::popupDirection()
{
    switch( _position ) {
    case pTop:     return Down;
    case pRight:   return Left;
    case pLeft:    return Right;
    default:
    case pBottom:  return Up;
    }
}

void KPanelApplet::action( Action a )
{
    if ( (a & About) != 0 )
	about();
    if ( (a & Help) != 0 )
	help();
    if ( (a & Preferences) != 0 )
	preferences();
    if ( (a & ReportBug) != 0 )
    reportBug();
}

void KPanelApplet::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

