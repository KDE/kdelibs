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
#include <kurl.h>
#include <qtl.h>

#include <X11/Xlib.h>

#include <kiconloader.h>

static void get_fonts( QStringList &lst )
{
    int numFonts;
    Display *kde_display;
    char** fontNames;
    char** fontNames_copy;
    QString qfontname;

    kde_display = kapp->getDisplay();

    bool have_installed = kapp->kdeFonts( lst );

    if ( have_installed )
	return;

    fontNames = XListFonts( kde_display, "*", 32767, &numFonts );
    fontNames_copy = fontNames;

    for( int i = 0; i < numFonts; i++ ) {

	if ( **fontNames != '-' ) {
	    fontNames ++;
	    continue;
	};

	qfontname = "";
	qfontname = *fontNames;
	int dash = qfontname.find ( '-', 1, TRUE );

	if ( dash == -1 ) {
	    fontNames ++;
	    continue;
	}

	int dash_two = qfontname.find ( '-', dash + 1 , TRUE );

	if ( dash == -1 ) {
	    fontNames ++;
	    continue;
	}


	qfontname = qfontname.mid( dash +1, dash_two - dash -1 );

	if ( !qfontname.contains( QString::fromLatin1("open look"), TRUE ) ) {
	    if( qfontname != QString::fromLatin1("nil") ){
		if( lst.find( qfontname ) == lst.end() )
		    lst.append( qfontname );
	    }
	}

	fontNames ++;

    }

    XFreeFontNames( fontNames_copy );
}

int KAction::getToolButtonID()
{
    static int toolbutton_no = -2;
    return toolbutton_no--;
}

class KActionPrivate
{
public:
  KActionPrivate()
  {
  }
  ~KActionPrivate()
  {
  }
  QString m_iconName;
};

KAction::KAction( const QString& text, int accel, QObject* parent, const char* name )
 : QAction( text, accel, parent, name ), kaccel(0)
{
    d = new KActionPrivate;
}

KAction::KAction( const QString& text, int accel,
	           const QObject* receiver, const char* slot, QObject* parent, const char* name )
 : QAction( text, accel, receiver, slot, parent, name ), kaccel(0)
{
    d = new KActionPrivate;
}

KAction::KAction( const QString& text, const QIconSet& pix, int accel,
                    QObject* parent, const char* name )
 : QAction( text, pix, accel, parent, name ), kaccel(0)
{
    d = new KActionPrivate;
}

KAction::KAction( const QString& text, const QString& pix, int accel,
                    QObject* parent, const char* name )
 : QAction( text, BarIcon(pix, KIconLoader::Small), accel, parent, name ), kaccel(0)
{
  d = new KActionPrivate;
}

KAction::KAction( const QString& text, const QIconSet& pix, int accel,
	            const QObject* receiver, const char* slot, QObject* parent, const char* name )
 : QAction( text, pix, accel, receiver, slot, parent, name ), kaccel(0)
{
  d = new KActionPrivate;
}

KAction::KAction( const QString& text, const QString& pix, int accel,
	            const QObject* receiver, const char* slot, QObject* parent, const char* name )
 : QAction( text, BarIcon(pix, KIconLoader::Small), accel, receiver, slot, parent, name ), kaccel(0)
{
  d = new KActionPrivate;
  d->m_iconName = pix;
}

KAction::KAction( QObject* parent, const char* name )
 : QAction( parent, name ), kaccel(0)
{
  d = new KActionPrivate;
}

KAction::~KAction()
{
    delete d;
}

int KAction::plug( QWidget *w, int index )
{
  if ( w->inherits( "KToolBar" ) )
  {
    KToolBar *bar = (KToolBar *)w;

    int id_ = getToolButtonID();
    if ( d->m_iconName == QString::null )
    {
      bar->insertButton( iconSet().pixmap(), id_, SIGNAL( clicked() ), this, SLOT( slotActivated() ),
		       isEnabled(), plainText(), index );
    }
    else
    {
      bar->insertButton( d->m_iconName, id_, SIGNAL( clicked() ), this,
                         SLOT( slotActivated() ), isEnabled(), plainText(),
                         index );
    }


    addContainer( bar, id_ );

    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }

  return QAction::plug( w, index );
}

void KAction::unplug( QWidget *w )
{
  if ( w->inherits( "KToolBar" ) )
  {
    KToolBar *bar = (KToolBar *)w;

    int idx = findContainer( bar );

    if ( idx != -1 )
    {
      bar->removeItem( menuId( idx ) );
      removeContainer( idx );
    }

    return;
  }
  QAction::unplug( w );
}

