/**
 * KURLLabel class implementation
 * kurllable.cpp
 *
 * Copyright (C) 1998 Kurt Granroth <granroth@kde.org>
 */
#include "kurllabel.h"
#include "kurllabel.moc"

#include <qtooltip.h>
#include <kcursor.h>

KURLLabel::KURLLabel(QWidget *parent, const char* name, WFlags f)
	: QLabel(parent, name, f),
	  m_textAlign(Bottom),
	  m_url(0),
	  m_tipText(0),
	  m_float(false),
	  m_tips(false),
	  m_glow(true),
	  m_underline(true),
	  m_inRegion(false),
	  m_haveCursor(true),
	  m_transparent(false)
{
	/* set the defaults */
	m_hc.setNamedColor("blue");
	m_sc.setNamedColor("red");
	m_bc        = backgroundColor();


	setUseCursor(true);
	setMouseTracking(true);

	m_resetPalette();
}

KURLLabel::~KURLLabel()
{
}

const char* KURLLabel::url() const
{
	return m_url;
}

const char* KURLLabel::text() const
{
	return (const char*)m_text;
}

const QPixmap* KURLLabel::pixmap() const
{
	return &m_pixmap;
}

void KURLLabel::setURL(const char* url)
{
	/* save the input */
	m_url = url;

	/* show the tool tip if we are allowed to */
	if (m_tips)
	{
		if (m_tipText)
			QToolTip::add(this, m_tipText);
		else
			QToolTip::add(this, m_url);
	}
}

void KURLLabel::setTextAlignment(TextAlignment align)
{
	m_textAlign = align;

	if (autoResize())
		adjustSize();
	else
		repaint(true);
}

void KURLLabel::setUseCursor(bool use_cursor, const QCursor* cursor)
{
	/* set the global var */
	m_haveCursor = use_cursor;

	/* if this is false, then use the default cursor */
	if (use_cursor == false)
	{
		m_customCursor = QCursor();
		return;
	}

	/* set the cursor to the user defined one if supplied */
	if (cursor)
	{
		m_customCursor = *cursor;
	}
	else
	{
		/* otherwise, use the "hand" cursor */
		m_customCursor = KCursor::handCursor();
	}
}

void KURLLabel::setTipText(const char* tip)
{
	/* make sure there is something coming in */
	if (!tip)
		return;

	/* assign the input */
	m_tipText = tip;

	/* reset the tooltips if we can */
	if (m_tips)
		QToolTip::add(this, m_tipText);
}

void KURLLabel::setUseTips(bool tips)
{
	/* save the input */
	m_tips = tips;

	/* setup the tool tip if we are ready */
	if (m_tips)
	{
		/* can we use a user tip? */
		if (m_tipText)
			QToolTip::add(this, m_tipText);
		else if (m_url)
			QToolTip::add(this, m_url);
	}
}

void KURLLabel::setFloat(bool do_float)
{
	/* save the input */
	m_float = do_float;
}

void KURLLabel::setFont(const QFont& font)
{
	/* set the underlining */
	QFont new_font = font;
	new_font.setUnderline(m_underline);

	/* use the base setFont to do all the real work */
	QLabel::setFont(new_font);
}

void KURLLabel::setText(const char* text)
{
	/**
	 * we set the underlining now and in setUnderline
	 * and in setFont to cover all the bases.
	 * this allows the user to invoke these functions
	 * in either order.
	 */
	QFont tmp_font = font();
	tmp_font.setUnderline(m_underline);
	setFont(tmp_font);

	/* set the palette to normal (at first)*/
	setPalette(m_nsp);

	/* save a copy of the text for our uses */
	m_text = text;

	/* use the native setText for some processing */
	QLabel::setText(text);

	/* if we don't have a decent URL, set it equal to our text */
	if (m_url == 0)
		m_url = text;

	/* show the tool tip if we are allowed to */
	if (m_tips)
	{
		if (m_tipText)
			QToolTip::add(this, m_tipText);
		else
			QToolTip::add(this, m_url);
	}
}

