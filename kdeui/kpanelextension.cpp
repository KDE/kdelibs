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

#include "kpanelextension.h"
#include "kpanelextension.moc"

class KPanelExtensionPrivate
{
public:
    KPanelExtensionPrivate() 
      : _size(KPanelExtension::SizeNormal), 
        _customSize(58) 
    {}

    KPanelExtension::Size _size;
    int _customSize;
};

KPanelExtension::KPanelExtension(const QString& configFile, Type type,
				 int actions, QWidget *parent, const char *name)
  : QFrame(parent, name)
  , _type(type)
  , _position( Top )
  , _alignment( LeftTop )
  , _config(0)
  , _actions(actions)
{
    d = new KPanelExtensionPrivate;
    setFrameStyle(NoFrame);
    _config = new KConfig(configFile);
}

KPanelExtension::~KPanelExtension()
{
    delete _config;
    delete d;
}

void KPanelExtension::setPosition( Position p )
{
  if( _position == p ) return;
  _position = p;
  positionChange( p );
}

void KPanelExtension::setAlignment( Alignment a )
{
  if( _alignment == a ) return;
  _alignment = a;
  alignmentChange( a );
}

void KPanelExtension::setSize( Size size, int customSize )
{
  if ( d->_size == size && d->_customSize == customSize ) return;
  d->_size = size;
  d->_customSize = customSize;
  emit updateLayout();
}

void KPanelExtension::action( Action a )
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

Qt::Orientation KPanelExtension::orientation()
{
    if (_position == Left || _position == Right)
	return Vertical;
    else
	return Horizontal;
}

KPanelExtension::Size KPanelExtension::sizeSetting() const
{
    return d->_size;
}

int KPanelExtension::customSize() const
{
    return d->_customSize;
}

int KPanelExtension::sizeInPixels() const
{
  if (d->_size == SizeTiny)
  {
    return 24;
  }
  else if (d->_size == SizeSmall)
  {
    return 30;
  }
  else if (d->_size == SizeNormal)
  {
    return 46;
  }
  else if (d->_size == SizeLarge)
  {
    return 58;
  }

  return d->_customSize;
}

void KPanelExtension::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

