#ifndef RICHPAGE_H
#define RICHPAGE_H

#include <kprintdialogpage.h>

class QSpinBox;
class QComboBox;

class RichPage : public KPrintDialogPage
{
public:
	RichPage(QWidget *parent = 0, const char *name = 0);
	~RichPage();

	void setOptions(const QMap<QString,QString>& opts);
	void getOptions(QMap<QString,QString>& opts, bool incldef = false);

private:
	QSpinBox	*margin_;
	QComboBox	*fontname_;
	QSpinBox	*fontsize_;
};

#endif