void KURLLabel::setAltPixmap(const QPixmap& pixmap)
{
	/* set the "alt" pixmap */
	m_altPixmap = pixmap;
}

void KURLLabel::setPixmap(const QPixmap& pixmap)
{
	/* save a copy of the pixmap for use later */
	m_pixmap = pixmap;

	/* let the base setPixmap do all the work */
	QLabel::setPixmap(pixmap);
}

void KURLLabel::setMovie(const QMovie& movie)
{
	/* save a copy of the movie */
	m_movie = movie;

	/* let the base function do all the work */
	QLabel::setMovie(movie);
}

void KURLLabel::setGlow(bool glow)
{
	/* save the input */
	m_glow = glow;
}

void KURLLabel::setUnderline(bool underline)
{
	/* save the input */
	m_underline = underline;

	/* turn on or off the underlining */
	QFont tmp_font = font();
	tmp_font.setUnderline(m_underline);
	setFont(tmp_font);
}

void KURLLabel::setHighlightedColor(const QColor& high)
{
	/* set the new color */
	m_hc = high;

	/* reset the palette to display the new color */
	m_resetPalette();
	setPalette(m_nsp);
}

void KURLLabel::setHighlightedColor(const char* high)
{
	/* set the new color */
	m_hc.setNamedColor(high);

	/* reset the palette to display the new color */
	m_resetPalette();
	setPalette(m_nsp);
}

void KURLLabel::setSelectedColor(const QColor& selected)
{
	/* set the new color */
	m_sc = selected;

	/* reset the palette to display the new color */
	m_resetPalette();
}

void KURLLabel::setSelectedColor(const char* selected)
{
	/* set the new color */
	m_sc.setNamedColor(selected);

	/* reset the palette to display the new color */
	m_resetPalette();
}

void KURLLabel::setBackgroundColor(const QColor& back)
{
	/* set the new color */
	m_bc = back;

	/* reset the palette to display the new color */
	m_resetPalette();
	setPalette(m_nsp);
}

void KURLLabel::setBackgroundColor(const char* back)
{
	/* set the new color */
	m_bc.setNamedColor(back);

	/* reset the palette to display the new color */
	m_resetPalette();
	setPalette(m_nsp);
}

QSize KURLLabel::sizeHint() const
{
	int x_min, x_max, y_min, y_max;

	/* get the bounding rectangles for text and pixmap */
	QRect text_rect = m_textRect();
	QRect pixmap_rect = m_pixmapRect();

	/* get the outer x coordinates */
	x_min = QMIN(text_rect.topLeft().x(), pixmap_rect.topLeft().x());
	x_max = QMAX(text_rect.topRight().x(), pixmap_rect.topRight().x());

	/* get the outer y coordinates */
	y_min = QMIN(text_rect.topLeft().y(), pixmap_rect.topLeft().y());
	y_max = QMAX(text_rect.bottomLeft().y(), pixmap_rect.bottomLeft().y());

	return QSize((x_max - x_min)+1, (y_max - y_min)+1);
}

void KURLLabel::mouseMoveEvent(QMouseEvent *event)
{
	/* get the boundries of the text and/or pixmap */
	QRect text_rect = m_textRect();
	QRect pixmap_text = m_pixmapRect();
	int tx_min = text_rect.topLeft().x();
	int ty_min = text_rect.topLeft().y();
	int tx_max = text_rect.bottomRight().x();
	int ty_max = text_rect.bottomRight().y();
	int px_min = pixmap_text.topLeft().x();
	int py_min = pixmap_text.topLeft().y();
	int px_max = pixmap_text.bottomRight().x();
	int py_max = pixmap_text.bottomRight().y();

	/* process this event only if we are within the text boundry */
	if (((event->x() > tx_min) && (event->x() < tx_max) &&
	    (event->y() > ty_min) && (event->y() < ty_max)) ||
	   ((event->x() > px_min) && (event->x() < px_max) &&
	    (event->y() > py_min) && (event->y() < py_max))) 
	{
		/**
		 * if we were not within the region before, then this is
		 * a enter event
		 */
		if (m_inRegion == false)
		{
			m_inRegion = true;
			m_enterEvent();
		}
	}
	/* if we were in the region before, then this is a leave event */
	else if (m_inRegion == true)
	{
		m_inRegion = false;
		m_leaveEvent();
	}
}


