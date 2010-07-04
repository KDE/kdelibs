/* This file is part of the KDE libraries
   Copyright (C) 2008, 2009 Matthew Woehlke <mw_triad@users.sourceforge.net>
   Copyright (C) 2007 Mirko Stocker <me@misto.ch>
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2001 Anders Lund <anders@alweb.dk>
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kateviewhelpers.h"

#include "katecmd.h"
#include <ktexteditor/attribute.h>
#include <ktexteditor/annotationinterface.h>
#include <ktexteditor/movingrange.h>
#include <ktexteditor/containerinterface.h>
#include "katecodefolding.h"
#include "kateconfig.h"
#include "katedocument.h"
#include "katerenderer.h"
#include "kateview.h"
#include "kateviewinternal.h"
#include "katelayoutcache.h"
#include "katetextlayout.h"
#include "kateglobal.h"

#include <kapplication.h>
#include <kcharsets.h>
#include <kcolorscheme.h>
#include <kcolorutils.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <knotification.h>
#include <kglobal.h>
#include <kmenu.h>
#include <kiconloader.h>
#include <kconfiggroup.h>

#include <QtAlgorithms>
#include <QVariant>
#include <QtGui/QCursor>
#include <QtGui/QMenu>
#include <QtGui/QPainter>
#include <QtGui/QStyle>
#include <QtCore/QTimer>
#include <QtCore/QRegExp>
#include <QtCore/QTextCodec>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainterPath>
#include <QtGui/QStyleOption>
#include <QtGui/QPalette>
#include <QtGui/QPen>
#include <QtGui/QBoxLayout>
#include <QtGui/QToolButton>
#include <QtGui/QToolTip>
#include <QtGui/QAction>
#include <QtGui/QWhatsThis>

#include <math.h>

//BEGIN KateScrollBar
KateScrollBar::KateScrollBar (Qt::Orientation orientation, KateViewInternal* parent)
  : QScrollBar (orientation, parent->m_view)
  , m_middleMouseDown (false)
  , m_view(parent->m_view)
  , m_doc(parent->doc())
  , m_viewInternal(parent)
  , m_showMarks(false)
{
  connect(this, SIGNAL(valueChanged(int)), this, SLOT(sliderMaybeMoved(int)));
  connect(m_doc, SIGNAL(marksChanged(KTextEditor::Document*)), this, SLOT(marksChanged()));

  styleChange(*style());
}

void KateScrollBar::mousePressEvent(QMouseEvent* e)
{
  if (e->button() == Qt::MidButton)
    m_middleMouseDown = true;

  QScrollBar::mousePressEvent(e);

  redrawMarks();
}

void KateScrollBar::mouseReleaseEvent(QMouseEvent* e)
{
  QScrollBar::mouseReleaseEvent(e);

  m_middleMouseDown = false;

  redrawMarks();
}

void KateScrollBar::mouseMoveEvent(QMouseEvent* e)
{
  QScrollBar::mouseMoveEvent(e);

  if (e->buttons() | Qt::LeftButton)
    redrawMarks();
}

void KateScrollBar::paintEvent(QPaintEvent *e)
{
  QScrollBar::paintEvent(e);

  if (!m_showMarks)
    return;

  QPainter painter(this);

  QStyleOptionSlider opt;
  opt.init(this);
  opt.subControls = QStyle::SC_None;
  opt.activeSubControls = QStyle::SC_None;
  opt.orientation = orientation();
  opt.minimum = minimum();
  opt.maximum = maximum();
  opt.sliderPosition = sliderPosition();
  opt.sliderValue = value();
  opt.singleStep = singleStep();
  opt.pageStep = pageStep();

  QRect rect = style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarSlider, this);

  QHashIterator<int, QColor> it = m_lines;
  while (it.hasNext())
  {
    it.next();
    if (it.key() < rect.top() || it.key() > rect.bottom())
    {
      painter.setPen(it.value());
      painter.drawLine(0, it.key(), width(), it.key());
    }
  }
}

void KateScrollBar::resizeEvent(QResizeEvent *e)
{
  QScrollBar::resizeEvent(e);
  recomputeMarksPositions();
}

void KateScrollBar::styleChange(QStyle &s)
{
  QScrollBar::styleChange(s);
  recomputeMarksPositions();
}

void KateScrollBar::sliderChange ( SliderChange change )
{
  // call parents implementation
  QScrollBar::sliderChange (change);

  if (change == QAbstractSlider::SliderValueChange)
  {
    redrawMarks();
  }
  else if (change == QAbstractSlider::SliderRangeChange)
  {
    recomputeMarksPositions();
  }
}

void KateScrollBar::wheelEvent(QWheelEvent *e)
{
  QCoreApplication::sendEvent(m_viewInternal, e);
}

void KateScrollBar::marksChanged()
{
  recomputeMarksPositions();
}

void KateScrollBar::redrawMarks()
{
  if (!m_showMarks)
    return;

  update();
}

void KateScrollBar::recomputeMarksPositions()
{
  m_lines.clear();
  int visibleLines = m_doc->visibleLines();

  QStyleOptionSlider opt;
  initStyleOption(&opt);

  int topMargin = style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarSubPage, this).top() + 1;
  int realHeight = style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarAddPage, this).bottom() - topMargin - 1;

  const QHash<int, KTextEditor::Mark*> &marks = m_doc->marks();
  KateCodeFoldingTree *tree = m_doc->foldingTree();

  for (QHash<int, KTextEditor::Mark*>::const_iterator i = marks.constBegin(); i != marks.constEnd(); ++i)
  {
    KTextEditor::Mark *mark = i.value();

    uint line = mark->line;

    if (tree)
    {
      KateCodeFoldingNode *node = tree->findNodeForLine(line);

      while (node)
      {
        if (!node->isVisible())
          line = tree->getStartLine(node);
        node = node->getParentNode();
      }
    }

    line = m_doc->getVirtualLine(line);

    double d = (double)line / (visibleLines - 1);
    m_lines.insert(topMargin + (int)(d * realHeight),
                   QColor(KateRendererConfig::global()->lineMarkerColor((KTextEditor::MarkInterface::MarkTypes)mark->type)));
  }

  // with Qt4 we don't have the luxury of painting outside a paint event
  // and a paint event wipes the widget... so just update
  update();
}

void KateScrollBar::sliderMaybeMoved(int value)
{
  if (m_middleMouseDown) {
    // we only need to emit this signal once, as for the following slider
    // movements the signal sliderMoved() is already emitted.
    // Thus, set m_middleMouseDown to false right away.
    m_middleMouseDown = false;
    emit sliderMMBMoved(value);
  }
}
//END


//BEGIN KateCmdLineEditFlagCompletion
/**
 * This class provide completion of flags. It shows a short description of
 * each flag, and flags are appended.
 */
class KateCmdLineEditFlagCompletion : public KCompletion
{
  public:
    KateCmdLineEditFlagCompletion() {;}

    QString makeCompletion( const QString & /*s*/ )
    {
      return QString();
    }

};
//END KateCmdLineEditFlagCompletion

//BEGIN KateCmdLineEdit
KateCommandLineBar::KateCommandLineBar (KateView *view, QWidget *parent)
    : KateViewBarWidget (true, parent)
{
    QVBoxLayout *topLayout = new QVBoxLayout ();
    centralWidget()->setLayout(topLayout);
    topLayout->setMargin(0);
    m_lineEdit = new KateCmdLineEdit (this, view);
    connect(m_lineEdit, SIGNAL(hideRequested()), SIGNAL(hideMe()));
    topLayout->addWidget (m_lineEdit);

    setFocusProxy (m_lineEdit);
}

KateCommandLineBar::~KateCommandLineBar()
{
}

// inserts the given string in the command line edit and (if selcted = true) selects it so the user
// can type over it if she wants to
// inserts the given string in the command line edit and selects it so the user can type over it if
// she wants to
void KateCommandLineBar::setText(const QString &text, bool selected)
{
  m_lineEdit->setText(text);
  if (selected) {
    m_lineEdit->selectAll();
  }
}

