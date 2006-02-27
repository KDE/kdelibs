#include "kdxschanges.h"

#include <khtml_part.h>
#include <khtmlview.h>
#include <klocale.h>

#include <qlayout.h>
#include <qapplication.h>

KDXSChanges::KDXSChanges(QWidget *parent)
: KDialogBase(parent, "changes", true, i18n("Changelog"),
	KDialogBase::Ok, KDialogBase::Ok, true)
{
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

	vbox = new QVBoxLayout(root, 5);
	vbox->add(m_part->view());
}

void KDXSChanges::addChangelog(QString version, QString log)
{
	m_part->write("<table class='itemBox'>");
	m_part->write("<tr>");
	m_part->write("<td class='contentsColumn'>");
	m_part->write("<table class='contentsHeader' cellspacing='2' cellpadding='0'><tr>");
	m_part->write("<td>" + i18n("Version %1").arg(version) + "</td>");
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