void KURLLabel::m_enterEvent()
{
	/* emit this signal with our URL*/
	emit(enteredURL(m_url));
	emit(enteredURL());

	/* check if we have an "alt" pixmap */
	if (!m_altPixmap.isNull() && (m_float || m_glow))
	{
		/* display it instead of the regular pixmap */
		m_origPixmap = m_pixmap;
		m_pixmap = m_altPixmap;
	}

	/* if we are using a custom cursor, use it */
	if (m_haveCursor);
		setCursor(m_customCursor);

	/* check if we are in float mode */
	if (m_float)
	{
		/* turn on underlining */
		QFont tmp_font = font();
		tmp_font.setUnderline(true);
		QLabel::setFont(tmp_font);

		/* and "glow" this */
		setPalette(m_sp);
	}
	else
	/* if we are in "glow" mode, turn on the selected palette */
	if (m_glow)
		setPalette(m_sp);
}

void KURLLabel::leaveEvent(QEvent*)
{
	/* let m_leaveEvent handle this if we are in the region */
	if (m_inRegion)
	{
		m_inRegion = false;
		m_leaveEvent();
	}
}

void KURLLabel::m_leaveEvent()
{
	/* emit this signal with our URL*/
	emit(leftURL(m_url));
	emit(leftURL());

	/* check if we have an "alt" pixmap */
	if (!m_altPixmap.isNull() && (m_float || m_glow))
	{
		/* change back to the original */
		m_pixmap = m_origPixmap;
	}

	/* if we are using a custom cursor, set it back */
	if (m_haveCursor)
		setCursor(QCursor());

	/* check if we are in float mode */
	if (m_float)
	{
		/* switch underlining back to original state */
		QFont tmp_font = font();
		tmp_font.setUnderline(m_underline);
		QLabel::setFont(tmp_font);

		/* set palette back to normal*/
		setPalette(m_nsp);
	}
	else
	/* if we are in "glow" mode, turn off the selected palette */
	if (m_glow)
		setPalette(m_nsp);
} 

void KURLLabel::mousePressEvent(QMouseEvent *event)
{
	if (m_inRegion)
	{
		/* set the palette to "selected"*/
		setPalette(m_sp);

		/* select the pixmap only if there is one */
		if (!m_pixmap.isNull())
		{
			/* save the original pixmap */
			m_unselPixmap = m_pixmap;

			/* select the pixmap */
			QBrush b(m_sc, Dense4Pattern);
			QPainter p(&m_pixmap);
			p.fillRect(0, 0, m_pixmap.width(), m_pixmap.height(), b);
		}

		/**
		 * set the timer for 1/2 second.  this allows time
		 * to show that this is selected
		 */
		startTimer(500);

		/**
		 * emit the proper signal based on which button the
		 * user clicked
		 */
		switch (event->button())
		{
			case LeftButton:
				emit(leftClickedURL(m_url));
				emit(leftClickedURL());
				break;
			case RightButton:
				emit(rightClickedURL(m_url));
				emit(rightClickedURL());
				break;
			case MidButton:
				emit(middleClickedURL(m_url));
				emit(middleClickedURL());
				break;
		}
	}
}

void KURLLabel::timerEvent(QTimerEvent *event)
{
	/* reset the the palette to normal */
	setPalette(m_nsp);

	if (!m_unselPixmap.isNull())
	{
		/* redraw the original pixmap */
		QPainter p(&m_pixmap);
		p.drawPixmap(0, 0, m_unselPixmap);
	}

	/* kill the timer */
	killTimer(event->timerId());
}