KateCmdLineEdit::KateCmdLineEdit (KateCommandLineBar *bar, KateView *view)
  : KLineEdit ()
  , m_view (view)
  , m_bar (bar)
  , m_msgMode (false)
  , m_histpos( 0 )
  , m_cmdend( 0 )
  , m_command( 0L )
{
  connect (this, SIGNAL(returnPressed(const QString &)),
           this, SLOT(slotReturnPressed(const QString &)));

  setCompletionObject(KateCmd::self()->commandCompletionObject());
  setAutoDeleteCompletionObject( false );
  m_cmdRange.setPattern("^([0-9$]+|\\.([+-]\\d+)?)?,([0-9$]+|\\.([+-]\\d+)?)?");
  m_cmdExpr.setPattern("^(\\d+)([+-])(\\d+)$");
  m_gotoLine.setPattern("[+-]?\\d+");

  m_hideTimer = new QTimer(this);
  m_hideTimer->setSingleShot(true);
  connect(m_hideTimer, SIGNAL(timeout()), this, SLOT(hideLineEdit()));

  // make sure the timer is stopped when the user switches views. if not, focus will be given to the
  // wrong view when KateViewBar::hideCurrentBarWidget() is called after 4 seconds. (the timer is
  // used for showing things like "Success" for four seconds after the user has used the kate
  // command line)
  connect(m_view, SIGNAL(focusOut (KTextEditor::View*)), m_hideTimer, SLOT(stop()));
}

void KateCmdLineEdit::hideEvent(QHideEvent *e)
{
  Q_UNUSED(e);
  m_view->showViModeBar();
}


QString KateCmdLineEdit::helptext( const QPoint & ) const
{
  QString beg = "<qt background=\"white\"><div><table width=\"100%\"><tr><td bgcolor=\"brown\"><font color=\"white\"><b>Help: <big>";
  QString mid = "</big></b></font></td></tr><tr><td>";
  QString end = "</td></tr></table></div><qt>";

  QString t = text();
  QRegExp re( "\\s*help\\s+(.*)" );
  if ( re.indexIn( t ) > -1 )
  {
    QString s;
    // get help for command
    QString name = re.cap( 1 );
    if ( name == "list" )
    {
      return beg + i18n("Available Commands") + mid
          + KateCmd::self()->commandList().join(" ")
          + i18n("<p>For help on individual commands, do <code>'help &lt;command&gt;'</code></p>")
          + end;
    }
    else if ( ! name.isEmpty() )
    {
      KTextEditor::Command *cmd = KateCmd::self()->queryCommand( name );
      if ( cmd )
      {
        if ( cmd->help( m_view, name, s ) )
          return beg + name + mid + s + end;
        else
          return beg + name + mid + i18n("No help for '%1'",  name ) + end;
      }
      else
        return beg + mid + i18n("No such command <b>%1</b>", name) + end;
    }
  }

  return beg + mid + i18n(
      "<p>This is the Katepart <b>command line</b>.<br />"
      "Syntax: <code><b>command [ arguments ]</b></code><br />"
      "For a list of available commands, enter <code><b>help list</b></code><br />"
      "For help for individual commands, enter <code><b>help &lt;command&gt;</b></code></p>")
      + end;
}



bool KateCmdLineEdit::event(QEvent *e) {
  if (e->type() == QEvent::QueryWhatsThis) {
    setWhatsThis(helptext(QPoint()));
    e->accept();
    return true;
  }
  return KLineEdit::event(e);
}

void KateCmdLineEdit::slotReturnPressed ( const QString& text )
{
  if (text.isEmpty()) return;
  // silently ignore leading space characters and colon characters (for vi-heads)
  uint n = 0;
  const uint textlen=text.length();
  while( (n<textlen) && ( text[n].isSpace() || text[n] == ':' ) )
    n++;

  if (n>=textlen) return;

  QString cmd = text.mid( n );

  // expand '%' to '1,$' ("all lines") if at the start of the line
  if ( cmd.at( 0 ) == '%' ) {
    cmd.replace( 0, 1, "1,$" );
  }

  KTextEditor::Range range(-1, 0, -1, 0);

  // check if a range was given
  if (m_cmdRange.indexIn(cmd) != -1 && m_cmdRange.matchedLength() > 0) {

    cmd.remove( m_cmdRange );

    QString s = m_cmdRange.capturedTexts().at(1);
    QString e = m_cmdRange.capturedTexts().at(3);

    if ( s.isEmpty() )
      s = '.';
    if ( e.isEmpty() )
      e = s;

    // replace '$' with the number of the last line and '.' with the current line
    s.replace('$', QString::number( m_view->doc()->lines() ) );
    e.replace('$', QString::number( m_view->doc()->lines() ) );
    s.replace('.', QString::number( m_view->cursorPosition().line()+1 ) );
    e.replace('.', QString::number( m_view->cursorPosition().line()+1 ) );

    // evaluate expressions (a+b or a-b) if we have any
    if (m_cmdExpr.indexIn(s) != -1) {
      if (m_cmdExpr.capturedTexts().at(2) == "+") {
        s = QString::number(m_cmdExpr.capturedTexts().at(1).toInt()
            + m_cmdExpr.capturedTexts().at(3).toInt());
      } else {
        s = QString::number(m_cmdExpr.capturedTexts().at(1).toInt()
            - m_cmdExpr.capturedTexts().at(3).toInt());
      }
    }
    if (m_cmdExpr.indexIn(e) != -1) {
      if (m_cmdExpr.capturedTexts().at(2) == "+") {
        e = QString::number(m_cmdExpr.capturedTexts().at(1).toInt()
            + m_cmdExpr.capturedTexts().at(3).toInt());
      } else {
        e = QString::number(m_cmdExpr.capturedTexts().at(1).toInt()
            - m_cmdExpr.capturedTexts().at(3).toInt());
      }
    }

    range.setRange(KTextEditor::Range(s.toInt()-1, 0, e.toInt()-1, 0));
  }

  // special case: if the command is just a number with an optional +/- prefix, rewrite to "goto"
  if (m_gotoLine.exactMatch(cmd)) {
    cmd.prepend("goto ");
  }

  // Built in help: if the command starts with "help", [try to] show some help
  if ( cmd.startsWith( QLatin1String("help") ) )
  {
    QWhatsThis::showText(mapToGlobal(QPoint(0,0)), helptext( QPoint() ) );
    clear();
    KateCmd::self()->appendHistory( cmd );
    m_histpos = KateCmd::self()->historyLength();
    m_oldText.clear();
    return;
  }

  if (cmd.length () > 0)
  {
    KTextEditor::Command *p = KateCmd::self()->queryCommand (cmd);
    KTextEditor::RangeCommand *ce = dynamic_cast<KTextEditor::RangeCommand*>(p);

    m_oldText = m_cmdRange.capturedTexts().at(0) + cmd;
    m_msgMode = true;

    // we got a range and a valid command, but the command does not inherit the RangeCommand
    // extension. bail out.
    if ( ( !ce && range.isValid() && p ) || ( range.isValid() && ce && !ce->supportsRange(cmd) ) ) {
      setText (i18n ("Error: No range allowed for command \"%1\".",  cmd));
    } else {

      if (p)
      {
        QString msg;

        if ((ce && ce->exec(m_view, cmd, msg, range)) || p->exec (m_view, cmd, msg))
        {

          // append command along with range (will be empty if none given) to history
          KateCmd::self()->appendHistory( m_cmdRange.capturedTexts().at(0) + cmd );
          m_histpos = KateCmd::self()->historyLength();
          m_oldText.clear();

          if (msg.length() > 0)
            setText (i18n ("Success: ") + msg);
          else
            m_bar->hide(); // always hide on success without message
        }
        else
        {
          if (msg.length() > 0)
            setText (i18n ("Error: ") + msg);
          else
            setText (i18n ("Command \"%1\" failed.",  cmd));
          KNotification::beep();
        }
      }
      else
      {
        setText (i18n ("No such command: \"%1\"",  cmd));
        KNotification::beep();
      }
    }
  }

  // clean up
  if (completionObject() != KateCmd::self()->commandCompletionObject())
  {
    KCompletion *c = completionObject();
    setCompletionObject(KateCmd::self()->commandCompletionObject());
    delete c;
  }
  m_command = 0;
  m_cmdend = 0;

  // the following commands change the focus themselves
  // FIXME: ugly :-(
  if (cmd != "bn" && cmd != "bp" && cmd != "new" && cmd != "vnew" && cmd != "enew") {
    m_view->setFocus ();
  }

  m_hideTimer->start(4000);
}

