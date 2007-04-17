/*
    This file is part of KNewStuff2.
    Copyright (c) 2006, 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kdxschanges.h"

#include <khtml_part.h>
#include <khtmlview.h>
#include <klocale.h>

#include <QtGui/QLayout>
#include <QtGui/QApplication>

KDXSChanges::KDXSChanges(QWidget *parent)
: KDialog(parent)
{
	setCaption(i18n("Changelog"));
	setButtons(KDialog::Close);

	QVBoxLayout *vbox;

	QWidget *root = new QWidget(this);
	setMainWidget(root);

	m_part = new KHTMLPart(root);

	m_part->setStandardFont(QApplication::font().family());
	m_part->setZoomFactor(70);

	prepare();

	m_part->begin();
	m_part->write("<html>"),
	m_part->write("<body>");

	vbox = new QVBoxLayout(root);
	vbox->addWidget(m_part->view());
}

void KDXSChanges::addChangelog(QString version, QString log)
{
	m_part->write("<table class='itemBox'>");
	m_part->write("<tr>");
	m_part->write("<td class='contentsColumn'>");
	m_part->write("<table class='contentsHeader' cellspacing='2' cellpadding='0'><tr>");
	m_part->write("<td>" + i18n("Version %1", version) + "</td>");
	m_part->write("</tr></table>");
	m_part->write("<div class='contentsBody'>");
	m_part->write(log);
	m_part->write("</div>");
	m_part->write("</td>");
	m_part->write("</tr>");
	m_part->write("</table>");
}

void KDXSChanges::finish()
{
	m_part->write("</body>");
	m_part->write("</html>");
	m_part->end();
}

void KDXSChanges::prepare()
{
	QString style;

	style += "body { background-color: white; color: black; padding: 0; margin: 0; }";
	style += "table, td, th { padding: 0; margin: 0; text-align: left; }";

	style += ".itemBox { background-color: white; color: black; width: 100%;  border-bottom: 1px solid gray; margin: 0px 0px; }";
	style += ".itemBox:hover { background-color: #f8f8f8; color: #000000; }";

	style += ".contentsColumn { vertical-align: top; }";
	style += ".contentsHeader { width: 100%; font-size: 120%; font-weight: bold; border-bottom: 1px solid #c8c8c8; }";
	style += ".contentsBody {}";
	style += ".contentsFooter {}";

	m_part->setUserStyleSheet(style);
}

#include "kdxschanges.moc"
