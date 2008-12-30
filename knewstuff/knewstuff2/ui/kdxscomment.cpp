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

#include "kdxscomment.h"

#include <QtGui/QLayout>

#include <klocale.h>
#include <ktextedit.h>

KDXSComment::KDXSComment(QWidget *parent)
        : KDialog(parent)
{
    setCaption(i18n("Leave a comment"));
    setButtons(KDialog::Ok | KDialog::Cancel);

    QVBoxLayout *vbox;

    QWidget *root = new QWidget(this);
    setMainWidget(root);

    m_textbox = new KTextEdit(root);

    vbox = new QVBoxLayout(root);
    vbox->addWidget(m_textbox);
}

QString KDXSComment::comment()
{
    return m_textbox->toPlainText();
}

#include "kdxscomment.moc"