void KateCmdLineEdit::hideLineEdit () // unless i have focus ;)
{
  if ( ! hasFocus() ) {
      emit hideRequested();
  }
}

void KateCmdLineEdit::focusInEvent ( QFocusEvent *ev )
{
  if (m_msgMode)
  {
    m_msgMode = false;
    setText (m_oldText);
    selectAll();
  }

  KLineEdit::focusInEvent (ev);
}

void KateCmdLineEdit::keyPressEvent( QKeyEvent *ev )
{
  if (ev->key() == Qt::Key_Escape ||
      (ev->key() == Qt::Key_BracketLeft && ev->modifiers() == Qt::ControlModifier))
  {
    m_view->setFocus ();
    hideLineEdit();
    clear();
  }
  else if ( ev->key() == Qt::Key_Up )
    fromHistory( true );
  else if ( ev->key() == Qt::Key_Down )
    fromHistory( false );

  uint cursorpos = cursorPosition();
  KLineEdit::keyPressEvent (ev);

  // during typing, let us see if we have a valid command
  if ( ! m_cmdend || cursorpos <= m_cmdend  )
  {
    QChar c;
    if ( ! ev->text().isEmpty() )
      c = ev->text()[0];

    if ( ! m_cmdend && ! c.isNull() ) // we have no command, so lets see if we got one
    {
      if ( ! c.isLetterOrNumber() && c != '-' && c != '_' )
      {
        m_command = KateCmd::self()->queryCommand( text().trimmed() );
        if ( m_command )
        {
          //kDebug(13025)<<"keypress in commandline: We have a command! "<<m_command<<". text is '"<<text()<<"'";
          // if the typed character is ":",
          // we try if the command has flag completions
          m_cmdend = cursorpos;
          //kDebug(13025)<<"keypress in commandline: Set m_cmdend to "<<m_cmdend;
        }
        else
          m_cmdend = 0;
      }
    }
    else // since cursor is inside the command name, we reconsider it
    {
      kDebug(13025)<<"keypress in commandline: \\W -- text is "<<text();
      m_command = KateCmd::self()->queryCommand( text().trimmed() );
      if ( m_command )
      {
        //kDebug(13025)<<"keypress in commandline: We have a command! "<<m_command;
        QString t = text();
        m_cmdend = 0;
        bool b = false;
        for ( ; (int)m_cmdend < t.length(); m_cmdend++ )
        {
          if ( t[m_cmdend].isLetter() )
            b = true;
          if ( b && ( ! t[m_cmdend].isLetterOrNumber() && t[m_cmdend] != '-' && t[m_cmdend] != '_' ) )
            break;
        }

        if ( c == ':' && cursorpos == m_cmdend )
        {
          // check if this command wants to complete flags
          //kDebug(13025)<<"keypress in commandline: Checking if flag completion is desired!";
        }
      }
      else
      {
        // clean up if needed
        if (completionObject() != KateCmd::self()->commandCompletionObject())
        {
          KCompletion *c = completionObject();
          setCompletionObject(KateCmd::self()->commandCompletionObject());
          delete c;
        }

        m_cmdend = 0;
      }
    }

    // if we got a command, check if it wants to do something.
    if ( m_command )
    {
      //kDebug(13025)<<"Checking for CommandExtension..";
      KTextEditor::CommandExtension *ce = dynamic_cast<KTextEditor::CommandExtension*>(m_command);
      if ( ce )
      {
        KCompletion *cmpl = ce->completionObject( m_view, text().left( m_cmdend ).trimmed() );
        if ( cmpl )
        {
        // We need to prepend the current command name + flag string
        // when completion is done
          //kDebug(13025)<<"keypress in commandline: Setting completion object!";

          setCompletionObject( cmpl );
        }
      }
    }
  }
  else if ( m_command )// check if we should call the commands processText()
  {
    KTextEditor::CommandExtension *ce = dynamic_cast<KTextEditor::CommandExtension*>( m_command );
    if ( ce && ce->wantsToProcessText( text().left( m_cmdend ).trimmed() )
         && ! ( ev->text().isNull() || ev->text().isEmpty() ) )
      ce->processText( m_view, text() );
  }
}

void KateCmdLineEdit::fromHistory( bool up )
{
  if ( ! KateCmd::self()->historyLength() )
    return;

  QString s;

  if ( up )
  {
    if ( m_histpos > 0 )
    {
      m_histpos--;
      s = KateCmd::self()->fromHistory( m_histpos );
    }
  }
  else
  {
    if ( m_histpos < ( KateCmd::self()->historyLength() - 1 ) )
    {
      m_histpos++;
      s = KateCmd::self()->fromHistory( m_histpos );
    }
    else
    {
      m_histpos = KateCmd::self()->historyLength();
      setText( m_oldText );
    }
  }
  if ( ! s.isEmpty() )
  {
    // Select the argument part of the command, so that it is easy to overwrite
    setText( s );
    static QRegExp reCmd = QRegExp(".*[\\w\\-]+(?:[^a-zA-Z0-9_-]|:\\w+)(.*)");
    if ( reCmd.indexIn( text() ) == 0 )
      setSelection( text().length() - reCmd.cap(1).length(), reCmd.cap(1).length() );
  }
}
//END KateCmdLineEdit

//BEGIN KateIconBorder
using namespace KTextEditor;

const int halfIPW = 8;

KateIconBorder::KateIconBorder ( KateViewInternal* internalView, QWidget *parent )
  : QWidget(parent)
  , m_view( internalView->m_view )
  , m_doc( internalView->doc() )
  , m_viewInternal( internalView )
  , m_iconBorderOn( false )
  , m_lineNumbersOn( false )
  , m_foldingMarkersOn( false )
  , m_dynWrapIndicatorsOn( false )
  , m_annotationBorderOn( false )
  , m_dynWrapIndicators( 0 )
  , m_cachedLNWidth( 0 )
  , m_maxCharWidth( 0 )
  , iconPaneWidth (16)
  , m_annotationBorderWidth (6)
  , m_foldingRange(0)
  , m_nextHighlightBlock(-2)
  , m_currentBlockLine(-1)
{
  initializeFoldingColors();

  setAttribute( Qt::WA_StaticContents );
  setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Minimum );
  setMouseTracking(true);
  m_doc->setMarkDescription( MarkInterface::markType01, i18n("Bookmark") );
  m_doc->setMarkPixmap( MarkInterface::markType01, KIcon("bookmarks").pixmap(16, 16) );

  updateFont();

  m_delayFoldingHlTimer.setSingleShot(true);
  m_delayFoldingHlTimer.setInterval(250);
  connect(&m_delayFoldingHlTimer, SIGNAL(timeout()), this, SLOT(showBlock()));
}

void KateIconBorder::initializeFoldingColors()
{
  // Get the schema
  KateRendererConfig *config = m_view->renderer()->config();
  // FIXME next 3 lines temporary until this moves to config
  const KColorScheme scheme( QPalette::Normal );
  const QColor middle( KColorUtils::tint( config->iconBarColor(), scheme.foreground( KColorScheme::NeutralText ).color(), 0.7 ) );
  const QColor final( KColorUtils::tint( config->iconBarColor(), scheme.foreground( KColorScheme::PositiveText ).color(), 0.7 ) );

  const QColor start( config->iconBarColor() );
  static const int MIDFOLDINGCOLORS = MAXFOLDINGCOLORS / 2;
  static const qreal n = 2.0 / MAXFOLDINGCOLORS;

  int i, j;
  for( i = 0; i < MIDFOLDINGCOLORS; i++ ) {
    const qreal a = 0.9 * pow(qreal(i) * n, 1.0);
    m_foldingColors[i] = KColorUtils::tint( start, middle, a );
  }
  for( j = 0; i < MAXFOLDINGCOLORS; i++, j++ ) {
    const qreal a = 0.9 * pow(qreal(j) * n, 1.0);
    m_foldingColors[i] = KColorUtils::tint( middle, final, a );
  }
}


