/*
* kactconfigwidget.cpp -- Implementation of class KActionConfigWidget.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Tue May 25 23:38:51 EST 1999
*/

#include<assert.h>
#include<qlistview.h>
#include<qlayout.h>

#include<kaction.h>
#include<kglobal.h>
#include<kuiactions.h>
#include<kiconloaderdialog.h>

#include"kactconfigwidget.h"
#include"kactcfgitem.h"

KActionConfigWidget::KActionConfigWidget( KUIActions *actions, 
			QWidget *parent )
	: QWidget( parent ),
	_actions( actions ),
	_listView( new QListView( this, "ActionListView" ) )

{
	assert( _actions );

	_items.setAutoDelete( false );

	connect( _listView, SIGNAL(currentChanged(QListViewItem *)),
			this, SLOT(configItem(QListViewItem*)));

	_listView->addColumn( "Action" );
	_listView->addColumn( "Icon" );
	_listView->addColumn( "Key" );
	_listView->setColumnAlignment( 1, Qt::AlignCenter );

	fillListView();

	_itemcfg = new KActItemCfg( this );
	connect( _itemcfg->_iconBtn, SIGNAL(iconChanged(const QString&)),
		this, SLOT(setIcon(const QString&)) );

	QBoxLayout *layout = new QVBoxLayout( this, 1 );
	layout->addWidget( _listView );
	layout->addWidget( _itemcfg );
}

KActionConfigWidget::~KActionConfigWidget()
{
}

void KActionConfigWidget::allDefault()
{

	QListIterator<CfgItem> iter( _items );

	for ( ; iter.current(); ++iter ) {
		iter.current()->loadFromAction();
	}
}

void KActionConfigWidget::listSync()
{
	QListIterator<CfgItem> iter( _items );

	for ( CfgItem *item = iter.current(); item != 0;  item = ++iter ) {
		if( item->dirty ) {
			item->updateAction();
		}
	}

}

void KActionConfigWidget::fillListView()
{
	_items.clear();
	_listView->clear();

	KUIActions::KActionIterator iter = _actions->iterator();

	for ( ; iter.current(); ++iter ) {
		CfgItem *item = new CfgItem( _listView, iter.current() );
		_listView->insertItem( item );
		_items.append( item );
	}
}

void KActionConfigWidget::configItem( QListViewItem *vitem )
{
	// warning: downcast
	CfgItem *item = (CfgItem *)vitem;
	_itemcfg->_iconBtn->setIcon( item->action->iconPath() );

}

void KActionConfigWidget::setIcon( const QString& icon )
{
	CfgItem *item = (CfgItem *)_listView->currentItem();
	assert( item != 0 );

	debug( "setIcon called for %s with %s, currently %s",
			item->action->desc().latin1(),
			icon.latin1(), item->iconPath.latin1() );

	if( item->iconPath == icon ) {
		return;
	}

	item->iconPath = icon;
	delete item->icon; item->icon = 0;
	item->dirty = true;
	item->setAllText();
}

#include"kactconfigwidget.moc"


// ------------------------ KActCfgListItem ------------------
/////
/////


KActCfgListItem::KActCfgListItem( QListView *view, KAction *act )
	: QListViewItem( view ),
	action( act ), 
	dirty( false ),
	accel( act->accel() ),
	icon( 0 ),
	iconPath( act->iconPath() )
{
	setAllText();
}

void KActCfgListItem::loadFromAction()
{
	accel = action->accel();
	
	if( iconPath != action->iconPath() ) {
		iconPath = action->iconPath();
		delete icon; icon = 0;
	}
	
	dirty = false;
}

void KActCfgListItem::updateAction()
{
	if( !dirty ) {
		return;
	}

	if( accel != action->accel() ) {
		action->setAccel( accel );
	}

	if( iconPath != action->iconPath() ) {
		action->setIcon( iconPath );
	}

	dirty = false;
}

KActCfgListItem::~KActCfgListItem()
{
	delete icon; icon = 0;
}

void KActCfgListItem::setAllText()
{

	setText( 0, action->localDesc() );

	if( icon ) {
		setPixmap( 1, icon->pixmap() );
	}
	else if ( !action->iconPath().isEmpty() ) {
		icon = new QIconSet( 
				KGlobal::iconLoader()->loadIcon( iconPath,	
					false ) );
		setPixmap( 1, icon->pixmap() );
	}
	else {
		setPixmap( 1, QPixmap() );
	}


	setText( 2, KAction::keyToString( accel ) );
}

///-----------------------------------------------------

KActItemCfg::KActItemCfg( QWidget *parent )
	: QWidget( parent )
{
	_iconBtn = new KIconLoaderButton( this );
	QBoxLayout *layout = new QHBoxLayout( this );
	layout->addWidget( _iconBtn );
}
