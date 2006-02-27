#ifndef KDXS_RATING_H
#define KDXS_RATING_H

#include <kdialogbase.h>

class QSlider;
class QLabel;
class QStarFrame;

class KDXSRating : public KDialogBase
{
Q_OBJECT
public:
	KDXSRating(QWidget *parent);
	int rating();
public slots:
	void slotRating(int rating);
private:
	QLabel *m_rating;
	QStarFrame *m_starrating;
	QSlider *m_slider;
};

#endif
