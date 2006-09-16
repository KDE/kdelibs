#include "qstarframe.h"

#include <qpainter.h>
#include <qpixmap.h>

#include <kstandarddirs.h>

QStarFrame::QStarFrame(QWidget *parent)
: QFrame(parent)
{
	setFixedHeight(24);
	setFrameStyle(QFrame::Sunken | QFrame::Panel);

	m_rating = 0;
}

void QStarFrame::slotRating(int rating)
{
	m_rating = rating;

	drawstars();
}

void QStarFrame::drawstars()
{
	QString starpath = locate("data", "kpdf/pics/ghns_star.png");
	QString graystarpath = locate("data", "kpdf/pics/ghns_star_gray.png");

	QPixmap star(starpath);
	QPixmap graystar(graystarpath);

	int wpixels = (int)(width() * (float)m_rating / 100.0);

	QPainter p;
	p.begin(this);
	int w = star.width();
	for(int i = 0; i < wpixels; i += star.width())
	{
		w = wpixels - i;
		if(w > star.width()) w = star.width();
		p.drawPixmap(i, 0, star, 0, 0, w, -1);
	}
	p.drawPixmap(wpixels, 0, graystar, w, 0, graystar.width() - w, -1);
	wpixels += graystar.width() - w;
	for(int i = wpixels; i < width(); i += graystar.width())
	{
		w = width() - i;
		if(w > graystar.width()) w = graystar.width();
		p.drawPixmap(i, 0, graystar, 0, 0, w, -1);
	}
	p.end();
}

void QStarFrame::paintEvent(QPaintEvent *e)
{
	Q_UNUSED(e);

	drawstars();
}

#include "qstarframe.moc"
