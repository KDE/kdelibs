/* This file is part of the KDE project

   Copyright (C) 1997 Martin Jones (mjones@kde.org)
             (C) 1998 Waldo Bastian (bastian@kde.org)
             (C) 1998, 1999 Torben Weis (weis@kde.org)
             (C) 1999 Lars Knoll (knoll@kde.org)
             (C) 1999 Antti Koivisto (koivisto@kde.org)

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

#ifndef KHTMLVIEW_H
#define KHTMLVIEW_H

// qt includes and classes
#include <q3scrollview.h>

#include <kdelibs_export.h>

class QPainter;
class QRect;
template< typename T > class QVector;

namespace DOM {
    class HTMLDocumentImpl;
    class DocumentImpl;
    class ElementImpl;
    class HTMLElementImpl;
    class HTMLTitleElementImpl;
    class HTMLGenericFormElementImpl;
    class HTMLFormElementImpl;
    class HTMLAnchorElementImpl;
    class HTMLInputElementImpl;
    class Range;
    class NodeImpl;
    class CSSProperty;
}

namespace KJS {
    class WindowFunc;
    class ExternalFunc;
}

namespace khtml {
    class RenderObject;
    class RenderCanvas;
    class RenderStyle;
    class RenderLineEdit;
    class RenderPartObject;
    class RenderWidget;
    class CSSStyleSelector;
    class LineEditWidget;
    class CaretBox;
    void applyRule(DOM::CSSProperty *prop);
}

class KHTMLPart;
class KHTMLViewPrivate;

/**
 * Renders and displays HTML in a QScrollView.
 *
 * Suitable for use as an application's main view.
 **/
class KHTML_EXPORT KHTMLView : public Q3ScrollView
{
    Q_OBJECT

    friend class DOM::HTMLDocumentImpl;
    friend class DOM::HTMLTitleElementImpl;
    friend class DOM::HTMLGenericFormElementImpl;
    friend class DOM::HTMLFormElementImpl;
    friend class DOM::HTMLAnchorElementImpl;
    friend class DOM::HTMLInputElementImpl;
    friend class DOM::DocumentImpl;
    friend class KHTMLPart;
    friend class khtml::RenderCanvas;
    friend class khtml::RenderObject;
    friend class khtml::RenderLineEdit;
    friend class khtml::RenderPartObject;
    friend class khtml::RenderWidget;
    friend class khtml::CSSStyleSelector;
    friend class khtml::LineEditWidget;
    friend class KJS::WindowFunc;
    friend class KJS::ExternalFunc;
    friend void khtml::applyRule(DOM::CSSProperty *prop);


public:
    /**
     * Constructs a KHTMLView.
     */
    KHTMLView( KHTMLPart *part, QWidget *parent, const char *name=0 );
    virtual ~KHTMLView();

    /**
     * Returns a pointer to the KHTMLPart that is
     * rendering the page.
     **/
    KHTMLPart *part() const { return m_part; }

    int frameWidth() const { return _width; }

    /**
     * Sets a margin in x direction.
     */
    void setMarginWidth(int x);

    /**
     * Returns the margin width.
     *
     * A return value of -1 means the default value will be used.
     */
    int marginWidth() const { return _marginWidth; }

    /*
     * Sets a margin in y direction.
     */
    void setMarginHeight(int y);

    /**
     * Returns the margin height.
     *
     * A return value of -1 means the default value will be used.
     */
    int marginHeight() { return _marginHeight; }

    /**
     * Sets verticals scrollbar mode. Reimplemented for internal reasons.
     */
    virtual void setVScrollBarMode ( ScrollBarMode mode );

    /**
     * Sets horizontal scrollbar mode. Reimplemented for internal reasons.
     */
    virtual void setHScrollBarMode ( ScrollBarMode mode );

    /**
     * Prints the HTML document.
     * @param quick if true, fully automated printing, without print dialog
     */
    void print( bool quick = false ); 

    /**
     * ensure the display is up to date
     */
    void layout(); // KDE 4.0: make private
    /**
     * Display all accesskeys in small tooltips
     */
    void displayAccessKeys();



signals:
    void finishedLayout();
    void cleared();
    void zoomView( int );
    void hideAccessKeys();
    void repaintAccessKeys();
    void findAheadActive( bool );

protected:
    void clear();

