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
#include <kiconloader.h>
#include <kglobalsettings.h>

class KXMLGUIBuilderPrivate
{
public:
    KXMLGUIBuilderPrivate() {
    }
  ~KXMLGUIBuilderPrivate() {
  }

    QWidget *m_widget;

    QString tagMainWindow;
    QString tagMenuBar;
    QString tagMenu;
    QString tagToolBar;
    QString tagStatusBar;

    QString tagSeparator;
    QString tagTearOffHandle;

    QString attrName;
    QString attrLineSeparator;

    QString attrText1;
    QString attrText2;

    QString attrIcon;

    QString attrFullWidth;
    QString attrPosition;
    QString attrIndex;
    QString attrOffset;
    QString attrNewLine;
    QString attrIconText;
    QString attrIconSize;

    KInstance *m_instance;
    KXMLGUIClient *m_client;
};

KXMLGUIBuilder::KXMLGUIBuilder( QWidget *widget )
{
  d = new KXMLGUIBuilderPrivate;
  d->m_widget = widget;

  d->tagMainWindow = QString::fromLatin1( "mainwindow" );
  d->tagMenuBar = QString::fromLatin1( "menubar" );
  d->tagMenu = QString::fromLatin1( "menu" );
  d->tagToolBar = QString::fromLatin1( "toolbar" );
  d->tagStatusBar = QString::fromLatin1( "statusbar" );

  d->tagSeparator = QString::fromLatin1( "separator" );
  d->tagTearOffHandle = QString::fromLatin1( "tearoffhandle" );

  d->attrName = QString::fromLatin1( "name" );
  d->attrLineSeparator = QString::fromLatin1( "lineseparator" );

  d->attrText1 = QString::fromLatin1( "text" );
  d->attrText2 = QString::fromLatin1( "Text" );

  d->attrIcon = QString::fromLatin1( "icon" );
  d->attrFullWidth = QString::fromLatin1( "fullWidth" );
  d->attrPosition = QString::fromLatin1( "position" );
  d->attrIconText = QString::fromLatin1( "iconText" );
  d->attrIconSize = QString::fromLatin1( "iconSize" );
  d->attrIndex = QString::fromLatin1( "index" );
  d->attrOffset = QString::fromLatin1( "offset" );
  d->attrNewLine = QString::fromLatin1( "newline" );

  d->m_instance = 0;
  d->m_client = 0;
}

KXMLGUIBuilder::~KXMLGUIBuilder()
{
  delete d;
}

QStringList KXMLGUIBuilder::containerTags() const
{
  QStringList res;
  res << d->tagMainWindow << d->tagMenuBar << d->tagMenu << d->tagToolBar << d->tagStatusBar;
  return res;
}

