#ifndef	POSITION_H
#define	POSITION_H

#include <qwidget.h>
#include <qstring.h>

class ImagePosition : public QWidget
{
public:
	enum PositionType {TopLeft,
			   Top,
			   TopRight,
			   Left,
			   Center,
			   Right,
			   BottomLeft,
			   Bottom,
			   BottomRight};

	ImagePosition(QWidget *parent = 0, const char *name = 0);
	~ImagePosition();

	void setPosition(const char *type = "center");
	void setPosition(PositionType type = Center);
	void setPosition(int horiz = 1, int vert = 1);
	PositionType position() const { return (PositionType)position_; }
	QString positionString() const;

	QSize sizeHint() const { return QSize(65,80); }

protected:
	void paintEvent(QPaintEvent *);

private:
	int	position_;
};

#endif
