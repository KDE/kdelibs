// -*- c++ -*-
// KSpinBox.h - cycle through a bunch of items
// started: 970506 afo
// [Last modified: 970717 09:51:42 by afo@fh-zwickau.de (Andre Fornacon)]

// (c) 1997 by Andre Fornacon (afo@fh-zwickau.de)


#ifndef _KSPINBOX_H
#define _KSPINBOX_H

#include <qpushbt.h>
#include <qlined.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qstrlist.h>

/// KSpinBox - cycle trough a bunch of items

/** The KSpinBox Widget lets the user cycle trough a bunch of items.
	this class alone doesn't do really usefull things
	it provides the base for things like KNumericSpinBox or
	KListSpinBox.

	You can use it in two ways:

	1. The "is A" Variant : derive a subclass from KSpinBox
	   and implement your own functionality
	
	2. The "has A" Variant : instantiate a KSpinBox object and
	   catch the valueIncreased() and valueDecreased() signals
	   and do what you want in the slots.

*/
class KSpinBox : public QWidget
{
  Q_OBJECT

public:
  /** construct a KSpinBox Widget */
  KSpinBox(QWidget *parent=0,const char *name=0,int align=AlignLeft);
  ~KSpinBox();

  /** get the value shown the text field */
  const char *getValue();

  /** set the value which should be displayed in the text field */
  void setValue(const char *);

  /** query if the user is allowed to edit the values */
  bool isEditable();

  /** decide if the user can edit the values or not */
  void setEditable(bool);

  /** set the alignment of the text field.
	  possible values for align are AlignLeft, AlignCenter, AlignRight */
  void setAlign(int align);

  /** get the alignment of the text field.
	  possible return values AlignLeft, AlignCenter, AlignRight */
  int getAlign();
	
  signals:
  void valueIncreased();
  void valueDecreased();

 public slots:
 void slotIncrease();
  void slotDecrease();
	
 protected slots:
 void slotStartIncr();
  void slotStopIncr();
  void slotStartDecr();
  void slotStopDecr();
	
protected:
  void resizeEvent(QResizeEvent *);

  QLabel *_label;
  QLineEdit *_edit;
  QPushButton *_incr,*_decr;	

private:
  bool _editable;
  QTimer *_incrTimer,*_decrTimer;
  int _align;
	
};


/// KNumericSpinBox - cycle trough a range of numeric values

/** The KNumericSpinBox Widget lets you cycle trough a range of numeric
	values. <br>
	you can set the minimum and minimum of the range .
	it's possible to specify the step - the value which is used for
	increment and decrement.
	<p>
	Warning: if you set let the user edit the values through a
	call to setEditable(TRUE) the behaviour is undefined, e.g..
	the range isn't checked
  */

class KNumericSpinBox : public KSpinBox
{
  Q_OBJECT

public:
  /** called if you create a KNumericSpinBox widget */
  KNumericSpinBox(QWidget *parent=0,const char *name=0,int align=AlignLeft);

  /** called if you destroy a KNumericSpinBox widget */
  ~KNumericSpinBox();

  /** set the value which should be displayed in the text field */
  void setValue(int value);

  /** get the value shown in the text field */
  int getValue();

  /** return the current value used for increment and decrement */
  int getStep();
		
  /** set the value used for increment and decrement */
  void setStep(int step);

  /** get the currently allowed range of values */
  void getRange(int &minimum, int &maximum);

  /** set the range allowed range of values */
  void setRange(int minimum, int maximum);

 public slots:
 void slotIncrease();
  void slotDecrease();

private:
  int _step;
  int _min,_max;
};


/// KListSpinBox - cycle trough a supplied list of items

/** The KNumericSpinBox Widget cycles trough a supplied list of items
	you can use it to let the user select things like day of week,
	month' or somethings like that.
	look into testkspinbox.cc for samples
  */

class KListSpinBox : public KSpinBox
{
  Q_OBJECT

public:
  /** called if you create a KListSpinBox widget */
  KListSpinBox(QStrList *list,QWidget *parent=0,const char *name=0,int align=AlignLeft);

  /** called if you destroy a KListSpinBox widget */
  ~KListSpinBox();

  /** set the index of the list item to be shown */
  void setIndex(unsigned int index);

  /** get the index of currently displayed list item */
  unsigned int getIndex();
	
 public slots:
 void slotIncrease();
  void slotDecrease();

private:
  QStrList *_list;
  unsigned int _index;
};


#endif // _KSPINBOX_H
