/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#include "kaction.h"

#include <ktoolbar.h>
#include <ktoolbarbutton.h>
#include <kmenubar.h>
#include <qobjectlist.h>
#include <kapp.h>
#include <kaccel.h>
#include <kconfig.h>
#include <kstdaccel.h>
#include <kurl.h>
#include <qtl.h>
#include <qptrdict.h>
#include <qfontdatabase.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kmainwindow.h>
#include <kglobalsettings.h>
#include <kcombobox.h>
#include <kfontcombo.h>
#include <kdebug.h>
#include <assert.h>

static QFontDatabase *fontDataBase = 0;

static void cleanupFontDatabase()
{
    delete fontDataBase;
    fontDataBase = 0;
}

static void get_fonts( QStringList &lst )
{
    if ( !fontDataBase ) {
        fontDataBase = new QFontDatabase();
        qAddPostRoutine( cleanupFontDatabase );
    }
    lst.clear();
    QStringList families = fontDataBase->families();
    for ( QStringList::Iterator it = families.begin(); it != families.end(); ++it )
    {
        QString family = *it;
        if ( family. contains('-') ) // remove foundry
            family = family.right( family.length() - family.find('-' ) - 1);
        if ( !lst.contains( family ) )
            lst.append( family );
    }
    lst.sort();
}

static QValueList<int> get_standard_font_sizes()
{
    if ( !fontDataBase ) {
        fontDataBase = new QFontDatabase();
        qAddPostRoutine( cleanupFontDatabase );
    }
    return fontDataBase->standardSizes();
}

int KAction::getToolButtonID()
{
    static int toolbutton_no = -2;
    return toolbutton_no--;
}

class KAction::KActionPrivate
{
public:
  KActionPrivate()
  {
    m_kaccel    = 0;
    m_bIconSet  = false;
    m_bIconSetNotYetLoaded=false;
    m_enabled   = true;
    m_accel     = 0;
  }
  ~KActionPrivate()
  {
  }
  KAccel *m_kaccel;

  QString m_iconName;
  QString m_text;
  QString m_plainText;
  QString m_whatsThis;
  QString m_groupText;
  QString m_group;
  QString m_toolTip;

  QPixmap m_pixmap;
  QIconSet m_iconSetDemand;
  bool m_bIconSet:1;
  bool m_bIconSetNotYetLoaded:1;
  bool m_enabled:1;
  int m_accel;

  struct Container
  {
    Container() { m_container = 0; m_representative = 0; m_id = 0; }
    Container( const Container& s ) { m_container = s.m_container;
                                      m_id = s.m_id; m_representative = s.m_representative; }
    QWidget* m_container;
    int m_id;
    QWidget* m_representative;
  };

  QValueList<Container> m_containers;
};

KAction::KAction( const QString& text, int accel, QObject* parent,
                  const char* name )
 : QObject( parent, name )
{
    d = new KActionPrivate;

    m_parentCollection = parentCollection();
    if ( m_parentCollection ) {
        d->m_accel = accel;      // default key binding
        m_parentCollection->insert( this );
    }

    setAccel( accel );
    setText( text );
}

KAction::KAction( const QString& text, int accel, const QObject* receiver,
                  const char* slot, QObject* parent, const char* name )
 : QObject( parent, name )
{
    d = new KActionPrivate;

    m_parentCollection = parentCollection();
    if ( m_parentCollection ) {
        d->m_accel = accel;      // default key binding
        m_parentCollection->insert( this );
    }

    setAccel( accel );
    setText( text );

    if ( receiver )
    connect( this, SIGNAL( activated() ), receiver, slot );
}

KAction::KAction( const QString& text, const QIconSet& pix, int accel,
                  QObject* parent, const char* name )
 : QObject( parent, name )
{
    d = new KActionPrivate;

    m_parentCollection = parentCollection();
    if ( m_parentCollection ) {
        d->m_accel = accel;      // default key binding
        m_parentCollection->insert( this );
    }

    setAccel( accel );
    setText( text );
    setIconSet( pix );
}

KAction::KAction( const QString& text, const QString& pix, int accel,
                  QObject* parent, const char* name )
: QObject( parent, name )
{
    d = new KActionPrivate;

    m_parentCollection = parentCollection();
    if ( m_parentCollection ) {
        d->m_accel = accel;      // default key binding
        m_parentCollection->insert( this );
    }

    setText( text );
    setAccel( accel );

    d->m_iconName=pix;
    d->m_bIconSetNotYetLoaded=true;
    d->m_bIconSet=true;
}

KAction::KAction( const QString& text, const QIconSet& pix, int accel,
                  const QObject* receiver, const char* slot, QObject* parent,
                  const char* name )
 : QObject( parent, name )
{
    d = new KActionPrivate;

    m_parentCollection = parentCollection();
    if ( m_parentCollection ) {
        d->m_accel = accel;      // default key binding
        m_parentCollection->insert( this );
    }

    setAccel( accel );
    setText( text );
    setIconSet( pix );

    if ( receiver )
      connect( this, SIGNAL( activated() ), receiver, slot );
}

KAction::KAction( const QString& text, const QString& pix, int accel,
                  const QObject* receiver, const char* slot, QObject* parent,
                  const char* name )
  : QObject( parent, name )
{
    d = new KActionPrivate;

    m_parentCollection = parentCollection();
    if ( m_parentCollection ) {
        d->m_accel = accel;      // default key binding
        m_parentCollection->insert( this );
    }

    setAccel( accel );
    setText( text );

    d->m_iconName=pix;
    d->m_bIconSetNotYetLoaded=true;
    d->m_bIconSet=true;

    if ( receiver )
      connect( this, SIGNAL( activated() ), receiver, slot );
}

KAction::KAction( QObject* parent, const char* name )
 : QObject( parent, name )
{
    d = new KActionPrivate;
    m_parentCollection = parentCollection();
    if ( m_parentCollection ) {
        m_parentCollection->insert( this );
    }
}

KAction::~KAction()
{
    if (d->m_kaccel)
      unplugAccel();
    if ( m_parentCollection )
      m_parentCollection->take( this );

    delete d; d = 0;
}

bool KAction::isPlugged() const
{
  if (d->m_kaccel)
    return true;
  else
    return ( containerCount() > 0 );
}

bool KAction::isPlugged( const QWidget *container, int id ) const
{
  int i = findContainer( container );

  if ( i == -1 )
    return false;

  if ( itemId( i ) != id )
    return false;

  return true;
}

bool KAction::isPlugged( const QWidget *container, const QWidget *_representative ) const
{
  int i = findContainer( container );

  if ( i == -1 )
    return false;

  if ( representative( i ) != _representative )
    return false;

  return true;
}

void KAction::setAccel( int a )
{
  d->m_accel = a;

  if ( m_parentCollection )
  {
    KKeyEntryMap& keys = m_parentCollection->keyMap();
    if (keys.contains(name())) {
        keys[name()].aCurrentKeyCode = a;
    }
  }

  if (d->m_kaccel)
      d->m_kaccel->updateItem(name(), a);
  int len = containerCount();
  for( int i = 0; i < len; ++i )
    setAccel( i, a );
}

void KAction::setAccel( int i, int a )
{
  QWidget* w = container( i );
  if ( w->inherits( "QPopupMenu" ) )
    static_cast<QPopupMenu*>(w)->setAccel( a, itemId( i ) );
  else if ( w->inherits( "QMenuBar" ) )
    static_cast<QMenuBar*>(w)->setAccel( a, itemId( i ) );
}

int KAction::accel() const
{
  return d->m_accel;
}

void KAction::setGroup( const QString& grp )
{
  d->m_group = grp;

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    setGroup( i, grp );
}

void KAction::setGroup( int, const QString& )
{
  // DO SOMETHING
}

QString KAction::group() const
{
  return d->m_group;
}

bool KAction::isEnabled() const
{
  return d->m_enabled;
}

void KAction::setToolTip( const QString& tt )
{
  d->m_toolTip = tt;

  int len = containerCount();
  for( int i = 0; i < len; ++i )
  {
    setToolTip( i, tt );
    setStatusText( i, tt );
  }
}

void KAction::setToolTip( int i, const QString& tt )
{
  QWidget *w = container( i );

  if ( w->inherits( "KToolBar" ) )
     QToolTip::add( static_cast<KToolBar*>(w)->getWidget( itemId( i ) ), tt );
}

QString KAction::toolTip() const
{
  return d->m_toolTip;
}

// remove in KDE 3.0
void KAction::setStatusText( const QString &text )
{
  setToolTip(text);
}

void KAction::setStatusText( int, const QString & )
{
  // ### remove in KDE 3.0
}

// remove in KDE 3.0
QString KAction::statusText() const
{
  return toolTip();
}

