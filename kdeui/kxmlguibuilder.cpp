/* This file is part of the KDE project
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>
                      David Faure <faure@kde.org>

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

#include "kxmlguibuilder.h"
#include "kmenubar.h"
#include "ktoolbar.h"
#include "kstatusbar.h"
#include "ktmainwindow.h"
#include <klocale.h>

class KXMLGUIBuilderPrivate
{
public:
  KXMLGUIBuilderPrivate()
  {
  }
  ~KXMLGUIBuilderPrivate()
  {
  }

  QWidget *m_widget;
};

KXMLGUIBuilder::KXMLGUIBuilder( QWidget *widget )
{
  d = new KXMLGUIBuilderPrivate;
  d->m_widget = widget;
}

KXMLGUIBuilder::~KXMLGUIBuilder()
{
  delete d;
}

QWidget *KXMLGUIBuilder::createContainer( QWidget *parent, int index, const QDomElement &element, const QByteArray &containerStateBuffer, int &id )
{
  id = -1;

  if ( element.tagName().lower() == "menubar" )
  {
    KMenuBar *bar;

    if ( d->m_widget->inherits( "KTMainWindow" ) )
      bar = static_cast<KTMainWindow *>(d->m_widget)->menuBar();
    else
      bar = new KMenuBar( d->m_widget );

    if ( !bar->isVisible() )
      bar->show();
    return bar;
  }

  if ( element.tagName().lower() == "menu" && parent )
  {
    QPopupMenu *popup = new QPopupMenu( d->m_widget, element.attribute( "name" ).utf8());
    popup->setFont(KGlobal::menuFont());

    QString text = i18n(element.namedItem( "text" ).toElement().text().utf8());
    if (text.isEmpty())  // try with capital T
      text = i18n(element.namedItem( "Text" ).toElement().text().utf8());
    if (text.isEmpty())  // still no luck
      text = i18n("No text!");

    if ( parent->inherits( "KMenuBar" ) )
      id = ((KMenuBar *)parent)->insertItem( text, popup, -1, index );
    else if ( parent->inherits( "QPopupMenu" ) )
      id = ((QPopupMenu *)parent)->insertItem( text, popup, -1, index );

    return popup;
  }

  if ( element.tagName().lower() == "toolbar" )
  {
    bool honor = (element.attribute( "name" ) == "mainToolBar") ? true : false;
    KToolBar *bar = new KToolBar( d->m_widget, element.attribute( "name" ).utf8(), honor);

    if ( d->m_widget->inherits( "KTMainWindow" ) )
      static_cast<KTMainWindow *>(d->m_widget)->addToolBar( bar );

    QString attrFullWidth = element.attribute( "fullWidth" ).lower();
    QString attrPosition = element.attribute( "position" ).lower();

    if ( honor || ( !attrFullWidth.isEmpty() && attrFullWidth == "true" ) )
      bar->setFullWidth( true );
    else
      bar->setFullWidth( false );

    if ( !attrPosition.isEmpty() && containerStateBuffer.size() == 0 )
    {
      if ( attrPosition == "top" )
        bar->setBarPos( KToolBar::Top );
      else if ( attrPosition == "left" )
        bar->setBarPos( KToolBar::Left );
      else if ( attrPosition == "right" )
	bar->setBarPos( KToolBar::Right );
      else if ( attrPosition == "bottom" )
	bar->setBarPos( KToolBar::Bottom );
      else if ( attrPosition == "floating" )
	bar->setBarPos( KToolBar::Floating );
      else if ( attrPosition == "flat" )
	bar->setBarPos( KToolBar::Flat );
    }
    else if ( containerStateBuffer.size() > 0 )
    {
      QDataStream stream( containerStateBuffer, IO_ReadOnly );
      QVariant iconText, barPos, fullSize;
      stream >> iconText >> barPos >> fullSize;
      bar->setProperty( "iconText", iconText );
      bar->setProperty( "barPos", barPos );
      bar->setProperty( "fullSize", fullSize );
      /*
      Q_INT32 i;
      stream >> i;
      bar->setBarPos( (KToolBar::BarPosition)i );
      stream >> i;
      bar->setIconText( (KToolBar::IconText)i );
      */
    }

    bar->show();

    return bar;
  }

  if ( element.tagName().lower() == "statusbar" )
  {
    if ( d->m_widget->inherits( "KTMainWindow" ) )
    {
      KTMainWindow *mainWin = static_cast<KTMainWindow *>(d->m_widget);
      mainWin->enableStatusBar( KStatusBar::Show );
      return mainWin->statusBar();
    }
    KStatusBar *bar = new KStatusBar( d->m_widget );
    return bar;
  }

  return 0L;
}

QByteArray KXMLGUIBuilder::removeContainer( QWidget *container, QWidget *parent, int id )
{
  // Warning parent can be 0L
  QByteArray stateBuff;

  if ( container->inherits( "QPopupMenu" ) )
  {
    if ( parent->inherits( "KMenuBar" ) )
      ((KMenuBar *)parent)->removeItem( id );
    else if ( parent->inherits( "QPopupMenu" ) )
      ((QPopupMenu *)parent)->removeItem( id );

    delete container;
  }
  else if ( container->inherits( "KToolBar" ) )
  {
    QDataStream stream( stateBuff, IO_WriteOnly );
    //    stream << (int)((KToolBar *)container)->barPos() << (int)((KToolBar *)container)->iconText();
    stream << container->property( "iconText" ) << container->property( "barPos" ) << container->property( "fullSize" );
    delete (KToolBar *)container;
  }
  else if ( container->inherits( "KStatusBar" ) )
  {
    if ( d->m_widget->inherits( "KTMainWindow" ) )
      static_cast<KTMainWindow *>(d->m_widget)->enableStatusBar( KStatusBar::Hide );
    else
      delete (KStatusBar *)container;
  }

  return stateBuff;
}

int KXMLGUIBuilder::insertSeparator( QWidget *parent, int index )
{
  if ( parent->inherits( "QPopupMenu" ) )
    return ((QPopupMenu *)parent)->insertSeparator( index );
  else if ( parent->inherits( "QMenuBar" ) )
    return ((QMenuBar *)parent)->insertSeparator( index );
  else if ( parent->inherits( "KToolBar" ) )
    return ((KToolBar *)parent)->insertSeparator( index );

  return 0;
}

void KXMLGUIBuilder::removeSeparator( QWidget *parent, int id )
{
  if ( parent->inherits( "QPopupMenu" ) )
    ((QPopupMenu *)parent)->removeItem( id );
  else if ( parent->inherits( "QMenuBar" ) )
    ((QMenuBar *)parent)->removeItem( id );
  else if ( parent->inherits( "KToolBar" ) )
    ((KToolBar *)parent)->removeItem( id );
}