    virtual bool event ( QEvent * event );
    virtual void resizeEvent ( QResizeEvent * event );
    virtual void showEvent ( QShowEvent * );
    virtual void hideEvent ( QHideEvent *);
    virtual bool focusNextPrevChild( bool next );
    virtual void drawContents ( QPainter * p, int clipx, int clipy, int clipw, int cliph );
    virtual void drawContents( QPainter* );
    virtual void viewportMousePressEvent( QMouseEvent * );
    virtual void focusInEvent( QFocusEvent * );
    virtual void focusOutEvent( QFocusEvent * );
    virtual void viewportMouseDoubleClickEvent( QMouseEvent * );
    virtual void viewportMouseMoveEvent(QMouseEvent *);
    virtual void viewportMouseReleaseEvent(QMouseEvent *);
    virtual void viewportResizeEvent(QResizeEvent*);
#ifndef QT_NO_WHEELEVENT
    virtual void viewportWheelEvent(QWheelEvent*);
#endif
    virtual void dragEnterEvent( QDragEnterEvent* );
    virtual void dropEvent( QDropEvent* );
    virtual void closeEvent ( QCloseEvent * );
    virtual bool eventFilter(QObject *, QEvent *);

    void keyPressEvent( QKeyEvent *_ke );
    void keyReleaseEvent ( QKeyEvent *_ke );
    void contentsContextMenuEvent ( QContextMenuEvent *_ce );
    void doAutoScroll();
    void timerEvent ( QTimerEvent * );
protected slots:
    void slotPaletteChanged();
    void slotScrollBarMoved();

private slots:
    void tripleClickTimeout();
    void findTimeout();
    void accessKeysTimeout();

    /**
    * @internal
    * @since 3.4
    * used for autoscrolling with MMB
    */
    void slotMouseScrollTimer();

private:

    void scheduleRelayout(khtml::RenderObject* clippedObj=0);
    void unscheduleRelayout();

    void scheduleRepaint(int x, int y, int w, int h, bool asap=false);
    void unscheduleRepaint();

    bool needsFullRepaint() const;

    void closeChildDialogs();
    bool dialogsAllowed();

    /**
     * Paints the HTML document to a QPainter.
     * The document will be scaled to match the width of
     * rc and clipped to fit in the height.
     * yOff determines the vertical offset in the document to start with.
     * more, if nonzero will be set to true if the documents extends
     * beyond the rc or false if everything below yOff was painted.
     **/
    void paint(QPainter *p, const QRect &rc, int yOff = 0, bool *more = 0);

#if 0
    /**
     * Paints the HTML document to a QPainter.
     * The document will be scaled to match the width.
     **/
    void paint(QPainter *p, int width);
#endif

    /**
     * Get/set the CSS Media Type.
     *
     * Media type is set to "screen" for on-screen rendering and "print"
     * during printing. Other media types lack the proper support in the
     * renderer and are not activated. The DOM tree and the parser itself,
     * however, properly handle other media types. To make them actually work
     * you only need to enable the media type in the view and if necessary
     * add the media type dependent changes to the renderer.
     */
    void setMediaType( const QString &medium );
    QString mediaType() const;

    bool pagedMode() const;

    bool scrollTo(const QRect &);

    bool focusNextPrevNode(bool next);
    bool handleAccessKey(const QKeyEvent* ev);
    bool focusNodeWithAccessKey(QChar c, KHTMLView* caller = NULL);
    QMap< DOM::ElementImpl*, QChar > buildFallbackAccessKeys() const;
    void displayAccessKeys( KHTMLView* caller, KHTMLView* origview, QVector< QChar >& taken, bool use_fallbacks );

    void useSlowRepaints();

    void setIgnoreWheelEvents(bool e);

    void init();

    DOM::NodeImpl *nodeUnderMouse() const;
    DOM::NodeImpl *nonSharedNodeUnderMouse() const;

    void restoreScrollBar();

    QStringList formCompletionItems(const QString &name) const;
    void clearCompletionHistory(const QString& name);
    void addFormCompletionItem(const QString &name, const QString &value);

    void addNonPasswordStorableSite( const QString& host );
    bool nonPasswordStorableSite( const QString& host ) const;

    bool dispatchMouseEvent(int eventId, DOM::NodeImpl *targetNode,
			    DOM::NodeImpl *targetNodeNonShared, bool cancelable,
			    int detail,QMouseEvent *_mouse, bool setUnder,
			    int mouseEventType);
    bool dispatchKeyEvent( QKeyEvent *_ke );
    bool dispatchKeyEventHelper( QKeyEvent *_ke, bool generate_keypress );

    void complete( bool pendingAction );

#ifndef KHTML_NO_TYPE_AHEAD_FIND
    void findAhead(bool increase);
    void updateFindAheadTimeout();
    void startFindAhead( bool linksOnly );
#endif // KHTML_NO_TYPE_AHEAD_FIND

#ifndef KHTML_NO_CARET
    // -- caret-related member functions (for caretMode as well as designMode)

