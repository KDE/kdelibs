#ifndef KMDRIVERDIALOG_H
#define KMDRIVERDIALOG_H

#include <kdialogbase.h>

class DriverView;
class DrMain;

class KMDriverDialog : public KDialogBase
{
public:
	KMDriverDialog(QWidget *parent = 0, const char *name = 0);
	~KMDriverDialog();

	void setDriver(DrMain*);

private:
	DriverView	*m_view;
};

#endif
