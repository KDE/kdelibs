/*  This file is part of the KDE libraries
    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2 
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kapplication.h>

#include "kactivelabel.h"

KActiveLabel::KActiveLabel(QWidget * parent, const char * name)
 : QTextBrowser(parent, name)
{
   init();
}

KActiveLabel::KActiveLabel(const QString &text, QWidget * parent, const char * name)
 : QTextBrowser(parent, name)
{
   init();
   setText(text);
}

void KActiveLabel::init()
{
   setTextFormat(Qt::RichText);
   setVScrollBarMode(QScrollView::AlwaysOff);
   setHScrollBarMode(QScrollView::AlwaysOff);
   setFrameStyle(QFrame::NoFrame);
   paletteChanged();
   
   connect(this, SIGNAL(linkClicked(const QString &)), 
           this, SLOT(openLink(const QString &)));
   if (kapp)
   {
      connect(kapp, SIGNAL(kdisplayPaletteChanged()), 
              this, SLOT(paletteChanged()));
   }
}

void KActiveLabel::paletteChanged()
{
   QPalette p = kapp ? kapp->palette() : palette();
   p.setBrush(QColorGroup::Base, p.brush(QPalette::Normal, QColorGroup::Background));
   p.setColor(QColorGroup::Text, p.color(QPalette::Normal, QColorGroup::Foreground));
   setPalette(p);
}

void KActiveLabel::openLink(const QString & link)
{
   QStringList args;
   args << "exec" << link;
   kapp->kdeinitExec("kfmclient", args);
}

void KActiveLabel::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kactivelabel.moc"
