#include <qdialog.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <qmsgbox.h>

#include <kapp.h>
#include <kkeyconf.h>

#include "kkeyconfigtest.h"
#include "kkeyconfigtest.moc"

KKeyTestView::KKeyTestView( QWidget *parent, const char *name )
    : QDialog( parent, name )
{
	setCaption("KKey Test");

	dWidget2 = new Widget2(this);
	dWidget2->hide();
	
	/* create functionName/keyCode association */
	kKeys->addKey("Function 1", "Backspace", FALSE);
	kKeys->addKey("Function 2", "B+ALT");
	kKeys->addKey("Quit", "CTRL+Q", FALSE);     
	kKeys->addKey("Configure", "F10"); 
	kKeys->addKey("showSecondWidget", "ALT+F1");
	kKeys->addKey("closeSecondWidget", "Up");
    kKeys->addKey("Close dialog", "Return");
	kKeys->addKey("Dummy III", "CTRL+SHIFT+ALT+Plus", FALSE);      
	
	/* register the main widget */
	kKeys->registerWidget("main widget", this);
	/* define the connection for the main widget */
	kKeys->connectFunction( "main widget", "Function 1",
						    this, SLOT(functionTest1()));
	kKeys->connectFunction( "main widget", "Function 2",
						    this, SLOT(functionTest2()));
	kKeys->connectFunction( "main widget", "Quit", 
						    kapp, SLOT(quit()) );
	kKeys->connectFunction( "main widget", "Configure", 
						    this, SLOT(config()) );
	kKeys->connectFunction( "main widget", "showSecondWidget",
                            this, SLOT(widget2()) ); 
	kKeys->connectFunction( "main widget", "Configure",
						    this, SLOT(closeWidget2()) );
	
	/* register this widget */
	kKeys->registerWidget("widget 2", dWidget2);
	/* define the connection for this widget
	   note that some functionName/keyCode are also used in the main widget
	   but not connected to the same functions 
	*/
	kKeys->connectFunction( "widget 2", "Function 1",
						    dWidget2, SLOT(function1()));
    kKeys->connectFunction( "widget 2", "Function 2",
						    dWidget2, SLOT(function2()));
	kKeys->connectFunction( "widget 2", "Configure",
						    this, SLOT(config()) );
	kKeys->connectFunction( "widget 2", "closeSecondWidget",
						    this, SLOT(closeWidget2()) );
	
	lAction = new QLabel(this);
    lAction->setGeometry(10,10,270,20);
	
	bConfig = new QPushButton(this);
	bConfig->setText("Keys Config");
	bConfig->setGeometry(20,70,100,30);
	connect(bConfig, SIGNAL(clicked()), SLOT(config()));
	
	bWidget2 = new QPushButton(this);
	bWidget2->setText("Another widget");
	bWidget2->setGeometry(150, 70, 100, 30 );
	connect(bWidget2, SIGNAL(clicked()), SLOT(widget2()));
	
	setGeometry(30, 30, 280, 110);
}

void KKeyTestView::config()
{
	kKeys->configureKeys(this);
}

void KKeyTestView::functionTest1()
{
	lAction->setText("function 1 called in main widget");
	char str[100];
	QMessageBox ab;
	
	ab.setCaption("kmines : About");
	
	sprintf(str,"test box");
	ab.setText(str);
	ab.setButtonText("Close");
	D_CLOSE_KEY( "about", &ab );
	ab.show(); 
}

void KKeyTestView::functionTest2()
{
	lAction->setText("function 2 called in main widget");
}

void KKeyTestView::widget2()
{
	dWidget2->show();
}

void KKeyTestView::closeWidget2()
{
	dWidget2->hide();
}

Widget2::Widget2( QWidget *parent, const char *name )
    :QDialog(parent, name, TRUE)
{
	lMsg = new QLabel(this);
	lMsg->setGeometry(20, 30, 270, 30);
	resize(290,90);
}

void Widget2::function1()
{
	lMsg->setText("function 1 called in widget 2");
}

void Widget2::function2()
{
	lMsg->setText("function 2 called in widget 2");
}

int main( int argc, char ** argv )
{
	KApplication a(argc, argv);
	KKeyTestView *v = new KKeyTestView;
	a.setMainWidget(v);
	v->show();
	return a.exec();
}
    
