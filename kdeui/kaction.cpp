/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
	      (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2002 Joseph Wenninger <jowenn@kde.org>

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

#ifdef KDE_USE_FINAL
#undef Always
#include <qdockwindow.h>
#endif

#include "kaction.h"
#include "kactionshortcutlist.h"

#include <assert.h>

#include <qfontdatabase.h>
#include <qobjectlist.h>
#include <qptrdict.h>
#include <qregexp.h>
#include <qtl.h>
#include <qtooltip.h>
#include <qvariant.h>
#include <qwhatsthis.h>
#include <qtimer.h>

#include <kaccel.h>
#include <kaccelbase.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kfontcombo.h>
#include <kglobalsettings.h>
#include <kguiitem.h>
#include <kiconloader.h>
#include <kmainwindow.h>
#include <kmenubar.h>
#include <kpopupmenu.h>
#include <kstdaccel.h>
#include <ktoolbar.h>
#include <ktoolbarbutton.h>
#include <kurl.h>
#include <klocale.h>

/**
* How it works.
* KActionCollection is an organizing container for KActions.
* KActionCollection keeps track of the information necessary to handle
* configuration and shortcuts.
*
* Focus Widget pointer:
* This is the widget which is the focus for action shortcuts.
* It is set either by passing a QWidget* to the KActionCollection constructor
* or by calling setWidget() if the widget wasn't known when the object was
* initially constructed (as in KXMLGUIClient and KParts::PartBase)
*
* Main Window pointer:
* The KMainWindow* is used during destruction.  If it is flagged that KMainWindow
* is destroying itself, then KAction::~KAction shouldn't unplug itself.
* This is set when the Focus Widget pointer is set.
* <not yet implemented>
*
* Shortcuts:
* An action's shortcut will not not be connected unless a focus widget has
* been specified in KActionCollection.
*
* XML Filename:
* This is used to save user-modified settings back to the *ui.rc file.
* It is set by KXMLGUIFactory.
*/

static KAccel* s_kaccelXML = 0;
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

//---------------------------------------------------------------------
// KAction::KActionPrivate
//---------------------------------------------------------------------

class KAction::KActionPrivate : public KGuiItem
{
public:
  KActionPrivate() : KGuiItem()
  {
    m_kaccel = 0;
    m_configurable = true;
    m_connected = false;
  }

  KAccel *m_kaccel;
  QValueList<KAccel*> m_kaccelList;

  QString m_groupText;
  QString m_group;

  KShortcut m_cut;
  KShortcut m_cutDefault;

  bool m_configurable;
  bool m_connected;	// 'true' if this action has been connected to a slot.
  QVariant m_slotParam;

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

//---------------------------------------------------------------------
// KAction
//---------------------------------------------------------------------

KAction::KAction( const QString& text, const KShortcut& cut,
             const QObject* receiver, const char* slot,
             KActionCollection* parent, const char* name )
: QObject( parent, name )
{
	initPrivate( text, cut, receiver, slot );
}

KAction::KAction( const QString& text, const QString& sIconName, const KShortcut& cut,
	const QObject* receiver, const char* slot,
	KActionCollection* parent, const char* name )
: QObject( parent, name )
{
	initPrivate( text, cut, receiver, slot );
	d->setIconName( sIconName );
}

KAction::KAction( const QString& text, const QIconSet& pix, const KShortcut& cut,
	const QObject* receiver, const char* slot,
	KActionCollection* parent, const char* name )
: QObject( parent, name )
{
	initPrivate( text, cut, receiver, slot );
	d->setIconSet( pix );
}

KAction::KAction( const KGuiItem& item, const KShortcut& cut,
	const QObject* receiver, const char* slot,
	KActionCollection* parent, const char* name )
: QObject( parent, name )
{
	initPrivate( item.text(), cut, receiver, slot );
	if( item.hasIconSet() )
		setIcon( item.iconName() );
}

#if KDE_VERSION < 400
KAction::KAction( const QString& text, const KShortcut& cut,
                  QObject* parent, const char* name )
 : QObject( parent, name )
{
    initPrivate( text, cut, 0, 0 );
}

KAction::KAction( const QString& text, const KShortcut& cut,
                  const QObject* receiver,
                  const char* slot, QObject* parent, const char* name )
 : QObject( parent, name )
{
    initPrivate( text, cut, receiver, slot );
}

KAction::KAction( const QString& text, const QIconSet& pix,
                  const KShortcut& cut,
                  QObject* parent, const char* name )
 : QObject( parent, name )
{
    initPrivate( text, cut, 0, 0 );
    setIconSet( pix );
}

KAction::KAction( const QString& text, const QString& pix,
                  const KShortcut& cut,
                  QObject* parent, const char* name )
: QObject( parent, name )
{
    initPrivate( text, cut, 0, 0 );
    d->setIconName( pix );
}

KAction::KAction( const QString& text, const QIconSet& pix,
                  const KShortcut& cut,
                  const QObject* receiver, const char* slot, QObject* parent,
                  const char* name )
 : QObject( parent, name )
{
    initPrivate( text, cut, receiver, slot );
    setIconSet( pix );
}

KAction::KAction( const QString& text, const QString& pix,
                  const KShortcut& cut,
                  const QObject* receiver, const char* slot, QObject* parent,
                  const char* name )
  : QObject( parent, name )
{
    initPrivate( text, cut, receiver, slot );
    d->setIconName(pix);
}

KAction::KAction( QObject* parent, const char* name )
 : QObject( parent, name )
{
    initPrivate( QString::null, KShortcut(), 0, 0 );
}
#endif // KDE_VERSION < 400

KAction::~KAction()
{
    kdDebug(129) << "KAction::~KAction( this = \"" << name() << "\" )" << endl; // -- ellis
#ifndef KDE_NO_COMPAT
     if (d->m_kaccel)
       unplugAccel();
#endif

    // If actionCollection hasn't already been destructed,
    if ( m_parentCollection ) {
        m_parentCollection->take( this );
        for( uint i = 0; i < d->m_kaccelList.count(); i++ )
            d->m_kaccelList[i]->remove( name() );
    }

    // Do not call unplugAll from here, as tempting as it sounds.
    // KAction is designed around the idea that you need to plug
    // _and_ to unplug it "manually". Unplugging leads to an important
    // slowdown when e.g. closing the window, in which case we simply
    // want to destroy everything asap, not to remove actions one by one
    // from the GUI.

    delete d; d = 0;
}

void KAction::initPrivate( const QString& text, const KShortcut& cut,
                  const QObject* receiver, const char* slot )
{
    d = new KActionPrivate;

    d->m_cutDefault = cut;

    m_parentCollection = dynamic_cast<KActionCollection *>( parent() );
    kdDebug(129) << "KAction::initPrivate(): this = " << this << " name = \"" << name() << "\" cut = " << cut.toStringInternal() << " m_parentCollection = " << m_parentCollection << endl;
    if ( m_parentCollection )
        m_parentCollection->insert( this );

    if ( receiver && slot ) {
        d->m_connected = true;
        if ( QString(slot).find( QRegExp("\\(\\s*int\\s*\\)") ) >= 0 ) {
            QRegExp rxVal("\\{(.+)\\}");
            rxVal.setMinimal( true );
            if ( rxVal.search( name() ) >= 0 ) {
                d->m_slotParam = rxVal.cap(1).toInt();
                connect( this, SIGNAL( activated( int ) ), receiver, slot );
            }
        }
        else
            connect( this, SIGNAL( activated() ), receiver, slot );
    }

    if( !cut.isNull() && qstrcmp( name(), "unnamed" ) == 0 )
        kdWarning(129) << "KAction::initPrivate(): trying to assign a shortcut (" << cut.toStringInternal() << ") to an unnamed action." << endl;
    d->setText( text );
    initShortcut( cut );
}

bool KAction::isPlugged() const
{
  return (containerCount() > 0) || d->m_kaccel;
}

bool KAction::isPlugged( const QWidget *container, int id ) const
{
  int i = findContainer( container );
  return ( i > -1 && itemId( i ) == id );
}

bool KAction::isPlugged( const QWidget *container, const QWidget *_representative ) const
{
  int i = findContainer( container );
  return ( i > -1 && representative( i ) == _representative );
}


/*
Three actionCollection conditions:
	1) Scope is known on creation and KAccel object is created (e.g. KMainWindow)
	2) Scope is unknown and no KAccel object is available (e.g. KXMLGUIClient)
		a) addClient() will be called on object
		b) we just want to add the actions to another KXMLGUIClient object

The question is how to do we incorporate #2b into the XMLGUI framework?


We have a KCommandHistory object with undo and redo actions in a passed actionCollection
We have a KoDoc object which holds a KCommandHistory object and the actionCollection
We have two KoView objects which both point to the same KoDoc object
Undo and Redo should be available in both KoView objects, and
	calling the undo->setEnabled() should affect both KoViews

When addClient is called, it needs to be able to find the undo and redo actions
When it calls plug() on them, they need to be inserted into the KAccel object of the appropriate KoView

In this case, the actionCollection belongs to KoDoc and we need to let it know that its shortcuts
have the same scope as the KoView actionCollection

KXMLGUIClient::addSubActionCollection

Document:
	create document actions

View
	create view actions
	add document actionCollection as sub-collection

A parentCollection is created
Scenario 1: parentCollection has a focus widget set (e.g. via KMainWindow)
	A KAccel object is created in the parentCollection
	A KAction is created with parent=parentCollection
	The shortcut is inserted into this actionCollection
	Scenario 1a: xml isn't used
		done
	Scenario 1b: KXMLGUIBuilder::addClient() called
		setWidget is called -- ignore
		shortcuts are set
Scenario 2: parentCollection has no focus widget (e.g., KParts)
	A KAction is created with parent=parentCollection
	Scenario 2a: xml isn't used
		no shortcuts
	Scenario 2b: KXMLGUIBuilder::addClient() called
		setWidget is called
		shortcuts are inserted into current KAccel
		shortcuts are set in all other KAccels, if the action is present in the other KAccels
*/

/*
shortcut may be set:
	- on construction
	- on plug
	- on reading XML
	- on plugAccel (deprecated)

On Construction: [via initShortcut()]
	insert into KAccel of m_parentCollection,
		if kaccel() && isAutoConnectShortcuts() exists

On Plug: [via plug() -> plugShortcut()]
	insert into KAccel of m_parentCollection, if exists and not already inserted into

On Read XML: [via setShortcut()]
	set in all current KAccels
	insert into KAccel of m_parentCollection, if exists and not already inserted into
*/

KAccel* KAction::kaccelCurrent()
{
  KAccel* kaccel = 0;

  if( s_kaccelXML )
    kaccel = s_kaccelXML;
  else if( m_parentCollection && m_parentCollection->kaccel() )
    kaccel = m_parentCollection->kaccel();

  return kaccel;
}

// Only to be called from initPrivate()
bool KAction::initShortcut( const KShortcut& cut )
{
    d->m_cut = cut;

    // Only insert action into KAccel if it has a valid name,
    if( qstrcmp( name(), "unnamed" ) != 0 &&
        m_parentCollection &&
        m_parentCollection->isAutoConnectShortcuts() &&
        m_parentCollection->kaccel() )
    {
        insertKAccel( m_parentCollection->kaccel() );
        return true;
    } else
        return false;
 }

// Only to be called from plug()
void KAction::plugShortcut()
{
  KAccel* kaccel = kaccelCurrent();

  //kdDebug(129) << "KAction::plugShortcut(): this = " << this << " kaccel() = " << (m_parentCollection ? m_parentCollection->kaccel() : 0) << endl;
  if( kaccel && qstrcmp( name(), "unnamed" ) != 0 ) {
    // Check if already plugged into current KAccel object
    for( uint i = 0; i < d->m_kaccelList.count(); i++ ) {
      if( d->m_kaccelList[i] == kaccel )
        return;
    }

    insertKAccel( kaccel );
  }
}

bool KAction::setShortcut( const KShortcut& cut )
{
  bool bChanged = (d->m_cut != cut);
  d->m_cut = cut;

  KAccel* kaccel = kaccelCurrent();
  bool bInsertRequired = true;
  // Apply new shortcut to all existing KAccel objects
  for( uint i = 0; i < d->m_kaccelList.count(); i++ ) {
    // Check whether shortcut has already been plugged into
    //  the current kaccel object.
    if( d->m_kaccelList[i] == kaccel )
      bInsertRequired = false;
    if( bChanged )
      updateKAccelShortcut( d->m_kaccelList[i] );
  }

  // Only insert action into KAccel if it has a valid name,
  if( kaccel && bInsertRequired && qstrcmp( name(), "unnamed" ) )
    insertKAccel( kaccel );

  if( bChanged ) {
#if KDE_VERSION < 400
    if ( d->m_kaccel )
      d->m_kaccel->setShortcut( name(), cut );
#endif
      int len = containerCount();
      for( int i = 0; i < len; ++i )
          updateShortcut( i );
  }
  return true;
}

bool KAction::updateKAccelShortcut( KAccel* kaccel )
{
  bool b = true;

  if ( !kaccel->actions().actionPtr( name() ) ) {
    if(!d->m_cut.isNull() ) {
      kdDebug(129) << "Inserting " << name() << ", " << d->text() << ", " << d->plainText() << endl;
      b = kaccel->insert( name(), d->plainText(), QString::null,
          d->m_cut,
          this, SLOT(slotActivated()),
          isShortcutConfigurable(), isEnabled() );
      if( !b )
        kdWarning(129) << "KAction::updateKAccelShortcut: KAccel::insert() failed." << endl;
    }
  }
  else
    b = kaccel->setShortcut( name(), d->m_cut );

  return b;
}

void KAction::insertKAccel( KAccel* kaccel )
{
  //kdDebug(129) << "KAction::insertKAccel( " << kaccel << " ): this = " << this << endl;
  if ( !kaccel->actions().actionPtr( name() ) ) {
    if( updateKAccelShortcut( kaccel ) ) {
      d->m_kaccelList.append( kaccel );
      connect( kaccel, SIGNAL(destroyed()), this, SLOT(slotDestroyed()) );
    }
    else
      kdWarning(129) << "KAction::insertKAccel: KAccel::insert() failed." << endl;
  }
  else
    kdWarning(129) << "KAction::insertKAccel( kaccel = " << kaccel << " ): KAccel object already contains an action name \"" << name() << "\"" << endl; // -- ellis
}

void KAction::removeKAccel( KAccel* kaccel )
{
  //kdDebug(129) << "KAction::removeKAccel( " << i << " ): this = " << this << endl;
  for( uint i = 0; i < d->m_kaccelList.count(); i++ ) {
    if( d->m_kaccelList[i] == kaccel ) {
      kaccel->remove( name() );
      d->m_kaccelList.remove( d->m_kaccelList.at( i ) );
      disconnect( kaccel, SIGNAL(destroyed()), this, SLOT(slotDestroyed()) );
      break;
    }
  }
}

#if KDE_VERSION < 400
void KAction::setAccel( int keyQt )
{
  setShortcut( KShortcut(keyQt) );
}
#endif

void KAction::updateShortcut( int i )
{
  int id = itemId( i );

  QWidget* w = container( i );
  if ( w->inherits( "QPopupMenu" ) ) {
    QPopupMenu* menu = static_cast<QPopupMenu*>(w);
    updateShortcut( menu, id );
  }
  // FIXME: It may be that the following should only be done
  //  if d->m_kaccel == 0, otherwise we may be setting up
  //  the shortcut twice. --ellis 2002/01/18
  else if ( w->inherits( "QMenuBar" ) )
    static_cast<QMenuBar*>(w)->setAccel( d->m_cut.keyCodeQt(), id );
}

// TODO: continue copying over diffs! -- ellis xxx

void KAction::updateShortcut( QPopupMenu* menu, int id )
{
  //kdDebug(129) << "KAction::updateShortcut(): this = " << this << " d->m_kaccelList.count() = " << d->m_kaccelList.count() << endl;
  // If the action has a KAccel object,
  //  show the string representation of its shortcut.
  if ( d->m_kaccel || d->m_kaccelList.count() ) {
    QString s = menu->text( id );
    int i = s.find( '\t' );
    if ( i >= 0 )
      s.replace( i+1, s.length()-i, d->m_cut.seq(0).toString() );
    else
      s += "\t" + d->m_cut.seq(0).toString();

    menu->changeItem( id, s );
  }
  // Otherwise insert the shortcut itself into the popup menu.
  else {
    // This is a fall-hack in case the KAction is missing a proper parent collection.
    //  It should be removed eventually. --ellis
    menu->setAccel( d->m_cut.keyCodeQt(), id );
    kdWarning(129) << "KAction::updateShortcut(): name = \"" << name() << "\", cut = " << d->m_cut.toStringInternal() << "; No KAccel, probably missing a parent collection." << endl;
  }
}

const KShortcut& KAction::shortcut() const
{
  return d->m_cut;
}

const KShortcut& KAction::shortcutDefault() const
{
  return d->m_cutDefault;
}

QString KAction::shortcutText() const
{
  return d->m_cut.toStringInternal();
}

void KAction::setShortcutText( const QString& s )
{
  setShortcut( KShortcut(s) );
}

int KAction::accel() const
{
  return d->m_cut.keyCodeQt();
}

void KAction::setGroup( const QString& grp )
{
  d->m_group = grp;

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    updateGroup( i );
}

void KAction::updateGroup( int )
{
  // DO SOMETHING
}

QString KAction::group() const
{
  return d->m_group;
}

bool KAction::isEnabled() const
{
  return d->isEnabled();
}

bool KAction::isShortcutConfigurable() const
{
  return d->m_configurable;
}

void KAction::setToolTip( const QString& tt )
{
  d->setToolTip( tt );

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    updateToolTip( i );
}

void KAction::updateToolTip( int i )
{
  QWidget *w = container( i );

  if ( w->inherits( "KToolBar" ) )
    QToolTip::add( static_cast<KToolBar*>(w)->getWidget( itemId( i ) ), d->toolTip() );
}

QString KAction::toolTip() const
{
  return d->toolTip();
}

int KAction::plug( QWidget *w, int index )
{
  //kdDebug(129) << "KAction::plug( " << w << ", " << index << " )" << endl;
  if (w == 0) {
	kdWarning(129) << "KAction::plug called with 0 argument\n";
 	return -1;
  }

#ifndef NDEBUG
  KAccel* kaccel = kaccelCurrent();
  // If there is a shortcut, but no KAccel available
  if( !d->m_cut.isNull() && kaccel == 0 ) {
    kdWarning(129) << "KAction::plug(): has no KAccel object; this = " << this << " name = " << name() << " parentCollection = " << m_parentCollection << endl; // ellis
    kdDebug(129) << kdBacktrace() << endl;
  }
#endif

  // Check if action is permitted
  if (kapp && !kapp->authorizeKAction(name()))
    return -1;

  plugShortcut();

  if ( w->inherits("QPopupMenu") )
  {
    QPopupMenu* menu = static_cast<QPopupMenu*>( w );
    int id;
    // Don't insert shortcut into menu if it's already in a KAccel object.
    int keyQt = (d->m_kaccelList.count() || d->m_kaccel) ? 0 : d->m_cut.keyCodeQt();

    if ( d->hasIcon() )
    {
        KInstance *instance;
        if ( m_parentCollection )
          instance = m_parentCollection->instance();
        else
          instance = KGlobal::instance();
        id = menu->insertItem( d->iconSet( KIcon::Small, 0, instance ), d->text(), this,//dsweet
                                 SLOT( slotActivated() ), keyQt,
                                 -1, index );
    }
    else
        id = menu->insertItem( d->text(), this,
                               SLOT( slotActivated() ),  //dsweet
                               keyQt, -1, index );

    // If the shortcut is already in a KAccel object, then
    //  we need to set the menu item's shortcut text.
    if ( d->m_kaccelList.count() || d->m_kaccel )
        updateShortcut( menu, id );

    // call setItemEnabled only if the item really should be disabled,
    // because that method is slow and the item is per default enabled
    if ( !d->isEnabled() )
        menu->setItemEnabled( id, false );

    if ( !d->whatsThis().isEmpty() )
        menu->setWhatsThis( id, whatsThisWithIcon() );

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
    KInstance *instance;
    if ( m_parentCollection )
      instance = m_parentCollection->instance();
    else
      instance = KGlobal::instance();

    if ( icon().isEmpty() ) // old code using QIconSet directly
    {
      bar->insertButton( iconSet().pixmap(), id_, SIGNAL( clicked() ), this,
                         SLOT( slotActivated() ),
                         d->isEnabled(), d->plainText(), index );
    }
    else
      bar->insertButton( d->iconName(), id_, SIGNAL( clicked() ), this,
                         SLOT( slotActivated() ),
                         d->isEnabled(), d->plainText(), index, instance );

    bar->getButton( id_ )->setName( QCString("toolbutton_")+name() );

    if ( !d->whatsThis().isEmpty() )
        QWhatsThis::add( bar->getButton(id_), whatsThisWithIcon() );

    if ( !d->toolTip().isEmpty() )
      QToolTip::add( bar->getButton(id_), d->toolTip() );

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
  int i = findContainer( w );
  if ( i == -1 )
    return;
  int id = itemId( i );

  if ( w->inherits( "QPopupMenu" ) )
  {
    QPopupMenu *menu = static_cast<QPopupMenu *>( w );
    menu->removeItem( id );
  }
  else if ( w->inherits( "KToolBar" ) )
  {
    KToolBar *bar = static_cast<KToolBar *>( w );
    bar->removeItem( id );
  }
  else if ( w->inherits( "QMenuBar" ) )
  {
    QMenuBar *bar = static_cast<QMenuBar *>( w );
    bar->removeItem( id );
  }

  removeContainer( i );
  if ( m_parentCollection )
    m_parentCollection->disconnectHighlight( w, this );
}

void KAction::plugAccel(KAccel *kacc, bool configurable)
{
  kdWarning(129) << "KAction::plugAccel(): call to deprecated action." << endl;
  kdDebug(129) << kdBacktrace() << endl;
  //kdDebug(129) << "KAction::plugAccel( kacc = " << kacc << " ): name \"" << name() << "\"" << endl;
  if ( d->m_kaccel )
    unplugAccel();

  // If the parent collection's accel ptr isn't set yet
  //if ( m_parentCollection && !m_parentCollection->accel() )
  //  m_parentCollection->setAccel( kacc );

  // We can only plug this action into the given KAccel object
  //  if it does not already contain an action with the same name.
  if ( !kacc->actions().actionPtr(name()) )
  {
    d->m_kaccel = kacc;
    d->m_kaccel->insert(name(), d->plainText(), QString::null,
        KShortcut(d->m_cut),
        this, SLOT(slotActivated()),
        configurable, isEnabled());
    connect(d->m_kaccel, SIGNAL(destroyed()), this, SLOT(slotDestroyed()));
    //connect(d->m_kaccel, SIGNAL(keycodeChanged()), this, SLOT(slotKeycodeChanged()));
  }
  else
    kdWarning(129) << "KAction::plugAccel( kacc = " << kacc << " ): KAccel object already contains an action name \"" << name() << "\"" << endl; // -- ellis
}

void KAction::unplugAccel()
{
  //kdDebug(129) << "KAction::unplugAccel() " << this << " " << name() << endl;
  if ( d->m_kaccel )
  {
    d->m_kaccel->remove(name());
    d->m_kaccel = 0;
  }
}

void KAction::plugMainWindowAccel( QWidget *w )
{
  // Note: topLevelWidget() stops too early, we can't use it.
  QWidget * tl = w;
  QWidget * n;
  while ( !tl->isDialog() && ( n = tl->parentWidget() ) ) // lookup parent and store
    tl = n;

  KMainWindow * mw = dynamic_cast<KMainWindow *>(tl); // try to see if it's a kmainwindow
  if (mw)
    plugAccel( mw->accel() );
  else
    kdDebug(129) << "KAction::plugMainWindowAccel: Toplevel widget isn't a KMainWindow, can't plug accel. " << tl << endl;
}

void KAction::setEnabled(bool enable)
{
  //kdDebug(129) << "KAction::setEnabled( " << enable << " ): this = " << this << " d->m_kaccelList.count() = " << d->m_kaccelList.count() << endl;
  if ( enable == d->isEnabled() )
    return;

#if KDE_VERSION < 400
  if (d->m_kaccel)
    d->m_kaccel->setEnabled(name(), enable);
#endif

  for ( uint i = 0; i < d->m_kaccelList.count(); i++ )
    d->m_kaccelList[i]->setEnabled( name(), enable );

  d->setEnabled( enable );

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    updateEnabled( i );

  emit enabled( d->isEnabled() );
}

void KAction::updateEnabled( int i )
{
    QWidget *w = container( i );

    if ( w->inherits("QPopupMenu") )
      static_cast<QPopupMenu*>(w)->setItemEnabled( itemId( i ), d->isEnabled() );
    else if ( w->inherits("QMenuBar") )
      static_cast<QMenuBar*>(w)->setItemEnabled( itemId( i ), d->isEnabled() );
    else if ( w->inherits( "KToolBar" ) )
      static_cast<KToolBar*>(w)->setItemEnabled( itemId( i ), d->isEnabled() );
}

void KAction::setShortcutConfigurable( bool b )
{
    d->m_configurable = b;
}

void KAction::setText( const QString& text )
{
#if KDE_VERSION < 400
  if (d->m_kaccel) {
    KAccelAction* pAction = d->m_kaccel->actions().actionPtr(name());
    if (pAction)
      pAction->setLabel( text );
  }
#endif

  for( uint i = 0; i < d->m_kaccelList.count(); i++ ) {
    KAccelAction* pAction = d->m_kaccelList[i]->actions().actionPtr(name());
    if (pAction)
      pAction->setLabel( text );
  }

  d->setText( text );

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    updateText( i );
}

void KAction::updateText( int i )
{
  QWidget *w = container( i );

  if ( w->inherits( "QPopupMenu" ) ) {
    int id = itemId( i );
    static_cast<QPopupMenu*>(w)->changeItem( id, d->text() );
    updateShortcut( static_cast<QPopupMenu*>(w), id );
  }
  else if ( w->inherits( "QMenuBar" ) )
    static_cast<QMenuBar*>(w)->changeItem( itemId( i ), d->text() );
  else if ( w->inherits( "KToolBar" ) )
  {
    QWidget *button = static_cast<KToolBar *>(w)->getWidget( itemId( i ) );
    if ( button->inherits( "KToolBarButton" ) )
      static_cast<KToolBarButton *>(button)->setText( d->plainText() );
  }
}

QString KAction::text() const
{
  return d->text();
}

QString KAction::plainText() const
{
  return d->plainText( );
}

void KAction::setIcon( const QString &icon )
{
  d->setIconName( icon );

  // now handle any toolbars
  int len = containerCount();
  for ( int i = 0; i < len; ++i )
    updateIcon( i );
}

void KAction::updateIcon( int id )
{
  QWidget* w = container( id );

  if ( w->inherits( "QPopupMenu" ) ) {
    int itemId_ = itemId( id );
    static_cast<QPopupMenu*>(w)->changeItem( itemId_, d->iconSet( KIcon::Small ), d->text() );
    updateShortcut( static_cast<QPopupMenu*>(w), itemId_ );
  }
  else if ( w->inherits( "QMenuBar" ) )
    static_cast<QMenuBar*>(w)->changeItem( itemId( id ), d->iconSet( KIcon::Small ), d->text() );
  else if ( w->inherits( "KToolBar" ) )
    static_cast<KToolBar *>(w)->setButtonIcon( itemId( id ), d->iconName() );
}

QString KAction::icon() const
{
  return d->iconName( );
}

void KAction::setIconSet( const QIconSet &iconset )
{
  d->setIconSet( iconset );

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    updateIconSet( i );
}


void KAction::updateIconSet( int id )
{
  QWidget *w = container( id );

  if ( w->inherits( "QPopupMenu" ) )
  {
    int itemId_ = itemId( id );
    static_cast<QPopupMenu*>(w)->changeItem( itemId_, d->iconSet(), d->text() );
    updateShortcut( static_cast<QPopupMenu*>(w), itemId_ );
  }
  else if ( w->inherits( "QMenuBar" ) )
    static_cast<QMenuBar*>(w)->changeItem( itemId( id ), d->iconSet(), d->text() );
  else if ( w->inherits( "KToolBar" ) )
  {
    if ( icon().isEmpty() && d->hasIconSet() ) // only if there is no named icon ( scales better )
      static_cast<KToolBar *>(w)->setButtonIconSet( itemId( id ), d->iconSet() );
    else
      static_cast<KToolBar *>(w)->setButtonIconSet( itemId( id ), d->iconSet( KIcon::Small ) );
  }
}

QIconSet KAction::iconSet( KIcon::Group group, int size ) const
{
    return d->iconSet( group, size );
}

bool KAction::hasIcon() const
{
  return d->hasIcon();
}

void KAction::setWhatsThis( const QString& text )
{
  d->setWhatsThis(  text );

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    updateWhatsThis( i );
}

void KAction::updateWhatsThis( int i )
{
  QPopupMenu* pm = popupMenu( i );
  if ( pm )
  {
    pm->setWhatsThis( itemId( i ), d->whatsThis() );
    return;
  }

  KToolBar *tb = toolBar( i );
  if ( tb )
  {
    QWidget *w = tb->getButton( itemId( i ) );
    QWhatsThis::remove( w );
    QWhatsThis::add( w, d->whatsThis() );
    return;
  }
}

QString KAction::whatsThis() const
{
  return d->whatsThis();
}

QString KAction::whatsThisWithIcon() const
{
    QString text = whatsThis();
    if (!d->iconName().isEmpty())
      return QString::fromLatin1("<img source=\"small|%1\"> %2").arg(d->iconName() ).arg(text);
    return text;
}

QWidget* KAction::container( int index ) const
{
  assert( index < containerCount() );
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

uint KAction::kaccelCount() const
{
  return d->m_kaccelList.count();
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
  switch( d->m_slotParam.type() ) {
    case QVariant::Invalid:
      emit activated();
      break;
    case QVariant::Int:
      emit activated( d->m_slotParam.toInt() );
      break;
    default:
      break;
  }
}

void KAction::slotDestroyed()
{
  kdDebug(129) << "KAction::slotDestroyed(): this = " << this << ", name = \"" << name() << "\", sender = " << sender() << endl;
  const QObject* o = sender();

#if KDE_VERSION < 400
  if ( o == d->m_kaccel )
  {
    d->m_kaccel = 0;
    return;
  }
#endif

  for( uint i = 0; i < d->m_kaccelList.count(); i++ )
  {
    if ( o == d->m_kaccelList[i] )
    {
      disconnect( d->m_kaccelList[i], SIGNAL(destroyed()), this, SLOT(slotDestroyed()) );
      d->m_kaccelList.remove( d->m_kaccelList.at( i ) );
      return;
    }
  }

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

// FIXME: Remove this (ellis)
void KAction::slotKeycodeChanged()
{
  kdDebug(129) << "KAction::slotKeycodeChanged()" << endl; // -- ellis
  KAccelAction* pAction = d->m_kaccel->actions().actionPtr(name());
  if( pAction )
    setShortcut(pAction->shortcut());
}

KActionCollection *KAction::parentCollection() const
{
    return m_parentCollection;
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

KToggleAction::KToggleAction( const QString& text, const KShortcut& cut,
                              QObject* parent,
                              const char* name )
    : KAction( text, cut, parent, name )
{
  d = new KToggleActionPrivate;
}

KToggleAction::KToggleAction( const QString& text, const KShortcut& cut,
                              const QObject* receiver, const char* slot,
                              QObject* parent, const char* name )
  : KAction( text, cut, receiver, slot, parent, name )
{
  d = new KToggleActionPrivate;
}

KToggleAction::KToggleAction( const QString& text, const QIconSet& pix,
                              const KShortcut& cut,
                              QObject* parent, const char* name )
  : KAction( text, pix, cut, parent, name )
{
  d = new KToggleActionPrivate;
}

KToggleAction::KToggleAction( const QString& text, const QString& pix,
                              const KShortcut& cut,
                              QObject* parent, const char* name )
 : KAction( text, pix, cut, parent, name )
{
  d = new KToggleActionPrivate;
}

KToggleAction::KToggleAction( const QString& text, const QIconSet& pix,
                              const KShortcut& cut,
                              const QObject* receiver,
                              const char* slot, QObject* parent,
                              const char* name )
  : KAction( text, pix, cut, receiver, slot, parent, name )
{
  d = new KToggleActionPrivate;
}

KToggleAction::KToggleAction( const QString& text, const QString& pix,
                              const KShortcut& cut,
                              const QObject* receiver,
                              const char* slot, QObject* parent,
                              const char* name )
  : KAction( text, pix, cut, receiver, slot, parent, name )
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
  if (kapp && !kapp->authorizeKAction(name()))
    return -1;

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
  //kdDebug(129) << "KToggleAction::setChecked(" << c << ") " << this << " " << name() << endl;

  d->m_checked = c;

  int len = containerCount();

  for( int i = 0; i < len; ++i )
    updateChecked( i );

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
}

void KToggleAction::updateChecked( int id )
{
  QWidget *w = container( id );

  if ( w->inherits( "QPopupMenu" ) )
    static_cast<QPopupMenu*>(w)->setItemChecked( itemId( id ), d->m_checked );
  else if ( w->inherits( "QMenuBar" ) )
    static_cast<QMenuBar*>(w)->setItemChecked( itemId( id ), d->m_checked );
  else if ( w->inherits( "KToolBar" ) )
  {
    QWidget* r = static_cast<KToolBar*>( w )->getButton( itemId( id ) );
    if ( r && r->inherits( "KToolBarButton" ) )
      static_cast<KToolBar*>( w )->setButton( itemId( id ), d->m_checked );
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


KRadioAction::KRadioAction( const QString& text, const KShortcut& cut,
                            QObject* parent, const char* name )
: KToggleAction( text, cut, parent, name )
{
}

KRadioAction::KRadioAction( const QString& text, const KShortcut& cut,
                            const QObject* receiver, const char* slot,
                            QObject* parent, const char* name )
: KToggleAction( text, cut, receiver, slot, parent, name )
{
}

KRadioAction::KRadioAction( const QString& text, const QIconSet& pix,
                            const KShortcut& cut,
                            QObject* parent, const char* name )
: KToggleAction( text, pix, cut, parent, name )
{
}

KRadioAction::KRadioAction( const QString& text, const QString& pix,
                            const KShortcut& cut,
                            QObject* parent, const char* name )
: KToggleAction( text, pix, cut, parent, name )
{
}

KRadioAction::KRadioAction( const QString& text, const QIconSet& pix,
                            const KShortcut& cut,
                            const QObject* receiver, const char* slot,
                            QObject* parent, const char* name )
: KToggleAction( text, pix, cut, receiver, slot, parent, name )
{
}

KRadioAction::KRadioAction( const QString& text, const QString& pix,
                            const KShortcut& cut,
                            const QObject* receiver, const char* slot,
                            QObject* parent, const char* name )
: KToggleAction( text, pix, cut, receiver, slot, parent, name )
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
    m_bRemoveAmpersandsInCombo = false;
    m_menu = 0;
    m_current = -1;
    m_comboWidth = -1;
  }
  bool m_edit;
  bool m_bRemoveAmpersandsInCombo;
  QPopupMenu *m_menu;
  int m_current;
  int m_comboWidth;
  QStringList m_list;
};

KSelectAction::KSelectAction( const QString& text, const KShortcut& cut,
                              QObject* parent, const char* name )
  : KAction( text, cut, parent, name )
{
  d = new KSelectActionPrivate;
}

KSelectAction::KSelectAction( const QString& text, const KShortcut& cut,
                              const QObject* receiver, const char* slot,
                              QObject* parent, const char* name )
  : KAction( text, cut, receiver, slot, parent, name )
{
  d = new KSelectActionPrivate;
}

KSelectAction::KSelectAction( const QString& text, const QIconSet& pix,
                              const KShortcut& cut,
                              QObject* parent, const char* name )
  : KAction( text, pix, cut, parent, name )
{
  d = new KSelectActionPrivate;
}

KSelectAction::KSelectAction( const QString& text, const QString& pix,
                              const KShortcut& cut,
                              QObject* parent, const char* name )
  : KAction( text, pix, cut, parent, name )
{
  d = new KSelectActionPrivate;
}

KSelectAction::KSelectAction( const QString& text, const QIconSet& pix,
                              const KShortcut& cut,
                              const QObject* receiver,
                              const char* slot, QObject* parent,
                              const char* name )
  : KAction( text, pix, cut, receiver, slot, parent, name )
{
  d = new KSelectActionPrivate;
}

KSelectAction::KSelectAction( const QString& text, const QString& pix,
                              const KShortcut& cut,
                              const QObject* receiver,
                              const char* slot, QObject* parent,
                              const char* name )
  : KAction( text, pix, cut, receiver, slot, parent, name )
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
        Q_ASSERT(id < (int)d->m_list.count());
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
        updateCurrentItem( i );

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
    updateComboWidth( i );

}
QPopupMenu* KSelectAction::popupMenu() const
{
	kdDebug(129) << "KAction::popupMenu()" << endl; // remove -- ellis
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
	kdDebug(129) << "KAction::setItems()" << endl; // remove -- ellis
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
    updateItems( i );

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

void KSelectAction::updateCurrentItem( int id )
{
  if ( d->m_current < 0 )
        return;

  QWidget* w = container( id );
  if ( w->inherits( "KToolBar" ) ) {
    QWidget* r = static_cast<KToolBar*>( w )->getWidget( itemId( id ) );
    if ( r->inherits( "QComboBox" ) ) {
      QComboBox *b = static_cast<QComboBox*>( r );
      b->setCurrentItem( d->m_current );
    }
  }
}

int KSelectAction::comboWidth() const
{
  return d->m_comboWidth;
}

void KSelectAction::updateComboWidth( int id )
{
  QWidget* w = container( id );
  if ( w->inherits( "KToolBar" ) ) {
    QWidget* r = static_cast<KToolBar*>( w )->getWidget( itemId( id ) );
    if ( r->inherits( "QComboBox" ) ) {
      QComboBox *cb = static_cast<QComboBox*>( r );
      cb->setMinimumWidth( d->m_comboWidth );
      cb->setMaximumWidth( d->m_comboWidth );
    }
  }
}

void KSelectAction::updateItems( int id )
{
	kdDebug(129) << "KAction::updateItems( " << id << ", lst )" << endl; // remove -- ellis
  QWidget* w = container( id );
  if ( w->inherits( "KToolBar" ) ) {
    QWidget* r = static_cast<KToolBar*>( w )->getWidget( itemId( id ) );
    if ( r->inherits( "QComboBox" ) ) {
      QComboBox *cb = static_cast<QComboBox*>( r );
      cb->clear();
      QStringList lst = comboItems();
      QStringList::ConstIterator it = lst.begin();
      for( ; it != lst.end(); ++it )
        cb->insertItem( *it );
      // Ok, this currently doesn't work due to a bug in QComboBox
      // (the sizehint is cached for ever and never recalculated)
      // Bug reported (against Qt 2.3.1).
      cb->setMinimumWidth( cb->sizeHint().width() );
    }
   }
}

int KSelectAction::plug( QWidget *widget, int index )
{
  if (kapp && !kapp->authorizeKAction(name()))
    return -1;
  kdDebug(129) << "KAction::plug( " << widget << ", " << index << " )" << endl; // remove -- ellis
  if ( widget->inherits("QPopupMenu") )
  {
    // Create the PopupMenu and store it in m_menu
    (void)popupMenu();

    QPopupMenu* menu = static_cast<QPopupMenu*>( widget );
    int id;
    if ( hasIconSet() )
      id = menu->insertItem( iconSet(), text(), d->m_menu, -1, index );
    else
      id = menu->insertItem( text(), d->m_menu, -1, index );

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
    bar->insertCombo( comboItems(), id_, isEditable(),
                      SIGNAL( activated( const QString & ) ), this,
                      SLOT( slotActivated( const QString & ) ), isEnabled(),
                      toolTip(), -1, index );

    QComboBox *cb = bar->getCombo( id_ );
    if ( cb )
    {
      if (!isEditable()) cb->setFocusPolicy(QWidget::NoFocus);
      cb->setMinimumWidth( cb->sizeHint().width() );
      if ( d->m_comboWidth > 0 )
      {
        cb->setMinimumWidth( d->m_comboWidth );
        cb->setMaximumWidth( d->m_comboWidth );
      }
      cb->setInsertionPolicy( QComboBox::NoInsertion );
    }

    addContainer( bar, id_ );

    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    updateCurrentItem( containerCount() - 1 );

    return containerCount() - 1;
  }

  kdWarning() << "Can not plug KAction in " << widget->className() << endl;
  return -1;
}

QStringList KSelectAction::comboItems() const
{
  if( d->m_bRemoveAmpersandsInCombo ) {
    QStringList lst;
    QStringList::ConstIterator it = d->m_list.begin();
    for( ; it != d->m_list.end(); ++it )
    {
      QString item = *it;
      int i = item.find( '&' );
      if ( i > -1 )
        item = item.remove( i, 1 );
      lst.append( item );
    }
    return lst;
  }
  else
    return d->m_list;
}

void KSelectAction::clear()
{
  if ( d->m_menu )
    d->m_menu->clear();

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    updateClear( i );
}

void KSelectAction::updateClear( int id )
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
  // Delay this. Especially useful when the slot connected to activated() will re-create
  // the menu, e.g. in the recent files action. This prevents a crash.
  QTimer::singleShot( 0, this, SLOT( slotActivated() ) );
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

  int i = items().findIndex( text );
  if ( i > -1 )
      setCurrentItem( i );
  else
      setCurrentItem( comboItems().findIndex( text ) );
  // Delay this. Especially useful when the slot connected to activated() will re-create
  // the menu, e.g. in the recent files action. This prevents a crash.
  QTimer::singleShot( 0, this, SLOT( slotActivated() ) );
}

void KSelectAction::slotActivated()
{
  KAction::slotActivated();
  kdDebug(129) << "KSelectAction::slotActivated currentItem=" << currentItem() << " currentText=" << currentText() << endl;
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

void KSelectAction::setRemoveAmpersandsInCombo( bool b )
{
    d->m_bRemoveAmpersandsInCombo = b;
}

bool KSelectAction::removeAmpersandsInCombo() const
{
    return d->m_bRemoveAmpersandsInCombo;
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

KListAction::KListAction( const QString& text, const KShortcut& cut,
                          QObject* parent, const char* name )
  : KSelectAction( text, cut, parent, name )
{
  d = new KListActionPrivate;
}

KListAction::KListAction( const QString& text, const KShortcut& cut,
                          const QObject* receiver, const char* slot,
                          QObject* parent, const char* name )
  : KSelectAction( text, cut, receiver, slot, parent, name )
{
  d = new KListActionPrivate;
}

KListAction::KListAction( const QString& text, const QIconSet& pix,
                          const KShortcut& cut,
                          QObject* parent, const char* name )
  : KSelectAction( text, pix, cut, parent, name )
{
  d = new KListActionPrivate;
}

KListAction::KListAction( const QString& text, const QString& pix,
                          const KShortcut& cut,
                          QObject* parent, const char* name )
  : KSelectAction( text, pix, cut, parent, name )
{
  d = new KListActionPrivate;
}

KListAction::KListAction( const QString& text, const QIconSet& pix,
                          const KShortcut& cut, const QObject* receiver,
                          const char* slot, QObject* parent,
                          const char* name )
  : KSelectAction( text, pix, cut, receiver, slot, parent, name )
{
  d = new KListActionPrivate;
}

KListAction::KListAction( const QString& text, const QString& pix,
                          const KShortcut& cut, const QObject* receiver,
                          const char* slot, QObject* parent,
                          const char* name )
  : KSelectAction( text, pix, cut, receiver, slot, parent, name )
{
  d = new KListActionPrivate;
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

KRecentFilesAction::KRecentFilesAction( const QString& text,
                                        const KShortcut& cut,
                                        QObject* parent, const char* name,
                                        unsigned int maxItems )
  : KListAction( text, cut, parent, name)
{
  d = new KRecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  connect( this, SIGNAL( activated( const QString& ) ),
           this, SLOT( itemSelected( const QString& ) ) );
}

KRecentFilesAction::KRecentFilesAction( const QString& text,
                                        const KShortcut& cut,
                                        const QObject* receiver,
                                        const char* slot,
                                        QObject* parent, const char* name,
                                        uint maxItems )
  : KListAction( text, cut, parent, name)
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
                                        const QIconSet& pix,
                                        const KShortcut& cut,
                                        QObject* parent, const char* name,
                                        uint maxItems )
  : KListAction( text, pix, cut, parent, name)
{
  d = new KRecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  connect( this, SIGNAL( activated( const QString& ) ),
           this, SLOT( itemSelected( const QString& ) ) );
}

KRecentFilesAction::KRecentFilesAction( const QString& text,
                                        const QString& pix,
                                        const KShortcut& cut,
                                        QObject* parent, const char* name,
                                        uint maxItems )
  : KListAction( text, pix, cut, parent, name)
{
  d = new KRecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  connect( this, SIGNAL( activated( const QString& ) ),
           this, SLOT( itemSelected( const QString& ) ) );
}

KRecentFilesAction::KRecentFilesAction( const QString& text,
                                        const QIconSet& pix,
                                        const KShortcut& cut,
                                        const QObject* receiver,
                                        const char* slot,
                                        QObject* parent, const char* name,
                                        uint maxItems )
  : KListAction( text, pix, cut, parent, name)
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
                                        const QString& pix,
                                        const KShortcut& cut,
                                        const QObject* receiver,
                                        const char* slot,
                                        QObject* parent, const char* name,
                                        uint maxItems )
  : KListAction( text, pix, cut, parent, name)
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

KFontAction::KFontAction( const QString& text,
                          const KShortcut& cut, QObject* parent,
                          const char* name )
  : KSelectAction( text, cut, parent, name )
{
    d = new KFontActionPrivate;
    get_fonts( d->m_fonts );
    KSelectAction::setItems( d->m_fonts );
    setEditable( TRUE );
}

KFontAction::KFontAction( const QString& text, const KShortcut& cut,
                          const QObject* receiver, const char* slot,
                          QObject* parent, const char* name )
    : KSelectAction( text, cut, receiver, slot, parent, name )
{
    d = new KFontActionPrivate;
    get_fonts( d->m_fonts );
    KSelectAction::setItems( d->m_fonts );
    setEditable( TRUE );
}

KFontAction::KFontAction( const QString& text, const QIconSet& pix,
                          const KShortcut& cut,
                          QObject* parent, const char* name )
    : KSelectAction( text, pix, cut, parent, name )
{
    d = new KFontActionPrivate;
    get_fonts( d->m_fonts );
    KSelectAction::setItems( d->m_fonts );
    setEditable( TRUE );
}

KFontAction::KFontAction( const QString& text, const QString& pix,
                          const KShortcut& cut,
                          QObject* parent, const char* name )
    : KSelectAction( text, pix, cut, parent, name )
{
    d = new KFontActionPrivate;
    get_fonts( d->m_fonts );
    KSelectAction::setItems( d->m_fonts );
    setEditable( TRUE );
}

KFontAction::KFontAction( const QString& text, const QIconSet& pix,
                          const KShortcut& cut,
                          const QObject* receiver, const char* slot,
                          QObject* parent, const char* name )
    : KSelectAction( text, pix, cut, receiver, slot, parent, name )
{
    d = new KFontActionPrivate;
    get_fonts( d->m_fonts );
    KSelectAction::setItems( d->m_fonts );
    setEditable( TRUE );
}

KFontAction::KFontAction( const QString& text, const QString& pix,
                          const KShortcut& cut,
                          const QObject* receiver, const char* slot,
                          QObject* parent, const char* name )
    : KSelectAction( text, pix, cut, receiver, slot, parent, name )
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

/*
 * Maintenance note: Keep in sync with KFontCombo::setCurrentFont()
 */
void KFontAction::setFont( const QString &family )
{
    QString lowerName = family.lower();
    int i = 0;
    for ( QStringList::Iterator it = d->m_fonts.begin(); it != d->m_fonts.end(); ++it, ++i )
    {
       if ((*it).lower() == lowerName)
       {
          setCurrentItem(i);
          return;
       }
    }
    i = lowerName.find(" [");
    if (i>-1)
    {
       lowerName = lowerName.left(i);
       i = 0;
       for ( QStringList::Iterator it = d->m_fonts.begin(); it != d->m_fonts.end(); ++it, ++i )
       {
          if ((*it).lower() == lowerName)
          {
             setCurrentItem(i);
             return;
          }
       }
    }

    lowerName += " [";
    i = 0;
    for ( QStringList::Iterator it = d->m_fonts.begin(); it != d->m_fonts.end(); ++it, ++i )
    {
       if ((*it).lower().startsWith(lowerName))
       {
          setCurrentItem(i);
          return;
       }
    }
    kdDebug(129) << "Font not found " << family.lower() << endl;
}

int KFontAction::plug( QWidget *w, int index )
{
  if (kapp && !kapp->authorizeKAction(name()))
    return -1;
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

    updateCurrentItem( containerCount() - 1 );

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

KFontSizeAction::KFontSizeAction( const QString& text,
                                  const KShortcut& cut,
                                  QObject* parent, const char* name )
  : KSelectAction( text, cut, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( const QString& text,
                                  const KShortcut& cut,
                                  const QObject* receiver, const char* slot,
                                  QObject* parent, const char* name )
  : KSelectAction( text, cut, receiver, slot, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( const QString& text, const QIconSet& pix,
                                  const KShortcut& cut,
                                  QObject* parent, const char* name )
  : KSelectAction( text, pix, cut, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( const QString& text, const QString& pix,
                                  const KShortcut& cut,
                                  QObject* parent, const char* name )
  : KSelectAction( text, pix, cut, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( const QString& text, const QIconSet& pix,
                                  const KShortcut& cut,
                                  const QObject* receiver,
                                  const char* slot, QObject* parent,
                                  const char* name )
    : KSelectAction( text, pix, cut, receiver, slot, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( const QString& text, const QString& pix,
                                  const KShortcut& cut,
                                  const QObject* receiver,
                                  const char* slot, QObject* parent,
                                  const char* name )
  : KSelectAction( text, pix, cut, receiver, slot, parent, name )
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

    if ( size < 1 ) {
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
  setShortcutConfigurable( false );
}

KActionMenu::KActionMenu( const QString& text, QObject* parent,
                          const char* name )
  : KAction( text, 0, parent, name )
{
  d = new KActionMenuPrivate;
  setShortcutConfigurable( false );
}

KActionMenu::KActionMenu( const QString& text, const QIconSet& icon,
                          QObject* parent, const char* name )
  : KAction( text, icon, 0, parent, name )
{
  d = new KActionMenuPrivate;
  setShortcutConfigurable( false );
}

KActionMenu::KActionMenu( const QString& text, const QString& icon,
                          QObject* parent, const char* name )
  : KAction( text, icon, 0, parent, name )
{
  d = new KActionMenuPrivate;
  setShortcutConfigurable( false );
}

KActionMenu::~KActionMenu()
{
    unplugAll();
    kdDebug(129) << "KActionMenu::~KActionMenu()" << endl; // ellis
    delete d; d = 0;
}

void KActionMenu::popup( const QPoint& global )
{
  popupMenu()->popup( global );
}

KPopupMenu* KActionMenu::popupMenu() const
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
  if (kapp && !kapp->authorizeKAction(name()))
    return -1;
  kdDebug(129) << "KAction::plug( " << widget << ", " << index << " )" << endl; // remove -- ellis
  if ( widget->inherits("QPopupMenu") )
  {
    QPopupMenu* menu = static_cast<QPopupMenu*>( widget );
    int id;
    if ( hasIconSet() )
      id = menu->insertItem( iconSet(), text(), d->m_popup, -1, index );
    else
      id = menu->insertItem( text(), d->m_popup, -1, index );

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

////////


class KToolBarMenuAction::KToolBarMenuActionPrivate
{
	public:
	KMainWindow *window;
	QMap<KToolBar*,	KToggleToolBarAction*> toolbarActionMap;
        KPopupMenu *popup;

};

class KToolBarMenuAction::NullItem: public QCustomMenuItem
{
public:
	NullItem():QCustomMenuItem(){;}
	virtual ~NullItem(){;}
	virtual QSize sizeHint(){return QSize(0,0);}
	virtual	void paint ( QPainter * /*p*/, const QColorGroup & /*cg*/, bool /*act*/, bool /*enabled*/, int /*x*/, int /*y*/, int /*w*/, int /*h*/ ) {
		return;
	}
};

KToolBarMenuAction::KToolBarMenuAction( KMainWindow* parent,
                 const char* name):KAction(parent->actionCollection(),name)
{
	d=new KToolBarMenuActionPrivate;
	d->window=parent;
        d->popup = new KPopupMenu(parent,"KToolBarMenuAction::KToolBarMenuAction");
}

KToolBarMenuAction::~KToolBarMenuAction()
{
	delete d;
 	d=0;
}   

void KToolBarMenuAction::clear()
{
	d->popup->clear();
	for (QMap<KToolBar*,KToggleToolBarAction*>::iterator it=d->toolbarActionMap.begin();it!=d->toolbarActionMap.end();++it)
	{
		delete it.data();
	}
	d->toolbarActionMap.clear();
}

void KToolBarMenuAction::addToolbar(KToolBar *bar)
{
	uint oldcount=d->toolbarActionMap.count();
	KToggleToolBarAction *act;
	d->toolbarActionMap.insert(bar,act=new KToggleToolBarAction(
                bar->name(), i18n("Show %1").arg(bar->label()),
                0, 0));
	connect(act,SIGNAL(activated()),this,SLOT(slotActivated()));
	if (oldcount==0)
	{
		setText(i18n("Show %1").arg(d->toolbarActionMap.begin().key()->label()));
		connect(act,SIGNAL(toggled(bool)),this,SLOT(singleBarToggled(bool)));
		act->plug(d->popup);
		for (int i=0;i<containerCount();i++)
		{
		  	if ( container(i)->inherits("QPopupMenu") )
			{

				QPopupMenu *menu=static_cast<QPopupMenu*>(container(i));
				int id=itemId(i);
				int pos=menu->indexOf(id);
				unplug(menu);
				plug(menu,pos);
//				menu->removeItem(id);  I don't know why that doesn't work
//				menu->insertItem(text(),d->popup,id,pos);		I don't know why that doesn't work
			}
		}
	}
	else if (oldcount==1)
	{
		setText(i18n("Toolbars"));
		act->plug(d->popup);
		for (int i=0;i<containerCount();i++)
		{
		  	if ( container(i)->inherits("QPopupMenu") )
			{

				QPopupMenu *menu=static_cast<QPopupMenu*>(container(i));
				int id=itemId(i);
				int pos=menu->indexOf(id);
				unplug(menu);
				plug(menu,pos);
//				menu->removeItem(id);  I don't know why that doesn't work
//				menu->insertItem(text(),d->popup,id,pos);		I don't know why that doesn't work
			}
		}

	}
	else
	{
		act->plug(d->popup);
	}
}

void KToolBarMenuAction::removeToolbar(KToolBar * /*bar*/)
{
//	d->toolbarActionMap.remove(bar);

}

int KToolBarMenuAction::plug( QWidget* widget, int index )
{
//  return KAction::plug(widget,index);
  if (kapp && !kapp->authorizeKAction(name()))
    return -1;
  kdDebug(129) << "KAction::plug( " << widget << ", " << index << " )" << endl; // remove -- ellis
  if ( widget->inherits("QPopupMenu") )
  {
    QPopupMenu* menu = static_cast<QPopupMenu*>( widget );
    int id;
      if (d->toolbarActionMap.count()>1)
	      id = menu->insertItem( text(), d->popup, -1, index );
	else 
	if (d->toolbarActionMap.count()==1)
	{
		id=menu->insertItem(text(),-1,index);
		menu->setItemChecked(id,d->toolbarActionMap.begin().data()->isChecked());
		menu->connectItem(id,d->toolbarActionMap.begin().data(),SLOT(slotActivated()));
		
	}
	else 
	{
		id=menu->insertItem(new KToolBarMenuAction::NullItem(),-1,index);
//		id=menu->insertItem(text(),-1,index);
		menu->setItemEnabled(id,false);
	}

    if ( !isEnabled() )
        menu->setItemEnabled( id, false );

    addContainer( menu, id );
    connect( menu, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  } else return -1;
}


void KToolBarMenuAction::singleBarToggled(bool)
{
	if (d->toolbarActionMap.count()==1)
	{
		bool checked=d->toolbarActionMap.begin().data()->isChecked();
                for (int i=0;i<containerCount();i++)
                {
                        if ( container(i)->inherits("QPopupMenu") )
                        {

                                QPopupMenu *menu=static_cast<QPopupMenu*>(container(i));
                                int id=itemId(i);
				menu->setItemChecked(id,checked);
                        }
                }
	
	}
	
}

void KToolBarMenuAction::slotActivated()
{
	emit activated();
}

////////

KToolBarPopupAction::KToolBarPopupAction( const QString& text,
                                          const QString& icon,
                                          const KShortcut& cut,
                                          QObject* parent, const char* name )
  : KAction( text, icon, cut, parent, name )
{
  m_popup = 0;
  m_delayed = true;
  m_stickyMenu = true;
}

KToolBarPopupAction::KToolBarPopupAction( const QString& text,
                                          const QString& icon,
                                          const KShortcut& cut,
                                          const QObject* receiver,
                                          const char* slot, QObject* parent,
                                          const char* name )
  : KAction( text, icon, cut, receiver, slot, parent, name )
{
  m_popup = 0;
  m_delayed = true;
  m_stickyMenu = true;
}

KToolBarPopupAction::KToolBarPopupAction( const KGuiItem& item,
                                          const KShortcut& cut,
                                          const QObject* receiver,
                                          const char* slot, KActionCollection* parent,
                                          const char* name )
  : KAction( item, cut, receiver, slot, parent, name )
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
  if (kapp && !kapp->authorizeKAction(name()))
    return -1;
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

KPopupMenu *KToolBarPopupAction::popupMenu() const
{
    if ( !m_popup ) {
	KToolBarPopupAction *that = const_cast<KToolBarPopupAction*>(this);
	that->m_popup = new KPopupMenu;
    }
    return m_popup;
}

////////

KToggleToolBarAction::KToggleToolBarAction( const char* toolBarName,
         const QString& text, KActionCollection* parent, const char* name )
  : KToggleAction( text, KShortcut(), parent, name )
  , m_toolBarName( toolBarName )
  , m_toolBar( 0L )
{
}

KToggleToolBarAction::KToggleToolBarAction( KToolBar *toolBar, const QString &text,
                                            KActionCollection *parent, const char *name )
  : KToggleAction( text, KShortcut(), parent, name )
  , m_toolBarName( 0 ), m_toolBar( toolBar )
{
}

KToggleToolBarAction::~KToggleToolBarAction()
{
}

int KToggleToolBarAction::plug( QWidget* w, int index )
{
  if ( !m_toolBar ) {
    // Note: topLevelWidget() stops too early, we can't use it.
    QWidget * tl = w;
    QWidget * n;
    while ( !tl->isDialog() && ( n = tl->parentWidget() ) ) // lookup parent and store
      tl = n;

    KMainWindow * mw = dynamic_cast<KMainWindow *>(tl); // try to see if it's a kmainwindow

    if ( mw )
	m_toolBar = mw->toolBar( m_toolBarName );
  }

  if( m_toolBar ) {
    setChecked( m_toolBar->isVisible() );
    connect( m_toolBar, SIGNAL(visibilityChanged(bool)), this, SLOT(setChecked(bool)) );
    // Also emit toggled when the toolbar's visibility changes (see comment in header)
    connect( m_toolBar, SIGNAL(visibilityChanged(bool)), this, SIGNAL(toggled(bool)) );
  } else {
    setEnabled( false );
  }

  return KToggleAction::plug( w, index );
}

void KToggleToolBarAction::setChecked( bool c )
{
  if( m_toolBar && c != m_toolBar->isVisible() ) {
    if( c ) {
      m_toolBar->show();
    } else {
      m_toolBar->hide();
    }
  }
  KToggleAction::setChecked( c );
}

////////

KWidgetAction::KWidgetAction( QWidget* widget,
    const QString& text, const KShortcut& cut,
    const QObject* receiver, const char* slot,
    KActionCollection* parent, const char* name )
  : KAction( text, cut, receiver, slot, parent, name )
  , m_widget( widget )
  , m_autoSized( false )
{
}

KWidgetAction::~KWidgetAction()
{
}

void KWidgetAction::setAutoSized( bool autoSized )
{
  if( m_autoSized == autoSized )
    return;

  m_autoSized = autoSized;

  if( !m_widget || !isPlugged() )
    return;

  KToolBar* toolBar = (KToolBar*)m_widget->parent();
  int i = findContainer( toolBar );
  if ( i == -1 )
    return;
  int id = itemId( i );

  toolBar->setItemAutoSized( id, m_autoSized );
}

int KWidgetAction::plug( QWidget* w, int index )
{
  if ( !w->inherits( "KToolBar" ) ) {
    kdError() << "KWidgetAction::plug: KWidgetAction must be plugged into KToolBar." << endl;
    return -1;
  }
  if ( !m_widget ) {
    kdError() << "KWidgetAction::plug: Widget was deleted or null!" << endl;
    return -1;
  }

  KToolBar* toolBar = static_cast<KToolBar*>( w );

  int id = KAction::getToolButtonID();

  m_widget->reparent( toolBar, QPoint() );
  toolBar->insertWidget( id, 0, m_widget, index );
  toolBar->setItemAutoSized( id, m_autoSized );

  addContainer( toolBar, id );

  connect( toolBar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

  return containerCount() - 1;
}

void KWidgetAction::unplug( QWidget *w )
{
  // ### shouldn't this method check if w == m_widget->parent() ? (Simon)
  if( !m_widget )
    return;

  m_widget->reparent( 0L, QPoint(), false /*showIt*/ );

  KAction::unplug( w );
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

class KActionCollection::KActionCollectionPrivate
{
public:
  KActionCollectionPrivate()
  {
    m_instance = 0;
    //m_bOneKAccelOnly = false;
    //m_iWidgetCurrent = 0;
    m_bAutoConnectShortcuts = true;
    m_widget = m_builderWidget = 0;
    m_kaccel = m_builderKAccel = 0;
    m_dctHighlightContainers.setAutoDelete( true );
    m_highlight = false;
    m_currentHighlightAction = 0;
    m_statusCleared = true;
    m_mw=0;
  }

  KInstance *m_instance;
  QString m_sXMLFile;
  bool m_bAutoConnectShortcuts;
  //bool m_bOneKAccelOnly;
  //int m_iWidgetCurrent;
  //QValueList<QWidget*> m_widgetList;
  //QValueList<KAccel*> m_kaccelList;
  QValueList<KActionCollection*> m_docList;
  QWidget *m_widget;
  KAccel *m_kaccel;
  QWidget *m_builderWidget;
  KAccel *m_builderKAccel;
 
  QAsciiDict<KAction> m_actionDict;
  QPtrDict< QPtrList<KAction> > m_dctHighlightContainers;
  bool m_highlight;
  KAction *m_currentHighlightAction;
  bool m_statusCleared;
  KMainWindow *m_mw;
};

KActionCollection::KActionCollection( QWidget *parent, const char *name,
                                      KInstance *instance )
  : QObject( parent, name )
{
  kdDebug(129) << "KActionCollection::KActionCollection( " << parent << ", " << name << " ): this = " << this << endl; // ellis
  d = new KActionCollectionPrivate;
  if( parent )
    setWidget( parent );
  //d->m_bOneKAccelOnly = (d->m_kaccelList.count() > 0);
  setInstance( instance );
}


KActionCollection::KActionCollection( QWidget *watch, QObject* parent, const char *name,
                                      KInstance *instance )
  : QObject( parent, name )
{
  kdDebug(129) << "KActionCollection::KActionCollection( " << watch << ", " << parent << ", " << name << " ): this = " << this << endl; //ellis
  d = new KActionCollectionPrivate;
  if( watch )
    setWidget( watch );
  //d->m_bOneKAccelOnly = (d->m_kaccelList.count() > 0);
  setInstance( instance );
}

#if KDE_VERSION < 400
KActionCollection::KActionCollection( QObject *parent, const char *name,
                                      KInstance *instance )
  : QObject( parent, name )
{
  kdWarning(129) << "KActionCollection::KActionCollection( QObject *parent, const char *name, KInstance *instance )" << endl; //ellis
  kdDebug(129) << kdBacktrace() << endl;
  d = new KActionCollectionPrivate;
  QWidget* w = dynamic_cast<QWidget*>( parent );
  if( w )
    setWidget( w );
  //d->m_bOneKAccelOnly = (d->m_kaccelList.count() > 0);
  setInstance( instance );
}

KActionCollection::KActionCollection( const KActionCollection &copy )
    : QObject()
{
  kdWarning(129) << "KActionCollection::KActionCollection( const KActionCollection & ): function is severely deprecated." << endl;
  d = new KActionCollectionPrivate;
  *this = copy;
}
#endif

KActionCollection::~KActionCollection()
{
  kdDebug(129) << "KActionCollection::~KActionCollection(): this = " << this << endl;
  for ( QAsciiDictIterator<KAction> it( d->m_actionDict ); it.current(); ++it ) {
    KAction* pAction = it.current();
    if ( pAction->m_parentCollection == this )
      pAction->m_parentCollection = 0L;
  }

  delete d->m_kaccel;
  delete d->m_builderKAccel;
  delete d; d = 0;
}

void KActionCollection::setMainActionCollectionFor(KMainWindow *w){d->m_mw=w;}
KMainWindow *KActionCollection::mainActionCollectionFor(){return d->m_mw;}


void KActionCollection::setWidget( QWidget* w )
{
  //if ( d->m_actionDict.count() > 0 ) {
  //  kdError(129) << "KActionCollection::setWidget(): must be called before any actions are added to collection!" << endl;
  //  kdDebug(129) << kdBacktrace() << endl;
  //}
  //else
  if ( !d->m_widget ) {
    d->m_widget = w;
    d->m_kaccel = new KAccel( w, this, "KActionCollection-KAccel" );
  }
  else if ( d->m_widget != w )
    kdWarning(129) << "KActionCollection::setWidget(): tried to change widget from " << d->m_widget << " to " << w << endl;
}

void KActionCollection::setAutoConnectShortcuts( bool b )
{
  d->m_bAutoConnectShortcuts = b;
}

bool KActionCollection::isAutoConnectShortcuts()
{
  return d->m_bAutoConnectShortcuts;
}

bool KActionCollection::addDocCollection( KActionCollection* pDoc )
{
	d->m_docList.append( pDoc );
	return true;
}

void KActionCollection::beginXMLPlug( QWidget *widget )
{
	kdDebug(129) << "KActionCollection::beginXMLPlug( buildWidget = " << widget << " ): this = " <<  this << " d->m_kaccel = " << d->m_kaccel << endl;
	if( d->m_builderWidget )
		kdWarning() << "KActionCollection::beginXMLPlug() already called!" << endl;

	d->m_builderWidget = widget;

	if( d->m_kaccel )
		s_kaccelXML = d->m_kaccel;
	else if( d->m_builderWidget ) {
		d->m_builderKAccel = new KAccel( d->m_builderWidget, this, "KActionCollection-BuilderKAccel" );
		s_kaccelXML = d->m_builderKAccel;
	}
}

void KActionCollection::endXMLPlug()
{
	kdDebug(129) << "KActionCollection::endXMLPlug(): this = " <<  this << endl;
	s_kaccelXML = 0;
}

void KActionCollection::prepareXMLUnplug()
{
	kdDebug(129) << "KActionCollection::prepareXMLUnplug(): this = " <<  this << endl;
	unplugShortcuts( d->m_kaccel );

	d->m_builderWidget = 0;
	if( d->m_builderKAccel ) {
		unplugShortcuts( d->m_builderKAccel );
		delete d->m_builderKAccel;
		d->m_builderKAccel = 0;
	}
}

void KActionCollection::unplugShortcuts( KAccel* kaccel )
{
  for ( QAsciiDictIterator<KAction> it( d->m_actionDict ); it.current(); ++it ) {
    KAction* pAction = it.current();
    pAction->removeKAccel( kaccel );
  }

  for( uint i = 0; i < d->m_docList.count(); i++ )
    d->m_docList[i]->unplugShortcuts( kaccel );
}

/*void KActionCollection::addWidget( QWidget* w )
{
  if( !d->m_bOneKAccelOnly ) {
    kdDebug(129) << "KActionCollection::addWidget( " << w << " ): this = " << this << endl;
    for( uint i = 0; i < d->m_widgetList.count(); i++ ) {
      if( d->m_widgetList[i] == w ) {
        d->m_iWidgetCurrent = i;
        return;
      }
  }
    d->m_iWidgetCurrent = d->m_widgetList.count();
    d->m_widgetList.append( w );
    d->m_kaccelList.append( new KAccel( w, this, "KActionCollection-KAccel" ) );
  }
}

void KActionCollection::removeWidget( QWidget* w )
{
  if( !d->m_bOneKAccelOnly ) {
    kdDebug(129) << "KActionCollection::removeWidget( " << w << " ): this = " << this << endl;
    for( uint i = 0; i < d->m_widgetList.count(); i++ ) {
      if( d->m_widgetList[i] == w ) {
        // Remove KAccel object from children.
        KAccel* pKAccel = d->m_kaccelList[i];
        for ( QAsciiDictIterator<KAction> it( d->m_actionDict ); it.current(); ++it ) {
          KAction* pAction = it.current();
          if ( pAction->m_parentCollection == this ) {
            pAction->removeKAccel( pKAccel );
          }
        }
        delete pKAccel;

        d->m_widgetList.remove( d->m_widgetList.at( i ) );
        d->m_kaccelList.remove( d->m_kaccelList.at( i ) );

        if( d->m_iWidgetCurrent == (int)i )
          d->m_iWidgetCurrent = -1;
        else if( d->m_iWidgetCurrent > (int)i )
          d->m_iWidgetCurrent--;
        return;
      }
    }
    kdWarning(129) << "KActionCollection::removeWidget( " << w << " ): widget not in list." << endl;
  }
}

bool KActionCollection::ownsKAccel() const
{
  return d->m_bOneKAccelOnly;
}

uint KActionCollection::widgetCount() const
{
  return d->m_widgetList.count();
}

const KAccel* KActionCollection::widgetKAccel( uint i ) const
{
  return d->m_kaccelList[i];
}*/

KAccel* KActionCollection::kaccel()
{
  //if( d->m_kaccelList.count() > 0 )
  //  return d->m_kaccelList[d->m_iWidgetCurrent];
  //else
  //  return 0;
  return d->m_kaccel;
}

const KAccel* KActionCollection::kaccel() const
{
  //if( d->m_kaccelList.count() > 0 )
  //  return d->m_kaccelList[d->m_iWidgetCurrent];
  //else
  //  return 0;
  return d->m_kaccel;
}

/*void KActionCollection::findMainWindow( QWidget *w )
{
  // Note: topLevelWidget() stops too early, we can't use it.
  QWidget * tl = w;
  while ( tl->parentWidget() ) // lookup parent and store
    tl = tl->parentWidget();

  KMainWindow * mw = dynamic_cast<KMainWindow *>(tl); // try to see if it's a kmainwindow
  if (mw)
    d->m_mainwindow = mw;
  else
    kdDebug(129) << "KAction::plugMainWindowAccel: Toplevel widget isn't a KMainWindow, can't plug accel. " << tl << endl;
}*/

void KActionCollection::_insert( KAction* action )
{
  KAction *a = d->m_actionDict[ action->name() ];
  if ( a == action )
      return;

  d->m_actionDict.insert( action->name(), action );

  emit inserted( action );
}

void KActionCollection::_remove( KAction* action )
{
  delete _take( action );
}

KAction* KActionCollection::_take( KAction* action )
{
  KAction *a = d->m_actionDict.take( action->name() );
  if ( !a || a != action )
      return 0;

  emit removed( action );
  return a;
}

void KActionCollection::_clear()
{
  QAsciiDictIterator<KAction> it( d->m_actionDict );
  while ( it.current() )
    _remove( it.current() );
}

void KActionCollection::insert( KAction* action )   { _insert( action ); }
void KActionCollection::remove( KAction* action )   { _remove( action ); }
KAction* KActionCollection::take( KAction* action ) { return _take( action ); }
void KActionCollection::clear()                     { _clear(); }
KAccel* KActionCollection::accel()                  { return kaccel(); }
const KAccel* KActionCollection::accel() const      { return kaccel(); }

KAction* KActionCollection::action( const char* name, const char* classname ) const
{
  KAction* pAction = 0;

  if ( !classname && name )
    pAction = d->m_actionDict[ name ];

  else {
    QAsciiDictIterator<KAction> it( d->m_actionDict );
    for( ; it.current(); ++it )
    {
      if ( ( !name || strcmp( it.current()->name(), name ) == 0 ) &&
          ( !classname || strcmp( it.current()->className(), classname ) == 0 ) ) {
        pAction = it.current();
        break;
      }
    }
  }

  if( !pAction ) {
    for( uint i = 0; i < d->m_docList.count() && !pAction; i++ )
      pAction = d->m_docList[i]->action( name, classname );
  }

  return pAction;
}

KAction* KActionCollection::action( int index ) const
{
  QAsciiDictIterator<KAction> it( d->m_actionDict );
  it += index;
  return it.current();
//  return d->m_actions.at( index );
}

bool KActionCollection::readShortcutSettings( const QString& sConfigGroup, KConfigBase* pConfig )
{
  return KActionShortcutList(this).readSettings( sConfigGroup, pConfig );
}

bool KActionCollection::writeShortcutSettings( const QString& sConfigGroup, KConfigBase* pConfig ) const
{
  return KActionShortcutList((KActionCollection*)this).writeSettings( sConfigGroup, pConfig );
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

KActionPtrList KActionCollection::actions( const QString& group ) const
{
  KActionPtrList lst;

  QAsciiDictIterator<KAction> it( d->m_actionDict );
  for( ; it.current(); ++it )
    if ( it.current()->group() == group )
      lst.append( it.current() );
    else if ( it.current()->group().isEmpty() && group.isEmpty() )
      lst.append( it.current() );

  return lst;
}

KActionPtrList KActionCollection::actions() const
{
  KActionPtrList lst;

  QAsciiDictIterator<KAction> it( d->m_actionDict );
  for( ; it.current(); ++it )
    lst.append( it.current() );

  return lst;
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

void KActionCollection::setXMLFile( const QString& sXMLFile )
{
  d->m_sXMLFile = sXMLFile;
}

const QString& KActionCollection::xmlFile() const
{
  return d->m_sXMLFile;
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

  QPtrList<KAction> *actionList = d->m_dctHighlightContainers[ container ];

  if ( !actionList )
  {
    actionList = new QPtrList<KAction>;

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

  QPtrList<KAction> *actionList = d->m_dctHighlightContainers[ container ];

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
  QPtrList<KAction> *actionList = d->m_dctHighlightContainers[ reinterpret_cast<void *>( container ) ];

  if ( !actionList )
    return 0;

  QPtrListIterator<KAction> it( *actionList );
  for (; it.current(); ++it )
    if ( it.current()->isPlugged( container, id ) )
      return it.current();

  return 0;
}

#if KDE_VERSION < 400
KActionCollection KActionCollection::operator+(const KActionCollection &c ) const
{
  kdWarning(129) << "KActionCollection::operator+(): function is severely deprecated." << endl;
  KActionCollection ret( *this );

  QValueList<KAction *> actions = c.actions();
  QValueList<KAction *>::ConstIterator it = actions.begin();
  QValueList<KAction *>::ConstIterator end = actions.end();
  for (; it != end; ++it )
    ret.insert( *it );

  return ret;
}

KActionCollection &KActionCollection::operator=( const KActionCollection &copy )
{
  kdWarning(129) << "KActionCollection::operator=(): function is severely deprecated." << endl;
  //d->m_bOneKAccelOnly = copy.d->m_bOneKAccelOnly;
  //d->m_iWidgetCurrent = copy.d->m_iWidgetCurrent;
  //d->m_widgetList = copy.d->m_widgetList;
  //d->m_kaccelList = copy.d->m_kaccelList;
  d->m_widget = copy.d->m_widget;
  d->m_kaccel = copy.d->m_kaccel;
  d->m_actionDict = copy.d->m_actionDict;
  setInstance( copy.instance() );
  return *this;
}

KActionCollection &KActionCollection::operator+=( const KActionCollection &c )
{
  kdWarning(129) << "KActionCollection::operator+=(): function is severely deprecated." << endl;
  QAsciiDictIterator<KAction> it(c.d->m_actionDict);
  for ( ; it.current(); ++it )
    insert( it.current() );

  return *this;
}
#endif

//---------------------------------------------------------------------
// KActionShortcutList
//---------------------------------------------------------------------

KActionShortcutList::KActionShortcutList( KActionCollection* pColl )
: m_actions( *pColl )
	{ }
KActionShortcutList::~KActionShortcutList()
	{ }
uint KActionShortcutList::count() const
	{ return m_actions.count(); }
QString KActionShortcutList::name( uint i ) const
	{ return m_actions.action(i)->name(); }
QString KActionShortcutList::label( uint i ) const
	{ return m_actions.action(i)->text(); }
QString KActionShortcutList::whatsThis( uint i ) const
	{ return m_actions.action(i)->whatsThis(); }
const KShortcut& KActionShortcutList::shortcut( uint i ) const
	{ return m_actions.action(i)->shortcut(); }
const KShortcut& KActionShortcutList::shortcutDefault( uint i ) const
	{ return m_actions.action(i)->shortcutDefault(); }
bool KActionShortcutList::isConfigurable( uint i ) const
	{ return m_actions.action(i)->isShortcutConfigurable(); }
bool KActionShortcutList::setShortcut( uint i, const KShortcut& cut )
	{ return m_actions.action(i)->setShortcut( cut ); }
const KInstance* KActionShortcutList::instance() const
	{ return m_actions.instance(); }
QVariant KActionShortcutList::getOther( Other, uint ) const
	{ return QVariant(); }
bool KActionShortcutList::setOther( Other, uint, QVariant )
	{ return false; }

bool KActionShortcutList::save() const
{
	kdDebug(129) << "KActionShortcutList::save(): xmlFile = " << m_actions.xmlFile() << endl;

	if( m_actions.xmlFile().isEmpty() )
		return writeSettings();

	QString tagActionProp = QString::fromLatin1("ActionProperties");
	QString tagAction     = QString::fromLatin1("Action");
	QString attrName      = QString::fromLatin1("name");
	QString attrShortcut  = QString::fromLatin1("shortcut");
	QString attrAccel     = QString::fromLatin1("accel"); // Depricated attribute

	// Read XML file
	QString sXml( KXMLGUIFactory::readConfigFile( m_actions.xmlFile(), false, instance() ) );
	QDomDocument doc;
	doc.setContent( sXml );

	// Process XML data

	// first, lets see if we have existing properties
	QDomElement elem;
	QDomElement it = doc.documentElement();
	// KXMLGUIFactory::removeDOMComments( it ); <-- What was this for? --ellis
	it = it.firstChild().toElement();
	for( ; !it.isNull(); it = it.nextSibling().toElement() ) {
		if( it.tagName() == tagActionProp ) {
			elem = it;
			break;
		}
	}

	// if there was none, create one
	if( elem.isNull() ) {
		elem = doc.createElement( tagActionProp );
		doc.firstChild().appendChild( elem );
	}

	// now, iterate through our actions
	uint nSize = count();
	for( uint i = 0; i < nSize; i++ ) {
		const QString& sName = name(i);

		bool bSameAsDefault = (shortcut(i) == shortcutDefault(i));
		//kdDebug(129) << "name = " << sName << " shortcut = " << shortcut(i).toStringInternal() << " def = " << shortcutDefault(i).toStringInternal() << endl;

		// now see if this element already exists
		QDomElement act_elem;
		for( it = elem.firstChild().toElement(); !it.isNull(); it = it.nextSibling().toElement() ) {
			if( it.attribute( attrName ) == sName ) {
				act_elem = it;
				break;
			}
		}

		// nope, create a new one
		if( act_elem.isNull() ) {
			if( bSameAsDefault )
				continue;
			//kdDebug(129) << "\tnode doesn't exist." << endl;
			act_elem = doc.createElement( tagAction );
			act_elem.setAttribute( attrName, sName );
		}

		act_elem.removeAttribute( attrAccel );
		if( bSameAsDefault ) {
			act_elem.removeAttribute( attrShortcut );
			//kdDebug(129) << "act_elem.attributes().count() = " << act_elem.attributes().count() << endl;
			if( act_elem.attributes().count() == 1 )
				elem.removeChild( act_elem );
		} else {
			act_elem.setAttribute( attrShortcut, shortcut(i).toStringInternal() );
			elem.appendChild( act_elem );
		}
	}

	// Write back to XML file
	return KXMLGUIFactory::saveConfigFile( doc, m_actions.xmlFile(), instance() );
}

//---------------------------------------------------------------------
// KActionPtrShortcutList
//---------------------------------------------------------------------

KActionPtrShortcutList::KActionPtrShortcutList( KActionPtrList& list )
: m_actions( list )
	{ }
KActionPtrShortcutList::~KActionPtrShortcutList()
	{ }
uint KActionPtrShortcutList::count() const
	{ return m_actions.count(); }
QString KActionPtrShortcutList::name( uint i ) const
	{ return m_actions[i]->name(); }
QString KActionPtrShortcutList::label( uint i ) const
	{ return m_actions[i]->text(); }
QString KActionPtrShortcutList::whatsThis( uint i ) const
	{ return m_actions[i]->whatsThis(); }
const KShortcut& KActionPtrShortcutList::shortcut( uint i ) const
	{ return m_actions[i]->shortcut(); }
const KShortcut& KActionPtrShortcutList::shortcutDefault( uint i ) const
	{ return m_actions[i]->shortcutDefault(); }
bool KActionPtrShortcutList::isConfigurable( uint i ) const
	{ return m_actions[i]->isShortcutConfigurable(); }
bool KActionPtrShortcutList::setShortcut( uint i, const KShortcut& cut )
	{ return m_actions[i]->setShortcut( cut ); }
QVariant KActionPtrShortcutList::getOther( Other, uint ) const
	{ return QVariant(); }
bool KActionPtrShortcutList::setOther( Other, uint, QVariant )
	{ return false; }
bool KActionPtrShortcutList::save() const
	{ return false; }

void KActionShortcutList::virtual_hook( int id, void* data )
{ KShortcutList::virtual_hook( id, data ); }

void KActionPtrShortcutList::virtual_hook( int id, void* data )
{ KShortcutList::virtual_hook( id, data ); }

void KAction::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KToggleAction::virtual_hook( int id, void* data )
{ KAction::virtual_hook( id, data ); }

void KRadioAction::virtual_hook( int id, void* data )
{ KToggleAction::virtual_hook( id, data ); }

void KSelectAction::virtual_hook( int id, void* data )
{ KAction::virtual_hook( id, data ); }

void KListAction::virtual_hook( int id, void* data )
{ KSelectAction::virtual_hook( id, data ); }

void KRecentFilesAction::virtual_hook( int id, void* data )
{ KListAction::virtual_hook( id, data ); }

void KFontAction::virtual_hook( int id, void* data )
{ KSelectAction::virtual_hook( id, data ); }

void KFontSizeAction::virtual_hook( int id, void* data )
{ KSelectAction::virtual_hook( id, data ); }

void KActionMenu::virtual_hook( int id, void* data )
{ KAction::virtual_hook( id, data ); }

void KToolBarPopupAction::virtual_hook( int id, void* data )
{ KAction::virtual_hook( id, data ); }

void KToggleToolBarAction::virtual_hook( int id, void* data )
{ KToggleAction::virtual_hook( id, data ); }

void KWidgetAction::virtual_hook( int id, void* data )
{ KAction::virtual_hook( id, data ); }

void KActionSeparator::virtual_hook( int id, void* data )
{ KAction::virtual_hook( id, data ); }

void KActionCollection::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

/* vim: et sw=2 ts=2
 */

#include "kaction.moc"
