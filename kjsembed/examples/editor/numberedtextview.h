// -*- c++ -*-
#ifndef NUMBERED_TEXT_VIEW_H
#define NUMBERED_TEXT_VIEW_H

#include <QFrame>
#include <QPixmap>
#include <QTextCursor>

class QTextEdit;
class QHBoxLayout;

/**
 * @internal Used to display the numbers.
 */
class NumberBar : public QWidget
{
    Q_OBJECT

public:
    NumberBar( QWidget *parent );
    ~NumberBar();

    void setCurrentLine( int lineno );
    void setStopLine( int lineno );
    void setBugLine( int lineno );

    void setTextEdit( QTextEdit *edit );
    void paintEvent( QPaintEvent *ev );

protected:
    bool event( QEvent *ev );

private:
    QTextEdit *edit;
    QPixmap stopMarker;
    QPixmap currentMarker;
    QPixmap bugMarker;
    int stopLine;
    int currentLine;
    int bugLine;
    QRect stopRect;
    QRect currentRect;
    QRect bugRect;
};

/**
 * Displays a QTextEdit with line numbers.
 */
class NumberedTextView : public QFrame
{
    Q_OBJECT

public:
    NumberedTextView( QWidget *parent = 0 );
    ~NumberedTextView();

    /** Returns the QTextEdit of the main view. */
    QTextEdit *textEdit() const { return view; }

    /**
     * Sets the line that should have the current line indicator.
     * A value of -1 indicates no line should show the indicator.
     */
    void setCurrentLine( int lineno );

    /**
     * Sets the line that should have the stop line indicator.
     * A value of -1 indicates no line should show the indicator.
     */
    void setStopLine( int lineno );

    /**
     * Sets the line that should have the bug line indicator.
     * A value of -1 indicates no line should show the indicator.
     */
    void setBugLine( int lineno );

    /** @internal Used to get tooltip events from the view for the hover signal. */
    bool eventFilter( QObject *obj, QEvent *event );

signals:
    /**
     * Emitted when the mouse is hovered over the text edit component.
     * @param word The word under the mouse pointer
     */ 
    void mouseHover( const QString &word );

    /**
     * Emitted when the mouse is hovered over the text edit component.
     * @param pos The position of the mouse pointer.
     * @param word The word under the mouse pointer
     */ 
    void mouseHover( const QPoint &pos, const QString &word );

protected slots:
    /** @internal Used to update the highlight on the current line. */
    void textChanged( int pos, int added, int removed ); 

private:
    QTextEdit *view;
    NumberBar *numbers;
    QHBoxLayout *box;
    int currentLine;
    QTextCursor highlight;
};


#endif // NUMBERED_TEXT_VIEW_H


