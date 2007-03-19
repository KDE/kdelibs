#include "kdxscomment.h"

#include <qlayout.h>
#include <qtextedit.h>

#include <klocale.h>

KDXSComment::KDXSComment(QWidget *parent)
: KDialog(parent)
{
	setCaption(i18n("Leave a comment"));
	setButtons(KDialog::Ok | KDialog::Cancel);

	QVBoxLayout *vbox;

	QWidget *root = new QWidget(this);
	setMainWidget(root);

	m_textbox = new QTextEdit(root);

	vbox = new QVBoxLayout(root);
	vbox->addWidget(m_textbox);
}

QString KDXSComment::comment()
{
	return m_textbox->toPlainText();
}

#include "kdxscomment.moc"
