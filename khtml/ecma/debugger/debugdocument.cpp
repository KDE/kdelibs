#include "debugdocument.h"
#include "debugwindow.h"

#include <QHash>
#include <QVector>
#include <QStringList>

#include "kjs_binding.h"
#include "khtml_part.h"

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/configinterface.h>
#include <ktexteditor/markinterface.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kmessagebox.h>

using namespace KJS;
using namespace KJSDebugger;

DebugDocument::DebugDocument(const QString& url, const QString& iuKey)
{
    m_url   = url;
    m_iuKey = iuKey;

    KUrl kurl(url);
    m_name = kurl.fileName();

    // Might have to fall back in case of query-like things;
    // ad scripts tend to do that
    while (m_name.contains("=") || m_name.contains("&")) {
        kurl = kurl.upUrl();
        m_name = kurl.fileName();
    }

    if (m_name.isEmpty())
        m_name = kurl.host();

    if (m_name.isEmpty())
        m_name = "????"; //Probably better than un-i18n'd 'undefined'...

    m_kteDoc  = 0;
    m_kteView = 0;
    m_rebuilding = false;
}

DebugDocument::~DebugDocument()
{
    emit documentDestroyed(this);

    // View has an another parent for UI purposes, so we have to clean it up
    delete m_kteView;
}

QString DebugDocument::name() const
{
    return m_name;
}

QString DebugDocument::url() const
{
    return m_url;
}

QString DebugDocument::iuKey() const
{
    return m_iuKey;
}

DebugDocument::FragmentInfo DebugDocument::deleteFragment(int sourceId)
{
    m_codeFragments.remove(sourceId);
    if (m_codeFragments.isEmpty())
        return LastFragment;
    else
        return NonLastFragment;
}

SourceFragment DebugDocument::fragment(int sourceId)
{
    if (m_codeFragments.contains(sourceId))
        return m_codeFragments[sourceId];
    else
        return SourceFragment();
}

void DebugDocument::addCodeFragment(int sourceId, int baseLine, const QString &source)
{
    SourceFragment code;
    code.sourceId = sourceId;
    code.baseLine = baseLine;
    code.source = source;

    m_codeFragments[sourceId] = code;

    if (m_kteDoc) // Update docu if needed
        buildViewerDocument();
}

void DebugDocument::setBreakpoint(int lineNumber)
{
    if (m_rebuilding) return;

    breakpoints().append(lineNumber);
}

void DebugDocument::removeBreakpoint(int lineNumber)
{
    if (m_rebuilding) return;

    QVector<int>& br = breakpoints();
    int idx = breakpoints().indexOf(lineNumber);
    if (idx != -1)
    {
        br.remove(idx);
        if (br.isEmpty() && !m_url.isEmpty())
        {
            // We just removed the last breakpoint per URL,
            // so we can kill the entire list
            s_perUrlBreakPoints->remove(url());
        }
    }
}

bool DebugDocument::hasBreakpoint(int lineNumber)
{
    return breakpoints().contains(lineNumber);
}

QHash<QString, QVector<int> >* DebugDocument::s_perUrlBreakPoints = 0;

QVector<int>& DebugDocument::breakpoints()
{
    if (m_url.isEmpty())
        return m_localBreakpoints;
    else
    {
        if (!s_perUrlBreakPoints)
            s_perUrlBreakPoints = new QHash<QString, QVector<int> >;

        return (*s_perUrlBreakPoints)[m_url];
    }
}

KTextEditor::Document* DebugDocument::viewerDocument()
{
    if (!m_kteDoc)
        buildViewerDocument();
    return m_kteDoc;
}

// A couple helpers to transparently grow an array
static void ensureIndexSafe(QStringList& lines, int line)
{
    while (line >= lines.size())
        lines.append(QString());
}

static void setLine(QStringList& lines, int line, const QString& lineText)
{
    ensureIndexSafe(lines, line);
    lines[line] = lineText;
}

static QString getLine(QStringList& lines, int line)
{
    ensureIndexSafe(lines, line);
    return lines[line];
}

KTextEditor::Editor* DebugDocument::s_kate = 0;

KTextEditor::Editor* DebugDocument::kate()
{
    if (!s_kate)
        s_kate = KTextEditor::editor("katepart");

    if (!s_kate)
    {
        KMessageBox::error(DebugWindow::window(), i18n("A KDE text-editor component could not be found;\n"
                                      "please check your KDE installation."));
        kapp->exit(1);
    }

    return s_kate;
}


