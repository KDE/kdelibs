/* This file is part of the KDE libraries
    Copyright (C) 1997 Mark Donohoe (donohoe@kde.org)
              (C) 1997,1998, 2000 Sven Radej (radej@kde.org)

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kdebug.h>
#include <kstatusbar.h>
#include <kconfig.h>
#include <kglobal.h>

// $Id$

KStatusBarLabel::KStatusBarLabel( const QString& text, int _id,
                                 KStatusBar *parent, const char *name) :
  QLabel( parent, name)
{
  id = _id;

  setText( text );

  // umm... Mosfet? Can you help here?

  // Warning: QStatusBar draws shaded rectangle around every item - which
  // IMHO is stupid.
  // So NoFrame|Plain is the best you get. the problem is that only in case of
  // StyledPanel|Something you get QFrame to call QStyle::drawPanel().

  setLineWidth  (0);
  setFrameStyle (QFrame::NoFrame);

  setAlignment( AlignHCenter | AlignVCenter );

  connect (this, SIGNAL(itemPressed(int)), parent, SIGNAL(pressed(int)));
  connect (this, SIGNAL(itemReleased(int)), parent, SIGNAL(released(int)));
}

void KStatusBarLabel::mousePressEvent (QMouseEvent *)
{
  emit itemPressed (id);
}

void KStatusBarLabel::mouseReleaseEvent (QMouseEvent *)
{
  emit itemReleased (id);
}


KStatusBar::KStatusBar( QWidget *parent, const char *name )
  : QStatusBar( parent, name )
{
  // make the size grip stuff configurable
  // ...but off by default (sven)
  KConfig *config = KGlobal::config();
  QString group(config->group());
  config->setGroup(QString::fromLatin1("StatusBar style"));
  bool grip_enabled = config->readBoolEntry(QString::fromLatin1("SizeGripEnabled"), false);
  setSizeGripEnabled(grip_enabled);
  config->setGroup(group);
}

KStatusBar::~KStatusBar ()
{
}

void KStatusBar::insertItem( const QString& text, int id, int stretch, bool permanent)
{
  KStatusBarLabel *l = new KStatusBarLabel (text, id, this);
  l->setFixedHeight(fontMetrics().height()+2);
  items.insert(id, l);
  addWidget (l, stretch, permanent);
  l->show();
}

void KStatusBar::removeItem (int id)
{
  KStatusBarLabel *l = items[id];
  if (l)
  {
    removeWidget (l);
    items.remove(id);
    delete l;
  }
  else
    kdDebug() << "KStatusBar::removeItem: bad item id: " << id << endl;
}

void KStatusBar::changeItem( const QString& text, int id )
{
  KStatusBarLabel *l = items[id];
  if (l)
  {
    clear();
    l->setText(text);
    reformat();
  }
  else
    kdDebug() << "KStatusBar::changeItem: bad item id: " << id << endl;
}

void KStatusBar::setItemAlignment (int id, int align)
{
  KStatusBarLabel *l = items[id];
  if (l)
  {
    l->setAlignment(align);
  }
  else
    kdDebug() << "KStatusBar::setItemAlignment: bad item id: " << id << endl;
}

void KStatusBar::setItemFixed(int id, int w)
{
  KStatusBarLabel *l = items[id];
  if (l)
  {
    if (w==-1)
      w=fontMetrics().boundingRect(l->text()).width()+3;

    l->setFixedSize(w, l->sizeHint().height());
  }
  else
    kdDebug() << "KStatusBar::setItemFixed: bad item id: " << id << endl;
}

#include "kstatusbar.moc"

//Eh!!!
//Eh what ? :)

