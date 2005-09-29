/*
 * klanguagebutton.cpp - Adds some methods for inserting languages.
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
#include <qpopupmenu.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include "klanguagebutton.h"
#include "klanguagebutton.moc"

#include <kdebug.h>

static void checkInsertPos( QPopupMenu *popup, const QString & str,
                            int &index )
{
  if ( index == -1 )
    return;

  int a = 0;
  int b = popup->count();
  while ( a < b )
  {
    int w = ( a + b ) / 2;

    int id = popup->idAt( w );
    int j = str.localeAwareCompare( popup->text( id ) );

    if ( j > 0 )
      a = w + 1;
    else
      b = w;
  }

  index = a; // it doesn't really matter ... a == b here.

  Q_ASSERT( a == b );
}

static QPopupMenu * checkInsertIndex( QPopupMenu *popup,
                                      const QStringList *tags, const QString &submenu )
{
  int pos = tags->findIndex( submenu );

  QPopupMenu *pi = 0;
  if ( pos != -1 )
  {
    QMenuItem *p = popup->findItem( pos );
    pi = p ? p->popup() : 0;
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

KLanguageButton::KLanguageButton( QWidget * parent, const char *name )
  : QWidget( parent, name )
{
  init(name);
}

KLanguageButton::KLanguageButton( const QString & text, QWidget * parent, const char *name )
  : QWidget( parent, name )
{
  init(name);

  setText(text);
}

void KLanguageButton::setText(const QString & text)
{
  d->staticText = true;
  d->button->setText(text);
  d->button->setIconSet(QIconSet()); // remove the icon
}

void KLanguageButton::init(const char * name)
{
  m_current = 0;
  m_ids = new QStringList;
  m_popup = 0;
  m_oldPopup = 0;
  d = new KLanguageButtonPrivate;

  d->staticText = false;

  QHBoxLayout *layout = new QHBoxLayout(this, 0, 0);
  layout->setAutoAdd(true);
  d->button = new QPushButton( this, name ); // HPB don't touch this!!

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
  QString output = name + QString::fromLatin1( " (" ) + path +
                   QString::fromLatin1( ")" );
#if 0
  // Nooooo ! Country != language
  QPixmap flag( locate( "locale", sub + path +
                QString::fromLatin1( "/flag.png" ) ) );
#endif
  insertItem( output, path, submenu, index );
}

void KLanguageButton::insertItem( const QIconSet& icon, const QString &text,
                                  const QString & id, const QString &submenu, int index )
{
  QPopupMenu *pi = checkInsertIndex( m_popup, m_ids, submenu );
  checkInsertPos( pi, text, index );
  pi->insertItem( icon, text, count(), index );
  m_ids->append( id );
}

void KLanguageButton::insertItem( const QString &text, const QString & id,
                                  const QString &submenu, int index )
{
  insertItem( QIconSet(), text, id, submenu, index );
}

void KLanguageButton::insertSeparator( const QString &submenu, int index )
{
  QPopupMenu *pi = checkInsertIndex( m_popup, m_ids, submenu );
  pi->insertSeparator( index );
  m_ids->append( QString::null );
}

void KLanguageButton::insertSubmenu( const QIconSet & icon,
                                     const QString &text, const QString &id,
                                     const QString &submenu, int index )
{
  QPopupMenu *pi = checkInsertIndex( m_popup, m_ids, submenu );
  QPopupMenu *p = new QPopupMenu( pi );
  checkInsertPos( pi, text, index );
  pi->insertItem( icon, text, p, count(), index );
  m_ids->append( id );
  connect( p, SIGNAL( activated( int ) ),
           SLOT( slotActivated( int ) ) );
  connect( p, SIGNAL( highlighted( int ) ), this,
           SLOT( slotHighlighted( int ) ) );
}

void KLanguageButton::insertSubmenu( const QString &text, const QString &id,
                                     const QString &submenu, int index )
{
  insertSubmenu(QIconSet(), text, id, submenu, index);
}

void KLanguageButton::slotActivated( int index )
{
  //kdDebug() << "slotActivated" << index << endl;

  setCurrentItem( index );

  // Forward event from popup menu as if it was emitted from this widget:
  QString id = *m_ids->at( index );
  emit activated( id );
}

void KLanguageButton::slotHighlighted( int index )
{
  //kdDebug() << "slotHighlighted" << index << endl;

  QString id = *m_ids->at( index );
  emit ( highlighted(id) );
}

int KLanguageButton::count() const
{
  return m_ids->count();
}

void KLanguageButton::clear()
{
  m_ids->clear();

  delete m_oldPopup;
  m_oldPopup = m_popup;
  m_popup = new QPopupMenu( this );

  d->button->setPopup( m_popup );

  connect( m_popup, SIGNAL( activated( int ) ),
           SLOT( slotActivated( int ) ) );
  connect( m_popup, SIGNAL( highlighted( int ) ),
           SLOT( slotHighlighted( int ) ) );

  if ( !d->staticText )
  {
    d->button->setText( QString::null );
    d->button->setIconSet( QIconSet() );
  }
}

bool KLanguageButton::contains( const QString & id ) const
{
  return m_ids->contains( id ) > 0;
}

QString KLanguageButton::current() const
{
  return *m_ids->at( currentItem() );
}


QString KLanguageButton::id( int i ) const
{
  if ( i < 0 || i >= count() )
  {
    kdDebug() << "KLanguageButton::tag(), unknown tag " << i << endl;
    return QString::null;
  }
  return *m_ids->at( i );
}


int KLanguageButton::currentItem() const
{
  return m_current;
}

void KLanguageButton::setCurrentItem( int i )
{
  if ( i < 0 || i >= count() )
    return;
  m_current = i;

  if ( !d->staticText )
  {
    d->button->setText( m_popup->text( m_current ) );
    QIconSet *icon = m_popup->iconSet( m_current );
    if ( icon )
      d->button->setIconSet( *icon );
    else
      d->button->setIconSet( QIconSet() );
  }
}

void KLanguageButton::setCurrentItem( const QString & id )
{
  int i = m_ids->findIndex( id );
  if ( id.isNull() )
    i = 0;
  if ( i != -1 )
    setCurrentItem( i );
}