void KAction::plugAccel(KAccel *kacc, bool configurable)
{
    if (kaccel) unplugAccel();
	kaccel = kacc;
	kaccel->insertItem(plainText(), name(), accel(), configurable);
	kaccel->connectItem(name(), this, SLOT(slotActivated()));
	connect(kaccel, SIGNAL(destroyed()), this, SLOT(slotDestroyed()));
	connect(kaccel, SIGNAL(keycodeChanged()), this, SLOT(slotKeycodeChanged()));
}

void KAction::unplugAccel()
{
	if ( kaccel==0 ) return;
	kaccel->removeItem(name());
	kaccel->disconnect(this);
	kaccel = 0;
}

void KAction::setEnabled(bool enable)
{
	if (kaccel) kaccel->setItemEnabled(name(), enable);
	QAction::setEnabled(enable);
}

void KAction::setEnabled( int i, bool enable )
{
    QWidget *w = container( i );

    if ( w->inherits( "KToolBar" ) )
      ((KToolBar *)w)->setItemEnabled( menuId( i ), enable );

    QAction::setEnabled( i, enable );
}

void KAction::setText( const QString& text )
{
	if (kaccel) kaccel->setDescription(name(), text);
	QAction::setText( text );
}

void KAction::setText( int i, const QString &text )
{
   QWidget *w = container( i );

   if ( w->inherits( "KToolBar" ) )
   {
     QWidget *button = ((KToolBar *)w)->getWidget( menuId( i ) );
     if ( button->inherits( "KToolBarButton" ) )
       ((KToolBarButton *)button)->setText( text );
   }

   QAction::setText( i, text );
}

void KAction::setIcon( const QString &icon )
{
  d->m_iconName = icon;
}

void KAction::setIconSet( const QIconSet &iconSet )
{
  QAction::setIconSet( iconSet );
}

void KAction::setIconSet( int id, const QIconSet& iconSet )
{
    QWidget *w = container( id );

    if ( w->inherits( "KToolBar" ) )
      ((KToolBar *)w)->setButtonPixmap( menuId( id ), iconSet.pixmap() );

	QAction::setIconSet( id, iconSet );
}

void KAction::setAccel(int a)
{
	if (kaccel) kaccel->updateItem(name(), a);
	QAction::setAccel(a);
}

void KAction::slotDestroyed()
{
	if (sender()==kaccel) kaccel = 0;
	QAction::slotDestroyed();
}

void KAction::slotKeycodeChanged()
{
	QAction::setAccel(kaccel->currentKey(name()));
}

bool KAction::isPlugged() const
{
	if (kaccel) return TRUE;
	return QAction::isPlugged();
}


KToggleAction::KToggleAction( const QString& text, int accel, QObject* parent, const char* name )
    : QToggleAction( text, accel, parent, name )
{
    locked = FALSE;
    checked = FALSE;
    locked2 = FALSE;
}

KToggleAction::KToggleAction( const QString& text, int accel,
	       const QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QToggleAction( text, accel, receiver, slot, parent, name )
{
    locked = FALSE;
    checked = FALSE;
    locked2 = FALSE;
}

KToggleAction::KToggleAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* parent, const char* name )
    : QToggleAction( text, pix, accel, parent, name )
{
    locked = FALSE;
    checked = FALSE;
    locked2 = FALSE;
}

KToggleAction::KToggleAction( const QString& text, const QString& pix, int accel,
	       QObject* parent, const char* name )
    : QToggleAction( text, BarIcon(pix, KIconLoader::Small), accel, parent, name )
{
    locked = FALSE;
    checked = FALSE;
    locked2 = FALSE;
}

