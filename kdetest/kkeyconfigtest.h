#ifndef TESTKEY_H
#define TESTKEY_H  

class Widget2;

class KKeyTestView : public QDialog
{
 Q_OBJECT
		
 public:
	KKeyTestView( QWidget *parent=0, const char *name=0 );
	
 public slots:
	void config();
	void functionTest1();
	void functionTest2();
	void widget2();
	void closeWidget2();
	
 private:
	QLabel *lAction;
	QPushButton *bConfig, *bQuit, *bWidget2;
	Widget2 *dWidget2;
};
 
class Widget2 : public QDialog
{
 Q_OBJECT
		
 public:
	Widget2( QWidget *parent=0, const char *name=0 );
	
 public slots:
	void function1();
	void function2();
	
 private:
	QLabel *lMsg;
};
  
#endif
