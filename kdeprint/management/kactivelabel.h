#ifndef KACTIVELABEL_H
#define KACTIVELABEL_H

#include <qframe.h>

class KActiveLabel : public QFrame
{
	friend class KActiveToolTip;
	Q_OBJECT
public:
	KActiveLabel(QWidget *parent = 0, const char *name = 0);
	KActiveLabel(const QString& txt, QWidget *parent = 0, const char *name = 0);
	~KActiveLabel();

	void setAutoRun(bool on = true);
	bool autoRun() const;
	QString text() const;
	void setAlignment(int);
	int alignment() const;

public slots:
	void setText(const QString&);

signals:
	void linkClicked(const QString&);

protected:
	void mouseMoveEvent(QMouseEvent*);
	void mousePressEvent(QMouseEvent*);
	void resizeEvent(QResizeEvent*);
	void fontChange(const QFont&);
	void doFormat(bool create = false);
	void drawContents(QPainter*);
	QPoint translatePoint(const QPoint&);
	void init();
	void maybeTip(const QPoint&);

private:
	class KActiveLabelPrivate;
	class KActiveToolTip;
	KActiveLabelPrivate	*d;
};

#endif
