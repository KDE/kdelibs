#include "kpdqoptiondlg.h"
#include "pdqinfos.h"

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qframe.h>
#include <klocale.h>
#include <kfiledialog.h>

KPdqOptionDlg::KPdqOptionDlg(QWidget *parent, const char *name)
: QDialog(parent,name,true)
{
	rcfile_ = new QLineEdit(this);

	QPushButton	*ok_ = new QPushButton(i18n("OK"), this);
	QPushButton	*cancel_ = new QPushButton(i18n("Cancel"), this);
	QPushButton	*browse_ = new QPushButton(i18n("Browse"), this);
	ok_->setDefault(true);

	QLabel	*text_ = new QLabel(i18n("Location of global configuration file:"), this);
	QFrame	*line_ = new QFrame(this);
	line_->setFrameStyle(QFrame::HLine|QFrame::Sunken);
	line_->setLineWidth(1);
	line_->setFixedHeight(5);

	QVBoxLayout	*main_ = new QVBoxLayout(this, 10, 10);
	QHBoxLayout	*btn_ = new QHBoxLayout(0, 0, 10);
	QHBoxLayout	*edit_ = new QHBoxLayout(0, 0, 10);

	main_->addWidget(text_);
	main_->addLayout(edit_);
	main_->addWidget(line_);
	main_->addLayout(btn_);
	edit_->addWidget(rcfile_);
	edit_->addWidget(browse_);
	btn_->addStretch(1);
	btn_->addWidget(ok_);
	btn_->addWidget(cancel_);

	connect(ok_,SIGNAL(clicked()),SLOT(accept()));
	connect(cancel_,SIGNAL(clicked()),SLOT(reject()));
	connect(browse_,SIGNAL(clicked()),SLOT(slotBrowse()));

	setCaption(i18n("PDQ configuration"));
	resize(350,50);
}

KPdqOptionDlg::~KPdqOptionDlg()
{
}

void KPdqOptionDlg::slotBrowse()
{
	QString	filename = KFileDialog::getOpenFileName(rcfile_->text(),i18n("printrc|Configuration file\n*|All files"),this,i18n("Select new configuration file"));
	if (!filename.isEmpty())
		rcfile_->setText(filename);
}

bool KPdqOptionDlg::configure(QWidget *parent)
{
	QString	filename = PdqInfos::self()->pdqGlobalRcFile();
	KPdqOptionDlg	dlg(parent);
	dlg.rcfile_->setText(filename);
	bool	flag = dlg.exec();
	if (flag)
		PdqInfos::self()->setPdqGlobalRcFile(dlg.rcfile_->text());
	return flag;
}
#include "kpdqoptiondlg.moc"
