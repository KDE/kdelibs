#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kvbox.h>

#include "k3listview.h"

int main( int argc, char **argv )
{
	KCmdLineArgs::init( argc, argv, "klistviewtest", 0, ki18n("K3ListViewTest"), "1.0", ki18n("klistview test app"));
	KApplication app;
	KDialog dialog;
	K3ListView *view = new K3ListView();
  dialog.setMainWidget(view);
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

	view->restoreLayout( KGlobal::config().data(), "ListView" );

	new K3ListViewItem( view, "Item 3" );

	dialog.exec();
	view->saveLayout( KGlobal::config().data(), "ListView" );

	return 0;
}
