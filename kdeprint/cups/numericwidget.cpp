#include "numericwidget.h"

#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qstring.h>
#include <qslider.h>

NumericWidget::NumericWidget(QWidget *parent, const char *name)
	: QWidget(parent, name)
{
	block_ = false;

	QLabel	*l1 = new QLabel(tr("Value: "), this);
	l1->setMinimumSize(l1->sizeHint());
	edit_ = new QLineEdit(this);
	QSize	s = edit_->sizeHint();
	s.setWidth(100);
	edit_->setMinimumSize(s);
	edit_->setMaximumSize(s);
	slider_ = new QSlider(QSlider::Horizontal, this);
	slider_->setTickmarks(QSlider::Below);
	s = slider_->sizeHint();
	slider_->setMinimumSize(s);
	slider_->setMaximumSize(QSize(32000,s.height()));

	connect(slider_, SIGNAL(valueChanged(int)), SLOT(sliderChanged(int)));
	connect(edit_, SIGNAL(textChanged(const QString&)), SLOT(editChanged(const QString&)));

	value_ = 1;
	factor_ = 1;
	minlabel_ = new QLabel(this);
	maxlabel_ = new QLabel(this);
	min_ = 1;
	max_ = 100;
	edit_->setText("1");

	setLimits(1.0, 100.0);

	QVBoxLayout	*main_ = new QVBoxLayout(this, 0, 10);
	QHBoxLayout	*sub1_ = new QHBoxLayout(0, 0, 10), *sub2_ = new QHBoxLayout(0, 0, 10);
	main_->addStretch(1);
	main_->addLayout(sub1_);
	main_->addStretch(1);
	main_->addLayout(sub2_);
	main_->addStretch(1);
	sub1_->addWidget(l1);
	sub1_->addWidget(edit_);
	sub1_->addStretch(1);
	sub2_->addWidget(minlabel_);
	sub2_->addWidget(slider_, 1);
	sub2_->addWidget(maxlabel_);
}

NumericWidget::~NumericWidget()
{
}

void NumericWidget::setLimits(double minval, double maxval)
{
	min_ = (int)(factor_*minval);
	max_ = (int)(factor_*maxval);

	block_ = true;
	slider_->setRange(min_,max_);
	slider_->setSteps(1,QMAX(1,(max_-min_)/20));
	block_ = false;

	QString	txt;	
	if (factor_ == 1) txt.setNum((int)minval);
	else txt.setNum(minval,'f',3);
	minlabel_->setText(txt);
	QSize	s = minlabel_->sizeHint();
	minlabel_->setMinimumSize(s);
	minlabel_->setMaximumSize(s);
	if (factor_ == 1) txt.setNum((int)maxval);
	else txt.setNum(maxval,'f',3);
	maxlabel_->setText(txt);
	s = maxlabel_->sizeHint();
	maxlabel_->setMinimumSize(s);
	maxlabel_->setMaximumSize(s);
}

void NumericWidget::setFactor(int fact)
{
	factor_ = fact;
}

void NumericWidget::setValue(double value)
{
	int	intval = (int)(value*double(factor_));
	intval = QMAX(min_,QMIN(intval,max_));
	slider_->setValue(intval);
}

void NumericWidget::sliderChanged(int value)
{
	if (block_) return;

	double	dbvalue = double(value) / factor_;
	QString	txt;
	if (factor_ == 1) txt.setNum(value);
	else txt.setNum(dbvalue,'f',3);
	block_ = true;
	edit_->setText(txt);
	block_ = false;
	emit valueChanged(dbvalue);
}

void NumericWidget::editChanged(const QString& str)
{
	if (block_) return;

	bool	ok;
	double	dbvalue;
	if (factor_ == 1)
	{
		dbvalue = (double)(str.toInt(&ok));
	}
	else
	{
		dbvalue = str.toDouble(&ok);
	}
	if (ok)
	{
		block_ = true;
		slider_->setValue((int)(dbvalue*factor_));
		block_ = false;
		emit valueChanged(dbvalue);
	}
	else
	{
		QApplication::beep();
		edit_->selectAll();
	}
}