KateIconBorder::~KateIconBorder()
{
  delete m_foldingRange;
  m_foldingRange = 0;
}

void KateIconBorder::setIconBorderOn( bool enable )
{
  if( enable == m_iconBorderOn )
    return;

  m_iconBorderOn = enable;

  updateGeometry();

  QTimer::singleShot( 0, this, SLOT(update()) );
}

void KateIconBorder::setAnnotationBorderOn( bool enable )
{
  if( enable == m_annotationBorderOn )
    return;

  m_annotationBorderOn = enable;

  emit m_view->annotationBorderVisibilityChanged(m_view, enable);

  updateGeometry();

  QTimer::singleShot( 0, this, SLOT(update()) );
}

void KateIconBorder::removeAnnotationHovering()
{
  // remove hovering if it's still there
  if (m_annotationBorderOn && !m_hoveredAnnotationText.isEmpty())
  {
    m_hoveredAnnotationText.clear();
    hideAnnotationTooltip();
    QTimer::singleShot( 0, this, SLOT(update()) );
  }
}

void KateIconBorder::setLineNumbersOn( bool enable )
{
  if( enable == m_lineNumbersOn )
    return;

  m_lineNumbersOn = enable;
  m_dynWrapIndicatorsOn = (m_dynWrapIndicators == 1) ? enable : m_dynWrapIndicators;

  updateGeometry();

  QTimer::singleShot( 0, this, SLOT(update()) );
}

void KateIconBorder::setDynWrapIndicators( int state )
{
  if (state == m_dynWrapIndicators )
    return;

  m_dynWrapIndicators = state;
  m_dynWrapIndicatorsOn = (state == 1) ? m_lineNumbersOn : state;

  updateGeometry ();

  QTimer::singleShot( 0, this, SLOT(update()) );
}

void KateIconBorder::setFoldingMarkersOn( bool enable )
{
  if( enable == m_foldingMarkersOn )
    return;

  m_foldingMarkersOn = enable;

  updateGeometry();

  QTimer::singleShot( 0, this, SLOT(update()) );
}

QSize KateIconBorder::sizeHint() const
{
  int w = 0;

  if (m_iconBorderOn)
    w += iconPaneWidth + 1;

  if (m_annotationBorderOn)
  {
    w += m_annotationBorderWidth + 1;
  }

  if (m_lineNumbersOn || (m_view->dynWordWrap() && m_dynWrapIndicatorsOn)) {
    w += lineNumberWidth() + 1;
  }

  if (m_foldingMarkersOn)
    w += iconPaneWidth + 1;

  w += 4;

  return QSize( w, 0 );
}

// This function (re)calculates the maximum width of any of the digit characters (0 -> 9)
// for graceful handling of variable-width fonts as the linenumber font.
void KateIconBorder::updateFont()
{
  QFontMetrics fm = m_view->renderer()->config()->fontMetrics();
  m_maxCharWidth = 0;
  // Loop to determine the widest numeric character in the current font.
  // 48 is ascii '0'
  for (int i = 48; i < 58; i++) {
    int charWidth = fm.width( QChar(i) );
    m_maxCharWidth = qMax(m_maxCharWidth, charWidth);
  }

  // the icon pane scales with the font...
  iconPaneWidth = fm.height();

  updateGeometry();

  QTimer::singleShot( 0, this, SLOT(update()) );
}

int KateIconBorder::lineNumberWidth() const
{
  int width = m_lineNumbersOn ? ((int)log10((double)(m_view->doc()->lines())) + 1) * m_maxCharWidth + 4 : 0;

  if (m_view->dynWordWrap() && m_dynWrapIndicatorsOn) {
    // HACK: 16 == style().scrollBarExtent().width()
    width = qMax(16 + 4, width);

    if (m_cachedLNWidth != width || m_oldBackgroundColor != m_view->renderer()->config()->iconBarColor()) {
      int w = 16;// HACK: 16 == style().scrollBarExtent().width() style().scrollBarExtent().width();
      int h = m_view->renderer()->config()->fontMetrics().height();

      QSize newSize(w, h);
      if ((m_arrow.size() != newSize || m_oldBackgroundColor != m_view->renderer()->config()->iconBarColor()) && !newSize.isEmpty()) {
        m_arrow = QPixmap(newSize);

        QPainter p(&m_arrow);
        p.fillRect( 0, 0, w, h, m_view->renderer()->config()->iconBarColor() );

        h = m_view->renderer()->config()->fontMetrics().ascent();

        p.setPen(m_view->renderer()->config()->lineNumberColor());

        QPainterPath path;
        path.moveTo(w/2, h/2);
        path.lineTo(w/2, 0);
        path.lineTo(w/4, h/4);
        path.lineTo(0, 0);
        path.lineTo(0, h/2);
        path.lineTo(w/2, h-1);
        path.lineTo(w*3/4, h-1);
        path.lineTo(w-1, h*3/4);
        path.lineTo(w*3/4, h/2);
        path.lineTo(0, h/2);
        p.drawPath(path);
      }
    }
  }

  return width;
}

QBrush KateIconBorder::foldingColor(KateLineInfo *info,int realLine, bool solid) {
  int depth;
  if (info != 0) {
    depth = info->depth;
  } else {
    KateLineInfo tmp;
    m_doc->lineInfo(&tmp, realLine);
    depth = tmp.depth;
  }

  QColor result;
  if (depth < MAXFOLDINGCOLORS)
    result = m_foldingColors[depth];
  else
    result = m_foldingColors[MAXFOLDINGCOLORS-1];
  if (!solid)
    result.setAlphaF(0.4);

  return QBrush( result );

}

void KateIconBorder::paintEvent(QPaintEvent* e)
{
  paintBorder(e->rect().x(), e->rect().y(), e->rect().width(), e->rect().height());
}

static void paintTriangle (QPainter &painter, const QColor &baseColor, int xOffset, int yOffset, int width, int height, bool open)
{
  painter.setRenderHint(QPainter::Antialiasing);

  qreal size = qMin (width, height);

  QColor c;
  if ( KColorUtils::luma( baseColor ) > 0.25 )
    c = KColorUtils::darken( baseColor );
  else
    c = KColorUtils::shade( baseColor, 0.2 );

  QPen pen;
  pen.setJoinStyle (Qt::RoundJoin);
  pen.setColor (c);
  pen.setWidthF (1.5);
  painter.setPen ( pen );

  painter.setBrush ( c );

  // let some border, if possible
  size *= 0.6;

  qreal halfSize = size / 2;
  qreal halfSizeP = halfSize * 0.6;
  QPointF middle (xOffset + (qreal)width / 2, yOffset + (qreal)height / 2);

  if (open)
  {
    QPointF points[3] = { middle+QPointF(-halfSize, -halfSizeP), middle+QPointF(halfSize, -halfSizeP), middle+QPointF(0, halfSizeP) };
    painter.drawConvexPolygon(points, 3);
  }
  else
  {
    QPointF points[3] = { middle+QPointF(-halfSizeP, -halfSize), middle+QPointF(-halfSizeP, halfSize), middle+QPointF(halfSizeP, 0) };
    painter.drawConvexPolygon(points, 3);
  }

  painter.setRenderHint(QPainter::Antialiasing, false);
}

