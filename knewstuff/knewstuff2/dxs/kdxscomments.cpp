#include "kdxscomments.h"

#include <khtml_part.h>
#include <khtmlview.h>
#include <klocale.h>

#include <qlayout.h>
#include <qapplication.h>

KDXSComments::KDXSComments(QWidget *parent)
: KDialogBase(parent, "comments", true, i18n("User comments"),
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

void KDXSComments::addComment(QString username, QString comment)
{
	m_part->write("<a href='mailto:spillner@kde.org'>josef the kns2 hacker</a>");
	m_part->write("<table class='itemBox'>");
	m_part->write("<tr>");
	m_part->write("<td class='contentsColumn'>");
	m_part->write("<table class='contentsHeader' cellspacing='2' cellpadding='0'><tr>");
	m_part->write("<td>Comment!</td>");
	m_part->write("</tr></table>");
	m_part->write("<div class='contentsBody'>");
	m_part->write(comment);
	m_part->write("</div>");
	m_part->write("<div class='contentsFooter'>");
	m_part->write("<em>" + username + "</em>");
	m_part->write("</div>");
	m_part->write("</td>");
	m_part->write("</tr>");
	m_part->write("</table>");
}

void KDXSComments::finish()
{
	m_part->write("</body>");
	m_part->write("</html>");
	m_part->end();
}

void KDXSComments::prepare()
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

/*
void urlSelected(const QString & link, int, int, const QString &, KParts::URLArgs)
{
KURL url(link);
QString urlProtocol = url.protocol();
QString urlPath = url.path();
if(urlProtocol == "mailto")
{
kapp->invokeMailer( url );
}
}
*/

#include "kdxscomments.moc"
