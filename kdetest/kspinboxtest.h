


class BoolTest : public QWidget
{
	Q_OBJECT;
	
  public:
	BoolTest(QWidget *parent=0,const char *name=0);
	~BoolTest();

  protected:
	void resizeEvent(QResizeEvent *);
	
 protected slots:
	void slotChangeValue();
 
private:
	KSpinBox *_spbt;
};

class StringTest : public QObject // Widget
{
	Q_OBJECT;
	
  public:
	StringTest(); // QWidget *parent=0,const char *name=0);
	~StringTest();

  public slots:
	  void catchIncr();
	  void catchDecr();

	
};
