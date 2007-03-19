#ifndef QSTAR_FRAME_H
#define QSTAR_FRAME_H

#include <qframe.h>

class QStarFrame: public QFrame
{
Q_OBJECT
public:
	QStarFrame(QWidget *parent);
public slots:
	void slotRating(int rating);
protected:
	void paintEvent(QPaintEvent *e);
private:
	void drawstars();

	int m_rating;
};

#endif
