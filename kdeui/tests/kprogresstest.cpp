#include <qwidget.h>
#include <qlabel.h>

#include <kapplication.h>
#include "kprogress.h"
#include "klineedit.h"


class MyWidget : public QWidget {
public:
	MyWidget() : QWidget()
	{
		setFixedSize(440, 100);
		lineEdit = new KLineEdit(this);
		lineEdit->setReadOnly(true);
		lineEdit->setEnableSmartTextUpdate (true);
		lineEdit->setText ("This is a test message");
		lineEdit->resize (400, 30);
		lineEdit->move(10, 10);
		
		label = new QLabel(this);
		label->setText ("0 %");
		label->resize (400, 30);
		label->move(10, 40);

		Progress = new KProgress(this);
		Progress->resize(400, 30);
		Progress->move(10, 70);
		startTimer(50);
	}
	
private:
	KProgress *Progress;
	KLineEdit *lineEdit;
	QLabel *label;
	
	void timerEvent(QTimerEvent *);
};

void MyWidget::timerEvent(QTimerEvent *)
{
  static enum { fwd, back } direction = fwd;
  //static KProgress::BarStyle style = KProgress::Solid;
  if (direction == fwd) 
	{
	  if (Progress->value() == Progress->maxValue())
		direction = back;
	  else
		Progress->advance(1);
		label->setText( Progress->progressString());
		lineEdit->setText ("This is a test message");
	} 
  else 
	{
	  if (Progress->value() == 0 /*Progress->minValue()*/) 
		{
				direction = fwd;
				//style = (style == KProgress::Solid)? KProgress::Blocked : KProgress::Solid;
				//Progress->setBarStyle(style);
		} 
	  else
		Progress->advance(-1);
		label->setText( Progress->progressString());
		lineEdit->setText ("This is a test message");
	}
}

int main(int argc, char *argv[])
{
	KApplication app(argc, argv, "KProgressTest");
	MyWidget w;
	
	app.setMainWidget(&w);

	w.show();
	
	int ret = app.exec();
	return ret;
}
