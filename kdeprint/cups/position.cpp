#include "position.h"

#include <qpainter.h>

void draw3DPage(QPainter *p, QRect r);

ImagePosition::ImagePosition(QWidget *parent, const char *name)
	: QWidget(parent,name)
{
	position_ = Center;
}

ImagePosition::~ImagePosition()
{
}

void ImagePosition::setPosition(const char *type)
{
	int	pos(Center);
	if (strcmp(type,"top-left") == 0) pos = TopLeft;
	else if (strcmp(type,"top") == 0) pos = Top;
	else if (strcmp(type,"top-right") == 0) pos = TopRight;
	else if (strcmp(type,"left") == 0) pos = Left;
	else if (strcmp(type,"center") == 0) pos = Center;
	else if (strcmp(type,"right") == 0) pos = Right;
	else if (strcmp(type,"bottom-left") == 0) pos = BottomLeft;
	else if (strcmp(type,"bottom") == 0) pos = Bottom;
	else if (strcmp(type,"bottom-right") == 0) pos = BottomRight;
	setPosition((PositionType)pos);
}

void ImagePosition::setPosition(PositionType type)
{
	if (position_ != type) {
		position_ = type;
		update();
	}
}

void ImagePosition::setPosition(int horiz, int vert)
{
	int	type = vert*3+horiz;
	setPosition((PositionType)type);
}

QString ImagePosition::positionString() const
{
	switch (position_) {
	   case TopLeft: return "top-left";
	   case Top: return "top";
	   case TopRight: return "top-right";
	   case Left: return "left";
	   case Center: return "center";
	   case Right: return "right";
	   case BottomLeft: return "bottom-left";
	   case Bottom: return "bottom";
	   case BottomRight: return "bottom-right";
	}
	return "center";
}

void ImagePosition::paintEvent(QPaintEvent*)
{
	int	horiz(0), vert(0), x(0), y(0);
	int	margin = 5;
	QRect	page(0,0,width(),height()), img(0,0,page.width()/2,page.height()/2);

	// compute img position
	horiz = position_%3;
	vert = position_/3;
	switch (horiz) {
	   case 0: x = page.left()+margin; break;
	   case 1: x = (page.left()+page.right()-img.width())/2; break;
	   case 2: x = page.right()-margin-img.width(); break;
	}
	switch (vert) {
	   case 0: y = page.top()+margin; break;
	   case 1: y = (page.top()+page.bottom()-img.height())/2; break;
	   case 2: y = page.bottom()-margin-img.height(); break;
	}
	img.moveTopLeft(QPoint(x,y));

	// draw page
	QPainter	p(this);
	draw3DPage(&p,page);

	// draw img
	p.setPen(darkRed);
	p.drawRect(img);
	p.drawLine(img.topLeft(),img.bottomRight());
	p.drawLine(img.topRight(),img.bottomLeft());

	p.end();
}
