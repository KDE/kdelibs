#include "kdxstranslation.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qcombobox.h>

#include <kurlrequester.h>
#include <klocale.h>

KDXSTranslation::KDXSTranslation(QWidget *parent)
: KDialog(parent)
{
	setCaption(i18n("Translate this entry"));
	setButtons(KDialog::Ok | KDialog::Cancel);

	QWidget *root = new QWidget(this);
	setMainWidget(root);

	m_name = new QLineEdit(root);
	m_description = new QTextEdit(root);
	m_payload = new KUrlRequester(root);

	QLineEdit *oname = new QLineEdit(root);
	QTextEdit *odescription = new QTextEdit(root);

	QLabel *lname = new QLabel(i18n("Name"), root);
	QLabel *ldescription = new QLabel(i18n("Description"), root);
	QLabel *lpayload = new QLabel(i18n("Payload"), root);

	QComboBox *languagebox = new QComboBox(root);
	languagebox->addItem("English");
	languagebox->addItem("German");

	oname->setEnabled(false);
	odescription->setEnabled(false);

	QVBoxLayout *vbox = new QVBoxLayout(root);

	QHBoxLayout *hbox = new QHBoxLayout();
	hbox->addStretch(1);
	hbox->addWidget(languagebox);

	QGridLayout *grid = new QGridLayout();
	grid->setSpacing(spacingHint());
	grid->addWidget(lname, 0, 0);
	grid->addWidget(oname, 0, 1);
	grid->addWidget(m_name, 0, 2);
	grid->addWidget(ldescription, 1, 0);
	grid->addWidget(odescription, 1, 1);
	grid->addWidget(m_description, 1, 2);
	grid->addWidget(lpayload, 2, 0);
	grid->addWidget(m_payload, 2, 2);

	vbox->addLayout(hbox);
	vbox->addLayout(grid);
}

#include "kdxstranslation.moc"