KToggleAction::KToggleAction( const QString& text, const QIconSet& pix, int accel,
			      const QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QToggleAction( text, pix, accel, receiver, slot, parent, name )
{
    locked = FALSE;
    checked = FALSE;
    locked2 = FALSE;
}

KToggleAction::KToggleAction( const QString& text, const QString& pix, int accel,
			      const QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QToggleAction( text, BarIcon(pix, KIconLoader::Small), accel, receiver, slot, parent, name )
{
    locked = FALSE;
    checked = FALSE;
    locked2 = FALSE;
}

KToggleAction::KToggleAction( QObject* parent, const char* name )
    : QToggleAction( parent, name )
{
    locked = FALSE;
    checked = FALSE;
    locked2 = FALSE;
}

int KToggleAction::plug( QWidget* widget, int index )
{
    if ( !widget->inherits("QPopupMenu") && !widget->inherits("QActionWidget" ) &&
         !widget->inherits("KToolBar") ) {
        qDebug("Can not plug KToggleAction in %s", widget->className() );
        return -1;	
    }

    int _index = -1;
    int id_ = -1;
    if ( widget->inherits( "KToolBar" ) ) {
        KToolBar *bar = (KToolBar *)widget;

        id_ = KAction::getToolButtonID();
        bar->insertButton( iconSet().pixmap(), id_, SIGNAL( clicked() ), this, SLOT( slotActivated() ),
                           isEnabled(), plainText(), index );

        addContainer( bar, id_ );

        connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

        _index =  containerCount() - 1;
    } else
        _index = QToggleAction::plug( widget, index );

    if ( _index == -1 )
        return _index;

    if ( widget->inherits("QPopupMenu") ) {
        int id = menuId( _index );

        popupMenu( _index )->setItemChecked( id, isChecked() );
    }
    else if ( widget->inherits("QActionWidget" ) )
    {
    }
    else if ( widget->inherits("KToolBar") )
    {
        KToolBar *bar = (KToolBar*)container( _index );
        bar->setToggle( menuId( _index ), TRUE );
        bar->setButton( menuId( _index ), isChecked() );
    }

    return _index;
}

void KToggleAction::unplug( QWidget *w )
{
  if ( w->inherits( "KToolBar" ) )
  {
    KToolBar *bar = (KToolBar *)w;

    int idx = findContainer( bar );

    if ( idx != -1 )
    {
      bar->removeItem( menuId( idx ) );
      removeContainer( idx );
    }

    return;
  }

  QToggleAction::unplug( w );
}

void KToggleAction::setChecked( bool c )
{
    if ( c == checked )
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
		     ((KToggleAction*)it.current())->exclusiveGroup() == exclusiveGroup() ) {
		    ((KToggleAction*)it.current())->setChecked( FALSE );
		}
	    }
	}
    }

    checked = c;

    locked = TRUE;
    emit activated();
    locked = FALSE;
    emit toggled( isChecked() );
}

void KToggleAction::setChecked( int id, bool checked )
{
  QWidget *w = container( id );

  if ( w->inherits( "KToolBar" ) )
  {
    QWidget* r = ( (KToolBar*)w )->getButton( menuId( id ) );
    if ( r->inherits( "KToolBarButton" ) )
      ( (KToolBar*)w )->setButton( menuId( id ), checked );
  }

  QToggleAction::setChecked( id, checked );
}

void KToggleAction::setText( int i, const QString &text )
{
   QWidget *w = container( i );

   if ( w->inherits( "KToolBar" ) )
   {
     QWidget *button = ((KToolBar *)w)->getWidget( menuId( i ) );
     if ( button->inherits( "KToolBarButton" ) )
       ((KToolBarButton *)button)->setText( text );
   }

  QToggleAction::setText( i, text );
}

void KToggleAction::slotActivated()
{
    if ( locked )
	return;

    locked = TRUE;
    setChecked( !isChecked() );
    locked = FALSE;
}

