#include "kpdqoptionview.h"
#include "pdqclass.h"
#include "kpdqbaseitem.h"
#include "kpdqoptionitem.h"
#include "kpdqargitem.h"

#include <qwidgetstack.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlayout.h>
#include <qwidget.h>
#include <klocale.h>

KPdqOptionView::KPdqOptionView(QWidget *parent, const char *name)
: QGroupBox(parent,name)
{
	setTitle(i18n("No option selected"));
	widgetlist_.setAutoDelete(false);
	item_ = 0;

	stack_ = new QWidgetStack(this);

	QWidget	*w = new QWidget(stack_);
	stack_->addWidget(w,0);
	widgetlist_.append(w);

	list_ = new QListBox(stack_);
	stack_->addWidget(list_,1);
	widgetlist_.append(list_);

	w = new QWidget(stack_);
	edit_ = new QLineEdit(w);
	stack_->addWidget(w,2);
	widgetlist_.append(w);

	QVBoxLayout	*main_ = new QVBoxLayout(this, 10, 10);
	main_->addSpacing(10);
	main_->addWidget(stack_,1);

	QVBoxLayout	*s_ = new QVBoxLayout(w, 0, 0);
	s_->addStretch(1);
	s_->addWidget(edit_,0);
	s_->addStretch(1);

	connect(list_,SIGNAL(highlighted(int)),SLOT(valueChanged(int)));
	connect(edit_,SIGNAL(textChanged(const QString&)),SLOT(valueChanged(const QString&)));
}

KPdqOptionView::~KPdqOptionView()
{
}

QSize KPdqOptionView::sizeHint() const
{
	return QSize(200,100);
}

void KPdqOptionView::optionSelected(QListViewItem *item)
{
	item_ = (KPdqBaseItem*)item;
	if (!item_ || !item_->pdqItem() || item_->type() == KPdqBaseItem::Base)
	{
		stack_->raiseWidget(widgetlist_.at(0));
		setTitle(i18n("No option selected"));
		return;
	}

	if (item_->type() == KPdqBaseItem::Option)
	{
		stack_->raiseWidget(widgetlist_.at(1));
		KPdqOptionItem	*oitem = (KPdqOptionItem*)item_;
		setTitle(oitem->option_->description_);
		list_->clear();
		QListIterator<PDQChoice>	it(oitem->option_->choices_);
		for (;it.current();++it)
		{
			list_->insertItem(it.current()->description_);
			if (it.current() == oitem->currentchoice_) list_->setCurrentItem(list_->count()-1);
		}
	}
	else if (item_->type() == KPdqBaseItem::Arg)
	{
		stack_->raiseWidget(widgetlist_.at(2));
		KPdqArgItem	*aitem = (KPdqArgItem*)item_;
		setTitle(aitem->argument_->description_);
		edit_->setText(aitem->value_);
	}
}

void KPdqOptionView::valueChanged(int index)
{
	if (item_ && item_->type() == KPdqBaseItem::Option)
	{
		KPdqOptionItem	*oitem = (KPdqOptionItem*)item_;
		oitem->setCurrentChoice(index);
	}
}

void KPdqOptionView::valueChanged(const QString& txt)
{
	if (item_ && item_->type() == KPdqBaseItem::Arg)
	{
		KPdqArgItem	*aitem = (KPdqArgItem*)item_;
		aitem->setValue(txt);
	}
}
#include "kpdqoptionview.moc"
