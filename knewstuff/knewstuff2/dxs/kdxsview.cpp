#include "kdxsview.h"

#include "kdxsbutton.h"

#include <qlayout.h>

KDXSView::KDXSView(QWidget *parent)
: QWidget(parent)
{
	setBackgroundColor(QColor(255, 255, 255));

	KDXSButton *b = new KDXSButton(this);

	QHBoxLayout *top_layout = new QHBoxLayout(this);
	top_layout->addStretch(1);
	QVBoxLayout *v_layout = new QVBoxLayout(top_layout);
	v_layout->addStretch(1);
	v_layout->add(b);
	v_layout->addStretch(1);
	top_layout->addStretch(1);
}

#include "kdxsview.moc"