bool KToggleAction::isChecked() const
{
    return checked;
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
: KToggleAction( text, BarIcon(pix, KIconLoader::Small), accel, parent, name )
{
}

KRadioAction::KRadioAction( const QString& text, const QIconSet& pix, int accel,
			    const QObject* receiver, const char* slot, QObject* parent, const char* name )
: KToggleAction( text, pix, accel, receiver, slot, parent, name )
{
}

KRadioAction::KRadioAction( const QString& text, const QString& pix, int accel,
			    const QObject* receiver, const char* slot, QObject* parent, const char* name )
: KToggleAction( text, BarIcon(pix, KIconLoader::Small), accel, receiver, slot, parent, name )
{
}

KRadioAction::KRadioAction( QObject* parent, const char* name )
: KToggleAction( parent, name )
{
}

void KRadioAction::slotActivated()
{
  if ( isChecked() )
    return;

  KToggleAction::slotActivated();
}

KSelectAction::KSelectAction( const QString& text, int accel,
			      QObject* parent, const char* name )
    : QSelectAction( text, accel, parent, name )
{
    m_lock = false;
}

KSelectAction::KSelectAction( const QString& text, int accel,
			      const QObject* receiver, const char* slot, QObject* parent,
			      const char* name )
    : QSelectAction( text, accel, parent, name )
{
    m_lock = false;
    connect( this, SIGNAL( activate() ), receiver, slot );
}

KSelectAction::KSelectAction( const QString& text, const QIconSet& pix, int accel,
			      QObject* parent, const char* name )
    : QSelectAction( text, pix, accel, parent, name )
{
    m_lock = false;
}

KSelectAction::KSelectAction( const QString& text, const QString& pix, int accel,
			      QObject* parent, const char* name )
    : QSelectAction( text, BarIcon(pix, KIconLoader::Small), accel, parent, name )
{
    m_lock = false;
}

KSelectAction::KSelectAction( const QString& text, const QIconSet& pix, int accel,
			      const QObject* receiver, const char* slot, QObject* parent,
			      const char* name )
    : QSelectAction( text, pix, accel, receiver, slot, parent, name )
{
    connect( this, SIGNAL( activate() ), receiver, slot );
    m_lock = false;
}

KSelectAction::KSelectAction( const QString& text, const QString& pix, int accel,
			      const QObject* receiver, const char* slot, QObject* parent,
			      const char* name )
    : QSelectAction( text, BarIcon(pix, KIconLoader::Small), accel, receiver, slot, parent, name )
{
    connect( this, SIGNAL( activate() ), receiver, slot );
    m_lock = false;
}

KSelectAction::KSelectAction( QObject* parent, const char* name )
    : QSelectAction( parent, name )
{
    m_lock = false;
}

void KSelectAction::setCurrentItem( int id )
{
    QSelectAction::setCurrentItem( id );
	
    // ?????????? (Simon)	
    emit activate();
}

void KSelectAction::setItems( const QStringList &lst )
{
    QSelectAction::setItems( lst);
}

void KSelectAction::clear()
{
    QSelectAction::clear();
}

void KSelectAction::setCurrentItem( int id, int index )
{
  QWidget* w = container( id );
  if ( w->inherits( "KToolBar" ) ) {
    QWidget* r = ( (KToolBar*)w )->getWidget( menuId( id ) );
    if ( r->inherits( "QComboBox" ) ) {
      QComboBox *b = (QComboBox*)r;
      b->setCurrentItem( index );
    }
  }

  QSelectAction::setCurrentItem( id, index );
}

void KSelectAction::setItems( int id, const QStringList& lst )
{
  QWidget* w = container( id );
  if ( w->inherits( "KToolBar" ) ) {
    QWidget* r = ( (KToolBar*)w )->getWidget( menuId( id ) );
    if ( r->inherits( "QComboBox" ) ) {
      QComboBox *b = (QComboBox*)r;
      b->clear();
      QStringList::ConstIterator it = lst.begin();
      for( ; it != lst.end(); ++it )
        b->insertItem( *it );
      }
   }

  QSelectAction::setItems( id, lst );
}

int KSelectAction::plug( QWidget *widget, int index )
{
    if ( widget->inherits("QPopupMenu") )
    {
	// Create the PopupMenu and store it in m_menu
	(void)popupMenu();
	
	QPopupMenu* menu = (QPopupMenu*)widget;
	int id;
	if ( !pixmap().isNull() )
        {
	    id = menu->insertItem( pixmap(), popupMenu(), -1, index );
	}
	else
        {
	    if ( hasIconSet() )
		id = menu->insertItem( iconSet(), text(), popupMenu(), -1, index );
	    else
		id = menu->insertItem( text(), popupMenu(), -1, index );     
	}

	menu->setItemEnabled( id, isEnabled() );
	menu->setWhatsThis( id, whatsThis() );

	addContainer( menu, id );
	connect( menu, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
	
	return containerCount() - 1;
    }
    else if ( widget->inherits("QActionWidget" ) )
    {
	connect( widget, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );	
	addContainer( widget, (int)0 );
	
	return containerCount() - 1;
    }
    else if ( widget->inherits("KToolBar") )
    {
	KToolBar* bar = (KToolBar*)widget;
	int id_ = KAction::getToolButtonID();
	bar->insertCombo( items(), id_, isEditable(), SIGNAL( activated( const QString & ) ),
			  this, SLOT( slotActivated( const QString & ) ), true, QString::null, 70, index );
	QComboBox *cb = bar->getCombo( id_ );
	if ( cb ) {
	    cb->setMinimumWidth( cb->sizeHint().width() );
	}
	addContainer( bar, id_ );

	connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
	
	return containerCount() - 1;
    }

    qDebug("Can not plug QAction in %s", widget->className() );
    return -1;
}

void KSelectAction::clear( int id )
{
  QWidget* w = container( id );
  if ( w->inherits( "KToolBar" ) ) {
    QWidget* r = ( (KToolBar*)w )->getWidget( menuId( id ) );
    if ( r->inherits( "QComboBox" ) ) {
      QComboBox *b = (QComboBox*)r;
      b->clear();
    }
  }

  QSelectAction::clear( id );
}

void KSelectAction::slotActivated( const QString &text )
{
  if ( m_lock )
    return;

  m_lock = true;
  if ( isEditable() )
  {
    QStringList lst = items();
    lst.append( text );
    QSelectAction::setItems( lst );
  }

  setCurrentItem( items().findIndex( text ) );
  m_lock = false;
}

KListAction::KListAction( const QString& text, int accel, QObject* parent,
                            const char* name )
    : KSelectAction( text, accel, parent, name )
{
    m_current = 0;
}

KListAction::KListAction( const QString& text, int accel,
                          const QObject* receiver, const char* slot,
                          QObject* parent, const char* name )
    : KSelectAction( text, accel, parent, name )
{
    connect( this, SIGNAL(activated(int)), receiver, slot );
    m_current = 0;
}

KListAction::KListAction( const QString& text, const QIconSet& pix,
                            int accel, QObject* parent, const char* name )
    : KSelectAction( text, pix, accel, parent, name )
{
    m_current = 0;
}

KListAction::KListAction( const QString& text, const QString& pix,
                            int accel, QObject* parent, const char* name )
    : KSelectAction( text, BarIcon(pix, KIconLoader::Small), accel, parent, name )
{
    m_current = 0;
}

KListAction::KListAction( const QString& text, const QIconSet& pix,
                          int accel, const QObject* receiver,
                          const char* slot, QObject* parent,
                          const char* name )
    : KSelectAction( text, pix, accel, receiver, slot, parent, name )
{
    connect( this, SIGNAL(activated(int)), receiver, slot );
    m_current = 0;
}

KListAction::KListAction( const QString& text, const QString& pix,
                            int accel, const QObject* receiver,
                            const char* slot, QObject* parent,
			                const char* name )
    : KSelectAction( text, BarIcon(pix, KIconLoader::Small), accel, receiver, slot, parent, name )
{
    connect( this, SIGNAL(activated(int)), receiver, slot );
    m_current = 0;
}

KListAction::KListAction( QObject* parent, const char* name )
    : KSelectAction( parent, name )
{
    m_current = 0;
}

void KListAction::setCurrentItem( int index )
{
    m_current = index;

    emit QAction::activated();
    emit activated( currentItem() );
    emit activated( currentText() );
}

QString KListAction::currentText()
{
    if ( currentItem() < 0 )
        return QString::null;

    return items()[ currentItem() ];
}

int KListAction::currentItem()
{
    return m_current;
}


KRecentFilesAction::KRecentFilesAction( const QString& text, int accel,
                                        QObject* parent, const char* name,
                                        unsigned int maxItems )
  : KListAction( text, accel, parent, name),
    m_maxItems( maxItems )
{
    connect( this, SIGNAL( activated( const QString& ) ),
             this, SLOT( itemSelected( const QString& ) ) );
}

KRecentFilesAction::KRecentFilesAction( const QString& text, int accel,
                                        const QObject* receiver,
                                        const char* slot,
                                        QObject* parent, const char* name,
                                        unsigned int maxItems )
  : KListAction( text, accel, parent, name),
    m_maxItems( maxItems )
{
    connect( this, SIGNAL( activated( const QString& ) ),
             this, SLOT( itemSelected( const QString& ) ) );

    connect( this,     SIGNAL(urlSelected(const KURL&)),
             receiver, slot );
}

KRecentFilesAction::KRecentFilesAction( const QString& text,
                                        const QIconSet& pix, int accel,
                                        QObject* parent, const char* name,
                                        unsigned int maxItems )
  : KListAction( text, pix, accel, parent, name),
    m_maxItems( maxItems )
{
    connect( this, SIGNAL( activated( const QString& ) ),
             this, SLOT( itemSelected( const QString& ) ) );
}

KRecentFilesAction::KRecentFilesAction( const QString& text,
                                        const QString& pix, int accel,
                                        QObject* parent, const char* name,
                                        unsigned int maxItems )
  : KListAction( text, BarIcon(pix, KIconLoader::Small), accel, parent, name),
    m_maxItems( maxItems )
{
    connect( this, SIGNAL( activated( const QString& ) ),
             this, SLOT( itemSelected( const QString& ) ) );
}

KRecentFilesAction::KRecentFilesAction( const QString& text,
                                        const QIconSet& pix, int accel,
                                        const QObject* receiver,
                                        const char* slot,
                                        QObject* parent, const char* name,
                                        unsigned int maxItems )
  : KListAction( text, pix, accel, parent, name),
    m_maxItems( maxItems )
{
    connect( this, SIGNAL( activated( const QString& ) ),
             this, SLOT( itemSelected( const QString& ) ) );

    connect( this,     SIGNAL(urlSelected(const KURL&)),
             receiver, slot );
}

KRecentFilesAction::KRecentFilesAction( const QString& text,
                                        const QString& pix, int accel,
                                        const QObject* receiver,
                                        const char* slot,
                                        QObject* parent, const char* name,
                                        unsigned int maxItems )
  : KListAction( text, BarIcon(pix, KIconLoader::Small), accel, parent, name),
    m_maxItems( maxItems )
{
    connect( this, SIGNAL( activated( const QString& ) ),
             this, SLOT( itemSelected( const QString& ) ) );

    connect( this,     SIGNAL(urlSelected(const KURL&)),
             receiver, slot );
}

KRecentFilesAction::KRecentFilesAction( QObject* parent, const char* name,
                                        unsigned int maxItems )
  : KListAction( parent, name ),
    m_maxItems( maxItems )
{
    connect( this, SIGNAL( activated( const QString& ) ),
             this, SLOT( itemSelected( const QString& ) ) );
}

KRecentFilesAction::~KRecentFilesAction()
{
}

unsigned int KRecentFilesAction::maxItems()
{
    return m_maxItems;
}

void KRecentFilesAction::setMaxItems( unsigned int maxItems )
{
    QStringList lst       = items();
    unsigned int oldCount = lst.count();
    
    // set new maxItems
    m_maxItems = maxItems;

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
    QString     file = url.url();
    QStringList lst = items();

    // remove file if already in list
    lst.remove( file );
    
    // remove las item if already maxitems in list
    if( lst.count() == m_maxItems )
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
    QString     file = url.url();

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

void KRecentFilesAction::loadEntries( KConfig* config )
{
    QString     key;
    QString     value;
    QStringList lst;
    
    config->setGroup( "RecentFiles" );

    // read file list
    for( unsigned int i = 1 ; i <= m_maxItems ; i++ )
    {
        key = QString( "File%1" ).arg( i );
        value = config->readEntry( key, QString::null );
        if (!value.isNull())
            lst.append( value );
    }
    
    // set file
    setItems( lst );
}

void KRecentFilesAction::saveEntries( KConfig* config )
{
    QString     key;
    QString     value;
    QStringList lst = items();
    
    config->setGroup( "RecentFiles" );

    // write file list
    for( unsigned int i = 1 ; i <= lst.count() ; i++ )
    {
        key = QString( "File%1" ).arg( i );
        value = lst[ i - 1 ];
        config->writeEntry( key, value );
    }
}

void KRecentFilesAction::itemSelected( const QString& text )
{
    emit urlSelected( KURL( text ) );
}

KFontAction::KFontAction( const QString& text, int accel, QObject* parent, const char* name )
    : KSelectAction( text, accel, parent, name )
{
    get_fonts( fonts );
    QSelectAction::setItems( fonts );
    setEditable( TRUE );
}

KFontAction::KFontAction( const QString& text, int accel,
			  const QObject* receiver, const char* slot, QObject* parent, const char* name )
    : KSelectAction( text, accel, receiver, slot, parent, name )
{
    get_fonts( fonts );
    QSelectAction::setItems( fonts );
    setEditable( TRUE );
}

KFontAction::KFontAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* parent, const char* name )
    : KSelectAction( text, pix, accel, parent, name )
{
    get_fonts( fonts );
    QSelectAction::setItems( fonts );
    setEditable( TRUE );
}

KFontAction::KFontAction( const QString& text, const QString& pix, int accel,
	       QObject* parent, const char* name )
    : KSelectAction( text, BarIcon(pix, KIconLoader::Small), accel, parent, name )
{
    get_fonts( fonts );
    QSelectAction::setItems( fonts );
    setEditable( TRUE );
}

KFontAction::KFontAction( const QString& text, const QIconSet& pix, int accel,
			  const QObject* receiver, const char* slot, QObject* parent, const char* name )
    : KSelectAction( text, pix, accel, receiver, slot, parent, name )
{
    get_fonts( fonts );
    QSelectAction::setItems( fonts );
    setEditable( TRUE );
}

KFontAction::KFontAction( const QString& text, const QString& pix, int accel,
			  const QObject* receiver, const char* slot, QObject* parent, const char* name )
    : KSelectAction( text, BarIcon(pix, KIconLoader::Small), accel, receiver, slot, parent, name )
{
    get_fonts( fonts );
    QSelectAction::setItems( fonts );
    setEditable( TRUE );
}


KFontAction::KFontAction( QObject* parent, const char* name )
    : KSelectAction( parent, name )
{
    get_fonts( fonts );
    QSelectAction::setItems( fonts );
    setEditable( TRUE );
}

void KFontAction::setFont( const QString &family )
{
    int i = fonts.findIndex( family.lower() );
    if ( i != -1 )
	setCurrentItem( i );
}

int KFontAction::plug( QWidget *w, int index )
{
    int container = KSelectAction::plug( w, index );

    if ( container != -1 && w->inherits( "KToolBar" ) )
	((KToolBar *)w)->getCombo( menuId( container ) )->setAutoCompletion( TRUE );

    return container;
}

KFontSizeAction::KFontSizeAction( const QString& text, int accel,
				  QObject* parent, const char* name )
    : KSelectAction( text, accel, parent, name )
{
    init();
}

KFontSizeAction::KFontSizeAction( const QString& text, int accel,
				  const QObject* receiver, const char* slot, QObject* parent,
				  const char* name )
    : KSelectAction( text, accel, receiver, slot, parent, name )
{
    init();
}

KFontSizeAction::KFontSizeAction( const QString& text, const QIconSet& pix, int accel,
				  QObject* parent, const char* name )
    : KSelectAction( text, pix, accel, parent, name )
{
    init();
}

KFontSizeAction::KFontSizeAction( const QString& text, const QString& pix, int accel,
				  QObject* parent, const char* name )
    : KSelectAction( text, BarIcon(pix, KIconLoader::Small), accel, parent, name )
{
    init();
}

KFontSizeAction::KFontSizeAction( const QString& text, const QIconSet& pix, int accel,
				  const QObject* receiver, const char* slot, QObject* parent,
				  const char* name )
    : KSelectAction( text, pix, accel, receiver, slot, parent, name )
{
    init();
}

KFontSizeAction::KFontSizeAction( const QString& text, const QString& pix, int accel,
				  const QObject* receiver, const char* slot, QObject* parent,
				  const char* name )
    : KSelectAction( text, BarIcon(pix, KIconLoader::Small), accel, receiver, slot, parent, name )
{
    init();
}

KFontSizeAction::KFontSizeAction( QObject* parent, const char* name )
    : KSelectAction( parent, name )
{
    init();
}

void KFontSizeAction::init()
{
    m_lock = FALSE;

    setEditable( TRUE );
    QStringList lst;
    for ( unsigned int i = 0; i < 100; ++i )
	lst.append( QString().setNum( i + 1 ) );

    QSelectAction::setItems( lst );
}

void KFontSizeAction::setFontSize( int size )
{
    if ( size == fontSize() )
	return;

    if ( size < 1 || size > 128 )
    {
	qDebug( "KFontSizeAction: Size %i is out of range", size );
	return;
    }

    int index = items().findIndex( QString::number( size ) );
    if ( index == -1 )
    {
	QStringList lst = items();
	lst.append( QString::number( size ) );
	qHeapSort( lst );
	QSelectAction::setItems( lst );
	index = lst.findIndex( QString::number( size ) );
	setCurrentItem( index );
    }
    else
    {
	// Avoid dupes in combo boxes ...
	//setItems( items() );
	setCurrentItem( index );
    }

    emit QAction::activated();
    emit activated( index );
    emit activated( QString::number( size ) );
    emit fontSizeChanged( size );
}

int KFontSizeAction::fontSize()
{
    return currentText().toInt();
}

void KFontSizeAction::slotActivated( int index )
{
    QSelectAction::slotActivated( index );

    emit fontSizeChanged( items()[ index ].toInt() );
}

void KFontSizeAction::slotActivated( const QString& size )
{
    if ( m_lock )
	return;

    if ( size.toInt() < 1 || size.toInt() > 128 )
    {
	qDebug( "KFontSizeAction: Size %s is out of range", size.latin1() );
	return;
    }

    m_lock = TRUE;
    setFontSize( size.toInt() );
    m_lock = FALSE;
}

KActionMenu::KActionMenu( QObject* parent, const char* name )
 : QActionMenu( parent, name )
{
    popupMenu()->setFont(KGlobal::menuFont());
}

KActionMenu::KActionMenu( const QString& text, QObject* parent, const char* name )
 : QActionMenu( text, parent, name )
{
    popupMenu()->setFont(KGlobal::menuFont());
}

KActionMenu::KActionMenu( const QString& text, const QIconSet& icon, QObject* parent, const char* name )
 : QActionMenu( text, icon, parent, name )
{
    popupMenu()->setFont(KGlobal::menuFont());
}

int KActionMenu::plug( QWidget* widget, int index )
{

  if ( widget->inherits( "KToolBar" ) )
  {
    KToolBar *bar = (KToolBar *)widget;

    int id_ = KAction::getToolButtonID();
    bar->insertButton( iconSet().pixmap(), id_, SIGNAL( clicked() ), this, SLOT( slotActivated() ),
		       isEnabled(), plainText(), index );

    addContainer( bar, id_ );

    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    bar->setDelayedPopup( id_, popupMenu() );

    return containerCount() - 1;
  }
  else if ( widget->inherits( "KMenuBar" ) )
  {
    KMenuBar *bar = (KMenuBar *)widget;

    int id;

    id = bar->insertItem( text(), popupMenu(), -1, index );

    addContainer( bar, id );
    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }

  return QActionMenu::plug( widget, index );
}

void KActionMenu::unplug( QWidget* widget )
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
  else if ( widget->inherits( "KMenuBar" ) )
  {
    KMenuBar *bar = (KMenuBar *)widget;
    int i = findContainer( bar );
    if ( i != -1 )
    {
      bar->removeItem( menuId( i ) );
      removeContainer( i );
    }

    return;
  }

  QActionMenu::unplug( widget );
}

void KActionMenu::setEnabled( int id, bool b )
{
  QWidget *w = container( id );

  if ( w->inherits( "KToolBar" ) )
    ((KToolBar *)w)->setItemEnabled( menuId( id ), b );

  QActionMenu::setEnabled( id, b );
}

void KActionMenu::setText( int id, const QString& text )
{
  QWidget *w = container( id );

  if ( w->inherits( "KToolBar" ) )
  {
    QWidget *button = ((KToolBar *)w)->getWidget( menuId( id ) );
    if ( button->inherits( "KToolBarButton" ) )
     ((KToolBarButton *)button)->setText( text );
  }

  QActionMenu::setText( id, text );
}


void KActionMenu::setIconSet( int id, const QIconSet& iconSet )
{
  QWidget *w = container( id );

  if ( w->inherits( "KToolBar" ) )
    ((KToolBar *)w)->setButtonPixmap( menuId( id ), iconSet.pixmap() );

  QActionMenu::setIconSet( id, iconSet );
}

KActionSeparator::KActionSeparator( QObject *parent, const char *name )
: QActionSeparator( parent, name )
{
}

int KActionSeparator::plug( QWidget *widget, int index )
{
  if ( widget->inherits( "KMenuBar" ) )
  {
    KMenuBar *menuBar = (KMenuBar *)widget;

    int id = menuBar->insertSeparator( index );

    addContainer( menuBar, id );

    connect( menuBar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }
  else if ( widget->inherits( "KToolBar" ) )
  {
    KToolBar *toolBar = (KToolBar *)widget;

    int id = toolBar->insertSeparator( index );

    addContainer( toolBar, id );

    connect( toolBar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }

  return QActionSeparator::plug( widget, index );
}

void KActionSeparator::unplug( QWidget *widget )
{
  if ( widget->inherits( "KMenuBar" ) )
  {
    KMenuBar *menuBar = (KMenuBar *)widget;

    int i = findContainer( menuBar );

    if ( i != -1 )
    {
      menuBar->removeItem( menuId( i ) );
      removeContainer( i );
    }
    return;
  }
  else if ( widget->inherits( "KToolBar" ) )
  {
    KToolBar *toolBar = (KToolBar *)widget;

    int i = findContainer( toolBar );

    if ( i != -1 )
    {
      toolBar->removeItem( menuId( i ) );
      removeContainer( i );
    }
    return;
  }

  QActionSeparator::unplug( widget );
  return;
}

#include "kaction.moc"

