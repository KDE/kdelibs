// KSpinBox.cpp - cycle through a list of values
// started: 970506 afo
// [Last modified: 970717 09:52:10 by afo@fh-zwickau.de (Andre Fornacon)]

// (c) 1997 by Andre Fornacon (afo@fh-zwickau.de)

// @TODO:
// - disable boxs at (min,max) and/or wraparound ?
// - other layouts (vert,horiz)

#include <stdlib.h>
#include <stdio.h>

#include <qapp.h>		// for beep()

#include "kspinbox.h"
#include "kspinbox.moc"

#if 0
#define DBG(s)		s
#else
#define DBG(s)
#endif


///////////////////////////////////////////////////////////////////////////
///
///                         KSpinBox
///
///////////////////////////////////////////////////////////////////////////

// timeout for autocounting after pressing the mousebox
static const int timerInterval=150;

// make these user settable ?!
static const char *STR_INCR="+";
static const char *STR_DECR="-";


// QLineEdit doesn't support non editable text fields
// and setEnabled(FALSE) doesn't look nice enough
// thats why i switch between an label and an lineedit widget
// the following macros should make it easier to deal with this situation
#define GET_VALUE()		(_editable ? _edit->text() : _label->text())
#define SET_VALUE(s)	(_editable ? _edit->setText(s) : _label->setText(s))


KSpinBox::KSpinBox(QWidget *parent,const char *name,int align)
  : QWidget(parent,name)
{
  // create the widgets
  _label=new QLabel(this,"_label");
  _label->setBackgroundColor(white);
  _label->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	
  _edit=new QLineEdit(this,"_edit");
	
  _decr=new QPushButton(this,"_decr");
  _decr->setText(STR_DECR);

  _incr=new QPushButton(this,"_incr");
  _incr->setText(STR_INCR);

  // create the timer objects and connect the signals
	
  _incrTimer=new QTimer(this,"_incrTimer");
  _decrTimer=new QTimer(this,"_decrTimer");

  connect(_incrTimer,SIGNAL(timeout()), this,SLOT(slotIncrease()) );
  connect(_decrTimer,SIGNAL(timeout()), this,SLOT(slotDecrease()) );

  connect(_incr,SIGNAL(pressed()), this,SLOT(slotStartIncr()) );
  connect(_incr,SIGNAL(released()), this,SLOT(slotStopIncr()) );

  connect(_decr,SIGNAL(pressed()), this,SLOT(slotStartDecr()) );
  connect(_decr,SIGNAL(released()), this,SLOT(slotStopDecr()) );

	// set initial state
  _align=align;
  setEditable(FALSE);
}

KSpinBox::~KSpinBox()
{
  delete _edit;
  delete _label;

  delete _incr;
  delete _decr;

  delete _incrTimer;
  delete _decrTimer;
}

const char *KSpinBox::getValue()
{
  return GET_VALUE();
}

void KSpinBox::setValue(const char *value)
{
  //	SET_VALUE(value);
  _edit->setText(value);
  _label->setText(value);
}

bool KSpinBox::isEditable()
{
  return _editable;
}

void KSpinBox::setEditable(bool flag)
{
  _editable=flag;

  if(_editable)
	{
	  _label->hide();
	  _edit->show();
	}
  else
	{
	  _edit->hide();
	  _label->show();
	}
}

void KSpinBox::setAlign(int align)
{
  if(align == AlignCenter || align == AlignLeft || align == AlignRight)
	{
	  _align=align;
	  resizeEvent(0);
	}
}

int KSpinBox::getAlign()
{
  return _align;
}


void KSpinBox::resizeEvent(QResizeEvent *)
{
  int w=width();
  int h=height();
  int h2=h/2;
	
  switch(_align)
	{
	case AlignCenter:		// horizontal, text centered
	  _edit->setGeometry(h,0,w-2*h,h);
	  _label->setGeometry(h,0,w-2*h,h);
	  _decr->setGeometry(0,0,h,h);
	  _incr->setGeometry(w-h,0,h,h);
	  break;

	case AlignRight:		// horizontal, text right
	  _edit->setGeometry(h2,0,w-h2,h);
	  _label->setGeometry(h2,0,w-h2,h);
	  _incr->setGeometry(0,0,h2,h2);
	  _decr->setGeometry(0,h2,h2,h2);
	  break;
		
	case AlignLeft:		// horizontal, text left
	  _edit->setGeometry(0,0,w-h2,h);
	  _label->setGeometry(0,0,w-h2,h);
	  _incr->setGeometry(w-h2,0,h2,h2);
	  _decr->setGeometry(w-h2,h2,h2,h2);
	  break;
	}
}


