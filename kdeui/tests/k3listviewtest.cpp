#define QT3_SUPPORT

#include "k3listview.h"
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdialogbase.h>
#include <kvbox.h>

int main( int argc, char **argv )
{
	KCmdLineArgs::init( argc, argv, "klistviewtest", "K3ListViewTest", "klistview test app", "1.0" );
	KApplication app;
	KDialogBase dialog;
	K3ListView *view = new K3ListView( dialog.makeVBoxMainWidget() );
	view->setSelectionModeExt( K3ListView::FileManager );
	view->setDragEnabled( true );
	view->setItemsMovable( false );
	view->setAcceptDrops( true );
	view->addColumn("Column 1");
	view->addColumn("Column 2");
	view->addColumn("Column 3");

	new K3ListViewItem( view, "Item 1");
	new K3ListViewItem( view, "Item 1");
	new K3ListViewItem( view, "Item 1");
	new K3ListViewItem( view, "Item 1");
	new K3ListViewItem( view, "Item 1");
	new K3ListViewItem( view, "Item 1");
	new K3ListViewItem( view, "Item 1");
	new K3ListViewItem( view, "Item 1");
	new K3ListViewItem( view, "Item 1");
	new K3ListViewItem( view, "Item 2", "Some more", "Hi Mom :)" );

	view->restoreLayout( KGlobal::config(), "ListView" );

	new K3ListViewItem( view, "Item 3" );

	dialog.exec();
	view->saveLayout( KGlobal::config(), "ListView" );

	return 0;
}
