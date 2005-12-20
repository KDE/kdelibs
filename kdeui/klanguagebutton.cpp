/*
 * Adds some methods for inserting languages.
 *
 * Copyright (c) 1999-2003 Hans Petter Bieker <bieker@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.trolltech.com/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#define INCLUDE_MENUITEM_DEF
#include <QMenu>
#include <qlayout.h>
#include <qpushbutton.h>
#include <QMenuItem>

#include "klanguagebutton.h"
#include "klanguagebutton.moc"

#include <kdebug.h>

static void checkInsertPos( QMenu *popup, const QString & str,
                            int &index )
{
  if ( index == -1 )
    return;

  int a = 0;
  QList <QAction*> actions=popup->actions();
  int b = actions.count();

  while ( a < b )
  {
    int w = ( a + b ) / 2;

    QAction *ac = actions[ w ];
    int j = str.localeAwareCompare( ac->text() );

    if ( j > 0 )
      a = w + 1;
    else
      b = w;
  }

  index = a; // it doesn't really matter ... a == b here.

  Q_ASSERT( a == b );
}

static QMenu * checkInsertIndex( QMenu *popup,
                                      const QStringList *tags, const QString &submenu )
{
  int pos = tags->indexOf( submenu );

  QMenu *pi = 0;
  if ( pos != -1 )
  {
    QList<QAction *> actions=popup->actions();
    if (pos<actions.count()) {
      QAction *a=actions[pos];
      pi=a->menu();
    }
  }
  if ( !pi )
    pi = popup;

  return pi;
}

class KLanguageButtonPrivate
{
public:
  QPushButton * button;
  bool staticText;
};

KLanguageButton::KLanguageButton( QWidget * parent )
  : QWidget( parent )
{
  init();
}

KLanguageButton::KLanguageButton( const QString & text, QWidget * parent )
  : QWidget( parent)
{
  init();

  setText(text);
}

void KLanguageButton::setText(const QString & text)
{
  d->staticText = true;
  d->button->setText(text);
  d->button->setIcon(QIcon()); // remove the icon
}

void KLanguageButton::init()
{
  m_current=QString();
  m_ids = new QStringList;
  m_popup = new QMenu( this );
  d = new KLanguageButtonPrivate;

  d->staticText = false;

  QHBoxLayout *layout = new QHBoxLayout(this, 0, 0);
  layout->setAutoAdd(true);
#ifdef __GNUC__
#warning check if the name is used by some code using KLanguageButton
#endif
  d->button = new QPushButton( this ); // HPB don't touch this!!

  clear();
}

KLanguageButton::~KLanguageButton()
{
  delete m_ids;

  delete d->button;
  delete d;
}


void KLanguageButton::insertLanguage( const QString& path, const QString& name,
                        const QString&, const QString &submenu, int index )
{
  QString output = name + QLatin1String( " (" ) + path +
                   QLatin1String( ")" );
#if 0
  // Nooooo ! Country != language
  QPixmap flag( locate( "locale", sub + path +
                QLatin1String( "/flag.png" ) ) );
#endif
  insertItem( output, path, submenu, index );
}

void KLanguageButton::insertItem( const QIcon& icon, const QString &text,
                                  const QString & id, const QString &submenu, int index )
{
  QMenu *pi = checkInsertIndex( m_popup, m_ids, submenu );
  checkInsertPos( pi, text, index );
  QAction *a=new QAction(icon,text,this);
  a->setData(id);
  if ( (index<(pi->actions().count()-1)) && (index>=0))
    pi->insertAction(a,(pi->actions())[index] );
  else
    pi->addAction(a);
  m_ids->append(id);
}

void KLanguageButton::insertItem( const QString &text, const QString & id,
                                  const QString &submenu, int index )
{
  insertItem( QIcon(), text, id, submenu, index );
}

void KLanguageButton::insertSeparator( const QString &submenu, int index )
{
  QMenu *pi = checkInsertIndex( m_popup, m_ids, submenu );
  if ( (index<(pi->actions().count()-1)) && (index>=0))
    pi->insertSeparator((pi->actions())[index] );
  else
    pi->addSeparator();
}

void KLanguageButton::insertSubmenu( const QIcon & icon,
                                     const QString &text, const QString &id,
                                     const QString &submenu, int index )
{
  QMenu *pi = checkInsertIndex( m_popup, m_ids, submenu );
  QMenu *p = new QMenu(text, pi );
  p->setIcon(icon);
  checkInsertPos( pi, text, index );
  if  ( (index<(pi->actions().count()-1)) && (index>=0))
    pi->insertMenu((pi->actions())[index],p );
  else
    pi->addMenu(p);

  m_ids->append(id);

  connect( p, SIGNAL( hovered( QAction* ) ),
           SLOT( slotHovered( QAction* ) ) );
  connect( p, SIGNAL( triggered( QAction* ) ), this,
           SLOT( slotTriggered( QAction* ) ) );
}

void KLanguageButton::insertSubmenu( const QString &text, const QString &id,
                                     const QString &submenu, int index )
{
  insertSubmenu(QIcon(), text, id, submenu, index);
}

void KLanguageButton::slotTriggered( QAction *a )
{
  //kdDebug() << "slotActivated" << index << endl;

  if (!a) return;

  setCurrentItem( a);

  // Forward event from popup menu as if it was emitted from this widget:
  emit activated( m_current);
}

void KLanguageButton::slotHovered( QAction *a )
{
  //kdDebug() << "slotHighlighted" << index << endl;

  emit ( highlighted(a->data().toString()) );
}

int KLanguageButton::count() const
{
  return m_ids->count();
}

void KLanguageButton::clear()
{
  m_ids->clear();

  m_popup->clear();

  d->button->setPopup( m_popup );

  connect( m_popup, SIGNAL( triggered( QAction* ) ),
           SLOT( slotTriggered( QAction* ) ) );
  connect( m_popup, SIGNAL( hovered( QAction* ) ),
           SLOT( slotHovered( QAction* ) ) );

  if ( !d->staticText )
  {
    d->button->setText( QString::null );
    d->button->setIcon( QIcon() );
  }
}

bool KLanguageButton::contains( const QString & id ) const
{
  return m_ids->contains( id ) > 0;
}

QString KLanguageButton::current() const
{
  if (m_current.isEmpty()) return "en";
  else return m_current;
}


QString KLanguageButton::id( int i ) const
{
  if ( i < 0 || i >= count() )
  {
    kdDebug() << "KLanguageButton::tag(), unknown tag " << i << endl;
    return QString::null;
  }
  return m_ids->at( i );
}



void KLanguageButton::setCurrentItem( QAction *a )
{
  if (!a->data().isValid()) return;
  m_current=a->data().toString();

  if ( !d->staticText )
  {
    d->button->setText( a->text() );
    QIcon icon = a->icon();
    d->button->setIcon( icon );
  }
}


static QAction *findAction(QMenu *menu ,const QString& data)
{
  foreach(QAction *a,menu->actions()) {
    if (a->data().toString().compare(data)==0) return a;
    else if (a->menu()) {
      QAction *tmp=findAction(a->menu(),data);
      if (tmp) return tmp;
    }
  }
  return 0;
}

void KLanguageButton::setCurrentItem( const QString & id )
{
  QAction *a;
  if (m_ids->count()==0) return;
  if (m_ids->indexOf(id)==-1)
    a=findAction(m_popup,(*m_ids)[0]);
  else
    a=findAction(m_popup,id);
  if (a) setCurrentItem(a);
}
