/*
 * KProgress -- a progress indicator widget for KDE. 
 * (c) 1996 Martynas Kunigelis
 */
 
#include "kprogress.h"
#include "kprogress.h"

#include <qpainter.h>
#include <qstring.h>
#include <qpixmap.h>
#include <kapp.h>

KProgress::KProgress(QWidget *parent, const char *name)
	: QFrame(parent, name),
	QRangeControl(0, 100, 1, 10, 0),
	orient(Horizontal)
{
	initialize();
}

KProgress::KProgress(Orientation orientation, QWidget *parent, const char *name)
	: QFrame(parent, name),
	QRangeControl(0, 100, 1, 10, 0),
	orient(orientation)
{
	initialize();
}

KProgress::KProgress(int minValue, int maxValue, int value, 
					Orientation orientation, QWidget *parent, const char *name)
	: QFrame(parent, name),
	QRangeControl(minValue, maxValue, 1, 10, value),
	orient(orientation)
{
	initialize();
}

KProgress::~KProgress()
{
}

void KProgress::advance(int offset)
{
	setValue(value() + offset);
}

void KProgress::initialize()
{
	//setFrameStyle(QFrame::Panel | QFrame::Sunken);
	//setLineWidth(2);
	//setMidLineWidth(2);
	bar_pixmap = 0;
	bar_style = Solid;
	bar_color = kapp->selectColor;
	bar_text_color = kapp->selectTextColor;
	text_color = kapp->textColor;
	setBackgroundColor( kapp->windowColor );
	setFont(QFont("helvetica", 12, QFont::Bold));
	text_enabled = TRUE;
	adjustStyle();
}


void KProgress::setBarPixmap(const QPixmap &pixmap)
{
	if (pixmap.isNull())
		return;
	if (bar_pixmap)
		delete bar_pixmap;
		
	bar_pixmap = new QPixmap(pixmap);
}

void KProgress::setBarColor(const QColor &color)
{
	bar_color = color;
	if (bar_pixmap) {
		delete bar_pixmap;
		bar_pixmap = 0;
	}
}

void KProgress::setBarStyle(BarStyle style)
{
	if (bar_style != style) {
		bar_style = style;
		update();
	}
}

void KProgress::setOrientation(Orientation orientation)
{
	if (orient != orientation) {
		orient = orientation;
		update();
	}
}
 
void KProgress::setValue(int value)
{
	QRangeControl::setValue(value);
}

void KProgress::setTextEnabled(bool enable)
{
	text_enabled = enable;
}

const QColor & KProgress::barColor() const
{
	return bar_color;
}

const QPixmap * KProgress::barPixmap() const
{
	return bar_pixmap;
}

bool KProgress::textEnabled() const
{
	return text_enabled;
}

QSize KProgress::sizeHint() const
{
  QSize s( size() );

  if(orientation() == KProgress::Vertical) {
    s.setWidth(24);
  } else {
    s.setHeight(24);
  }

  return s;
}


KProgress::Orientation KProgress::orientation() const
{
	return orient;
}

KProgress::BarStyle KProgress::barStyle() const
{
	return bar_style;
}

int KProgress::recalcValue(int range)
{
	int abs_value = value() - minValue();
	int abs_range = maxValue() - minValue();
	return range * abs_value / abs_range;
}

void KProgress::valueChange()
{
	repaint(contentsRect(), FALSE);
	emit percentageChanged(recalcValue(100));
}

void KProgress::rangeChange()
{
	repaint(contentsRect(), FALSE);
	emit percentageChanged(recalcValue(100));
}

void KProgress::styleChange(GUIStyle)
{
	adjustStyle();
}

void KProgress::adjustStyle()
{
	switch (style()) {
		case WindowsStyle:
			setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
			break;
		case MotifStyle:
		default:
			setFrameStyle(QFrame::Panel | QFrame::Sunken);
			setLineWidth( 2 );
			break;
	}
	update();
}

void KProgress::paletteChange( const QPalette & )
{
	bar_color = kapp->selectColor;
	bar_text_color = kapp->selectTextColor;
	text_color = kapp->textColor;
	setBackgroundColor( kapp->windowColor );
}
		
void KProgress::drawText(QPainter *p)
{
	QRect r(contentsRect());
	//QColor c(bar_color.rgb() ^ backgroundColor().rgb());
	QString s;
	
	s.sprintf("%i%%", recalcValue(100));
	p->setPen(text_color);
	//p->setRasterOp(XorROP);
	p->drawText(r, AlignCenter, s);
	p->setClipRegion( fr );
	p->setPen(bar_text_color);
	p->drawText(r, AlignCenter, s);
	
}

void KProgress::drawContents(QPainter *p)
{
	QRect cr = contentsRect(), er = cr;
	fr = cr;
	QBrush fb(bar_color), eb(backgroundColor());

	if (bar_pixmap)
		fb.setPixmap(*bar_pixmap);

	if (backgroundPixmap())
		eb.setPixmap(*backgroundPixmap());

	switch (bar_style) {
		case Solid:
			if (orient == Horizontal) {
				fr.setWidth(recalcValue(cr.width()));
				er.setLeft(fr.right() + 1);
			} else {
				fr.setTop(cr.bottom() - recalcValue(cr.height()));
				er.setBottom(fr.top() - 1);
			}
				
			p->setBrushOrigin(cr.topLeft());
			p->fillRect(fr, fb);
			p->fillRect(er, eb);
			
			break;
			
		case Blocked:
			const int margin = 2;
			int max, num, dx, dy;
			if (orient == Horizontal) {
				fr.setHeight(cr.height() - 2 * margin);
				fr.setWidth((int)(0.67 * fr.height()));
				fr.moveTopLeft(QPoint(cr.left() + margin, cr.top() + margin));
				dx = fr.width() + margin;
				dy = 0;
				max = (cr.width() - margin) / (fr.width() + margin) + 1;
				num = recalcValue(max);
			} else {
				fr.setWidth(cr.width() - 2 * margin);
				fr.setHeight((int)(0.67 * fr.width()));
				fr.moveBottomLeft(QPoint(cr.left() + margin, cr.bottom() - margin));
				dx = 0;
				dy = - (fr.height() + margin);
				max = (cr.height() - margin) / (fr.height() + margin) + 1;
				num = recalcValue(max);
			}
			p->setClipRect(cr.x() + margin, cr.y() + margin, 
						   cr.width() - margin, cr.height() - margin);
			for (int i = 0; i < num; i++) {
				p->setBrushOrigin(fr.topLeft());
				p->fillRect(fr, fb);
				fr.moveBy(dx, dy);
			}
			
			if (num != max) {
				if (orient == Horizontal) 
					er.setLeft(fr.right() + 1);
				else
					er.setBottom(fr.bottom() + 1);
				if (!er.isNull()) {
					p->setBrushOrigin(cr.topLeft());
					p->fillRect(er, eb);
				}
			}
			
			break;
	}	
				
	if (text_enabled && bar_style != Blocked)
		drawText(p);
		
}			
#include "kprogress.moc"
