#ifndef	MARGINPREVIEW_H
#define	MARGINPREVIEW_H

#include <qwidget.h>

void draw3DPage(QPainter *p, QRect r);

class MarginPreview : public QWidget
{
public:
	MarginPreview(QWidget *parent = 0, const char *name = 0);
	~MarginPreview();
	// note : unit -> points
	void setPageSize(int w, int h);
	void setMargins(int t, int b, int l, int r);
	void setNoPreview(bool on);
protected:
	void paintEvent(QPaintEvent *);
	void resizeEvent(QResizeEvent *);
private:
	int	width_, height_;
	int	top_, bottom_, left_, right_;
	QRect	box_;
	float	zoom_;
	bool nopreview_;
};

#endif