    /** initializes the caret if it hasn't been initialized yet.
     *
     * This method determines a suitable starting position, initializes
     * the internal structures, and calculates the caret's coordinates ready
     * for display.
     *
     * To "deinitialize" the caret, call caretOff
     * @param keepSelection @p true to keep any active selection. It may have
     *         been extended if the caret position is changed.
     */
    void initCaret(bool keepSelection = false);
    /** returns whether the text under the caret will be overridden.
      */
    bool caretOverrides() const;
    /** ensures that the given element is properly focused.
     *
     * If not in caret mode or design mode, keyboard events are only regarded for
     * focused nodes. Therefore, the function ensured that the focus will be
     * properly set on unfocused nodes (or on a suitable ancestor).
     * @param node node to focus
     */
    void ensureNodeHasFocus(DOM::NodeImpl *node);
    /** inquires the current caret position and stores it in the caret view
     * context. Also resets the blink frequency timer. It will not display
     * the caret on the canvas.
     * @param hintBox caret box whose coordinates will be used if the
     * caret position could not be determined otherwise.
     */
    void recalcAndStoreCaretPos(khtml::CaretBox *hintBox = 0);
    /** displays the caret and reinitializes the blink frequency timer.
     *
     * The caret will only be displayed on screen if the view has focus, or
     * the caret display policy allows it. The same holds true for the blink
     * frequency timer.
     */
    void caretOn();
    /** hides the caret and kills the blink frequency timer.
     *
     * These operations are executed unconditionally, regardless of the
     * focus, and the caret display policy.
     */
    void caretOff();
    /** makes the caret visible, but does not influence the frequency timer.
     * That means it probably won't get visible immediately.
     *
     * These operations are executed unconditionally, regardless of the
     * focus, and the caret display policy.
     * @param forceRepaint @p true to force an immediate repaint, otherwise
     *		do a scheduled repaint
     */
    void showCaret(bool forceRepaint = false);
    /** makes the caret invisible, but does not influence the frequency timer.
     * The caret is immediately hidden.
     *
     * These operations are executed unconditionally, regardless of the
     * focus, and the caret display policy.
     */
    void hideCaret();
    /** shifts the viewport to ensure that the caret is visible.
     *
     * Note: this will also work if the caret is hidden.
     */
    void ensureCaretVisible();

    /** folds the selection to the current caret position.
     *
     * Whatever selection has existed before will be removed by the invocation
     * of this method. Updates are only done if an actual selection has
     * been folded. After the call of this method, no selection will exist
     * any more.
     *
     * No validity checking is done on the parameters. Note that the parameters
     * refer to the old selection, the current caret may be way off.
     * @param startNode starting node of selection
     * @param startOffset offset within the start node.
     * @param endNode ending node of selection
     * @param endOffset offset within the end node.
     * @return @p true if there had been a selection, and it was folded.
     */
    bool foldSelectionToCaret(DOM::NodeImpl *startNode, long startOffset,
    				DOM::NodeImpl *endNode, long endOffset);

    /** places the caret on the current position.
     *
     * The caret is switched off, the position recalculated with respect to
     * the new position. The caret will only be redisplayed if it is on an
     * editable node, in design mode, or in caret mode.
     * @param hintBox caret box whose coordinates will be used if the
     * caret position could not be determined otherwise.
     * @return @p true if the caret has been displayed.
     */
    bool placeCaret(khtml::CaretBox *hintBox = 0);

    /** extends the selection up to the current caret position.
     *
     * When a selection exists, the function adds/removes pieces from the
     * beginning/end of the selection up to the current caret position.
     *
     * The selection values are *not* normalized, i. e. the resulting end
     * position may actually precede the starting position.
     *
     * No validity checking is done on the parameters. Note that the parameters
     * refer to the old selection, the current caret may be way off.
     * @param startNode starting node of selection
     * @param startOffset offset within the start node.
     * @param endNode ending node of selection
     * @param endOffset offset within the end node.
     * @return @p true when the current selection has been changed
     */
    bool extendSelection(DOM::NodeImpl *startNode, long startOffset,
				DOM::NodeImpl *endNode, long endOffset);

    /** updates the selection from the last to the current caret position.
     *
     * No validity checking is done on the parameters. Note that the parameters
     * refer to the old selection, the current caret may be way off.
     * @param startNode starting node of selection
     * @param startOffset offset within the start node.
     * @param endNode ending node of selection
     * @param endOffset offset within the end node.
     */
    void updateSelection(DOM::NodeImpl *startNode, long startOffset,
			DOM::NodeImpl *endNode, long endOffset);

