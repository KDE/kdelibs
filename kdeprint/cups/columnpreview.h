#ifndef COLUMNPREVIEW_H
#define COLUMNPREVIEW_H

#include <qwidget.h>

class ColumnPreview : public QWidget
{
	Q_OBJECT
public:
	ColumnPreview(QWidget *parent = 0, const char *name = 0);
	~ColumnPreview();
public slots:
	void setColumns(int c);
protected:
	void paintEvent(QPaintEvent *);
private:
	int	columns_;
};

#endif
