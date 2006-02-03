#include <QTextDocument>
#include <QTextBlock>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QPainter>
#include <QAbstractTextDocumentLayout>
#include <QToolTip>
#include <qdebug.h>

#include "numberedtextview.h"

NumberBar::NumberBar( QWidget *parent )
    : QWidget( parent ), edit(0), m_stopLine(-1), m_currentLine(-1), m_bugLine(-1)
{
    setFixedWidth( fontMetrics().width( QString("0000") + 10 + 32 ) );
    stopMarker = QPixmap( ":/images/no.png" );
    currentMarker = QPixmap( ":/images/next.png" );
    bugMarker = QPixmap( ":/images/bug.png" );
}

NumberBar::~NumberBar()
{
}

void NumberBar::setCurrentLine( int lineno )
{
    m_currentLine = lineno;
}

void NumberBar::setStopLine( int lineno )
{
    m_stopLine = lineno;
}

void NumberBar::setBugLine( int lineno )
{
    m_bugLine = lineno;
}

int NumberBar::currentLine() const
{
	return m_currentLine;
}

int NumberBar::stopLine() const
{
	return m_stopLine;
}

int NumberBar::bugLine() const
{
	return m_bugLine;
}

void NumberBar::setTextEdit( QTextEdit *edit )
{
    this->edit = edit;
    connect( edit->document()->documentLayout(), SIGNAL( update(const QRectF &) ),
	     this, SLOT( update() ) );
    connect( edit->verticalScrollBar(), SIGNAL(valueChanged(int) ),
	     this, SLOT( update() ) );
}

void NumberBar::paintEvent( QPaintEvent * )
{
    QAbstractTextDocumentLayout *layout = edit->document()->documentLayout();
    int contentsY = edit->verticalScrollBar()->value();
    qreal pageBottom = contentsY + edit->viewport()->height();
    const QFontMetrics fm = fontMetrics();
    const int ascent = fontMetrics().ascent() + 1; // height = ascent + descent + 1
    int lineCount = 1;

    QPainter p(this);

    bugRect = QRect();
    stopRect = QRect();
    currentRect = QRect();

    for ( QTextBlock block = edit->document()->begin();
	  block.isValid(); block = block.next(), ++lineCount ) {

        const QRectF boundingRect = layout->blockBoundingRect( block );

        QPointF position = boundingRect.topLeft();
        if ( position.y() + boundingRect.height() < contentsY )
            continue;
        if ( position.y() > pageBottom )
            break;

        const QString txt = QString::number( lineCount );
        p.drawText( width() - fm.width(txt), qRound( position.y() ) - contentsY + ascent, txt );

	// Bug marker
	if ( m_bugLine == lineCount ) {
	    p.drawPixmap( 1, qRound( position.y() ) - contentsY, bugMarker );
	    bugRect = QRect( 1, qRound( position.y() ) - contentsY, bugMarker.width(), bugMarker.height() );
	}

	// Stop marker
	if ( m_stopLine == lineCount ) {
	    p.drawPixmap( 1, qRound( position.y() ) - contentsY, stopMarker );
	    stopRect = QRect( 1, qRound( position.y() ) - contentsY, stopMarker.width(), stopMarker.height() );
	}

	// Current line marker
	if ( m_currentLine == lineCount ) {
	    p.drawPixmap( 1, qRound( position.y() ) - contentsY, currentMarker );
	    currentRect = QRect( 1, qRound( position.y() ) - contentsY, currentMarker.width(), currentMarker.height() );
	}
    }
}

bool NumberBar::event( QEvent *event )
{
    if ( event->type() == QEvent::ToolTip ) {
	QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);

	if ( stopRect.contains( helpEvent->pos() ) ) {
	    QToolTip::showText( helpEvent->globalPos(), "Stop Here" );
	}
	else if ( currentRect.contains( helpEvent->pos() ) ) {
	    QToolTip::showText( helpEvent->globalPos(), "Current Line" );
	}
	else if ( bugRect.contains( helpEvent->pos() ) ) {
	    QToolTip::showText( helpEvent->globalPos(), "Error Line" );
	}
    }

    return QWidget::event(event);
}

NumberedTextView::NumberedTextView( QWidget *parent )
    : QFrame( parent )
{
    setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    setLineWidth( 2 );

    // Setup the main view
    view = new QTextEdit( this );
    view->setFontFamily( "Monospace" );
    view->setLineWrapMode( QTextEdit::NoWrap );
    view->setFrameStyle( QFrame::NoFrame );
    view->installEventFilter( this );

    connect( view->document(), SIGNAL(contentsChange(int,int,int)), this, SLOT(textChanged(int,int,int)) );

    // Setup the line number pane
    numbers = new NumberBar( this );
    numbers->setTextEdit( view );
    // Testing...
    numbers->setStopLine( 3 );
    numbers->setBugLine( 1 );
    setCurrentLine( 5 );

    box = new QHBoxLayout( this );
    box->setSpacing( 0 );
    box->setMargin( 0 );
    box->addWidget( numbers );
    box->addWidget( view );
}

NumberedTextView::~NumberedTextView()
{
}

void NumberedTextView::setCurrentLine( int lineno )
{
    numbers->setCurrentLine( lineno );
    textChanged( 0, 0, 1 );
}

void NumberedTextView::setStopLine( int lineno )
{
    numbers->setStopLine( lineno );
}

void NumberedTextView::setBugLine( int lineno )
{
    numbers->setBugLine( lineno );
}

int NumberedTextView::currentLine() const
{
	return numbers->currentLine();
}

int NumberedTextView::stopLine() const
{
	return numbers->stopLine();
}

int NumberedTextView::bugLine() const
{
	return numbers->bugLine();
}

QString NumberedTextView::text() const
{
	return view->toPlainText ();
}

void NumberedTextView::setText( const QString &text )
{
	view->setPlainText(text);
}


void NumberedTextView::textChanged( int pos, int removed, int added )
{
    Q_UNUSED( pos );

    if ( removed == 0 && added == 0 )
	return;

    QTextBlock block = highlight.block();
    QTextBlockFormat fmt = block.blockFormat();
    QColor bg = view->palette().base().color();
    fmt.setBackground( bg );
    highlight.setBlockFormat( fmt );

    int lineCount = 1;
    for ( QTextBlock block = view->document()->begin();
	  block.isValid(); block = block.next(), ++lineCount ) {

	if ( lineCount == numbers->currentLine() ) {
	    fmt = block.blockFormat();
	    QColor bg = view->palette().highlight().color().light( 175 );
	    fmt.setBackground( bg );

	    highlight = QTextCursor( block );
	    highlight.movePosition( QTextCursor::EndOfBlock, QTextCursor::KeepAnchor );
	    highlight.setBlockFormat( fmt );

	    break;
	}
    }
}

bool NumberedTextView::eventFilter( QObject *obj, QEvent *event )
{
    if ( obj != view )
	return QFrame::eventFilter(obj, event);

    if ( event->type() == QEvent::ToolTip ) {
	QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);

	QTextCursor cursor = view->cursorForPosition( helpEvent->pos() );
	cursor.movePosition( QTextCursor::StartOfWord, QTextCursor::MoveAnchor );
	cursor.movePosition( QTextCursor::EndOfWord, QTextCursor::KeepAnchor );

	QString word = cursor.selectedText();
	emit mouseHover( word );
	emit mouseHover( helpEvent->pos(), word );

	// QToolTip::showText( helpEvent->globalPos(), word ); // For testing
    }

    return false;
}
