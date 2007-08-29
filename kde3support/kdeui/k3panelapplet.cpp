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

#include "k3panelapplet.h"
#include "k3panelapplet.moc"
#include <ksharedconfig.h>
#include <kglobal.h>
#include <QResizeEvent>

class K3PanelApplet::Private
{
public:
  Private()
    : position( K3PanelApplet::Bottom ),
      alignment( K3PanelApplet::LeftTop ),
      customMenu(0),
      hasFocus(false)
      {}

  K3PanelApplet::Type type;
  K3PanelApplet::Position position;
  K3PanelApplet::Alignment alignment;
  int actions;

  const QMenu* customMenu;
  KSharedConfig::Ptr sharedConfig;
  QList<QObject*> watchedForFocus;
  bool hasFocus;
};

K3PanelApplet::K3PanelApplet(const QString& configFile, K3PanelApplet::Type type,
                           int actions, QWidget *parent, Qt::WFlags f)
  : QFrame(parent, f),
    d(new Private())
{
  d->type = type;
  d->actions = actions;

  setFrameStyle(NoFrame);
  QPalette pal(palette());
  if(pal.active().mid() != pal.inactive().mid()){
    pal.setInactive(pal.active());
    setPalette(pal);
  }
  setBackgroundOrigin( AncestorOrigin );

  d->sharedConfig = KSharedConfig::openConfig(configFile);
}

K3PanelApplet::~K3PanelApplet()
{
  d->watchedForFocus.clear();
  needsFocus(false);
  delete d;
}

KConfig* K3PanelApplet::config() const
{
  return d->sharedConfig.data();
}

K3PanelApplet::Type K3PanelApplet::type() const
{
  return d->type;
}

int K3PanelApplet::actions() const
{
  return d->actions;
}

void K3PanelApplet::setPosition( K3PanelApplet::Position p )
{
  if( d->position == p ) return;
  d->position = p;
  positionChange( p );
}

void K3PanelApplet::setAlignment( K3PanelApplet::Alignment a )
{
  if( d->alignment == a ) return;
  d->alignment = a;
  alignmentChange( a );
}

// FIXME: Remove implementation for KDE 4
void K3PanelApplet::positionChange( K3PanelApplet::Position )
{
  orientationChange( orientation() );
  QResizeEvent e( size(), size() );
  resizeEvent( &e );
  popupDirectionChange( popupDirection() );
}

// FIXME: Remove for KDE 4
K3PanelApplet::Position K3PanelApplet::popupDirection()
{
    switch( d->position ) {
        case K3PanelApplet::Top:
            return K3PanelApplet::Down;
        case K3PanelApplet::Right:
            return K3PanelApplet::Left;
        case K3PanelApplet::Left:
            return K3PanelApplet::Right;
        case K3PanelApplet::Bottom:
        default:
          return K3PanelApplet::Up;
    }
}

Qt::Orientation K3PanelApplet::orientation() const
{
    if( d->position == K3PanelApplet::Top || d->position == K3PanelApplet::Bottom )
    {
        return Qt::Horizontal;
    }
    else
    {
        return Qt::Vertical;
    }
}

K3PanelApplet::Position K3PanelApplet::position() const
{
    return d->position;
}

K3PanelApplet::Alignment K3PanelApplet::alignment() const
{
    return d->alignment;
}

void K3PanelApplet::action( K3PanelApplet::Action a )
{
    if ( (a & K3PanelApplet::About) )
    {
        about();
    }
    if ( (a & K3PanelApplet::Help) )
    {
        help();
    }
    if ( (a & K3PanelApplet::Preferences) )
    {
        preferences();
    }
    if ( (a & K3PanelApplet::ReportBug) )
    {
        reportBug();
    }
}

const QMenu* K3PanelApplet::customMenu() const
{
    return d->customMenu;
}

void K3PanelApplet::setCustomMenu(const QMenu* menu)
{
    d->customMenu = menu;
}

void K3PanelApplet::watchForFocus(QWidget* widget, bool watch)
{
    if (!widget)
    {
        return;
    }

    if (watch)
    {
        if (!d->watchedForFocus.contains(widget))
        {
            d->watchedForFocus.append(widget);
            widget->installEventFilter(this);
        }
    }
    else if (!d->watchedForFocus.contains(widget))
    {
        d->watchedForFocus.removeAll(widget);
        widget->removeEventFilter(this);
    }
}

void K3PanelApplet::needsFocus(bool focus)
{
    if (focus == d->hasFocus)
    {
        return;
    }

    d->hasFocus = focus;
    emit requestFocus(focus);
}

bool K3PanelApplet::eventFilter(QObject *o, QEvent * e)
{
    if (!d->watchedForFocus.contains(o))
    {
        if (e->type() == QEvent::MouseButtonRelease ||
            e->type() == QEvent::FocusIn)
        {
            needsFocus(true);
        }
        else if (e->type() == QEvent::FocusOut)
        {
            needsFocus(false);
        }
    }

    return QFrame::eventFilter(o, e);
}

KSharedConfig::Ptr K3PanelApplet::sharedConfig() const
{
    return d->sharedConfig;
}
