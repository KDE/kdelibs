/* This file is part of the KDE libraries
   Copyright (C) 1996 Martynas Kunigelis

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
/**
 * KProgress -- a progress indicator widget for KDE.
 */

#include <qpainter.h>
#include <qpixmap.h>
#include <qstring.h>
#include <qregexp.h>
#include <qstyle.h>

#include "kprogress.h"

#include <kapplication.h>

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
	format_ = "%p%";
	use_supplied_bar_color = false;
	bar_pixmap = 0;
	bar_style = Solid;
	text_enabled = TRUE;
	setBackgroundMode( PaletteBase );
	connect(kapp, SIGNAL(appearanceChanged()), this, SLOT(paletteChange()));
	QFont f(QString::fromLatin1("helvetica"), 12, QFont::Bold);
	f.setPixelSize(12);
	setFont(f);
	paletteChange();
}

void KProgress::paletteChange()
{
	QPalette p = kapp->palette();
	const QColorGroup &colorGroup = p.active();
	if (!use_supplied_bar_color)
		bar_color = colorGroup.highlight();
	bar_text_color = colorGroup.highlightedText();
	text_color = colorGroup.text();
	setPalette(p);

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
	use_supplied_bar_color = true;
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

QSize KProgress::minimumSizeHint() const
{
	return sizeHint();
}

QSizePolicy KProgress::sizePolicy() const
{
	if ( orientation()==KProgress::Vertical )
		return QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
	else
		return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
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
	return abs_range ? range * abs_value / abs_range : 0;
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

void KProgress::styleChange(QStyle&)
{
	adjustStyle();
}

void KProgress::adjustStyle()
{
#if QT_VERSION < 300
	switch (style().guiStyle()) {
#else
	switch (style().styleHint(QStyle::SH_GUIStyle)) {
#endif
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

void KProgress::paletteChange( const QPalette &p )
{
	// This never gets called for global color changes 
	// because we call setPalette() ourselves.
	QFrame::paletteChange(p);
}

void KProgress::drawText(QPainter *p)
{
	QRect r(contentsRect());
	//QColor c(bar_color.rgb() ^ backgroundColor().rgb());

	// Rik: Replace the tags '%p', '%v' and '%m' with the current percentage,
	// the current value and the maximum value respectively.
	QString s(format_);

	s.replace(QRegExp(QString::fromLatin1("%p")), QString::number(recalcValue(100)));
	s.replace(QRegExp(QString::fromLatin1("%v")), QString::number(value()));
	s.replace(QRegExp(QString::fromLatin1("%m")), QString::number(maxValue()));

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

void KProgress::setFormat(const QString & format)
{
	format_ = format;
}

QString KProgress::format() const
{
	return format_;
}

#include "kprogress.moc"
