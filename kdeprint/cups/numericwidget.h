#ifndef NUMERICWIDGET_H
#define	NUMERICWIDGET_H

#include <qwidget.h>

class QLineEdit;
class QSlider;
class QLabel;

class NumericWidget : public QWidget
{
	Q_OBJECT
public:
	NumericWidget(QWidget *parent = 0, const char *name = 0);
	~NumericWidget();

	void setLimits(double minval, double maxval);
	void setFactor(int fact);

signals:
	void valueChanged(double value);

public slots:
	void setValue(double value);

protected slots:
	void sliderChanged(int value);
	void editChanged(const QString& txt);

private:
	QLineEdit	*edit_;
	QSlider		*slider_;
	int		min_, max_;
	int		value_;
	int		factor_;
	QLabel		*minlabel_, *maxlabel_;
	bool		block_;
};

#endif