void KateIconBorder::paintBorder (int /*x*/, int y, int /*width*/, int height)
{
  QMutexLocker lock(m_view->m_doc->smartMutex());
  uint h = m_view->renderer()->config()->fontMetrics().height();
  uint startz = (y / h);
  uint endz = startz + 1 + (height / h);
  uint lineRangesSize = m_viewInternal->cache()->viewCacheLineCount();

  // center the folding boxes
  int m_px = (h - 11) / 2;
  if (m_px < 0)
    m_px = 0;

  int lnWidth( 0 );
  if ( m_lineNumbersOn || (m_view->dynWordWrap() && m_dynWrapIndicatorsOn) ) // avoid calculating unless needed ;-)
  {
    lnWidth = lineNumberWidth();
    if ( lnWidth != m_cachedLNWidth || m_oldBackgroundColor != m_view->renderer()->config()->iconBarColor() )
    {
      // we went from n0 ->n9 lines or vice verca
      // this causes an extra updateGeometry() first time the line numbers
      // are displayed, but sizeHint() is supposed to be const so we can't set
      // the cached value there.
      m_cachedLNWidth = lnWidth;
      m_oldBackgroundColor = m_view->renderer()->config()->iconBarColor();
      updateGeometry();
      update ();
      return;
    }
  }

  int w( this->width() );                     // sane value/calc only once

  QPainter p ( this );
  p.setRenderHints (QPainter::TextAntialiasing);
  p.setFont ( m_view->renderer()->config()->font() ); // for line numbers

  KateLineInfo oldInfo;
  if (startz < lineRangesSize)
  {
    if ((m_viewInternal->cache()->viewLine(startz).line()-1) < 0)
      oldInfo.topLevel = true;
    else
      m_doc->lineInfo(&oldInfo,m_viewInternal->cache()->viewLine(startz).line()-1);
  }

  KTextEditor::AnnotationModel *model = m_view->annotationModel() ?
      m_view->annotationModel() : m_doc->annotationModel();

  for (uint z=startz; z <= endz; z++)
  {
    int y = h * z;
    int realLine = -1;

    if (z < lineRangesSize)
      realLine = m_viewInternal->cache()->viewLine(z).line();

    int lnX = 0;

    p.fillRect( 0, y, w-4, h, m_view->renderer()->config()->iconBarColor() );
    p.fillRect( w-4, y, 4, h, m_view->renderer()->config()->backgroundColor() );

    // icon pane
    if( m_iconBorderOn )
    {
      p.setPen ( m_view->renderer()->config()->lineNumberColor() );
      p.setBrush ( m_view->renderer()->config()->lineNumberColor() );
      p.drawLine(lnX+iconPaneWidth+1, y, lnX+iconPaneWidth+1, y+h);

      if( (realLine > -1) && (m_viewInternal->cache()->viewLine(z).startCol() == 0) )
      {
        uint mrk ( m_doc->mark( realLine ) ); // call only once

        if ( mrk )
        {
          for( uint bit = 0; bit < 32; bit++ )
          {
            MarkInterface::MarkTypes markType = (MarkInterface::MarkTypes)(1<<bit);
            if( mrk & markType )
            {
              QPixmap px_mark (m_doc->markPixmap( markType ));

              if (!px_mark.isNull() && h > 0 && iconPaneWidth > 0)
              {
                if (iconPaneWidth < px_mark.width() || h < (uint)px_mark.height())
                  px_mark = px_mark.scaled (iconPaneWidth, h, Qt::KeepAspectRatio);

                // center the mark pixmap
                int x_px = (iconPaneWidth - px_mark.width()) / 2;
                if (x_px < 0)
                  x_px = 0;

                int y_px = (h - px_mark.height()) / 2;
                if (y_px < 0)
                  y_px = 0;

                p.drawPixmap( lnX+x_px, y+y_px, px_mark);
              }
            }
          }
        }
      }

      lnX += iconPaneWidth + 2;
    }

    // annotation information
    if( m_annotationBorderOn )
    {
      // Draw a border line between annotations and the line numbers
      p.setPen ( m_view->renderer()->config()->lineNumberColor() );
      p.setBrush ( m_view->renderer()->config()->lineNumberColor() );

      int borderWidth = m_annotationBorderWidth;
      p.drawLine(lnX+borderWidth+1, y, lnX+borderWidth+1, y+h);

      if( (realLine > -1) && model )
      {
        // Fetch data from the model
        QVariant text = model->data( realLine, Qt::DisplayRole );
        QVariant foreground = model->data( realLine, Qt::ForegroundRole );
        QVariant background = model->data( realLine, Qt::BackgroundRole );
        // Fill the background
        if( background.isValid() )
        {
          p.fillRect( lnX, y, borderWidth + 1, h, background.value<QBrush>() );
        }
        // Set the pen for drawing the foreground
        if( foreground.isValid() )
        {
          p.setBrush( foreground.value<QBrush>() );
        }

        // Draw a border around all adjacent entries that have the same text as the currently hovered one
        if( m_hoveredAnnotationText == text.toString() )
        {
          p.drawLine( lnX, y, lnX, y+h );
          p.drawLine( lnX+borderWidth, y, lnX+borderWidth, y+h );

          QVariant beforeText = model->data( realLine-1, Qt::DisplayRole );
          QVariant afterText = model->data( realLine+1, Qt::DisplayRole );
          if( ((beforeText.isValid() && beforeText.canConvert<QString>()
              && text.isValid() && text.canConvert<QString>()
              && beforeText.toString() != text.toString()) || realLine == 0)
              && m_viewInternal->cache()->viewLine(z).viewLine() == 0)
          {
            p.drawLine( lnX+1, y, lnX+borderWidth, y );
          }

          if( ((afterText.isValid() && afterText.canConvert<QString>()
              && text.isValid() && text.canConvert<QString>()
              && afterText.toString() != text.toString())
                || realLine == m_view->doc()->lines() - 1)
              && m_viewInternal->cache()->viewLine(z).viewLine() == m_viewInternal->cache()->viewLineCount(realLine)-1)
          {
            p.drawLine( lnX+1, y+h-1, lnX+borderWidth, y+h-1 );
          }
        }
        if( foreground.isValid() )
        {
          QPen pen = p.pen();
          pen.setWidth( 1 );
          p.setPen( pen );
        }

        // Now draw the normal text
        if( text.isValid() && text.canConvert<QString>() && (m_viewInternal->cache()->viewLine(z).startCol() == 0)  )
        {
          p.drawText( lnX+3, y, borderWidth-3, h, Qt::AlignLeft|Qt::AlignVCenter, text.toString() );
        }
      }

      // adjust current X position and reset the pen and brush
      lnX += borderWidth + 2;
    }

    // line number
    if( m_lineNumbersOn || (m_view->dynWordWrap() && m_dynWrapIndicatorsOn) )
    {
      p.setPen ( m_view->renderer()->config()->lineNumberColor() );
      p.setBrush ( m_view->renderer()->config()->lineNumberColor() );

      if (realLine > -1) {
        if (m_viewInternal->cache()->viewLine(z).startCol() == 0) {
          if (m_lineNumbersOn)
            p.drawText( lnX, y, lnWidth-4, h, Qt::AlignRight|Qt::AlignVCenter, QString("%1").arg( realLine + 1 ) );
        } else if (m_view->dynWordWrap() && m_dynWrapIndicatorsOn) {
          p.drawPixmap(lnX + lnWidth - m_arrow.width() - 2, y, m_arrow);
        }
      }

      lnX += lnWidth + 2;
    }

    // folding markers
    if( m_foldingMarkersOn )
    {
      if( realLine > -1 )
      {
        KateLineInfo info;
        m_doc->lineInfo(&info,realLine);

        QBrush brush (foldingColor(&info,realLine,true));
        p.fillRect(lnX, y, iconPaneWidth, h, brush);

        if (!info.topLevel)
        {
          if (info.startsVisibleBlock && (m_viewInternal->cache()->viewLine(z).startCol() == 0))
          {
            paintTriangle (p, brush.color(), lnX, y, iconPaneWidth, h, true);
          }
          else if (info.startsInVisibleBlock && m_viewInternal->cache()->viewLine(z).startCol() == 0)
          {
            paintTriangle (p, brush.color(), lnX, y, iconPaneWidth, h, false);
          }
          else
          {
           // p.drawLine(lnX+halfIPW,y,lnX+halfIPW,y+h-1);

           // if (info.endsBlock && !m_viewInternal->cache()->viewLine(z).wrap())
            //  p.drawLine(lnX+halfIPW,y+h-1,lnX+iconPaneWidth-2,y+h-1);
          }
        }

        oldInfo = info;
      }

      lnX += iconPaneWidth + 2;
    }
  }
}