/////////////////////////////// slots //////////////////////////////////


void KSpinBox::slotIncrease()
{
  DBG(puts("Incr()"));

  emit valueIncreased();	
}

void KSpinBox::slotDecrease()
{
  DBG(puts("Decr()"));

  emit valueDecreased();
}


void KSpinBox::slotStartIncr()
{
  DBG(puts("startIncr()"));
	
  if(!_incrTimer->isActive())
	{
	  _incrTimer->start(timerInterval);
	  slotIncrease();
	}
}


void KSpinBox::slotStopIncr()
{
  DBG(puts("stopIncr()"));

  //	if(_incrTimer->isActive())
  _incrTimer->stop();
}

void KSpinBox::slotStartDecr()
{
  DBG(puts("startDecr()"));

  if(!_decrTimer->isActive())
	{
	  _decrTimer->start(timerInterval);
	  slotDecrease();
	}
}

void KSpinBox::slotStopDecr()
{
  DBG(puts("stopDecr()"));

  //	if(_decrTimer->isActive())
  _decrTimer->stop();
}


///////////////////////////////////////////////////////////////////////////
///
///                     KNumericSpinBox
///
///////////////////////////////////////////////////////////////////////////


KNumericSpinBox::KNumericSpinBox(QWidget *parent,const char *name,int align)
  : KSpinBox(parent,name,align)
{
  // set default values
  _min=1;
  _max=10;
  _step=1;
  setValue(1);
  //	setEditable(TRUE);

	// overwrite parents slots
  connect(this,SIGNAL(valueIncreased()), SLOT(slotIncrease()) );
  connect(this,SIGNAL(valueDecreased()), SLOT(slotDecrease()) );

}


KNumericSpinBox::~KNumericSpinBox()
{
	
}


int KNumericSpinBox::getValue()
{
  return atoi(KSpinBox::getValue());
}


void KNumericSpinBox::setValue(int value)
{
  DBG(printf("numSet(%d): %d .. %d\n",value,_min,_max));
	
  if(_min <= value && value <= _max)
	{
	  char buf[20];
	  sprintf(buf,"%d",value);
	  KSpinBox::setValue(buf);
	}
  else	
	qApp->beep();	// wrap around instead ?
}


int KNumericSpinBox::getStep()
{
  return _step;
}

void KNumericSpinBox::setStep(int step)
{
  _step=step;
}

void KNumericSpinBox::setRange(int min, int max)
{
  if(min < max)
	{
	  _min=min;
	  _max=max;
	}
}

void KNumericSpinBox::slotIncrease()
{
  DBG(puts("numIncr()"));

  setValue(getValue()+_step);	
}

void KNumericSpinBox::slotDecrease()
{
  DBG(puts("numDecr()"));

  setValue(getValue()-_step);	
}


///////////////////////////////////////////////////////////////////////////
///
///                     KListSpinBox
///
///////////////////////////////////////////////////////////////////////////


KListSpinBox::KListSpinBox(QStrList *list,QWidget *parent,const char *name,int align)
  : KSpinBox(parent,name,align)
{
  // set default values

  // overwrite parents slots
  connect(this,SIGNAL(valueIncreased()), SLOT(slotIncrease()) );
  connect(this,SIGNAL(valueDecreased()), SLOT(slotDecrease()) );

  _list=list;
  setIndex(_index=0);
}


KListSpinBox::~KListSpinBox()
{
	
}

void KListSpinBox::slotIncrease()
{
  DBG(puts("listIncr()"));

  _index=(_index < _list->count()-1 ? _index + 1 : 0);
  setValue(_list->at(_index));
}

void KListSpinBox::slotDecrease()
{
  DBG(puts("listDecr()"));

  _index=(_index > 0 ? _index : _list->count()) - 1;
  setValue(_list->at(_index));
}

void KListSpinBox::setIndex(unsigned int index)
{
  if(index < _list->count())
	setValue(_list->at(_index=index));
}

unsigned int KListSpinBox::getIndex()
{
  return _index;
}