int KAction::plug( QWidget *w, int index )
{
  if (w == 0) {
	kdWarning() << "KAction::plug called with 0 argument\n";
 	return -1;
  }
  // Plug into the KMainWindow accel so that keybindings work for
  // actions that are only plugged into a toolbar, and in case of
  // hiding the menubar.
  if (!d->m_kaccel && d->m_accel) // only if not already plugged into a kaccel, and only if there is a shortcut !
  {
    // Note: topLevelWidget() stops too early, we can't use it.
    QWidget * tl = w;
    while ( tl->parentWidget() )
        tl = tl->parentWidget();

    KMainWindow * mw = dynamic_cast<KMainWindow *>(tl); // try to see if it's a kmainwindow
    if (mw)
      plugAccel( mw->accel() );
//    else
//      kdDebug() << "KAction::plug: Toplevel widget isn't a KMainWindow, can't plug accel. " << tl << endl;
  }

  if ( w->inherits("QPopupMenu") )
  {
    QPopupMenu* menu = static_cast<QPopupMenu*>( w );
    int id;

    if (!inherits("KToggleAction")) {
        if ( !d->m_pixmap.isNull() ) {
            id = menu->insertItem( d->m_pixmap, this, SLOT( slotActivated() ),
                                   d->m_accel, -1, index );
        } else {
            if ( d->m_bIconSet )
                id = menu->insertItem( iconSet(), d->m_text, this,//dsweet
                                       SLOT( slotActivated() ), d->m_accel,
                                       -1, index );
            else
                id = menu->insertItem( d->m_text, this,
                                       SLOT( slotActivated() ),  //dsweet
                                       d->m_accel, -1, index );
        }
    } else {
        // toggle actions do not have a pixmap or icon in the menu
        // Lotzi B.
        id = menu->insertItem( d->m_text, this,
                               SLOT( slotActivated() ),
                               d->m_accel, -1, index );
    }

    // call setItemEnabled only if the item really should be disabled,
    // because that method is slow and the item is per default enabled
    if ( !d->m_enabled )
        menu->setItemEnabled( id, false );

    if ( !d->m_whatsThis.isEmpty() )
        menu->setWhatsThis( id, d->m_whatsThis );

    addContainer( menu, id );
    connect( menu, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    if ( m_parentCollection )
      m_parentCollection->connectHighlight( menu, this );

    return d->m_containers.count() - 1;
  }
  else if ( w->inherits( "KToolBar" ) )
  {
    KToolBar *bar = static_cast<KToolBar *>( w );

    int id_ = getToolButtonID();
    if ( icon().isEmpty() && d->m_bIconSet )
    {
      bar->insertButton( iconSet().pixmap(), id_, SIGNAL( clicked() ), this,
                         SLOT( slotActivated() ),
                         d->m_enabled, d->m_plainText, index );
    }
    else
    {
      KInstance *instance;
      if ( m_parentCollection )
        instance = m_parentCollection->instance();
      else
        instance = KGlobal::instance();

      bar->insertButton( d->m_iconName, id_, SIGNAL( clicked() ), this,
                         SLOT( slotActivated() ), d->m_enabled, d->m_plainText,
                         index, instance );

      bar->getButton( id_ )->setName( QCString("toolbutton_")+name() );
    }

    if ( !d->m_whatsThis.isEmpty() )
        QWhatsThis::add( bar->getButton(id_), d->m_whatsThis );

    if ( !d->m_toolTip.isEmpty() )
      QToolTip::add( bar->getButton(id_), d->m_toolTip );

    addContainer( bar, id_ );

    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    if ( m_parentCollection )
      m_parentCollection->connectHighlight( bar, this );

    return containerCount() - 1;
  }

  return -1;
}

void KAction::unplug( QWidget *w )
{
  if ( w->inherits("QPopupMenu") )
  {
    QPopupMenu* menu = static_cast<QPopupMenu*>( w );
    int i = findContainer( menu );
    if ( i != -1 )
    {
      menu->removeItem( itemId( i ) );
      removeContainer( i );
      if ( m_parentCollection )
        m_parentCollection->disconnectHighlight( menu, this );
    }
  }
  else if ( w->inherits( "KToolBar" ) )
  {
    KToolBar *bar = static_cast<KToolBar *>( w );

    int idx = findContainer( bar );

    if ( idx != -1 )
    {
      bar->removeItem( itemId( idx ) );
      removeContainer( idx );
      if ( m_parentCollection )
        m_parentCollection->disconnectHighlight( bar, this );
    }

    return;
  }
}

void KAction::plugAccel(KAccel *kacc, bool configurable)
{
  if (d->m_kaccel)
    unplugAccel();
  d->m_kaccel = kacc;
  d->m_kaccel->insertItem(d->m_plainText, name(), d->m_accel, configurable);
  d->m_kaccel->connectItem(name(), this, SLOT(slotActivated()));
  d->m_kaccel->setItemEnabled( name(), isEnabled() );
  connect(d->m_kaccel, SIGNAL(destroyed()), this, SLOT(slotDestroyed()));
  connect(d->m_kaccel, SIGNAL(keycodeChanged()), this, SLOT(slotKeycodeChanged()));
}

void KAction::unplugAccel()
{
  if ( d->m_kaccel==0 )
    return;

   d->m_kaccel->removeItem(name());
   d->m_kaccel->disconnect(this);
   d->m_kaccel = 0;
}

void KAction::setEnabled(bool enable)
{
  if (d->m_kaccel)
    d->m_kaccel->setItemEnabled(name(), enable);

  if ( enable == d->m_enabled )
    return;

  d->m_enabled = enable;

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    setEnabled( i, enable );

  emit enabled( d->m_enabled );
}

void KAction::setEnabled( int i, bool e )
{
    QWidget *w = container( i );

    if ( w->inherits("QPopupMenu") )
      static_cast<QPopupMenu*>(w)->setItemEnabled( itemId( i ), e );
    else if ( w->inherits("QMenuBar") )
      static_cast<QMenuBar*>(w)->setItemEnabled( itemId( i ), e );
    else if ( w->inherits( "KToolBar" ) )
      static_cast<KToolBar*>(w)->setItemEnabled( itemId( i ), e );
}

void KAction::setText( const QString& text )
{
  if (d->m_kaccel)
    d->m_kaccel->setDescription(name(), text);

  d->m_text = text;
  d->m_plainText = plainText();

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    setText( i, text );

  if ( m_parentCollection )
  {
    KKeyEntryMap& keys = m_parentCollection->keyMap();
    keys[name()].descr = d->m_plainText;
  }
}

void KAction::setText( int i, const QString &text )
{
  QWidget *w = container( i );

  if ( w->inherits( "QPopupMenu" ) )
    static_cast<QPopupMenu*>(w)->changeItem( itemId( i ), text );
  else if ( w->inherits( "QMenuBar" ) )
    static_cast<QMenuBar*>(w)->changeItem( itemId( i ), text );
  else if ( w->inherits( "KToolBar" ) )
  {
    QWidget *button = static_cast<KToolBar *>(w)->getWidget( itemId( i ) );
    if ( button->inherits( "KToolBarButton" ) ) {
      QString stripped( text );
      int pos;
      while( ( pos = stripped.find( '&' ) ) != -1 )
        stripped.replace( pos, 1, QString::null );
      static_cast<KToolBarButton *>(button)->setText( stripped );
    }
  }
}

QString KAction::text() const
{
  return d->m_text;
}

QString KAction::plainText() const
{
  QString stripped( d->m_text );
  int pos;
  while( ( pos = stripped.find( '&' ) ) != -1 )
    stripped.replace( pos, 1, QString::null );

  return stripped;
}

void KAction::setIcon( const QString &icon )
{
  d->m_iconName = icon;

  // We load the "Small" icon as the main one (for menu items)
  // and we let setIcon( int, QString ) deal with toolbars
  KInstance *instance;
  if ( m_parentCollection )
    instance = m_parentCollection->instance();
  else
    instance = KGlobal::instance();
  setIconSet( SmallIconSet( icon, 16, instance ) );

  // now handle any toolbars
  int len = containerCount();
  for ( int i = 0; i < len; ++i )
    setIcon( i, icon );
}

void KAction::setIcon( int id, const QString &icon )
{
  QWidget* w = container( id );

  if ( w->inherits( "KToolBar" ) )
    static_cast<KToolBar *>(w)->setButtonIcon( itemId( id ), icon );
}

QString KAction::icon() const
{
  return d->m_iconName;
}

void KAction::setIconSet( const QIconSet &iconset )
{
  d->m_iconSetDemand  = iconset;
  d->m_bIconSet = true;
  d->m_bIconSetNotYetLoaded=false;

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    setIconSet( i, iconset );
}

void KAction::setIconSet( int id, const QIconSet& iconset )
{
  QWidget *w = container( id );

  if ( w->inherits( "QPopupMenu" ) )
    static_cast<QPopupMenu*>(w)->changeItem( itemId( id ), iconset, d->m_text );
  else if ( w->inherits( "QMenuBar" ) )
    static_cast<QMenuBar*>(w)->changeItem( itemId( id ), iconset, d->m_text );
  else if ( w->inherits( "KToolBar" ) )
  {
    if ( icon().isEmpty() && d->m_bIconSet ) // only if there is no named icon ( scales better )
      static_cast<KToolBar *>(w)->setButtonIconSet( itemId( id ), iconset );
  }
}

QIconSet KAction::iconSet() const
{
   if (d->m_bIconSetNotYetLoaded)
      const_cast<KAction *>(this)->setIcon(d->m_iconName);
   return d->m_iconSetDemand;
}

bool KAction::hasIconSet() const
{
  return d->m_bIconSet;
}

void KAction::setWhatsThis( const QString& text )
{
  d->m_whatsThis = text;

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    setWhatsThis( i, text );
}

void KAction::setWhatsThis( int i, const QString& text )
{
  QPopupMenu* pm = popupMenu( i );
  if ( pm )
  {
    pm->setWhatsThis( itemId( i ), text );
    return;
  }

  KToolBar *tb = toolBar( i );
  if ( tb )
  {
    QWidget *w = tb->getButton( itemId( i ) );
    QWhatsThis::remove( w );
    QWhatsThis::add( w, text );
    return;
  }
}

QString KAction::whatsThis() const
{
  return d->m_whatsThis;
}

QPixmap KAction::pixmap() const
{
  return d->m_pixmap;
}

QWidget* KAction::container( int index ) const
{
  return d->m_containers[ index ].m_container;
}

KToolBar* KAction::toolBar( int index ) const
{
    return dynamic_cast<KToolBar *>( d->m_containers[ index ].m_container );
}

QPopupMenu* KAction::popupMenu( int index ) const
{
    return dynamic_cast<QPopupMenu *>( d->m_containers[ index ].m_container );
}

QWidget* KAction::representative( int index ) const
{
  return d->m_containers[ index ].m_representative;
}

int KAction::itemId( int index ) const
{
  return d->m_containers[ index ].m_id;
}

int KAction::containerCount() const
{
  return d->m_containers.count();
}

void KAction::addContainer( QWidget* c, int id )
{
  KActionPrivate::Container p;
  p.m_container = c;
  p.m_id = id;
  d->m_containers.append( p );
}

void KAction::addContainer( QWidget* c, QWidget* w )
{
  KActionPrivate::Container p;
  p.m_container = c;
  p.m_representative = w;
  d->m_containers.append( p );
}

void KAction::activate()
{
  slotActivated();
}

void KAction::slotActivated()
{
      emit activated();
}

void KAction::slotDestroyed()
{
  if ( sender() == d->m_kaccel )
  {
    d->m_kaccel = 0;
    return;
  }

  const QObject* o = sender();

  int i;
  do
  {
    i = findContainer( static_cast<const QWidget*>( o ) );
    if ( i != -1 )
      removeContainer( i );
  } while ( i != -1 );
}

int KAction::findContainer( const QWidget* widget ) const
{
  int pos = 0;
  QValueList<KActionPrivate::Container>::ConstIterator it = d->m_containers.begin();
  while( it != d->m_containers.end() )
  {
    if ( (*it).m_representative == widget || (*it).m_container == widget )
      return pos;
    ++it;
    ++pos;
  }

  return -1;
}

void KAction::removeContainer( int index )
{
  int i = 0;
  QValueList<KActionPrivate::Container>::Iterator it = d->m_containers.begin();
  while( it != d->m_containers.end() )
  {
    if ( i == index )
    {
      d->m_containers.remove( it );
      return;
    }
    ++it;
    ++i;
  }
}

void KAction::slotKeycodeChanged()
{
  setAccel(d->m_kaccel->currentKey(name()));
}

KActionCollection *KAction::parentCollection() const
{
    return dynamic_cast<KActionCollection *>( parent() );
}

void KAction::unplugAll()
{
  while ( containerCount() != 0 )
    unplug( container( 0 ) );
}

class KToggleAction::KToggleActionPrivate
{
public:
  KToggleActionPrivate()
  {
    m_checked = false;
  }

  bool m_checked;
  QString m_exclusiveGroup;
};

KToggleAction::KToggleAction( const QString& text, int accel, QObject* parent,
                              const char* name )
    : KAction( text, accel, parent, name )
{
  d = new KToggleActionPrivate;
}

KToggleAction::KToggleAction( const QString& text, int accel,
                              const QObject* receiver, const char* slot,
                              QObject* parent, const char* name )
  : KAction( text, accel, receiver, slot, parent, name )
{
  d = new KToggleActionPrivate;
}

KToggleAction::KToggleAction( const QString& text, const QIconSet& pix,
                              int accel, QObject* parent, const char* name )
  : KAction( text, pix, accel, parent, name )
{
  d = new KToggleActionPrivate;
}

KToggleAction::KToggleAction( const QString& text, const QString& pix,
                              int accel, QObject* parent, const char* name )
 : KAction( text, pix, accel, parent, name )
{
  d = new KToggleActionPrivate;
}

KToggleAction::KToggleAction( const QString& text, const QIconSet& pix,
                              int accel, const QObject* receiver,
                              const char* slot, QObject* parent,
                              const char* name )
  : KAction( text, pix, accel, receiver, slot, parent, name )
{
  d = new KToggleActionPrivate;
}

KToggleAction::KToggleAction( const QString& text, const QString& pix,
                              int accel, const QObject* receiver,
                              const char* slot, QObject* parent,
                              const char* name )
  : KAction( text, pix, accel, receiver, slot, parent, name )
{
  d = new KToggleActionPrivate;
}

KToggleAction::KToggleAction( QObject* parent, const char* name )
    : KAction( parent, name )
{
  d = new KToggleActionPrivate;
}

KToggleAction::~KToggleAction()
{
    delete d;
}

int KToggleAction::plug( QWidget* widget, int index )
{
  if ( !widget->inherits("QPopupMenu") && !widget->inherits("KToolBar") )
  {
    kdWarning() << "Can not plug KToggleAction in " << widget->className() << endl;
    return -1;
  }

  int _index = KAction::plug( widget, index );
  if ( _index == -1 )
    return _index;

  if ( widget->inherits("QPopupMenu") )
  {
    int id = itemId( _index );

    static_cast<QPopupMenu*>(widget)->setItemChecked( id, d->m_checked );
  } else if ( widget->inherits( "KToolBar" ) ) {
    KToolBar *bar = static_cast<KToolBar *>( widget );

    bar->setToggle( itemId( _index ), TRUE );
    bar->setButton( itemId( _index ), isChecked() );
  }

  return _index;
}

void KToggleAction::setChecked( bool c )
{
  if ( c == d->m_checked )
    return;

  int len = containerCount();

  for( int i = 0; i < len; ++i )
    setChecked( i, c );

  if ( c && parent() && !exclusiveGroup().isEmpty() ) {
    const QObjectList *list = parent()->children();
    if ( list ) {
      QObjectListIt it( *list );
      for( ; it.current(); ++it ) {
        if ( it.current()->inherits( "KToggleAction" ) && it.current() != this &&
            static_cast<KToggleAction*>(it.current())->exclusiveGroup() == exclusiveGroup() ) {
          static_cast<KToggleAction*>(it.current())->setChecked( FALSE );
        }
      }
    }
  }

  d->m_checked = c;
}

void KToggleAction::setChecked( int id, bool checked )
{
  QWidget *w = container( id );

  if ( w->inherits( "QPopupMenu" ) )
    static_cast<QPopupMenu*>(w)->setItemChecked( itemId( id ), checked );
  else if ( w->inherits( "QMenuBar" ) )
    static_cast<QMenuBar*>(w)->setItemChecked( itemId( id ), checked );
  else if ( w->inherits( "KToolBar" ) )
  {
    QWidget* r = static_cast<KToolBar*>( w )->getButton( itemId( id ) );
    if ( r->inherits( "KToolBarButton" ) )
      static_cast<KToolBar*>( w )->setButton( itemId( id ), checked );
  }
}

void KToggleAction::slotActivated()
{
  setChecked( !isChecked() );
  emit activated();
  emit toggled( isChecked() );
}

bool KToggleAction::isChecked() const
{
  return d->m_checked;
}

void KToggleAction::setExclusiveGroup( const QString& name )
{
  d->m_exclusiveGroup = name;
}

QString KToggleAction::exclusiveGroup() const
{
  return d->m_exclusiveGroup;
}


KRadioAction::KRadioAction( const QString& text, int accel, QObject* parent, const char* name )
: KToggleAction( text, accel, parent, name )
{
}

KRadioAction::KRadioAction( const QString& text, int accel,
                            const QObject* receiver, const char* slot, QObject* parent, const char* name )
: KToggleAction( text, accel, receiver, slot, parent, name )
{
}

KRadioAction::KRadioAction( const QString& text, const QIconSet& pix, int accel,
                            QObject* parent, const char* name )
: KToggleAction( text, pix, accel, parent, name )
{
}

KRadioAction::KRadioAction( const QString& text, const QString& pix, int accel,
                            QObject* parent, const char* name )
: KToggleAction( text, pix, accel, parent, name )
{
}

KRadioAction::KRadioAction( const QString& text, const QIconSet& pix, int accel,
                            const QObject* receiver, const char* slot, QObject* parent, const char* name )
: KToggleAction( text, pix, accel, receiver, slot, parent, name )
{
}

KRadioAction::KRadioAction( const QString& text, const QString& pix, int accel,
                            const QObject* receiver, const char* slot, QObject* parent, const char* name )
: KToggleAction( text, pix, accel, receiver, slot, parent, name )
{
}

KRadioAction::KRadioAction( QObject* parent, const char* name )
: KToggleAction( parent, name )
{
}

void KRadioAction::slotActivated()
{
  if ( isChecked() )
  {
    const QObject *senderObj = sender();

    if ( !senderObj || !senderObj->inherits( "KToolBarButton" ) )
      return;

    const_cast<KToolBarButton *>( static_cast<const KToolBarButton *>( senderObj ) )->on( true );

    return;
  }

  KToggleAction::slotActivated();
}

class KSelectAction::KSelectActionPrivate
{
public:
  KSelectActionPrivate()
  {
    m_edit = false;
    m_menu = 0;
    m_current = -1;
    m_comboWidth = 70;
  }
  bool m_edit;
  QPopupMenu *m_menu;
  int m_current;
  int m_comboWidth;
  QStringList m_list;
};

KSelectAction::KSelectAction( const QString& text, int accel, QObject* parent,
                              const char* name )
  : KAction( text, accel, parent, name )
{
  d = new KSelectActionPrivate;
}

KSelectAction::KSelectAction( const QString& text, int accel,
                              const QObject* receiver, const char* slot,
                              QObject* parent, const char* name )
  : KAction( text, accel, receiver, slot, parent, name )
{
  d = new KSelectActionPrivate;
}

KSelectAction::KSelectAction( const QString& text, const QIconSet& pix,
                              int accel, QObject* parent, const char* name )
  : KAction( text, pix, accel, parent, name )
{
  d = new KSelectActionPrivate;
}

KSelectAction::KSelectAction( const QString& text, const QString& pix,
                              int accel, QObject* parent, const char* name )
  : KAction( text, pix, accel, parent, name )
{
  d = new KSelectActionPrivate;
}

KSelectAction::KSelectAction( const QString& text, const QIconSet& pix,
                              int accel, const QObject* receiver,
                              const char* slot, QObject* parent,
                              const char* name )
  : KAction( text, pix, accel, receiver, slot, parent, name )
{
  d = new KSelectActionPrivate;
}

KSelectAction::KSelectAction( const QString& text, const QString& pix,
                              int accel, const QObject* receiver,
                              const char* slot, QObject* parent,
                              const char* name )
  : KAction( text, pix, accel, receiver, slot, parent, name )
{
  d = new KSelectActionPrivate;
}

KSelectAction::KSelectAction( QObject* parent, const char* name )
  : KAction( parent, name )
{
  d = new KSelectActionPrivate;
}

KSelectAction::~KSelectAction()
{
  assert(d);
  delete d->m_menu;
  delete d; d = 0;
}

void KSelectAction::setCurrentItem( int id )
{
    if ( id >= (int)d->m_list.count() ) {
        ASSERT(id < (int)d->m_list.count());
        return;
    }

    if ( d->m_menu )
    {
        if ( d->m_current >= 0 )
            d->m_menu->setItemChecked( d->m_current, FALSE );
        if ( id >= 0 )
            d->m_menu->setItemChecked( id, TRUE );
    }

    d->m_current = id;

    int len = containerCount();

    for( int i = 0; i < len; ++i )
        setCurrentItem( i, id );

    //    emit KAction::activated();
    //    emit activated( currentItem() );
    //    emit activated( currentText() );
}

void KSelectAction::setComboWidth( int width )
{
  if ( width < 0 )
    return;

  d->m_comboWidth=width;

  int len = containerCount();

  for( int i = 0; i < len; ++i )
    setComboWidth( i, width );

}
QPopupMenu* KSelectAction::popupMenu()
{
  if ( !d->m_menu )
  {
    d->m_menu = new KPopupMenu(0L, "KSelectAction::popupMenu()");
    QStringList::ConstIterator it = d->m_list.begin();
    int id = 0;
    for( ; it != d->m_list.end(); ++it ) {
      if (!((*it).isEmpty())) {
        d->m_menu->insertItem( *it, this, SLOT( slotActivated( int ) ), 0, id++ );
      } else {
        d->m_menu->insertSeparator();
      }
    }
    if ( d->m_current >= 0 )
      d->m_menu->setItemChecked( d->m_current, true );
  }

  return d->m_menu;
}

void KSelectAction::changeItem( int index, const QString& text )
{
  if ( index < 0 || index >= (int)d->m_list.count() )
  {
    kdWarning() << "KSelectAction::changeItem Index out of scope" << endl;
    return;
  }

  d->m_list[ index ] = text;

  if ( d->m_menu )
    d->m_menu->changeItem( index, text );

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    changeItem( i, index, text );
}

void KSelectAction::changeItem( int id, int index, const QString& text)
{
  if ( index < 0 )
        return;

  QWidget* w = container( id );
  if ( w->inherits( "KToolBar" ) )
  {
     QWidget* r = (static_cast<KToolBar*>( w ))->getWidget( itemId( id ) );
     if ( r->inherits( "QComboBox" ) )
     {
        QComboBox *b = static_cast<QComboBox*>( r );
        b->changeItem(text, index );
     }
  }
}

void KSelectAction::setItems( const QStringList &lst )
{
  d->m_list = lst;
  d->m_current = -1;

  if ( d->m_menu )
  {
    d->m_menu->clear();
    QStringList::ConstIterator it = d->m_list.begin();
    int id = 0;
    for( ; it != d->m_list.end(); ++it )
      if (!((*it).isEmpty())) {
        d->m_menu->insertItem( *it, this, SLOT( slotActivated( int ) ), 0, id++ );
      } else {
        d->m_menu->insertSeparator();
      }
  }

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    setItems( i, lst );

  // Disable if empty and not editable
  setEnabled ( lst.count() > 0 || d->m_edit );
}

QStringList KSelectAction::items() const
{
  return d->m_list;
}

QString KSelectAction::currentText() const
{
  if ( currentItem() < 0 )
    return QString::null;

  return d->m_list[ currentItem() ];
}

int KSelectAction::currentItem() const
{
  return d->m_current;
}

void KSelectAction::setCurrentItem( int id, int index )
{
  if ( index < 0 )
        return;

  QWidget* w = container( id );
  if ( w->inherits( "KToolBar" ) ) {
    QWidget* r = static_cast<KToolBar*>( w )->getWidget( itemId( id ) );
    if ( r->inherits( "QComboBox" ) ) {
      QComboBox *b = static_cast<QComboBox*>( r );
      b->setCurrentItem( index );
    }
  }
}

int KSelectAction::comboWidth() const
{
  return d->m_comboWidth;
}

void KSelectAction::setComboWidth( int id, int width )
{
  QWidget* w = container( id );
  if ( w->inherits( "KToolBar" ) ) {
    QWidget* r = static_cast<KToolBar*>( w )->getWidget( itemId( id ) );
    if ( r->inherits( "QComboBox" ) ) {
      QComboBox *b = static_cast<QComboBox*>( r );
      b->resize( width, b->height() );
    }
  }
}

void KSelectAction::setItems( int id, const QStringList& lst )
{
  QWidget* w = container( id );
  if ( w->inherits( "KToolBar" ) ) {
    QWidget* r = static_cast<KToolBar*>( w )->getWidget( itemId( id ) );
    if ( r->inherits( "QComboBox" ) ) {
      QComboBox *b = static_cast<QComboBox*>( r );
      b->clear();
      QStringList::ConstIterator it = lst.begin();
      for( ; it != lst.end(); ++it )
        b->insertItem( *it );
      }
   }
}

int KSelectAction::plug( QWidget *widget, int index )
{
  if ( widget->inherits("QPopupMenu") )
  {
    // Create the PopupMenu and store it in m_menu
    (void)popupMenu();

    QPopupMenu* menu = static_cast<QPopupMenu*>( widget );
    int id;
    if ( !pixmap().isNull() )
    {
      id = menu->insertItem( pixmap(), d->m_menu, -1, index );
    }
    else
    {
      if ( hasIconSet() )
        id = menu->insertItem( iconSet(), text(), d->m_menu, -1, index );
      else
        id = menu->insertItem( text(), d->m_menu, -1, index );
    }

    if ( !isEnabled() )
        menu->setItemEnabled( id, false );

    QString wth = whatsThis();
    if ( !wth.isEmpty() )
        menu->setWhatsThis( id, wth );

    addContainer( menu, id );
    connect( menu, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }
  else if ( widget->inherits("KToolBar") )
  {
    KToolBar* bar = static_cast<KToolBar*>( widget );
    int id_ = KAction::getToolButtonID();
    bar->insertCombo( items(), id_, isEditable(),
                      SIGNAL( activated( const QString & ) ), this,
                      SLOT( slotActivated( const QString & ) ), isEnabled(),
                      QString::null, d->m_comboWidth, index );

    QComboBox *cb = bar->getCombo( id_ );
    if ( cb )
    {
      cb->setMinimumWidth( cb->sizeHint().width() );
      cb->setInsertionPolicy( QComboBox::NoInsertion );
    }

    addContainer( bar, id_ );

    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    setCurrentItem( containerCount() - 1, currentItem() );

    return containerCount() - 1;
  }

  kdWarning() << "Can not plug KAction in " << widget->className() << endl;
  return -1;
}

void KSelectAction::clear()
{
  if ( d->m_menu )
    d->m_menu->clear();

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    clear( i );
}

void KSelectAction::clear( int id )
{
  QWidget* w = container( id );
  if ( w->inherits( "KToolBar" ) ) {
    QWidget* r = static_cast<KToolBar*>( w )->getWidget( itemId( id ) );
    if ( r->inherits( "QComboBox" ) ) {
      QComboBox *b = static_cast<QComboBox*>( r );
      b->clear();
    }
  }
}

void KSelectAction::slotActivated( int id )
{
  if ( d->m_current == id )
    return;

  setCurrentItem( id );
  emit KAction::activated();
  emit activated( currentItem() );
  emit activated( currentText() );
}

void KSelectAction::slotActivated( const QString &text )
{
  if ( isEditable() )
  {
    QStringList lst = items();
    if(lst.contains(text)==0)
    {
      lst.append( text );
      setItems( lst );
    }
  }

  setCurrentItem( items().findIndex( text ) );
  emit KAction::activated();
  emit activated( currentItem() );
  emit activated( currentText() );
}

void KSelectAction::setEditable( bool edit )
{
  d->m_edit = edit;
}

bool KSelectAction::isEditable() const
{
  return d->m_edit;
}

class KListAction::KListActionPrivate
{
public:
  KListActionPrivate()
  {
    m_current = 0;
  }
  int m_current;
};

KListAction::KListAction( const QString& text, int accel, QObject* parent,
                          const char* name )
  : KSelectAction( text, accel, parent, name )
{
  d = new KListActionPrivate;
}

KListAction::KListAction( const QString& text, int accel,
                          const QObject* receiver, const char* slot,
                          QObject* parent, const char* name )
  : KSelectAction( text, accel, parent, name )
{
  d = new KListActionPrivate;
  if ( receiver )
    connect( this, SIGNAL(activated(int)), receiver, slot );
}

KListAction::KListAction( const QString& text, const QIconSet& pix,
                          int accel, QObject* parent, const char* name )
  : KSelectAction( text, pix, accel, parent, name )
{
  d = new KListActionPrivate;
}

KListAction::KListAction( const QString& text, const QString& pix,
                            int accel, QObject* parent, const char* name )
  : KSelectAction( text, pix, accel, parent, name )
{
  d = new KListActionPrivate;
}

KListAction::KListAction( const QString& text, const QIconSet& pix,
                          int accel, const QObject* receiver,
                          const char* slot, QObject* parent,
                          const char* name )
  : KSelectAction( text, pix, accel, receiver, slot, parent, name )
{
  d = new KListActionPrivate;
  if ( receiver )
    connect( this, SIGNAL(activated(int)), receiver, slot );
}

KListAction::KListAction( const QString& text, const QString& pix,
                          int accel, const QObject* receiver,
                          const char* slot, QObject* parent,
                          const char* name )
  : KSelectAction( text, pix, accel, receiver, slot, parent, name )
{
  d = new KListActionPrivate;
  if ( receiver )
    connect( this, SIGNAL(activated(int)), receiver, slot );
}

KListAction::KListAction( QObject* parent, const char* name )
  : KSelectAction( parent, name )
{
  d = new KListActionPrivate;
}

KListAction::~KListAction()
{
  delete d; d = 0;
}

void KListAction::setCurrentItem( int index )
{
  KSelectAction::setCurrentItem( index );
  d->m_current = index;

  //  emit KAction::activated();
  //  emit activated( currentItem() );
  // emit activated( currentText() );
}

QString KListAction::currentText() const
{
  if ( currentItem() < 0 )
      return QString::null;

  return items()[ currentItem() ];
}

int KListAction::currentItem() const
{
  return d->m_current;
}

class KRecentFilesAction::KRecentFilesActionPrivate
{
public:
  KRecentFilesActionPrivate()
  {
    m_maxItems = 0;
  }
  uint m_maxItems;
};

KRecentFilesAction::KRecentFilesAction( const QString& text, int accel,
                                        QObject* parent, const char* name,
                                        unsigned int maxItems )
  : KListAction( text, accel, parent, name)
{
  d = new KRecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  connect( this, SIGNAL( activated( const QString& ) ),
           this, SLOT( itemSelected( const QString& ) ) );
}

KRecentFilesAction::KRecentFilesAction( const QString& text, int accel,
                                        const QObject* receiver,
                                        const char* slot,
                                        QObject* parent, const char* name,
                                        uint maxItems )
  : KListAction( text, accel, parent, name)
{
  d = new KRecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  connect( this, SIGNAL( activated( const QString& ) ),
           this, SLOT( itemSelected( const QString& ) ) );

  if ( receiver )
    connect( this,     SIGNAL(urlSelected(const KURL&)),
             receiver, slot );
}

KRecentFilesAction::KRecentFilesAction( const QString& text,
                                        const QIconSet& pix, int accel,
                                        QObject* parent, const char* name,
                                        uint maxItems )
  : KListAction( text, pix, accel, parent, name)
{
  d = new KRecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  connect( this, SIGNAL( activated( const QString& ) ),
           this, SLOT( itemSelected( const QString& ) ) );
}

KRecentFilesAction::KRecentFilesAction( const QString& text,
                                        const QString& pix, int accel,
                                        QObject* parent, const char* name,
                                        uint maxItems )
  : KListAction( text, pix, accel, parent, name)
{
  d = new KRecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  connect( this, SIGNAL( activated( const QString& ) ),
           this, SLOT( itemSelected( const QString& ) ) );
}

KRecentFilesAction::KRecentFilesAction( const QString& text,
                                        const QIconSet& pix, int accel,
                                        const QObject* receiver,
                                        const char* slot,
                                        QObject* parent, const char* name,
                                        uint maxItems )
  : KListAction( text, pix, accel, parent, name)
{
  d = new KRecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  connect( this, SIGNAL( activated( const QString& ) ),
           this, SLOT( itemSelected( const QString& ) ) );

  if ( receiver )
    connect( this,     SIGNAL(urlSelected(const KURL&)),
             receiver, slot );
}

KRecentFilesAction::KRecentFilesAction( const QString& text,
                                        const QString& pix, int accel,
                                        const QObject* receiver,
                                        const char* slot,
                                        QObject* parent, const char* name,
                                        uint maxItems )
  : KListAction( text, pix, accel, parent, name)
{
  d = new KRecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  connect( this, SIGNAL( activated( const QString& ) ),
           this, SLOT( itemSelected( const QString& ) ) );

  if ( receiver )
    connect( this,     SIGNAL(urlSelected(const KURL&)),
             receiver, slot );
}

KRecentFilesAction::KRecentFilesAction( QObject* parent, const char* name,
                                        uint maxItems )
  : KListAction( parent, name )
{
  d = new KRecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  connect( this, SIGNAL( activated( const QString& ) ),
           this, SLOT( itemSelected( const QString& ) ) );
}

KRecentFilesAction::~KRecentFilesAction()
{
  delete d; d = 0;
}

uint KRecentFilesAction::maxItems() const
{
    return d->m_maxItems;
}

void KRecentFilesAction::setMaxItems( uint maxItems )
{
    QStringList lst = items();
    uint oldCount   = lst.count();

    // set new maxItems
    d->m_maxItems = maxItems;

    // remove all items that are too much
    while( lst.count() > maxItems )
    {
        // remove last item
        lst.remove( lst.last() );
    }

    // set new list if changed
    if( lst.count() != oldCount )
        setItems( lst );
}

void KRecentFilesAction::addURL( const KURL& url )
{
    QString     file = url.prettyURL();
    QStringList lst = items();

    // remove file if already in list
    lst.remove( file );

    // remove las item if already maxitems in list
    if( lst.count() == d->m_maxItems )
    {
        // remove last item
        lst.remove( lst.last() );
    }

    // add file to list
    lst.prepend( file );
    setItems( lst );
}

void KRecentFilesAction::removeURL( const KURL& url )
{
    QStringList lst = items();
    QString     file = url.prettyURL();

    // remove url
    if( lst.count() > 0 )
    {
        lst.remove( file );
        setItems( lst );
    }
}

void KRecentFilesAction::clearURLList()
{
    clear();
}

void KRecentFilesAction::loadEntries( KConfig* config, QString groupname)
{
    QString     key;
    QString     value;
    QString     oldGroup;
    QStringList lst;

    oldGroup = config->group();

    if (groupname.isEmpty())
      groupname = "RecentFiles";
    config->setGroup( groupname );

    // read file list
    for( unsigned int i = 1 ; i <= d->m_maxItems ; i++ )
    {
        key = QString( "File%1" ).arg( i );
        value = config->readEntry( key, QString::null );

        if (!value.isNull())
            lst.append( value );
    }

    // set file
    setItems( lst );

    config->setGroup( oldGroup );
}

void KRecentFilesAction::saveEntries( KConfig* config, QString groupname )
{
    QString     key;
    QString     value;
    QString     oldGroup;
    QStringList lst = items();

    oldGroup = config->group();

    if (groupname.isEmpty())
      groupname = "RecentFiles";
    config->setGroup( groupname );

    // write file list
    for( unsigned int i = 1 ; i <= lst.count() ; i++ )
    {
        key = QString( "File%1" ).arg( i );
        value = lst[ i - 1 ];
        config->writeEntry( key, value );
    }

    config->setGroup( oldGroup );
}

void KRecentFilesAction::itemSelected( const QString& text )
{
    emit urlSelected( KURL( text ) );
}

class KFontAction::KFontActionPrivate
{
public:
  KFontActionPrivate()
  {
  }
  QStringList m_fonts;
};

KFontAction::KFontAction( const QString& text, int accel, QObject* parent,
                          const char* name )
  : KSelectAction( text, accel, parent, name )
{
    d = new KFontActionPrivate;
    get_fonts( d->m_fonts );
    KSelectAction::setItems( d->m_fonts );
    setEditable( TRUE );
}

KFontAction::KFontAction( const QString& text, int accel,
                          const QObject* receiver, const char* slot,
                          QObject* parent, const char* name )
    : KSelectAction( text, accel, receiver, slot, parent, name )
{
    d = new KFontActionPrivate;
    get_fonts( d->m_fonts );
    KSelectAction::setItems( d->m_fonts );
    setEditable( TRUE );
}

KFontAction::KFontAction( const QString& text, const QIconSet& pix, int accel,
                          QObject* parent, const char* name )
    : KSelectAction( text, pix, accel, parent, name )
{
    d = new KFontActionPrivate;
    get_fonts( d->m_fonts );
    KSelectAction::setItems( d->m_fonts );
    setEditable( TRUE );
}

KFontAction::KFontAction( const QString& text, const QString& pix, int accel,
                          QObject* parent, const char* name )
    : KSelectAction( text, pix, accel, parent, name )
{
    d = new KFontActionPrivate;
    get_fonts( d->m_fonts );
    KSelectAction::setItems( d->m_fonts );
    setEditable( TRUE );
}

KFontAction::KFontAction( const QString& text, const QIconSet& pix, int accel,
                          const QObject* receiver, const char* slot,
                          QObject* parent, const char* name )
    : KSelectAction( text, pix, accel, receiver, slot, parent, name )
{
    d = new KFontActionPrivate;
    get_fonts( d->m_fonts );
    KSelectAction::setItems( d->m_fonts );
    setEditable( TRUE );
}

KFontAction::KFontAction( const QString& text, const QString& pix, int accel,
                          const QObject* receiver, const char* slot,
                          QObject* parent, const char* name )
    : KSelectAction( text, pix, accel, receiver, slot, parent, name )
{
    d = new KFontActionPrivate;
    get_fonts( d->m_fonts );
    KSelectAction::setItems( d->m_fonts );
    setEditable( TRUE );
}


KFontAction::KFontAction( QObject* parent, const char* name )
  : KSelectAction( parent, name )
{
    d = new KFontActionPrivate;
    get_fonts( d->m_fonts );
    KSelectAction::setItems( d->m_fonts );
    setEditable( TRUE );
}

KFontAction::~KFontAction()
{
    delete d;
    d = 0;
}

void KFontAction::setFont( const QString &family )
{
  int i = 0;
  for ( QStringList::Iterator it = d->m_fonts.begin(); it != d->m_fonts.end(); ++it, ++i )
    if ( (*it).lower() == family.lower() )
    {
      setCurrentItem( i );
      return;
    }
  kdDebug() << "Font not found " << family.lower() << endl;
}

int KFontAction::plug( QWidget *w, int index )
{
  if ( w->inherits("KToolBar") )
  {
    KToolBar* bar = static_cast<KToolBar*>( w );
    int id_ = KAction::getToolButtonID();
    KFontCombo *cb = new KFontCombo( items(), bar );
    connect( cb, SIGNAL( activated( const QString & ) ),
             SLOT( slotActivated( const QString & ) ) );
    cb->setEnabled( isEnabled() );
    bar->insertWidget( id_, comboWidth(), cb, index );
    cb->setMinimumWidth( cb->sizeHint().width() );

    addContainer( bar, id_ );

    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    setCurrentItem( containerCount() - 1, currentItem() );

    return containerCount() - 1;
  }
  else return KSelectAction::plug( w, index );
}

class KFontSizeAction::KFontSizeActionPrivate
{
public:
  KFontSizeActionPrivate()
  {
  }
};

KFontSizeAction::KFontSizeAction( const QString& text, int accel,
                                  QObject* parent, const char* name )
  : KSelectAction( text, accel, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( const QString& text, int accel,
                                  const QObject* receiver, const char* slot,
                                  QObject* parent, const char* name )
  : KSelectAction( text, accel, receiver, slot, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( const QString& text, const QIconSet& pix,
                                  int accel, QObject* parent, const char* name )
  : KSelectAction( text, pix, accel, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( const QString& text, const QString& pix,
                                  int accel, QObject* parent, const char* name )
  : KSelectAction( text, pix, accel, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( const QString& text, const QIconSet& pix,
                                  int accel, const QObject* receiver,
                                  const char* slot, QObject* parent,
                                  const char* name )
    : KSelectAction( text, pix, accel, receiver, slot, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( const QString& text, const QString& pix,
                                  int accel, const QObject* receiver,
                                  const char* slot, QObject* parent,
                                  const char* name )
  : KSelectAction( text, pix, accel, receiver, slot, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( QObject* parent, const char* name )
  : KSelectAction( parent, name )
{
  init();
}

KFontSizeAction::~KFontSizeAction()
{
    delete d;
    d = 0;
}

void KFontSizeAction::init()
{
    d = new KFontSizeActionPrivate;

    setEditable( TRUE );
    QValueList<int> sizes = get_standard_font_sizes();
    QStringList lst;
    for ( QValueList<int>::Iterator it = sizes.begin(); it != sizes.end(); ++it )
        lst.append( QString::number( *it ) );

    setItems( lst );
}

void KFontSizeAction::setFontSize( int size )
{
    if ( size == fontSize() ) {
        setCurrentItem( items().findIndex( QString::number( size ) ) );
        return;
    }

    if ( size < 1 || size > 128 ) {
        kdWarning() << "KFontSizeAction: Size " << size << " is out of range" << endl;
        return;
    }

    int index = items().findIndex( QString::number( size ) );
    if ( index == -1 ) {
        // Insert at the correct position in the list (to keep sorting)
        QValueList<int> lst;
        // Convert to list of ints
        QStringList itemsList = items();
        for (QStringList::Iterator it = itemsList.begin() ; it != itemsList.end() ; ++it)
            lst.append( (*it).toInt() );
        // New size
        lst.append( size );
        // Sort the list
        qHeapSort( lst );
        // Convert back to string list
        QStringList strLst;
        for (QValueList<int>::Iterator it = lst.begin() ; it != lst.end() ; ++it)
            strLst.append( QString::number(*it) );
        KSelectAction::setItems( strLst );
        // Find new current item
        index = lst.findIndex( size );
        setCurrentItem( index );
    }
    else
        setCurrentItem( index );


    //emit KAction::activated();
    //emit activated( index );
    //emit activated( QString::number( size ) );
    //emit fontSizeChanged( size );
}

int KFontSizeAction::fontSize() const
{
  return currentText().toInt();
}

void KFontSizeAction::slotActivated( int index )
{
  KSelectAction::slotActivated( index );

  emit fontSizeChanged( items()[ index ].toInt() );
}

void KFontSizeAction::slotActivated( const QString& size )
{
  setFontSize( size.toInt() ); // insert sorted first
  KSelectAction::slotActivated( size );
  emit fontSizeChanged( size.toInt() );
}

class KActionMenu::KActionMenuPrivate
{
public:
  KActionMenuPrivate()
  {
    m_popup = new KPopupMenu(0L,"KActionMenu::KActionMenuPrivate");
    m_delayed = true;
    m_stickyMenu = true;
  }
  ~KActionMenuPrivate()
  {
    delete m_popup; m_popup = 0;
  }
  KPopupMenu *m_popup;
  bool m_delayed;
  bool m_stickyMenu;
};

KActionMenu::KActionMenu( QObject* parent, const char* name )
  : KAction( parent, name )
{
  d = new KActionMenuPrivate;
}

KActionMenu::KActionMenu( const QString& text, QObject* parent,
                          const char* name )
  : KAction( text, 0, parent, name )
{
  d = new KActionMenuPrivate;
}

KActionMenu::KActionMenu( const QString& text, const QIconSet& icon,
                          QObject* parent, const char* name )
  : KAction( text, icon, 0, parent, name )
{
  d = new KActionMenuPrivate;
}

KActionMenu::KActionMenu( const QString& text, const QString& icon,
                          QObject* parent, const char* name )
  : KAction( text, icon, 0, parent, name )
{
  d = new KActionMenuPrivate;
}

KActionMenu::~KActionMenu()
{
  delete d; d = 0;
}

void KActionMenu::popup( const QPoint& global )
{
  popupMenu()->popup( global );
}

KPopupMenu* KActionMenu::popupMenu()
{
  return d->m_popup;
}

void KActionMenu::insert( KAction* cmd, int index )
{
  if ( cmd )
    cmd->plug( d->m_popup, index );
}

void KActionMenu::remove( KAction* cmd )
{
  if ( cmd )
    cmd->unplug( d->m_popup );
}

bool KActionMenu::delayed() const {
    return d->m_delayed;
}

void KActionMenu::setDelayed(bool _delayed) {
    d->m_delayed = _delayed;
}

bool KActionMenu::stickyMenu() const {
    return d->m_stickyMenu;
}

void KActionMenu::setStickyMenu(bool sticky) {
    d->m_stickyMenu = sticky;
}

int KActionMenu::plug( QWidget* widget, int index )
{
  if ( widget->inherits("QPopupMenu") )
  {
    QPopupMenu* menu = static_cast<QPopupMenu*>( widget );
    int id;
    if ( !pixmap().isNull() )
      id = menu->insertItem( pixmap(), d->m_popup, -1, index );
    else
    {
      if ( hasIconSet() )
        id = menu->insertItem( iconSet(), text(), d->m_popup, -1, index );
      else
        id = menu->insertItem( text(), d->m_popup, -1, index );
    }

    if ( !isEnabled() )
        menu->setItemEnabled( id, false );

    addContainer( menu, id );
    connect( menu, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }
  else if ( widget->inherits( "KToolBar" ) )
  {
    KToolBar *bar = static_cast<KToolBar *>( widget );

    int id_ = KAction::getToolButtonID();

    if ( icon().isEmpty() && !iconSet().isNull() )
      bar->insertButton( iconSet().pixmap(), id_, SIGNAL( clicked() ), this,
                         SLOT( slotActivated() ), isEnabled(), plainText(),
                         index );
    else
    {
      KInstance *instance;

      if ( m_parentCollection )
        instance = m_parentCollection->instance();
      else
        instance = KGlobal::instance();

      bar->insertButton( icon(), id_, SIGNAL( clicked() ), this,
                         SLOT( slotActivated() ), isEnabled(), plainText(),
                         index, instance );
    }

    addContainer( bar, id_ );

    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
    if (delayed()) {
        bar->setDelayedPopup( id_, popupMenu(), stickyMenu() );
    } else {
        bar->getButton(id_)->setPopup(popupMenu(), stickyMenu() );
    }

    return containerCount() - 1;
  }
  else if ( widget->inherits( "QMenuBar" ) )
  {
    QMenuBar *bar = static_cast<QMenuBar *>( widget );

    int id;

    id = bar->insertItem( text(), popupMenu(), -1, index );

    if ( !isEnabled() )
        bar->setItemEnabled( id, false );

    addContainer( bar, id );
    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }

  return -1;
}

void KActionMenu::unplug( QWidget* widget )
{
  if ( widget->inherits( "KToolBar" ) )
  {
    KToolBar *bar = static_cast<KToolBar *>( widget );

    int idx = findContainer( bar );

    if ( idx != -1 )
    {
      bar->removeItem( itemId( idx ) );
      removeContainer( idx );
    }

    return;
  }
  else if ( widget->inherits( "QMenuBar" ) )
  {
    QMenuBar *bar = static_cast<QMenuBar *>( widget );
    int i = findContainer( bar );
    if ( i != -1 )
    {
      bar->removeItem( itemId( i ) );
      removeContainer( i );
    }

    return;
  }
  else
    KAction::unplug( widget );
}

void KActionMenu::setEnabled( bool b )
{
  KAction::setEnabled( b );
}

// To be removed (BCI)
void KActionMenu::setEnabled( int id, bool b )
{
  KAction::setEnabled( id, b );
}

void KActionMenu::setText( int id, const QString& text )
{
  QWidget *w = container( id );

  if ( w->inherits( "KToolBar" ) )
  {
    QWidget *button = static_cast<KToolBar *>( w )->getWidget( itemId( id ) );
    if ( button->inherits( "KToolBarButton" ) )
     static_cast<KToolBarButton *>( button )->setText( text );
  }

  KAction::setText( id, text );
}


void KActionMenu::setIconSet( int id, const QIconSet& iconSet )
{
  QWidget *w = container( id );

  if ( w->inherits( "KToolBar" ) )
    static_cast<KToolBar *>( w )->setButtonPixmap( itemId( id ), iconSet.pixmap() );

  KAction::setIconSet( id, iconSet );
}

////////

KToolBarPopupAction::KToolBarPopupAction( const QString& text, const QString& icon, int accel, QObject* parent, const char* name )
  : KAction( text, icon, accel, parent, name )
{
  m_popup = 0;
  m_delayed = true;
  m_stickyMenu = true;
}

KToolBarPopupAction::KToolBarPopupAction( const QString& text, const QString& icon, int accel, const QObject* receiver, const char* slot, QObject* parent, const char* name )
  : KAction( text, icon, accel, receiver, slot, parent, name )
{
  m_popup = 0;
  m_delayed = true;
  m_stickyMenu = true;
}

KToolBarPopupAction::~KToolBarPopupAction()
{
  if ( m_popup )
    delete m_popup;
}

bool KToolBarPopupAction::delayed() const {
    return m_delayed;
}

void KToolBarPopupAction::setDelayed(bool delayed) {
    m_delayed = delayed;
}

bool KToolBarPopupAction::stickyMenu() const {
    return m_stickyMenu;
}

void KToolBarPopupAction::setStickyMenu(bool sticky) {
    m_stickyMenu = sticky;
}

int KToolBarPopupAction::plug( QWidget *widget, int index )
{
  // This is very related to KActionMenu::plug.
  // In fact this class could be an interesting base class for KActionMenu
  if ( widget->inherits( "KToolBar" ) )
  {
    KToolBar *bar = (KToolBar *)widget;

    int id_ = KAction::getToolButtonID();

    KInstance * instance;
    if ( m_parentCollection )
        instance = m_parentCollection->instance();
    else
        instance = KGlobal::instance();

    bar->insertButton( icon(), id_, SIGNAL( clicked() ), this,
                       SLOT( slotActivated() ), isEnabled(), plainText(),
                       index, instance );

    addContainer( bar, id_ );

    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    if (delayed()) {
        bar->setDelayedPopup( id_, popupMenu(), stickyMenu() );
    } else {
        bar->getButton(id_)->setPopup(popupMenu(), stickyMenu());
    }

    return containerCount() - 1;
  }

  return KAction::plug( widget, index );
}

void KToolBarPopupAction::unplug( QWidget *widget )
{
  if ( widget->inherits( "KToolBar" ) )
  {
    KToolBar *bar = (KToolBar *)widget;

    int idx = findContainer( bar );

    if ( idx != -1 )
    {
      bar->removeItem( menuId( idx ) );
      removeContainer( idx );
    }

    return;
  }

  KAction::unplug( widget );
}

KPopupMenu *KToolBarPopupAction::popupMenu()
{
  if ( !m_popup )
      m_popup = new KPopupMenu;
  return m_popup;
}

////////

KActionSeparator::KActionSeparator( QObject *parent, const char *name )
  : KAction( parent, name )
{
}

KActionSeparator::~KActionSeparator()
{
}

int KActionSeparator::plug( QWidget *widget, int index )
{
  if ( widget->inherits("QPopupMenu") )
  {
    QPopupMenu* menu = static_cast<QPopupMenu*>( widget );

    int id = menu->insertSeparator( index );

    addContainer( menu, id );
    connect( menu, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }
  else if ( widget->inherits( "QMenuBar" ) )
  {
    QMenuBar *menuBar = static_cast<QMenuBar *>( widget );

    int id = menuBar->insertSeparator( index );

    addContainer( menuBar, id );

    connect( menuBar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }
  else if ( widget->inherits( "KToolBar" ) )
  {
    KToolBar *toolBar = static_cast<KToolBar *>( widget );

    int id = toolBar->insertSeparator( index );

    addContainer( toolBar, id );

    connect( toolBar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }

  return -1;
}

void KActionSeparator::unplug( QWidget *widget )
{
  if ( widget->inherits("QPopupMenu") )
  {
    QPopupMenu* menu = static_cast<QPopupMenu*>( widget );
    int i = findContainer( menu );
    if ( i != -1 )
    {
      menu->removeItem( itemId( i ) );
      removeContainer( i );
    }
  }
  else if ( widget->inherits( "QMenuBar" ) )
  {
    QMenuBar *menuBar = static_cast<QMenuBar *>( widget );

    int i = findContainer( menuBar );

    if ( i != -1 )
    {
      menuBar->removeItem( itemId( i ) );
      removeContainer( i );
    }
    return;
  }
  else if ( widget->inherits( "KToolBar" ) )
  {
    KToolBar *toolBar = static_cast<KToolBar *>( widget );

    int i = findContainer( toolBar );

    if ( i != -1 )
    {
      toolBar->removeItem( itemId( i ) );
      removeContainer( i );
    }
    return;
  }

  return;
}

class KActionCollection::KActionCollectionPrivate
{
public:
  KActionCollectionPrivate()
  {
    m_dctHighlightContainers.setAutoDelete( true );
    m_highlight = false;
    m_currentHighlightAction = 0;
    m_statusCleared = true;
  }
  ~KActionCollectionPrivate()
  {
  }
  KInstance *m_instance;
  QAsciiDict<KAction> m_actionDict;
  QPtrDict< QList<KAction> > m_dctHighlightContainers;
  bool m_highlight;
  KKeyEntryMap m_keyMap;
  KAction *m_currentHighlightAction;
  bool m_statusCleared;
};

KActionCollection::KActionCollection( QObject *parent, const char *name,
                                      KInstance *instance )
  : QObject( parent, name )
{
  d = new KActionCollectionPrivate;
  setInstance( instance );
}

KActionCollection::KActionCollection( const KActionCollection &copy )
    : QObject()
{
  d = new KActionCollectionPrivate;
  d->m_actionDict = copy.d->m_actionDict;
  d->m_keyMap = copy.d->m_keyMap;
  setInstance( copy.instance() );
}

KActionCollection::~KActionCollection()
{
  QAsciiDictIterator<KAction> it( d->m_actionDict );
  for (; it.current(); ++it )
      if ( it.current()->m_parentCollection == this )
          it.current()->m_parentCollection = 0L;

  delete d; d = 0;
}

void KActionCollection::insert( KAction* action )
{
  KAction *a = d->m_actionDict[ action->name() ];
  if ( a && a == action )
      return;

  d->m_actionDict.insert( action->name(), action );

  emit inserted( action );

  KKeyEntry entry;

  int accel = action->accel();
  entry.aDefaultKeyCode = accel;
  entry.aCurrentKeyCode = accel;
  entry.aConfigKeyCode  = accel;
  entry.bConfigurable   = true;
  entry.descr           = action->plainText();

  entry.aAccelId = 0;
  entry.receiver = 0;
  entry.member   = 0;
  entry.menuId   = 0;
  entry.menu     = 0;

  d->m_keyMap.insert( action->name(), entry );
}

void KActionCollection::remove( KAction* action )
{
  delete take (action);
}

KAction* KActionCollection::take( KAction* action )
{
  KAction *a = d->m_actionDict.take( action->name() );
  if ( !a || a != action )
      return 0;

  d->m_keyMap.remove( a->name() );
  emit removed( action );
  return a;
}

void KActionCollection::clear()
{
  QAsciiDictIterator<KAction> it( d->m_actionDict );
  while ( it.current() )
    remove( it.current() );
}

KAction* KActionCollection::action( const char* name, const char* classname ) const
{
  if ( !classname && name )
    return d->m_actionDict[ name ];

  QAsciiDictIterator<KAction> it( d->m_actionDict );
  for( ; it.current(); ++it )
  {
    if ( ( !name || strcmp( it.current()->name(), name ) == 0 ) &&
        ( !classname || strcmp( it.current()->className(), classname ) == 0 ) )
      return it.current();
  }
  return 0;
}

KAction* KActionCollection::action( int index ) const
{
  QAsciiDictIterator<KAction> it( d->m_actionDict );
  it += index;
  return it.current();
//  return d->m_actions.at( index );
}

void KActionCollection::setKeyMap( const KKeyEntryMap &map )
{
  d->m_keyMap = map;

  for (KKeyEntryMap::ConstIterator it = map.begin(); it != map.end(); ++it) {
    if ((*it).aCurrentKeyCode != (*it).aConfigKeyCode)
    {
      KAction *act = action( it.key().latin1() );
      act->setAccel( (*it).aConfigKeyCode );
    }
  }
}

KKeyEntryMap & KActionCollection::keyMap()
{
  return d->m_keyMap;
}

uint KActionCollection::count() const
{
  return d->m_actionDict.count();
}

QStringList KActionCollection::groups() const
{
  QStringList lst;

  QAsciiDictIterator<KAction> it( d->m_actionDict );
  for( ; it.current(); ++it )
    if ( !it.current()->group().isEmpty() && !lst.contains( it.current()->group() ) )
      lst.append( it.current()->group() );

  return lst;
}

QValueList<KAction*> KActionCollection::actions( const QString& group ) const
{
  QValueList<KAction*> lst;

  QAsciiDictIterator<KAction> it( d->m_actionDict );
  for( ; it.current(); ++it )
    if ( it.current()->group() == group )
      lst.append( it.current() );
    else if ( it.current()->group().isEmpty() && group.isEmpty() )
      lst.append( it.current() );

  return lst;
}

QValueList<KAction*> KActionCollection::actions() const
{
  QValueList<KAction*> lst;

  QAsciiDictIterator<KAction> it( d->m_actionDict );
  for( ; it.current(); ++it )
    lst.append( it.current() );

  return lst;
}

KActionCollection KActionCollection::operator+(const KActionCollection &c ) const
{
  KActionCollection ret( *this );

  QValueList<KAction *> actions = c.actions();
  QValueList<KAction *>::ConstIterator it = actions.begin();
  QValueList<KAction *>::ConstIterator end = actions.end();
  for (; it != end; ++it )
    ret.insert( *it );

  return ret;
}

KActionCollection &KActionCollection::operator=( const KActionCollection &c )
{
  d->m_actionDict = c.d->m_actionDict;
  d->m_keyMap = c.d->m_keyMap;
  setInstance( c.instance() );
  return *this;
}

KActionCollection &KActionCollection::operator+=( const KActionCollection &c )
{
  QAsciiDictIterator<KAction> it(c.d->m_actionDict);
  for ( ; it.current(); ++it )
    insert( it.current() );

  return *this;
}

void KActionCollection::setInstance( KInstance *instance )
{
  if ( instance )
    d->m_instance = instance;
  else
    d->m_instance = KGlobal::instance();
}

KInstance *KActionCollection::instance() const
{
  return d->m_instance;
}

void KActionCollection::setHighlightingEnabled( bool enable )
{
  d->m_highlight = enable;
}

bool KActionCollection::highlightingEnabled() const
{
  return d->m_highlight;
}

void KActionCollection::connectHighlight( QWidget *container, KAction *action )
{
  if ( !d->m_highlight )
    return;

  QList<KAction> *actionList = d->m_dctHighlightContainers[ container ];

  if ( !actionList )
  {
    actionList = new QList<KAction>;

    if ( container->inherits( "QPopupMenu" ) )
    {
      connect( container, SIGNAL( highlighted( int ) ),
               this, SLOT( slotMenuItemHighlighted( int ) ) );
      connect( container, SIGNAL( aboutToHide() ),
               this, SLOT( slotMenuAboutToHide() ) );
    }
    else if ( container->inherits( "KToolBar" ) )
    {
      connect( container, SIGNAL( highlighted( int, bool ) ),
               this, SLOT( slotToolBarButtonHighlighted( int, bool ) ) );
    }

    connect( container, SIGNAL( destroyed() ),
             this, SLOT( slotDestroyed() ) );

    d->m_dctHighlightContainers.insert( container, actionList );
  }

  actionList->append( action );
}

void KActionCollection::disconnectHighlight( QWidget *container, KAction *action )
{
  if ( !d->m_highlight )
    return;

  QList<KAction> *actionList = d->m_dctHighlightContainers[ container ];

  if ( !actionList )
    return;

  actionList->removeRef( action );

  if ( actionList->count() == 0 )
    d->m_dctHighlightContainers.remove( container );
}

void KActionCollection::slotMenuItemHighlighted( int id )
{
  if ( !d->m_highlight )
    return;

  if ( d->m_currentHighlightAction )
    emit actionHighlighted( d->m_currentHighlightAction, false );

  QWidget *container = static_cast<QWidget *>( const_cast<QObject *>( sender() ) );

  d->m_currentHighlightAction = findAction( container, id );

  if ( !d->m_currentHighlightAction )
  {
      if ( !d->m_statusCleared )
          emit clearStatusText();
      d->m_statusCleared = true;
      return;
  }

  d->m_statusCleared = false;
  emit actionHighlighted( d->m_currentHighlightAction );
  emit actionHighlighted( d->m_currentHighlightAction, true );
  emit actionStatusText( d->m_currentHighlightAction->toolTip() );
}

void KActionCollection::slotMenuAboutToHide()
{
    if ( d->m_currentHighlightAction )
        emit actionHighlighted( d->m_currentHighlightAction, false );
    d->m_currentHighlightAction = 0;

    if ( !d->m_statusCleared )
        emit clearStatusText();
    d->m_statusCleared = true;
}

void KActionCollection::slotToolBarButtonHighlighted( int id, bool highlight )
{
  if ( !d->m_highlight )
    return;

  QWidget *container = static_cast<QWidget *>( const_cast<QObject *>( sender() ) );

  KAction *action = findAction( container, id );

  if ( !action )
  {
      d->m_currentHighlightAction = 0;
      // use tooltip groups for toolbar status text stuff instead (Simon)
//      emit clearStatusText();
      return;
  }

  emit actionHighlighted( action, highlight );

  if ( highlight )
    d->m_currentHighlightAction = action;
  else
  {
    d->m_currentHighlightAction = 0;
//    emit clearStatusText();
  }
}

void KActionCollection::slotDestroyed()
{
  d->m_dctHighlightContainers.remove( reinterpret_cast<void *>( const_cast<QObject *>(sender()) ) );
}

KAction *KActionCollection::findAction( QWidget *container, int id )
{
  QList<KAction> *actionList = d->m_dctHighlightContainers[ reinterpret_cast<void *>( container ) ];

  if ( !actionList )
    return 0;

  QListIterator<KAction> it( *actionList );
  for (; it.current(); ++it )
    if ( it.current()->isPlugged( container, id ) )
      return it.current();

  return 0;
}

#include "kaction.moc"

