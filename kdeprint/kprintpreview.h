#ifndef KPRINTPREVIEW_H
#define KPRINTPREVIEW_H

#include <kparts/part.h>
#include <kparts/mainwindow.h>
#include <qstring.h>

class KPrintPreview : public KParts::MainWindow
{
	Q_OBJECT;
public:
	KPrintPreview(QWidget *parent = 0);
	~KPrintPreview();

	void openFile(const QString& file);
	bool status() const;
	void exec(const QString& file = QString::null);
	bool isValid() const;

	static bool preview(const QString& file);

protected slots:
	void accept();
	void reject();

protected:
	void done(bool st);
	void closeEvent(QCloseEvent *e);

private:
	KParts::ReadOnlyPart	*gvpart_;
	bool			status_;
};

inline bool KPrintPreview::status() const
{ return status_; }

#endif
