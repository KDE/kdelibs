#include "kdxsrating.h"

#include <qlayout.h>
#include <qslider.h>

#include <klocale.h>

KDXSRating::KDXSRating(QWidget *parent)
: KDialogBase(parent, "comment", true, i18n("Rate this entry"),
	KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, true)
{
	QVBoxLayout *vbox;

	QWidget *root = new QWidget(this);
	setMainWidget(root);

	m_slider = new QSlider(root);
	m_slider->setOrientation(QSlider::Horizontal);
	m_slider->setTickmarks(QSlider::Below);
	m_slider->setMinValue(0);
	m_slider->setMaxValue(100);

	vbox = new QVBoxLayout(root, 5);
	vbox->add(m_slider);
}

int KDXSRating::rating()
{
	return m_slider->value();
}