KateIconBorder::BorderArea KateIconBorder::positionToArea( const QPoint& p ) const
{
  int x = 0;
  if( m_iconBorderOn ) {
    x += iconPaneWidth;
    if( p.x() <= x )
      return IconBorder;
  }
  if( this->m_annotationBorderOn ) {
    x += m_annotationBorderWidth;
    if( p.x() <= x )
      return AnnotationBorder;
  }
  if( m_lineNumbersOn || m_dynWrapIndicators ) {
    x += lineNumberWidth();
    if( p.x() <= x )
      return LineNumbers;
  }
  if( m_foldingMarkersOn ) {
    x += iconPaneWidth;
    if( p.x() <= x )
      return FoldingMarkers;
  }
  return None;
}

void KateIconBorder::mousePressEvent( QMouseEvent* e )
{
  const KateTextLayout& t = m_viewInternal->yToKateTextLayout(e->y());
  if (t.isValid()) {
    m_lastClickedLine = t.line();
    if ( positionToArea( e->pos() ) != IconBorder && positionToArea( e->pos() ) != AnnotationBorder )
    {
      QMouseEvent forward( QEvent::MouseButtonPress,
        QPoint( 0, e->y() ), e->button(), e->buttons(),e->modifiers() );
      m_viewInternal->mousePressEvent( &forward );
    }
    return e->accept();
  }

  QWidget::mousePressEvent(e);
}

void KateIconBorder::showDelayedBlock(int line)
{
  // save the line over which the mouse hovers
  // either we start the timer for delay, or we show the block immediately
  // if the smart range already exists
  m_nextHighlightBlock = line;
  if (!m_foldingRange) {
    if (!m_delayFoldingHlTimer.isActive()) {
      m_delayFoldingHlTimer.start();
    }
  } else {
    showBlock();
  }
}

void KateIconBorder::showBlock()
{
  if (m_nextHighlightBlock == m_currentBlockLine) return;
  m_currentBlockLine = m_nextHighlightBlock;

  // get the new range, that should be highlighted
  KTextEditor::Range newRange = KTextEditor::Range::invalid();
  KateCodeFoldingTree *tree = m_doc->foldingTree();
  if (tree) {
    KateCodeFoldingNode *node = tree->findNodeForLine(m_currentBlockLine);
    KTextEditor::Cursor beg;
    KTextEditor::Cursor end;
    if (node != tree->rootNode () && node->getBegin(tree, &beg)) {
      if (node->getEnd(tree, &end))
        newRange = KTextEditor::Range(beg, end);
      else
        newRange = KTextEditor::Range(beg, m_viewInternal->doc()->documentEnd());
    }
    KateLineInfo info;
    tree->getLineInfo(&info, m_currentBlockLine);
    if ((info.startsVisibleBlock)){
      node=tree->findNodeStartingAt(m_currentBlockLine);
      if (node) {
        if (node != tree->rootNode () && node->getBegin(tree, &beg) && node->getEnd(tree, &end)) {
          newRange = KTextEditor::Range(beg, end);
        }
      }
    }

  }

  if (newRange.isValid() && m_foldingRange && *m_foldingRange == newRange) {
    // new range equals the old one, nothing to do.
    return;
  } else { // the ranges differ, delete the old, if it exists
    delete m_foldingRange;
    m_foldingRange = 0;
  }

  if (newRange.isValid()) {
    kDebug(13025) << "new folding hl-range:" << newRange;
    m_foldingRange = m_doc->newMovingRange(newRange, KTextEditor::MovingRange::ExpandRight);
    KTextEditor::Attribute::Ptr attr(new KTextEditor::Attribute());
    attr->setBackground(foldingColor(0, m_currentBlockLine, false));
    m_foldingRange->setView (m_view);
    // use z depth defined in moving ranges interface
    m_foldingRange->setZDepth (-100.0);
    m_foldingRange->setAttribute(attr);
  }
}

void KateIconBorder::hideBlock()
{
  if (m_delayFoldingHlTimer.isActive()) {
    m_delayFoldingHlTimer.stop();
  }

  m_nextHighlightBlock = -2;
  m_currentBlockLine = -1;
  delete m_foldingRange;
  m_foldingRange = 0;
}

void KateIconBorder::leaveEvent(QEvent *event)
{
  hideBlock();
  removeAnnotationHovering();

  QWidget::leaveEvent(event);
}

void KateIconBorder::mouseMoveEvent( QMouseEvent* e )
{
  const KateTextLayout& t = m_viewInternal->yToKateTextLayout(e->y());
  if (t.isValid()) {
    if ( positionToArea( e->pos() ) == FoldingMarkers) showDelayedBlock(t.line());
    else hideBlock();
    if ( positionToArea( e->pos() ) == AnnotationBorder )
    {
      KTextEditor::AnnotationModel *model = m_view->annotationModel() ?
        m_view->annotationModel() : m_doc->annotationModel();
      if (model)
      {
        m_hoveredAnnotationText = model->data( t.line(), Qt::DisplayRole ).toString();
        showAnnotationTooltip( t.line(), e->globalPos() );
        QTimer::singleShot( 0, this, SLOT(update()) );
      }
    }
    else
    {
      if( positionToArea( e->pos() ) == IconBorder )
        m_doc->requestMarkTooltip( t.line(), e->globalPos() );

      m_hoveredAnnotationText.clear();
      hideAnnotationTooltip();
      QTimer::singleShot( 0, this, SLOT(update()) );
    }
    if ( positionToArea( e->pos() ) != IconBorder )
    {
      QPoint p = m_viewInternal->mapFromGlobal( e->globalPos() );
      QMouseEvent forward( QEvent::MouseMove, p, e->button(), e->buttons(), e->modifiers() );
      m_viewInternal->mouseMoveEvent( &forward );
    }
  }
  else
  {
    // remove hovering if it's still there
    removeAnnotationHovering();
  }

  QWidget::mouseMoveEvent(e);
}

void KateIconBorder::mouseReleaseEvent( QMouseEvent* e )
{
  int cursorOnLine = m_viewInternal->yToKateTextLayout(e->y()).line();

  if (cursorOnLine == m_lastClickedLine &&
      cursorOnLine <= m_doc->lastLine() )
  {
    BorderArea area = positionToArea( e->pos() );
    if( area == IconBorder) {
      if (e->button() == Qt::LeftButton) {
        if( !m_doc->handleMarkClick(cursorOnLine) ) {
          if( m_doc->editableMarks() & KateViewConfig::global()->defaultMarkType() ) {
            if( m_doc->mark( cursorOnLine ) & KateViewConfig::global()->defaultMarkType() )
              m_doc->removeMark( cursorOnLine, KateViewConfig::global()->defaultMarkType() );
            else
              m_doc->addMark( cursorOnLine, KateViewConfig::global()->defaultMarkType() );
            } else {
              showMarkMenu( cursorOnLine, QCursor::pos() );
            }
        }
        }
        else
        if (e->button() == Qt::RightButton) {
          showMarkMenu( cursorOnLine, QCursor::pos() );
        }
    }

    if ( area == FoldingMarkers) {
      KateLineInfo info;
      m_doc->lineInfo(&info,cursorOnLine);
      if ((info.startsVisibleBlock) || (info.startsInVisibleBlock)) {
        emit toggleRegionVisibility(cursorOnLine);
      }
    }

    if ( area == AnnotationBorder ) {
      if( e->button() == Qt::LeftButton && KGlobalSettings::singleClick() ) {
        emit m_view->annotationActivated( m_view, cursorOnLine );
      } else if ( e->button() == Qt::RightButton ) {
        showAnnotationMenu( cursorOnLine, e->globalPos() );
      }
    }
  }

  QMouseEvent forward( QEvent::MouseButtonRelease,
    QPoint( 0, e->y() ), e->button(), e->buttons(),e->modifiers() );
  m_viewInternal->mouseReleaseEvent( &forward );
}