void DebugDocument::buildViewerDocument()
{
    // First, we collect the lines ourselves, not to bother katepart
    // so much.
    QStringList lines;
    foreach (SourceFragment fragment, m_codeFragments)
    {
        // Note: the KTextEditor interface counts the lines/columns from 0,
        // but the UI shows them from 1,1.
        // KHTML appears to report lines from 1 up.
        int baseLine = fragment.baseLine - 1;
        if (baseLine < 0)
            baseLine = 0;

        QString source = fragment.source;

        // ### can we guarantee this as a separator? probably not
        QStringList sourceLines = source.split("\n");

        for (int scanLine = 0; scanLine < sourceLines.size(); ++scanLine)
        {
            int absLine = baseLine + scanLine;

            // We have to be a bit careful here, since
            // in an ultra-stupid HTML documents, there may be more than
            // one script tag on a line. So we try to append things. 
            if (scanLine == 0 && !getLine(lines, absLine).isEmpty())
                setLine(lines, absLine, getLine(lines, absLine) + "  " + sourceLines[0]);
            else if (scanLine == sourceLines.size() -1 && !getLine(lines, absLine).isEmpty())
                setLine(lines, absLine, sourceLines[scanLine] + " " + getLine(lines, absLine));
            else
                setLine(lines, absLine, sourceLines[scanLine]);
        }
    }

    m_rebuilding = true;

    if (!m_kteDoc)
    {
        m_kteDoc = kate()->createDocument(this);
        setupViewerDocument();
    }

    m_kteDoc->setReadWrite(true);
    m_kteDoc->setText(lines.join("\n"));

    // Check off the pending/URL-based breakpoints. We have to do even
    // when the document is being updated as they may be on later lines
    KTextEditor::MarkInterface* imark = qobject_cast<KTextEditor::MarkInterface*>(m_kteDoc);
    if (imark)
    {
        QVector<int>& bps = breakpoints();
        foreach (int bpLine, bps)
            imark->addMark(bpLine - 1, KTextEditor::MarkInterface::BreakpointActive);
    }
    
    m_kteDoc->setReadWrite(false);
    m_rebuilding = false;
}

void DebugDocument::setupViewerDocument()
{
    // Highlight as JS..
    m_kteDoc->setMode("JavaScript");

    // Configure all the breakpoint/execution point marker stuff.
    // ### there is an odd split of mark use between here and DebugWindow.
    // Perhaps we should just emit a single and let it do it, and
    // limit ourselves to ownership?
    KTextEditor::MarkInterface* imark = qobject_cast<KTextEditor::MarkInterface*>(m_kteDoc);
    assert(imark);

    imark->setEditableMarks(KTextEditor::MarkInterface::BreakpointActive);
    connect(m_kteDoc, SIGNAL(markChanged(KTextEditor::Document*, KTextEditor::Mark, KTextEditor::MarkInterface::MarkChangeAction)),
                DebugWindow::window(), SLOT(markSet(KTextEditor::Document*, KTextEditor::Mark, KTextEditor::MarkInterface::MarkChangeAction)));
    // ### KDE4.1: fix this hack used to avoid new string
    imark->setMarkDescription(KTextEditor::MarkInterface::BreakpointActive,
                                          i18n("Breakpoints"));
    imark->setMarkPixmap(KTextEditor::MarkInterface::BreakpointActive,
                                     SmallIcon("flag-red"));
    imark->setMarkPixmap(KTextEditor::MarkInterface::Execution,
                                     SmallIcon("arrow-right"));
}

KTextEditor::View* DebugDocument::viewerView()
{
    if (m_kteView)
        return m_kteView;

    // Ensure document is created
    viewerDocument();

    m_kteView = m_kteDoc->createView(DebugWindow::window());
    KTextEditor::ConfigInterface* iconf = qobject_cast<KTextEditor::ConfigInterface*>(m_kteView);
    assert(iconf);
    if (iconf->configKeys().contains("line-numbers"))
        iconf->setConfigValue("line-numbers", true);
    if (iconf->configKeys().contains("icon-bar"))
        iconf->setConfigValue("icon-bar", true);
    if (iconf->configKeys().contains("dynamic-word-wrap"))
        iconf->setConfigValue("dynamic-word-wrap", true);

    return m_kteView;
}
