#include <klistview.h>
#include <kapplication.h>
#include <kdialogbase.h>
#include <qvbox.h>


int main( int argc, char **argv )
{
	KApplication app( argc, argv, "klistviewtest" );
	KDialogBase dialog;
	KListView *view = new KListView( dialog.makeVBoxMainWidget() );
	view->addColumn("Column 1");
	view->addColumn("Column 2");
	view->addColumn("Column 3");

	new QListViewItem( view, "Item 1");
	new QListViewItem( view, "Item 2", "Some more", "Hi Mom :)" );	

	view->restoreLayout( KGlobal::config(), "ListView" );

	new QListViewItem( view, "Item 3" );

	dialog.exec();
	view->saveLayout( KGlobal::config(), "ListView" );

	return 0;
}