QWidget *KXMLGUIBuilder::createContainer( QWidget *parent, int index, const QDomElement &element, const QByteArray &containerStateBuffer, int &id )
{
  id = -1;

  if ( element.tagName().lower() == d->tagMainWindow )
  {
    KTMainWindow *mainwindow = 0;
    if ( d->m_widget->inherits( "KTMainWindow" ) )
      mainwindow = static_cast<KTMainWindow *>(d->m_widget);

    return mainwindow;
  }

  if ( element.tagName().lower() == d->tagMenuBar )
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

  if ( element.tagName().lower() == d->tagMenu )
  {
    QPopupMenu *popup = new QPopupMenu( d->m_widget, element.attribute( d->attrName ).utf8());
    popup->setFont(KGlobalSettings::menuFont());

    QString i18nText;
    QCString text = element.namedItem( d->attrText1 ).toElement().text().utf8();
    if ( text.isEmpty() ) // try with capital T
      text = element.namedItem( d->attrText2 ).toElement().text().utf8();

    if ( text.isEmpty() ) // still no luck
      i18nText = i18n( "No text!" );
    else
      i18nText = i18n( text );
		
    QString icon = element.attribute( d->attrIcon );
    QPixmap pix;

    if ( !icon.isEmpty() )
    {
      KInstance *instance = d->m_instance;
      if ( !instance )
        instance = KGlobal::instance();

      pix = SmallIcon( icon, 16, KIcon::DefaultState, instance );
    }

    if ( parent && parent->inherits( "KMenuBar" ) )
    {
      if ( !icon.isEmpty() )
        id = static_cast<KMenuBar *>(parent)->insertItem( pix, i18nText, popup, -1, index );
      else
        id = static_cast<KMenuBar *>(parent)->insertItem( i18nText, popup, -1, index );
    }
    else if ( parent && parent->inherits( "QPopupMenu" ) )
    {
      if ( !icon.isEmpty() )
        id = static_cast<QPopupMenu *>(parent)->insertItem( pix, i18nText, popup, -1, index );
      else
        id = static_cast<QPopupMenu *>(parent)->insertItem( i18nText, popup, -1, index );
    }

    return popup;
  }

  if ( element.tagName().lower() == d->tagToolBar )
  {
    bool honor = (element.attribute( d->attrName ) == "mainToolBar");

    KToolBar *bar = 0;
    if ( d->m_widget->inherits( "KTMainWindow" ) )
	bar = new KToolBar( ( (KTMainWindow*)d->m_widget ), element.attribute( d->attrName ).utf8(), honor);
    else
	bar = new KToolBar( d->m_widget, element.attribute( d->attrName ).utf8(), honor);

    QCString text = element.namedItem( d->attrText1 ).toElement().text().utf8();
    if (text.isEmpty())  // try with capital T
      text = element.namedItem( d->attrText2 ).toElement().text().utf8();

    if (!text.isEmpty())
      bar->setText( i18n( text ) );

    if ( d->m_widget->inherits( "KTMainWindow" ) )
    {
      KTMainWindow *mw = static_cast<KTMainWindow *>( d->m_widget );
      mw->addToolBar( bar );
      if ( d->m_client && !d->m_client->xmlFile().isEmpty() )
        bar->setXMLGUIClient( d->m_client );
    }

    QCString attrFullWidth = element.attribute( d->attrFullWidth ).lower().latin1();
    QCString attrPosition = element.attribute( d->attrPosition ).lower().latin1();
    QCString attrIconText = element.attribute( d->attrIconText ).lower().latin1();
    QString attrIconSize = element.attribute( d->attrIconSize ).lower();
    QString attrIndex = element.attribute( d->attrIndex ).lower();
    QString attrOffset = element.attribute( d->attrOffset ).lower();
    QString attrNewLine = element.attribute( d->attrNewLine ).lower();

    if ( honor || ( !attrFullWidth.isEmpty() && attrFullWidth == "true" ) )
      bar->setFullSize( true );
    else
      bar->setFullSize( false );

    QMainWindow::ToolBarDock dock;
    int index = 0, offset = -1;
    bool nl = FALSE;
    
    if ( !attrPosition.isEmpty() && containerStateBuffer.size() == 0 ) {
	if ( attrPosition == "top" )
	    dock = QMainWindow::Top;
	else if ( attrPosition == "left" )
	    dock = QMainWindow::Left;
	else if ( attrPosition == "right" )
	    dock = QMainWindow::Right;
	else if ( attrPosition == "bottom" )
	    dock = QMainWindow::Bottom;
	else if ( attrPosition == "floating" )
	    dock = QMainWindow::TornOff;
	else if ( attrPosition == "flat" )
	    dock = QMainWindow::Minimized;
    }

    if ( !attrIndex.isEmpty() && containerStateBuffer.size() == 0 )
	index = attrIndex.toInt();
    if ( !attrOffset.isEmpty() && containerStateBuffer.size() == 0 )
	offset = attrOffset.toInt();
    if ( !attrNewLine.isEmpty() && containerStateBuffer.size() == 0 )
	nl = attrNewLine == "true" ? TRUE : FALSE;
    
    if ( d->m_widget->inherits( "QMainWindow") ) 
	( (QMainWindow*)d->m_widget )->moveToolBar( bar, dock, index, nl, offset );
    bar->setBarPos( (KToolBar::BarPosition)dock );
    
    if ( !attrIconText.isEmpty() && containerStateBuffer.size() == 0 )
    {
      if ( attrIconText == "icontextright" )
        bar->setIconText( KToolBar::IconTextRight );
      else if ( attrIconText == "textonly" )
        bar->setIconText( KToolBar::TextOnly );
      else if ( attrIconText == "icontextbottom" )
        bar->setIconText( KToolBar::IconTextBottom );
      else if ( attrIconText == "icononly" )
        bar->setIconText( KToolBar::IconOnly );
    }

    if ( !attrIconSize.isEmpty() && containerStateBuffer.size() == 0 )
    {
      bar->setIconSize( attrIconSize.toInt() );
    }

    if ( containerStateBuffer.size() > 0 )
    {
      QDataStream stream( containerStateBuffer, IO_ReadOnly );
      QVariant iconText, barPos, fullSize, iconSize;
      stream >> iconText >> barPos >> fullSize >> iconSize;
      bar->setProperty( "iconText", iconText );
      bar->setProperty( "barPos", barPos );
      bar->setProperty( "fullSize", fullSize );
      bar->setProperty( "iconSize", iconSize );
    }

    bar->show();

    return bar;
  }

  if ( element.tagName().lower() == d->tagStatusBar )
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
      static_cast<KMenuBar *>(parent)->removeItem( id );
    else if ( parent->inherits( "QPopupMenu" ) )
      static_cast<QPopupMenu *>(parent)->removeItem( id );

    delete container;
  }
  else if ( container->inherits( "KToolBar" ) )
  {
    QDataStream stream( stateBuff, IO_WriteOnly );
    stream << container->property( "iconText" ) << container->property( "barPos" ) << container->property( "fullSize" ) << container->property( "iconSize" );
    KToolBar *tb = static_cast<KToolBar *>( container );
    tb->saveState();
    delete tb;
  }
  else if ( container->inherits( "KStatusBar" ) )
  {
    if ( d->m_widget->inherits( "KTMainWindow" ) )
      static_cast<KTMainWindow *>(d->m_widget)->enableStatusBar( KStatusBar::Hide );
    else
      delete static_cast<KStatusBar *>(container);
  }

  return stateBuff;
}

