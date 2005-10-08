#include <klistview.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdialogbase.h>
#include <q3vbox.h>
#include <kvbox.h>

int main( int argc, char **argv )
{
	KCmdLineArgs::init( argc, argv, "klistviewtest", "KListViewTest", "klistview test app", "1.0" );
	KApplication app;
	KDialogBase dialog;
	KListView *view = new KListView( dialog.makeVBoxMainWidget() );
	view->setSelectionModeExt( KListView::FileManager );
	view->setDragEnabled( true );
	view->setItemsMovable( false );
	view->setAcceptDrops( true );
	view->addColumn("Column 1");
	view->addColumn("Column 2");
	view->addColumn("Column 3");

	new Q3ListViewItem( view, "Item 1");
	new Q3ListViewItem( view, "Item 1");
	new Q3ListViewItem( view, "Item 1");
	new Q3ListViewItem( view, "Item 1");
	new Q3ListViewItem( view, "Item 1");
	new Q3ListViewItem( view, "Item 1");
	new Q3ListViewItem( view, "Item 1");
	new Q3ListViewItem( view, "Item 1");
	new Q3ListViewItem( view, "Item 1");
	new Q3ListViewItem( view, "Item 2", "Some more", "Hi Mom :)" );

	view->restoreLayout( KGlobal::config(), "ListView" );

	new Q3ListViewItem( view, "Item 3" );

	dialog.exec();
	view->saveLayout( KGlobal::config(), "ListView" );

	return 0;
}