    /**
     * Returns the current caret policy when the view is not focused.
     * @return a KHTMLPart::CaretDisplay value
     */
    int caretDisplayPolicyNonFocused() const;

    /**
     * Sets the caret display policy when the view is not focused.
     * @param policy new display policy as
     *		defined by KHTMLPart::CaretDisplayPolicy
     * @since 3.2
     */
    void setCaretDisplayPolicyNonFocused(int policy);

    // -- caret event handler

    /**
     * Evaluates key presses on editable nodes.
     */
    void caretKeyPressEvent(QKeyEvent *);

    // -- caret navigation member functions

    /** moves the caret to the given position and displays it.
     *
     * If the node resembles an invalid position, the function sets the caret to a
     * nearby node that is valid.
     *
     * @param node node to be set to
     * @param offset zero-based offset within this node
     * @param clearSelection @p true if any selection should be cleared
     *	as well.
     * @return @p true if a previously existing selection has been changed.
     */
    bool moveCaretTo(DOM::NodeImpl *node, long offset, bool clearSelection);

    /**
     * Movement enumeration
     * @li CaretByCharacter move caret character-wise
     * @li CaretByWord move caret word-wise
     */
    enum CaretMovement { CaretByCharacter, CaretByWord };

    /** moves the caret.
     *
     * @param next @p true, move towards the following content, @p false,
     *		move towards the previous
     * @param cmv CaretMovement operation
     * @param n count the CaretMovement has to be carried out.
     * For latin documents, a positive number means moving the caret
     * these many characters to the right/downwards, a negative number
     * to the left/upwards. For RTL documents, the opposite applies.
     */
    void moveCaretBy(bool next, CaretMovement cmv, int n);

    /** moves the caret by line.
     */
    void moveCaretByLine(bool next, int n);

    /** moves the caret to the given line boundary.
     * @param end @p true if the caret is to be moved to the end of the line,
     *		otherwise to the beginning.
     */
    void moveCaretToLineBoundary(bool end);

    /** moves the caret to the given document boundary.
     * @param end @p true if the caret is to be moved to the end of the
     *		document, otherwise to the beginning.
     */
    void moveCaretToDocumentBoundary(bool end);

    /** does the actual caret placement so that it becomes visible at
     * the new position.
     *
     * This method is only suitable if the new caret position has already been
     * determined.
     * @param hintBox caret box whose coordinates will be used if the
     * caret position could not be determined otherwise.
     */
    void placeCaretOnChar(khtml::CaretBox *hintBox);

    /** does the actual caret placement so that it becomes visible at
     * the new position.
     *
     * Additionally, it calculates the new caret position from the given
     * box and coordinates.
     *
     * @param caretBox caret box serving as a measurement point for offset.
     * @param x x-coordinate relative to containing block. The offset will
     *		be approximated as closely as possible to this coordinate,
     *		but never less than caretBox->xPos() and greater than
     *		caretBox->xPos() + caretBox->width()
     * @param absx absolute x-coordinate of containing block, needed for
     *		calculation of final caret position
     * @param absy absolute y-coordinate of containing block, needed for
     *		calculation of final caret position
     */
    void placeCaretOnLine(khtml::CaretBox *caretBox, int x, int absx, int absy);

    /** moves the caret by a page length.
     * @param next @p true, move down, @p false, move up.
     */
    void moveCaretByPage(bool next);

    /** moves the caret to the beginning of the previous word.
     */
    void moveCaretPrevWord();

    /** moves the caret to the beginning of the next word.
     */
    void moveCaretNextWord();

    /** moves the caret to the previous line.
     *
     * @param n number of lines to move caret
     */
    void moveCaretPrevLine(int n = 1);

    /** moves the caret to the following line.
     *
     * @param n number of lines to move caret
     */
    void moveCaretNextLine(int n = 1);

    /** moves the caret to the previous page
     */
    void moveCaretPrevPage();

    /** moves the caret to the next page
     */
    void moveCaretNextPage();

    /** moves the caret to the beginning of the current line.
     */
    void moveCaretToLineBegin();

    /** moves the caret to the end of the current line.
     */
    void moveCaretToLineEnd();

#endif // KHTML_NO_CARET

    // ------------------------------------- member variables ------------------------------------
 private:

    void setWidgetVisible(::khtml::RenderWidget*, bool visible);

    int _width;
    int _height;

    int _marginWidth;
    int _marginHeight;

    KHTMLPart *m_part;
    KHTMLViewPrivate* const d;

    QString m_medium;   // media type
};

#endif

