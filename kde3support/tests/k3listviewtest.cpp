#include <qapplication.h>
#include <kconfig.h>
#include <ksharedconfig.h>
#include <kdialog.h>

#include "k3listview.h"

int main( int argc, char **argv )
{
    QApplication::setApplicationName("klistviewtest");
	QApplication app(argc, argv);
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

	view->restoreLayout( KSharedConfig::openConfig().data(), "ListView" );

	new K3ListViewItem( view, "Item 3" );

	dialog.exec();
	view->saveLayout( KSharedConfig::openConfig().data(), "ListView" );

	return 0;
}