void KateIconBorder::mouseDoubleClickEvent( QMouseEvent* e )
{
  int cursorOnLine = m_viewInternal->yToKateTextLayout(e->y()).line();

  if (cursorOnLine == m_lastClickedLine &&
      cursorOnLine <= m_doc->lastLine() )
  {
    BorderArea area = positionToArea( e->pos() );
    if( area == AnnotationBorder && !KGlobalSettings::singleClick() ) {
      emit m_view->annotationActivated( m_view, cursorOnLine );
    }
  }
  QMouseEvent forward( QEvent::MouseButtonDblClick,
    QPoint( 0, e->y() ), e->button(), e->buttons(),e->modifiers() );
  m_viewInternal->mouseDoubleClickEvent( &forward );
}

void KateIconBorder::showMarkMenu( uint line, const QPoint& pos )
{
  if( m_doc->handleMarkContextMenu( line, pos ) )
    return;

  KMenu markMenu;
  KMenu selectDefaultMark;

  QVector<int> vec( 33 );
  int i=1;

  for( uint bit = 0; bit < 32; bit++ ) {
    MarkInterface::MarkTypes markType = (MarkInterface::MarkTypes)(1<<bit);
    if( !(m_doc->editableMarks() & markType) )
      continue;

    QAction *mA;
    QAction *dMA;
    if( !m_doc->markDescription( markType ).isEmpty() ) {
      mA=markMenu.addAction( m_doc->markDescription( markType ));
      dMA=selectDefaultMark.addAction( m_doc->markDescription( markType ));
    } else {
      mA=markMenu.addAction( i18n("Mark Type %1",  bit + 1 ));
      dMA=selectDefaultMark.addAction( i18n("Mark Type %1",  bit + 1 ));
    }
    mA->setData(i);
    mA->setCheckable(true);
    dMA->setData(i+100);
    dMA->setCheckable(true);
    if( m_doc->mark( line ) & markType )
      mA->setChecked(true );

    if( markType & KateViewConfig::global()->defaultMarkType() )
      dMA->setChecked(true );

    vec[i++] = markType;
  }

  if( markMenu.actions().count() == 0 )
    return;

  if( markMenu.actions().count() > 1 )
    markMenu.addAction( i18n("Set Default Mark Type" ))->setMenu(&selectDefaultMark);

  QAction *rA = markMenu.exec( pos );
  if( !rA )
    return;
  int result=rA->data().toInt();
  if ( result > 100)
  {
     KateViewConfig::global()->setDefaultMarkType (vec[result-100]);
     // flush config, otherwise it isn't necessarily done
     KConfigGroup cg(KGlobal::config(), "Kate View Defaults");
     KateViewConfig::global()->writeConfig(cg);
  }
  else
  {
    MarkInterface::MarkTypes markType = (MarkInterface::MarkTypes) vec[result];
    if( m_doc->mark( line ) & markType ) {
      m_doc->removeMark( line, markType );
    } else {
        m_doc->addMark( line, markType );
    }
  }
}

void KateIconBorder::showAnnotationTooltip( int line, const QPoint& pos )
{
  KTextEditor::AnnotationModel *model = m_view->annotationModel() ?
    m_view->annotationModel() : m_doc->annotationModel();

  if( model )
  {
    QVariant data = model->data( line, Qt::ToolTipRole );
    QString tip = data.toString();
    if (!tip.isEmpty())
      QToolTip::showText( pos, data.toString(), this );
  }
}


int KateIconBorder::annotationLineWidth( int line )
{
  KTextEditor::AnnotationModel *model = m_view->annotationModel() ?
    m_view->annotationModel() : m_doc->annotationModel();

  if( model )
  {
    QVariant data = model->data( line, Qt::DisplayRole );
    return data.toString().length() * m_maxCharWidth + 8;
  }
  return 8;
}

void KateIconBorder::updateAnnotationLine( int line )
{
  if( annotationLineWidth(line) > m_annotationBorderWidth )
  {
    m_annotationBorderWidth = annotationLineWidth(line);
    updateGeometry();

    QTimer::singleShot( 0, this, SLOT(update()) );
  }
}

void KateIconBorder::showAnnotationMenu( int line, const QPoint& pos)
{
  KMenu menu;
  QAction a(i18n("Disable Annotation Bar"), &menu);
  menu.addAction(&a);
  emit m_view->annotationContextMenuAboutToShow( m_view, &menu, line  );
  if (menu.exec(pos) == &a)
    m_view->setAnnotationBorderVisible(false);
}

void KateIconBorder::hideAnnotationTooltip()
{
  QToolTip::hideText();
}

void KateIconBorder::updateAnnotationBorderWidth( )
{
  m_annotationBorderWidth = 6;
  KTextEditor::AnnotationModel *model = m_view->annotationModel() ?
    m_view->annotationModel() : m_doc->annotationModel();

  if( model ) {
    for( int i = 0; i < m_view->doc()->lines(); i++ ) {
      int curwidth = annotationLineWidth( i );
      if( curwidth > m_annotationBorderWidth )
        m_annotationBorderWidth = curwidth;
    }
  }

  updateGeometry();

  QTimer::singleShot( 0, this, SLOT(update()) );
}



void KateIconBorder::annotationModelChanged( KTextEditor::AnnotationModel * oldmodel, KTextEditor::AnnotationModel * newmodel )
{
  if( oldmodel )
  {
    oldmodel->disconnect( this );
  }
  if( newmodel )
  {
    connect( newmodel, SIGNAL(reset()), this, SLOT(updateAnnotationBorderWidth()) );
    connect( newmodel, SIGNAL(lineChanged( int )), this, SLOT(updateAnnotationLine( int )) );
  }
  updateAnnotationBorderWidth();
}

//END KateIconBorder

//BEGIN KateViewEncodingAction
// Acording to http://www.iana.org/assignments/ianacharset-mib
// the default/unknown mib value is 2.
#define MIB_DEFAULT 2

class KateViewEncodingAction::Private
{
  public:
    Private(KateViewEncodingAction *parent)
    : q(parent),
    currentSubAction(0)
    {
    }

    void init();

    void _k_subActionTriggered(QAction*);

    KateViewEncodingAction *q;
    QAction *currentSubAction;
};

bool lessThanAction(KSelectAction *a, KSelectAction *b)
{
  return a->text() < b->text();
}

void KateViewEncodingAction::Private::init()
{
  QList<KSelectAction *> actions;

  q->setToolBarMode(MenuMode);

  int i;
  foreach(const QStringList &encodingsForScript, KGlobal::charsets()->encodingsByScript())
  {
    KSelectAction* tmp = new KSelectAction(encodingsForScript.at(0),q);

    for (i=1; i<encodingsForScript.size(); ++i)
    {
      tmp->addAction(encodingsForScript.at(i));
    }
    q->connect(tmp,SIGNAL(triggered(QAction*)),q,SLOT(_k_subActionTriggered(QAction*)));
    //tmp->setCheckable(true);
    actions << tmp;
  }
  qSort(actions.begin(), actions.end(), lessThanAction);
  foreach (KSelectAction *action, actions)
    q->addAction(action);
}

void KateViewEncodingAction::Private::_k_subActionTriggered(QAction *action)
{
  if (currentSubAction==action)
    return;
  currentSubAction=action;
  bool ok = false;
  int mib = q->mibForName(action->text(), &ok);
  if (ok)
  {
    emit q->KSelectAction::triggered(action->text());
    emit q->triggered(q->codecForMib(mib));
  }
}

KateViewEncodingAction::KateViewEncodingAction(KateDocument *_doc, KateView *_view, const QString& text, QObject *parent)
: KSelectAction(text, parent), doc(_doc), view (_view), d(new Private(this))
{
  d->init();

  connect(menu(),SIGNAL(aboutToShow()),this,SLOT(slotAboutToShow()));
  connect(this,SIGNAL(triggered(const QString&)),this,SLOT(setEncoding(const QString&)));
}

KateViewEncodingAction::~KateViewEncodingAction()
{
    delete d;
}

void KateViewEncodingAction::slotAboutToShow()
{
  setCurrentCodec(doc->config()->encoding());
}