void KURLLabel::m_resetPalette()
{
	/* reset the palette with any colors that have changed */
	m_nsp.setNormal(
		QColorGroup(
			m_hc,
			m_bc,
			palette().normal().light(),
			palette().normal().dark(),
			palette().normal().mid(),
			m_hc,
			palette().normal().base()
		)
	);
	m_sp.setNormal(
		QColorGroup(
			m_sc,
			m_bc,
			palette().normal().light(),
			palette().normal().dark(),
			palette().normal().mid(),
			m_sc,
			palette().normal().base()
		)
	);
}

QRect KURLLabel::m_textRect() const
{
	int x_min = 0, y_min = 0;
	int pixmap_width = 0, pixmap_height = 0;

	/* get the pixmap info if it exists */
	if (!m_pixmap.isNull())
	{
		pixmap_height = m_pixmap.height();
		pixmap_width = m_pixmap.width();
	}

	/* return 0 if there is no text */
	if (m_text.isEmpty())
		return QRect(0, 0, 0, 0);

	/**
	 * calculate the boundry rect for the text based on the
	 * text metrics and the current alignment
	 */
	QFontMetrics fm(font());
	if (alignment() & AlignHCenter)
	{
		switch (m_textAlign)
		{
			case Bottom:
			case Top:
				if (autoResize())
					x_min = (pixmap_width > fm.width(m_text)) ? 
						         (pixmap_width - fm.width(m_text)) / 2 : 0;
				else
					x_min = (width() - fm.width(m_text)) / 2;
				break;
			case Left:
				if (autoResize())
					x_min = 0;
				else
					x_min = (width() - fm.width(m_text) - pixmap_width) / 2;
				break;
			case Right:
				if (autoResize())
					x_min = pixmap_width;
				else
					x_min = (width() - fm.width(m_text) + pixmap_width) / 2;
				break;
		}
	}

	if (alignment() & AlignVCenter)
	{
		switch (m_textAlign)
		{
			case Bottom:
				if (autoResize())
					y_min = pixmap_height;
				else
					y_min = (height() + pixmap_height - fm.height()) / 2;
				break;
			case Top:
				if (autoResize())
					y_min = 0;
				else
					y_min = ((height() - pixmap_height - fm.height()) / 2) + 3;
				break;
			case Left:
			case Right:
				y_min = (height() - fm.height()) / 2;
				break;
		}
	}

	if (alignment() & AlignLeft)
	{
		switch (m_textAlign)
		{
			case Top:
			case Bottom:
			case Left:
				x_min = 0;
				break;
			case Right:
				x_min = pixmap_width;
				break;
		}
	}	

	if (alignment() & AlignTop)
	{
		switch (m_textAlign)
		{
			case Top:
			case Left:
			case Right:
				y_min = 0;
				break;
			case Bottom:
				y_min = pixmap_height;
				break;
		}
	}

	if (alignment() & AlignRight)
	{
		switch (m_textAlign)
		{
			case Top:
			case Bottom:
			case Right:
				x_min = width() - fm.width(m_text);
				break;
			case Left:
				x_min = width() - pixmap_width - fm.width(m_text);
				break;
		}
	}

	if (alignment() & AlignBottom)
	{
		switch (m_textAlign)
		{
			case Top:
				y_min = height() - pixmap_height - fm.height();
				break;
			case Bottom:
			case Right:
			case Left:
				y_min = height() - fm.height();
				break;
		}
	}

	/* construct the bounding rectangle */
	return QRect(x_min, y_min, fm.width(m_text), fm.height());
}

