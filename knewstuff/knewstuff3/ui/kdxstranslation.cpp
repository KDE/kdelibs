/*
    This file is part of KNewStuff2.
    Copyright (c) 2006, 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kdxstranslation.h"

#include <QtGui/QLabel>
#include <QtGui/QLayout>

#include <kcombobox.h>
#include <klineedit.h>
#include <klocale.h>
#include <ktextedit.h>
#include <kurlrequester.h>

KDXSTranslation::KDXSTranslation(QWidget *parent)
        : KDialog(parent)
{
    setCaption(i18n("Translate this entry"));
    setButtons(KDialog::Ok | KDialog::Cancel);

    QWidget *root = new QWidget(this);
    setMainWidget(root);

    m_name = new KLineEdit(root);
    m_description = new KTextEdit(root);
    m_payload = new KUrlRequester(root);

    KLineEdit *oname = new KLineEdit(root);
    KTextEdit *odescription = new KTextEdit(root);

    QLabel *lname = new QLabel(i18n("Name"), root);
    QLabel *ldescription = new QLabel(i18n("Description"), root);
    QLabel *lpayload = new QLabel(i18n("Payload"), root);

    KComboBox *languagebox = new KComboBox(root);
    languagebox->addItem("English");
    languagebox->addItem("German");

    oname->setEnabled(false);
    odescription->setEnabled(false);

    QVBoxLayout *vbox = new QVBoxLayout(root);

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->addStretch(1);
    hbox->addWidget(languagebox);

    QGridLayout *grid = new QGridLayout();
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
