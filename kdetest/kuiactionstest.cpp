/*
* acttest.cpp -- Implementation of class KActTest.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Fri May  7 02:50:50 EST 1999
*/

#include<qmultilinedit.h>
#include<qpopupmenu.h>
#include<qmenubar.h>
#include<qnamespace.h>
#include<qmainwindow.h>

#include<kapp.h>
#include<kconfigbase.h>
#include<kconfig.h>
#include<kglobal.h>

#include<kuiactions.h>
#include<kactmenubuilder.h>
#include<kactconfigwidget.h>

/**
* Test harness for KAction and KUIAction
* @author Sirtaj Singh Kang (taj@kde.org)
* @version $Id$
*/
class KActTest : public QMainWindow
{
	Q_OBJECT

public:
	/**
	* KActTest Constructor
	*/
	KActTest( QWidget *parent = 0 );

	/**
	* KActTest Destructor
	*/
	virtual ~KActTest();
	
public slots:
	void save();
	void load();
	void saveToggle();

	void saveCfg();

private:
	KActTest& operator=( const KActTest& );
	KActTest( const KActTest& );

	KActionConfigWidget *_cfgw;
	KUIActions *_act;
};

KActTest::KActTest( QWidget *parent )
	: QMainWindow( parent )
{
	// actions
	_act = new KUIActions( this );
	debug( "created act" );

	_act->newAction( "Save", "Save", this, SLOT(save()), CTRL + Key_S );
	_act->newAction( "Update", "Update", this, SLOT(saveCfg()), 
			CTRL + Key_U );
	_act->newAction( "SaveEn", "Toggle Save", this, SLOT(saveToggle()),
			CTRL + Key_T );
	_act->newAction( "Quit", "Band Kar", qApp, 
			SLOT(quit()), CTRL + Key_Q );

	_act->action( "Save" )->setIcon( "save.xpm" );
	_act->action( "Update" )->setIcon( "fileopen.xpm" );
	_act->action( "Quit" )->setIcon( "quit.xpm" );

	debug( "created actions" );

	KConfigBase *cfg = KGlobal::config();
	_act->readConfig( *cfg );

//	connect( _act->action( "Quit" ), SIGNAL(activate()), 
//		this, SLOT(saveCfg()) );
	
	// menus
	QMenuBar *mymenu = new QMenuBar( this );
	QPopupMenu *file = new QPopupMenu( this );
	mymenu->insertItem( "File", file );
	
	KActionMenuBuilder mb( _act );
	mb.setMenu( file, KActionMenuBuilder::Text 
		| KActionMenuBuilder::Icon );

	mb << "Save" << "Update" << "SaveEn" << "Quit";

	debug( "connected actions" );

	_cfgw = new KActionConfigWidget( _act, this );

	setCentralWidget( _cfgw );
}

void KActTest::saveCfg()
{
	_act->writeConfig( *KGlobal::config() );
	KGlobal::config()->sync();
}

KActTest::~KActTest()
{
}

void KActTest::saveToggle()
{
	_act->setEnabled( "Save", !_act->enabled( "Save" ) );
}
void KActTest::save()
{
	_cfgw->listSync();
}

void KActTest::load()
{
	_act->action( "Load" )->setAccel( CTRL + Key_K );
}

int main( int argc, char **argv )
{
	KApplication app( argc, argv );
	debug( "created app" );

	KActTest tester;
	app.setMainWidget( &tester );
	debug( "created tester" );
	tester.show();

	app.exec();
}

#include"acttest.moc"