QRect KURLLabel::m_pixmapRect() const
{
	int x_min = 0, y_min = 0;
	int text_width = 0, text_height = 0;

	/* get the text info if necessary */
	if (!m_text.isEmpty())
	{
		QFontMetrics metrics(font());
		text_height = metrics.height();
		text_width = metrics.width(m_text);
	}

	/* return now if there is no pixmap */
	if (m_pixmap.isNull())
		return QRect(0, 0, 0, 0);

	/**
	 * calculate the boundry rect for the pixmap based on its
	 * size and the current alignment
	 */
	if (alignment() & AlignHCenter)
	{
		switch (m_textAlign)
		{
			case Bottom:
			case Top:
				if (autoResize())
					x_min = m_pixmap.width() > text_width ? 
					            0 : (text_width - m_pixmap.width()) / 2;
				else
					x_min = (width() - m_pixmap.width()) / 2;
				break;
			case Left:
				if (autoResize())
					x_min = text_width;
				else
					x_min = (width() - m_pixmap.width() + text_width) / 2;
				break;
			case Right:
				if (autoResize())
					x_min = 0;
				else
					x_min = (width() - m_pixmap.width() - text_width) / 2;
				break;
		}
	}

	if (alignment() & AlignVCenter)
	{
		switch (m_textAlign)
		{
			case Bottom:
				if (autoResize())
					y_min = 0;
				else
					y_min = (height() - m_pixmap.height() - text_height) / 2;
				break;
			case Top:
				if (autoResize())
					y_min = text_height;
				else
					y_min = (height() - m_pixmap.height() + text_height) / 2;
				break;
			case Left:
			case Right:
				if (autoResize())
					y_min = 0;
				else
					y_min = (height() - m_pixmap.height()) / 2;
				break;
		}
	}

	if (alignment() & AlignLeft)
	{
		switch (m_textAlign)
		{
			case Left:
				x_min = text_width;
				break;
			case Right:
			case Top:
			case Bottom:
				x_min = 0;
		}
	}

	if (alignment() & AlignTop)
	{
		switch (m_textAlign)
		{
			case Top:
				y_min = text_height;
				break;
			case Left:
			case Right:
			case Bottom:
				y_min = 0;
				break;
		}
	}

	if (alignment() & AlignRight)
	{
		switch (m_textAlign)
		{
			case Bottom:
			case Top:
			case Left:
				x_min = width() - m_pixmap.width();
				break;
			case Right:
				x_min = width() - m_pixmap.width() - text_width;
				break;
		}
	}

	if (alignment() & AlignBottom)
	{
		switch (m_textAlign)
		{
			case Top:
			case Left:
			case Right:
				y_min = height() - m_pixmap.height();
				break;
			case Bottom:
				y_min = height() - m_pixmap.height() - text_height;
				break;
		}
	}

	/* construct the bounding rectangle */
	return QRect(x_min, y_min, m_pixmap.width(), m_pixmap.height());
}

void KURLLabel::drawContents(QPainter* p)
{
	/* get the bounding rectangles for both the text and pixmap */
	QRect text_rect = m_textRect();
	QRect pixmap_rect = m_pixmapRect();

	/* draw the text only if it is not null */
	if (!m_text.isEmpty())
		p->drawText(text_rect.bottomLeft().x(), text_rect.bottomLeft().y()-3,
		            m_text);

	/* draw the pixmap only if it is not null */
	if (!m_pixmap.isNull())
	{
		p->drawPixmap(pixmap_rect.topLeft().x(), pixmap_rect.topLeft().y(),
		              m_pixmap);
	}
}

void KURLLabel::setTransparentMode(bool state)
{
	m_transparent = state;
	setBackgroundMode(state ? NoBackground : PaletteBackground); 
}

void KURLLabel::paintEvent(QPaintEvent*)
{ // Mirko Sucker, 1998/11/16:
	QPen pen;
	QPainter paint(this);
	// -----
	if(m_transparent && parentWidget()!=0)
	{
		QPixmap bg(width(), height());
		// -----
		bg.fill(parentWidget(), mapToParent(QPoint(0, 0)));
		paint.drawPixmap(0, 0, bg);
	}
	drawFrame(&paint);
	drawContents(&paint);
	paint.end();
}
