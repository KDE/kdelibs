#include "kactivelabel.h"

#include <qsimplerichtext.h>
#include <qtooltip.h>
#include <kcursor.h>
#include <krun.h>
#include <kurl.h>

class KActiveLabel::KActiveToolTip : public QToolTip
{
	friend class KActiveLabel;
public:
	KActiveToolTip(QWidget *parent) : QToolTip(parent) {}

protected:
	void maybeTip(const QPoint& p)
	{
		KActiveLabel	*l = static_cast<KActiveLabel*>(parentWidget());
		if (l)
			l->maybeTip(p);
	}
};

class KActiveLabel::KActiveLabelPrivate
{
public:
	KActiveLabelPrivate()
	{
		richText = 0;
		autoRun = true;
		align = Qt::AlignLeft|Qt::AlignVCenter;
		toolTip = 0;
	}
	~KActiveLabelPrivate()
	{
		delete richText;
		delete toolTip;
	}

	QSimpleRichText	*richText;
	bool		autoRun;
	QString		text;
	int	align;
	KActiveToolTip	*toolTip;
};

KActiveLabel::KActiveLabel(QWidget *parent, const char *name)
: QFrame(parent, name)
{
	init();
}

KActiveLabel::KActiveLabel(const QString& txt, QWidget *parent, const char *name)
: QFrame(parent, name)
{
	init();
	setText(txt);
}

void KActiveLabel::init()
{
	d = new KActiveLabelPrivate;
	d->toolTip = new KActiveToolTip(this);
	setMouseTracking(true);
}

KActiveLabel::~KActiveLabel()
{
	delete d;
}

void KActiveLabel::setAutoRun(bool on)
{
	d->autoRun = on;
}

bool KActiveLabel::autoRun() const
{
	return d->autoRun;
}

void KActiveLabel::setText(const QString& txt)
{
	d->text = txt;
	doFormat(true);
	update();
}

QString KActiveLabel::text() const
{
	return d->text;
}

void KActiveLabel::setAlignment(int align)
{
	d->align = align;
	update();
}

int KActiveLabel::alignment() const
{
	return d->align;
}

void KActiveLabel::resizeEvent(QResizeEvent *e)
{
	QFrame::resizeEvent(e);
	doFormat(false);
}

void KActiveLabel::mouseMoveEvent(QMouseEvent *e)
{
	if (d->richText)
	{
		QPoint	p = translatePoint(e->pos());
		QString	anchor = d->richText->anchorAt(p);
		if (!anchor.isEmpty())
			setCursor(KCursor::handCursor());
		else
		{
			setCursor(Qt::ArrowCursor);
			QToolTip::remove(this, contentsRect());
		}
	}
	QFrame::mouseMoveEvent(e);
}

void KActiveLabel::mousePressEvent(QMouseEvent *e)
{
	if (d->richText)
	{
		QPoint	p = translatePoint(e->pos());
		QString	anchor = d->richText->anchorAt(p);
		if (!anchor.isEmpty() && e->button() == Qt::LeftButton)
		{
			if (d->autoRun)
			{
				KOpenWithHandler	handler;
				new KRun(KURL(anchor));
			}
			emit linkClicked(anchor);
		}
	}
	QFrame::mousePressEvent(e);
}

void KActiveLabel::doFormat(bool create)
{
	if (!d->richText || create)
	{
		delete d->richText;
		d->richText = new QSimpleRichText(text(), font());
		Q_ASSERT(d->richText);
	}
	d->richText->setWidth(contentsRect().width());
}

void KActiveLabel::fontChange(const QFont& f)
{
	QFrame::fontChange(f);
	if (d->richText)
		d->richText->setDefaultFont(f);
	doFormat(false);
	update();
}

void KActiveLabel::drawContents(QPainter *p)
{
	if (d->richText)
	{
		QRect	r = contentsRect();
		int	yoff(0), docH(d->richText->height());
		if (d->align & Qt::AlignVCenter)
			yoff = (r.height()-docH)/2;
		else if (d->align & Qt::AlignBottom)
			yoff = (r.height()-docH);
		d->richText->draw(p, r.left(), r.top()+yoff, r, colorGroup());
	}
}

QPoint KActiveLabel::translatePoint(const QPoint& p)
{
	QRect	r = contentsRect();
	QPoint	pt = (p-r.topLeft());
	if (d->richText)
	{
		int	docH = d->richText->height();
		if (d->align & Qt::AlignVCenter)
			pt.setY(pt.y() - (r.height()-docH)/2);
		else if (d->align & Qt::AlignBottom)
			pt.setY(pt.y() - (r.height()-docH));
	}
	return pt;
}

void KActiveLabel::maybeTip(const QPoint& pos)
{
	if (d->richText)
	{
		QPoint	p = translatePoint(pos);
		QString	anchor = d->richText->anchorAt(p);
		if (!anchor.isEmpty())
			d->toolTip->tip(contentsRect(), anchor);
	}
}

#include "kactivelabel.moc"
