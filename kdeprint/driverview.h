#ifndef DRIVERVIEW_H
#define DRIVERVIEW_H

#include <qwidget.h>
#include <klistview.h>
#include <qmap.h>

class DrOptionView;
class DrMain;

class DrListView : public KListView
{
public:
	DrListView(QWidget *parent = 0, const char *name = 0);
};

class DriverView : public QWidget
{
	Q_OBJECT
public:
	DriverView(QWidget *parent = 0, const char *name = 0);
	~DriverView();

	void setDriver(DrMain*);
	void setOptions(const QMap<QString,QString>& opts);
	void getOptions(QMap<QString,QString>& opts, bool incldef = false);
	void setAllowFixed(bool on);
	bool hasConflict() const 	{ return (m_conflict != 0); }

protected slots:
	void slotChanged();

private:
	DrListView	*m_view;
	DrOptionView	*m_optview;
	DrMain		*m_driver;
	int 		m_conflict;
};

#endif