void KateViewEncodingAction::setEncoding (const QString &e)
{
  doc->setEncoding(e);

  view->reloadFile();

}
int KateViewEncodingAction::mibForName(const QString &codecName, bool *ok) const
{
  // FIXME logic is good but code is ugly

  bool success = false;
  int mib = MIB_DEFAULT;
  KCharsets *charsets = KGlobal::charsets();

    QTextCodec *codec = charsets->codecForName(codecName, success);
    if (!success)
    {
      // Maybe we got a description name instead
      codec = charsets->codecForName(charsets->encodingForName(codecName), success);
    }

    if (codec)
      mib = codec->mibEnum();

  if (ok)
    *ok = success;

  if (success)
    return mib;

  kWarning() << "Invalid codec name: "  << codecName;
  return MIB_DEFAULT;
}

QTextCodec *KateViewEncodingAction::codecForMib(int mib) const
{
  if (mib == MIB_DEFAULT)
  {
    // FIXME offer to change the default codec
    return QTextCodec::codecForLocale();
  }
  else
    return QTextCodec::codecForMib(mib);
}

QTextCodec *KateViewEncodingAction::currentCodec() const
{
  return codecForMib(currentCodecMib());
}

bool KateViewEncodingAction::setCurrentCodec( QTextCodec *codec )
{
  disconnect(this,SIGNAL(triggered(const QString&)),this,SLOT(setEncoding(const QString&)));

  int i,j;
  for (i=0;i<actions().size();++i)
  {
    if (actions().at(i)->menu())
    {
      for (j=0;j<actions().at(i)->menu()->actions().size();++j)
      {
        if (!j && !actions().at(i)->menu()->actions().at(j)->data().isNull())
          continue;
        if (actions().at(i)->menu()->actions().at(j)->isSeparator())
          continue;

        if (codec==KGlobal::charsets()->codecForName(actions().at(i)->menu()->actions().at(j)->text()))
        {
          d->currentSubAction=actions().at(i)->menu()->actions().at(j);
          d->currentSubAction->setChecked(true);
        }
        else
          actions().at(i)->menu()->actions().at(j)->setChecked (false);
      }
    }
  }

  connect(this,SIGNAL(triggered(const QString&)),this,SLOT(setEncoding(const QString&)));
  return true;
}

QString KateViewEncodingAction::currentCodecName() const
{
  return d->currentSubAction->text();
}

bool KateViewEncodingAction::setCurrentCodec( const QString &codecName )
{
  return setCurrentCodec(KGlobal::charsets()->codecForName(codecName));
}

int KateViewEncodingAction::currentCodecMib() const
{
  return mibForName(currentCodecName());
}

bool KateViewEncodingAction::setCurrentCodec( int mib )
{
  return setCurrentCodec(codecForMib(mib));
}
//END KateViewEncodingAction

//BEGIN KateViewBar related classes

KateViewBarWidget::KateViewBarWidget (bool addCloseButton, QWidget *parent)
 : QWidget (parent)
{
  QHBoxLayout *layout = new QHBoxLayout (this);

  // NOTE: Here be cosmetics.
  layout->setMargin(0);

  // hide button
  if (addCloseButton) {
    QToolButton *hideButton = new QToolButton(this);
    hideButton->setAutoRaise(true);
    hideButton->setIcon(KIcon("dialog-close"));
    connect(hideButton, SIGNAL(clicked()), SIGNAL(hideMe()));
    layout->addWidget(hideButton);
    layout->setAlignment( hideButton, Qt::AlignLeft|Qt::AlignTop );
  }

  // widget to be used as parent for the real content
  m_centralWidget = new QWidget (this);
  layout->addWidget(m_centralWidget);

  setLayout(layout);
  setFocusProxy(m_centralWidget);
}

KateStackedWidget::KateStackedWidget(QWidget* parent)
  : QStackedWidget(parent)
{}

QSize KateStackedWidget::sizeHint() const
{
  if (currentWidget())
    return currentWidget()->sizeHint();
  return QStackedWidget::sizeHint();
}

QSize KateStackedWidget::minimumSize() const
{
  if (currentWidget())
    return currentWidget()->minimumSize();
  return QStackedWidget::minimumSize();
}



KateViewBar::KateViewBar (bool external,KTextEditor::ViewBarContainer::Position pos,QWidget *parent, KateView *view)
 : QWidget (parent), m_external(external), m_pos(pos),m_view (view), m_permanentBarWidget(0)

{
  m_layout = new QVBoxLayout(this);
  m_stack = new KateStackedWidget(this);
  m_layout->addWidget(m_stack);
  m_layout->setMargin(0);

  m_stack->hide();
  hide ();
}

void KateViewBar::addBarWidget (KateViewBarWidget *newBarWidget)
{
  if (hasWidget(newBarWidget)) {
    kDebug(13025) << "this bar widget is already added";
    return;
  }
  // add new widget, invisible...
  newBarWidget->hide();
  m_stack->addWidget (newBarWidget);
  connect(newBarWidget, SIGNAL(hideMe()), SLOT(hideCurrentBarWidget()));

  kDebug(13025)<<"add barwidget " << newBarWidget;
}

void KateViewBar::addPermanentBarWidget (KateViewBarWidget *barWidget)
{
  // remove old widget from layout (if any)
  if (m_permanentBarWidget) {
    m_permanentBarWidget->hide();
    m_layout->removeWidget(m_permanentBarWidget);
  }

  m_layout->addWidget(barWidget, 0, Qt::AlignBottom);
  m_permanentBarWidget = barWidget;
  m_permanentBarWidget->show();

  setViewBarVisible(true);
}

void KateViewBar::removePermanentBarWidget (KateViewBarWidget *barWidget)
{
  if (m_permanentBarWidget != barWidget) {
    kDebug(13025) << "no such permanent widget exists in bar";
    return;
  }

  if (!m_permanentBarWidget)
    return;

  m_permanentBarWidget->hide();
  m_layout->removeWidget(m_permanentBarWidget);
  m_permanentBarWidget = 0;

  if (!m_stack->isVisible()) {
    setViewBarVisible(false);
  }
}

bool KateViewBar::hasPermanentWidget (KateViewBarWidget *barWidget ) const
{
    return (m_permanentBarWidget == barWidget);
}

void KateViewBar::showBarWidget (KateViewBarWidget *barWidget)
{
  // raise correct widget
  m_stack->setCurrentWidget (barWidget);
  barWidget->show();
  m_stack->show();

  // if we have any permanent widget, bar is always visible,
  // no need to show it
  if (!m_permanentBarWidget) {
    setViewBarVisible(true);
  }
}

bool KateViewBar::hasWidget(KateViewBarWidget* wid) const
{
    int count = m_stack->count();
    for (int i=0; i<count; ++i) {
        if (m_stack->widget(i) == wid) {
            return true;
        }
    }
    return false;
}

void KateViewBar::hideCurrentBarWidget ()
{
  KateViewBarWidget *current=qobject_cast<KateViewBarWidget*>(m_stack->currentWidget());
  if (current) {
    current->closed();
  }
  m_stack->hide();

  // if we have any permanent widget, bar is always visible,
  // no need to hide it
  if (!m_permanentBarWidget) {
    setViewBarVisible(false);
  }

  m_view->setFocus();
  kDebug(13025)<<"hide barwidget";
}

void KateViewBar::setViewBarVisible (bool visible)
{
  if (m_external) {
    KTextEditor::ViewBarContainer *viewBarContainer=qobject_cast<KTextEditor::ViewBarContainer*>( KateGlobal::self()->container() );
    if (viewBarContainer) {
      if (visible) {
        viewBarContainer->showViewBarForView(m_view,m_pos);
      } else {
        viewBarContainer->hideViewBarForView(m_view,m_pos);
      }
    }
  } else {
    setVisible (visible);
  }
}

void KateViewBar::keyPressEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_Escape) {
    hideCurrentBarWidget();
    return;
  }
  QWidget::keyPressEvent(event);

}

void KateViewBar::hideEvent(QHideEvent* event)
{
  Q_UNUSED(event);
//   if (!event->spontaneous())
//     m_view->setFocus();
}

//END KateViewBar related classes

#include "kateviewhelpers.moc"

// kate: space-indent on; indent-width 2; replace-tabs on;

