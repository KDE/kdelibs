// testkspinbox.cc - demo program for KSpinBox and subclasses
// started: 970607 afo
// [Last modified: 970717 09:51:29 by afo@fh-zwickau.de (Andre Fornacon)]

#include <kapp.h>
#include <qlabel.h>

#include <kspinbox.h>
#include <kspinboxtest.h>
#include <stdio.h>

//////////////////////////// bool test ////////////////////////////////////

BoolTest::BoolTest(QWidget *parent,const char *name)
	: QWidget(parent,name)
{
	_spbt=new KSpinBox(this);

	_spbt->setValue("True");
	connect(_spbt,SIGNAL(valueIncreased()), SLOT(slotChangeValue()) );
	connect(_spbt,SIGNAL(valueDecreased()), SLOT(slotChangeValue()) );

//	_spbt->show();
}

BoolTest::~BoolTest()
{
	delete _spbt;
}

void BoolTest::resizeEvent(QResizeEvent *e)
{
	_spbt->resize(e->size());
}


void BoolTest::slotChangeValue()
{
	QString tmp("True");
	
	_spbt->setValue(tmp == _spbt->getValue() ? "False" : "True");
}

//////////////////////////// StringTest ///////////////////////////////////

StringTest::StringTest()
{

}

StringTest::~StringTest()
{

}

void StringTest::catchIncr()
{
	puts("incr++");
}

void StringTest::catchDecr()
{
	puts("decr++");
}

//////////////////////////// main /////////////////////////////////////////


int main(int argc,char *argv[])
{
	KApplication app(argc,argv);

	QWidget top;
	top.resize(400,350);

	// plain
	QLabel label1("Plain KSpinBox Widget",&top);
	label1.setGeometry(20,20,260,25);

	KSpinBox count1(&top,"cnt", Qt::AlignCenter);
	count1.setGeometry(300,20,75,25);

	// numeric
	QLabel label2("KNumericSpinBox ([1 .. 50],step = 2)",&top);
	label2.setGeometry(20,60,260,25);
	
	KNumericSpinBox count2(&top,"num");
	count2.setGeometry(300,60,50,25);
	count2.setStep(2);
	count2.setRange(1,50);
	count2.setValue(17);

	// non derived
	QLabel label3("Sample for non derived Widget (see source)",&top);
	label3.setGeometry(20,100,260,25);

	BoolTest count3(&top,"bool");
	count3.setGeometry(300,100,50,25);

	// bool implemented trough a list
	QLabel label4("Bool implemented trough KListSpinBox",&top);
	label4.setGeometry(20,140,260,25);
	
	QStrList *list=new QStrList();
	list->append("true");
	list->append("false");
	
	KListSpinBox count4(list,&top,"list");
	count4.setGeometry(300,140,50,25);

	// day of week
	QLabel label5("Select day of week (KListSpinBox)",&top);
	label5.setGeometry(20,180,260,25);
	
	QStrList *days=new QStrList();
	days->append("Monday");
	days->append("Tuesday");
	days->append("Wednesday");
	days->append("Thursday");
	days->append("Friday");
	days->append("Saturday");
	days->append("Sonday");

	KListSpinBox count5(days,&top,"days");
	count5.setGeometry(300,180,80,25);

	// month
	QLabel label6("Select month (KListSpinBox)",&top);
	label6.setGeometry(20,220,260,25);
	
	QStrList *month=new QStrList();
	month->append("January");
	month->append("February");
	month->append("March");
	
	month->append("April");
	month->append("May");
	month->append("June");

	month->append("July");
	month->append("August");
	month->append("September");

	month->append("October");
	month->append("November");
	month->append("December");
	
	KListSpinBox count6(month,&top,"month");
	count6.setGeometry(300,220,80,25);
	count6.setIndex(5);

	StringTest *st=new StringTest();
	st->connect(&count6,SIGNAL(valueIncreased()),SLOT(catchIncr()) );
	st->connect(&count6,SIGNAL(valueDecreased()),SLOT(catchDecr()) );
	
	QPushButton pb(&top);
	pb.setGeometry(200,300,80,30);
	pb.setText("Quit");
	app.connect(&pb, SIGNAL(clicked()), SLOT(quit()) );

	top.show();
	app.setMainWidget(&top);	
	app.exec();
}
#include "kspinboxtest.moc"

