#ifndef KDXS_RATING_H
#define KDXS_RATING_H

#include <kdialogbase.h>

class QSlider;

class KDXSRating : public KDialogBase
{
Q_OBJECT
public:
	KDXSRating(QWidget *parent);
	int rating();
private:
	QSlider *m_slider;
};

#endif
