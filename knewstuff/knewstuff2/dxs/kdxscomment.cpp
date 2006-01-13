#include "kdxscomment.h"

#include <qlayout.h>
#include <qmultilineedit.h>

#include <klocale.h>

KDXSComment::KDXSComment(QWidget *parent)
: KDialogBase(parent, "comment", true, i18n("Leave a comment"),
	KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, true)
{
	QVBoxLayout *vbox;

	QWidget *root = new QWidget(this);
	setMainWidget(root);

	m_textbox = new QMultiLineEdit(root);

	vbox = new QVBoxLayout(root, 5);
	vbox->add(m_textbox);
}

QString KDXSComment::comment()
{
	return m_textbox->text();
}