QStringList KXMLGUIBuilder::customTags() const
{
  QStringList res;
  res << d->tagSeparator << d->tagTearOffHandle;
  return res;
}

int KXMLGUIBuilder::createCustomElement( QWidget *parent, int index, const QDomElement &element )
{
  if ( element.tagName().lower() == d->tagSeparator )
  {
    if ( parent->inherits( "QPopupMenu" ) )
      return static_cast<QPopupMenu *>(parent)->insertSeparator( index );
    else if ( parent->inherits( "QMenuBar" ) )
       return static_cast<QMenuBar *>(parent)->insertSeparator( index );
    else if ( parent->inherits( "KToolBar" ) )
    {
      KToolBar *bar = static_cast<KToolBar *>( parent );

      bool isLineSep = false;

      QDomNamedNodeMap attributes = element.attributes();
      unsigned int i = 0;
      for (; i < attributes.length(); i++ )
      {
        QDomAttr attr = attributes.item( i ).toAttr();
	
        if ( attr.name().lower() == d->attrLineSeparator &&
	     attr.value().lower() == QString::fromLatin1("true") )
	{
	  isLineSep = true;
	  break;
	}
      }

      if ( isLineSep )
        return bar->insertLineSeparator( index );

      return bar->insertSeparator( index );
    }
  }
  else if ( element.tagName().lower() == d->tagTearOffHandle )
  {
    if ( parent->inherits( "QPopupMenu" ) )
      return static_cast<QPopupMenu *>(parent)->insertTearOffHandle( -1, index );
  }
  return 0;
}

void KXMLGUIBuilder::removeCustomElement( QWidget *parent, int id )
{
  if ( parent->inherits( "QPopupMenu" ) )
    static_cast<QPopupMenu *>(parent)->removeItem( id );
  else if ( parent->inherits( "QMenuBar" ) )
    static_cast<QMenuBar *>(parent)->removeItem( id );
  else if ( parent->inherits( "KToolBar" ) )
    static_cast<KToolBar *>(parent)->removeItem( id );
}

KXMLGUIClient *KXMLGUIBuilder::builderClient() const
{
  return d->m_client;
}

void KXMLGUIBuilder::setBuilderClient( KXMLGUIClient *client )
{
  d->m_client = client;
  if ( client )
    setBuilderInstance( client->instance() );
}

KInstance *KXMLGUIBuilder::builderInstance() const
{
  return d->m_instance;
}

void KXMLGUIBuilder::setBuilderInstance( KInstance *instance )
{
  d->m_instance = instance;
}
