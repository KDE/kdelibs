/**
 * klinkdialog
 *
 * Copyright 2008  Stephen Kelly <steveire@gmailcom>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#include "klinkdialog_p.h"

#include <klocalizedstring.h>

#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>
#include <QPushButton>
#include <QVBoxLayout>

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KLinkDialogPrivate
{
public:
    QLabel *textLabel;
    QLineEdit *textLineEdit;
    QLabel *linkUrlLabel;
    QLineEdit *linkUrlLineEdit;
    QDialogButtonBox *buttonBox;
};
//@endcond


KLinkDialog::KLinkDialog(QWidget *parent)
        : QDialog(parent), d(new KLinkDialogPrivate)
{
    setWindowTitle(i18n("Manage Link"));
    setModal(true);

    QVBoxLayout *layout = new QVBoxLayout;

    QGridLayout *grid = new QGridLayout;

    d->textLabel = new QLabel(i18n("Link Text:"), this);
    d->textLineEdit = new QLineEdit(this);
    d->textLineEdit->setClearButtonEnabled(true);
    d->linkUrlLabel = new QLabel(i18n("Link URL:"), this);
    d->linkUrlLineEdit = new QLineEdit(this);
    d->linkUrlLineEdit->setClearButtonEnabled(true);

    grid->addWidget(d->textLabel, 0, 0);
    grid->addWidget(d->textLineEdit, 0, 1);
    grid->addWidget(d->linkUrlLabel, 1, 0);
    grid->addWidget(d->linkUrlLineEdit, 1, 1);

    layout->addLayout(grid);

    d->buttonBox = new QDialogButtonBox(this);
    d->buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(d->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(d->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(d->buttonBox);

    d->textLineEdit->setFocus();
    d->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    connect(d->textLineEdit, SIGNAL(textChanged(QString)), this, SLOT(slotTextChanged(QString)));
}

KLinkDialog::~KLinkDialog()
{
    delete d;
}

void KLinkDialog::slotTextChanged(const QString &text)
{
    d->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!text.isEmpty());
}

void KLinkDialog::setLinkText(const QString &linkText)
{
    d->textLineEdit->setText(linkText);
    if (!linkText.trimmed().isEmpty())
        d->linkUrlLineEdit->setFocus();
}

void KLinkDialog::setLinkUrl(const QString &linkUrl)
{
    d->linkUrlLineEdit->setText(linkUrl);
}


QString KLinkDialog::linkText() const
{
    return d->textLineEdit->text().trimmed();
}

QString KLinkDialog::linkUrl() const
{
    return d->linkUrlLineEdit->text();
}
