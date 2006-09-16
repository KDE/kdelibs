#include "kdxsrating.h"

#include "qstarframe.h"

#include <qlayout.h>
#include <qslider.h>
#include <qlabel.h>

#include <klocale.h>
#include <kstandarddirs.h>

KDXSRating::KDXSRating(QWidget *parent)
: KDialogBase(parent, "comment", true, i18n("Rate this entry"),
	KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, true)
{
	QVBoxLayout *vbox;
	QHBoxLayout *hbox;

	QWidget *root = new QWidget(this);
	setMainWidget(root);

	m_slider = new QSlider(root);
	m_slider->setOrientation(QSlider::Horizontal);
	m_slider->setTickmarks(QSlider::Below);
	m_slider->setMinValue(0);
	m_slider->setMaxValue(100);

	m_starrating = new QStarFrame(root);
	m_starrating->setMinimumWidth(100);

	m_rating = new QLabel(QString("0/100"), root);
	m_rating->setFixedWidth(100);

	vbox = new QVBoxLayout(root, spacingHint());
	hbox = new QHBoxLayout(vbox);
	hbox->add(m_rating);
	hbox->add(m_starrating);
	vbox->add(m_slider);

	connect(m_slider, SIGNAL(valueChanged(int)),
		m_starrating, SLOT(slotRating(int)));
	connect(m_slider, SIGNAL(valueChanged(int)), SLOT(slotRating(int)));
}

int KDXSRating::rating()
{
	return m_slider->value();
}

void KDXSRating::slotRating(int rating)
{
	m_rating->setText(QString("%1/100").arg(rating));
}

/*
void KDXSRating::slotRating(int rating)
{
	QString starpath = locate("data", "kpdf/pics/ghns_star.png");
	QString graystarpath = locate("data", "kpdf/pics/ghns_star_gray.png");

	QPixmap star(starpath);
	QPixmap graystar(graystarpath);

	int wpixels = (int)(m_starrating->width() * (float)rating / 100.0);

	QPainter p;
	p.begin(m_starrating);
	int w = star.width();
	for(int i = 0; i < wpixels; i += star.width())
	{
		w = wpixels - i;
		if(w > star.width()) w = star.width();
		p.drawPixmap(i, 0, star, 0, 0, w, -1);
	}
	p.drawPixmap(wpixels, 0, graystar, w, 0, graystar.width() - w, -1);
	wpixels += graystar.width() - w;
	for(int i = wpixels; i < m_starrating->width(); i += graystar.width())
	{
		w = m_starrating->width() - i;
		if(w > graystar.width()) w = graystar.width();
		p.drawPixmap(i, 0, graystar, 0, 0, w, -1);
	}
	p.end();

	m_rating->setText(QString("%1/100").arg(rating));
}

void KDXSRating::paintEvent(QPaintEvent e)
{
	Q_UNUSED(e);

	kdDebug() << "paint event!" << endl;

	int rating = m_slider->value();
	slotRating(rating);
}
*/

#include "kdxsrating.moc"
