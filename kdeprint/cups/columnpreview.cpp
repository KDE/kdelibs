#include "columnpreview.h"

#include <qpainter.h>

ColumnPreview::ColumnPreview(QWidget *parent, const char *name)
	: QWidget(parent,name)
{
	columns_ = 1;
}

ColumnPreview::~ColumnPreview()
{
}

void ColumnPreview::setColumns(int c)
{
	if (c > 0)
	{
		columns_ = c;
		update();
	}
}

void ColumnPreview::paintEvent(QPaintEvent *)
{
	QPainter	p(this);

	int	marg(5);
	int	space(3);
	int	len(0);
	len = (width()-2*marg-(columns_-1)*marg)/columns_;
	if (len <= 0)
		len = 1;
	int	x(marg), y(marg);
	for (int i=0;i<columns_ && x+len<width();i++,x+=(len+marg))
	{
		y = marg;
		while (y<height()-marg)
		{
			p.drawLine(x,y,x+len,y);
			y += space;
		}
	}
}
