/* This file is part of the KDE libraries
    Copyright (C) 1998 Jorge Monteiro <jomo@casema.net>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef __KCONTAINER_H__
#define __KCONTAINER_H__

#include <qframe.h>
#include <qlist.h>

/**
 * Alternative Layout manager widget.
 * Here is an example of using this class:
 *
 * @author Jorge Monteiro <jomo@casema.net>
 * @version 0.1
 */
class KContainerLayout : public QFrame
{
Q_OBJECT
public:
enum { Horizontal = 0, Vertical };
    /**
     * Constructs a KContainerLayout as child of parent
     * 
     * @param parent The parent widget of the KContainerLayout
     * @param name The name of the widget
     * @param orientation The orientation of the container, either
     * KContainerLayout::Horizontal or KContainer::Vertical
     * @param homogeneous If the container should split available size
     * by all KContainerLayoutItem in equal parts
     * @param spacing The space to add between each widget and between
     * the first/last and the borders
     * @param f Flags (see @ref QFrame#QFrame)
     * @param allowLines Flags (see @ref QFrame#QFrame)
     */
    KContainerLayout(QWidget * parent=0, const char * name=0, 
    		int orientation = KContainerLayout::Horizontal,
		bool homogeneos = FALSE,
		int spacing = 5, WFlags f=0, bool allowLines=TRUE);
    /**
     * The destructor
     */
    virtual ~KContainerLayout();
    /**
     * Returns the number of widgets inside this container
     */
    int getNumberOfWidgets() const;
    /**
     * Pack one widget to the start of the container after the previously packed on start widgets
     * @param w The widget to be packed
     * @param e If the widget should use the whole size allocated to it
     * @param f If the widget should be sized to fill the whole space allocated to it (only makes sense with Expand=TRUE)
     * @param p Padding that should be used as the borders in each side of the widget
     */
    int packStart(QWidget *w, bool e=FALSE, bool f=FALSE,int p=1);
    /**
     * Pack one widget to the end of the container after the previously packed on end widgets
     * @param w The widget to be packed
     * @param e If the widget should use the whole size allocated to it
     * @param f If the widget should be sized to fill the whole space allocated to it (only makes sense with Expand=TRUE)
     * @param p Padding that should be used as the borders in each side of the widget
     */
    int packEnd(QWidget *w, bool e=FALSE, bool f=FALSE,int p=1);
    /**
     * Sets the orientation of the container, one of KContainerLayout::Horizontal or KContainer::Vertical
     */
    void setOrientation(int i);
    /**
     * Sets the container to use the same size to each widget he contains (TRUE) or not (FALSE)
     * When homogeneous is true all widgets will be packed as if they had the Expand set to TRUE @see KContainerLayout#packStart @see KContainer#packEnd
     */
    void setHomogeneos(bool b);
    /*
     * Sets the space to be used between each widget and between the first/last widget and the container borders
     */
    void setSpacing(int i);
    /**
     * Sets the starting offset for this container @see _startOffset
     */
    void setStartOffset(int i);
    /**
     * Sets the ending offset for this container @see _endOffset
     */
    void setEndOffset(int i);
    /**
     * Returns the current orientation of the container @see
     * KContainerLayout#setOrientation
     */
    const int orientation() const	{ return _orientation; }
    /**
     * Returns the current homogeneous state of the container @see
     * KContainerLayout#setHomogeneous
     */
    const bool homogeneos() const	{ return _homogeneos; }
    /**
     * Returns the current spacing of the container @see
     * KContainerLayout#setSpacing
     */
    const int spacing() const		{ return _spacing; }
    /**
     * Returns the starting offset for this container @see _startOffset
     */
    const int startOffset() const	{ return _startOffset; }
    /**
     * Returns the ending offset for this container @see _endOffset
     */
    const int endOffset() const		{ return _endOffset; }
    /**
     * Resizes the container to be as small as necessary to display
     * all widgets
     */
    void sizeToFit();
protected:

    /**
     * This class represents one widget inside the one KContainerLayout.
     * 
     * @internal
     * @author Jorge Monteiro <jomo@casema.net>
     * @version 0.1
     * @see KContainerLayout
     */
    class KContainerLayoutItem
    {
    public:
	    /**
	     * Creates one KContIem
	     *
	     * @param w The widget associated with this KContainerLayoutItem
	     * @param e Expand will make the widget to use all space
	     * alocated to it
	     * @param f Fill will make the widget be sized to ocupy all the
	     * space allocated to it. Only makes sense with expand = TRUE
	     * @param p Padding is the size the widget will use as borders on
	     * both sides of the space allocated to it.
	     */
	    KContainerLayoutItem(QWidget *w,bool e=FALSE,bool f=FALSE,int p=0)
	    {
		    _widget = w;
		    _expand = e;
		    _fill = f;
		    _padding = p;
	    }
	    void setExpand(bool b)		{ _expand = b; }
	    void setFill(bool b)		{ _fill = b; }
	    void setPadding(int i)		{ _padding = i; }
	    QWidget *widget()			{ return _widget; }
	    const bool expand() const		{ return _expand; }
	    const bool fill() const		{ return _fill; }
	    const int padding() const		{ return _padding; }
    protected:
	    QWidget *_widget;
	    bool _expand;
	    bool _fill;
	    int _padding;
    };

    /**
     * Calculates the size necessary to display all widgets
     */
    void recalcLayout();
    /**
     * Returns the size necessary for the widget represented by this
     * KContainerLayoutItem
     */
    QSize widgetSize(KContainerLayoutItem *item)
    {
	QSize sz = item->widget()->sizeHint();
	if (!sz.isValid())
	   // well, some widgets will not return sizeHint()
	    sz = QSize(50,25); 
	return sz;
    }
    /**
     * Reposition all widgets on the container.
     */
    void repositionWidgets();
    /**
     * Returns the number of widgets that share the extra space on
     * the container.
     */
    int numberOfWidgetsWithExpand();
    /**
     * Calculate our size hint based on the sizeHint() of all out
     * widgets, on our properties - expand and homogeneous, and on the
     * KContainerLayoutItem properties.
     * @see KContainerLayoutItem @see packStart @see packEnd
     */
    void calculateSizeHint();
    /**
     * Return the size necessary by the largest widget on the container.
     */
    QSize sizeOfLargerWidget();
    /**
     * Returns the ideal size for the widget represented by this
     * KContainerLayoutItem.
     */
    QSize idealSizeOfWidget(KContainerLayoutItem *item);
    /**
     * Return TRUE if this is an horizontal container.
     */
    const bool horizontal() const 	
	{ return (_orientation == KContainerLayout::Horizontal); }
    /**
     * Resizes the widget and repositions all widgets.
     */
    virtual void resizeEvent(QResizeEvent *ev);
    /**
     * Used to filter resize events from our parent if it isn't a
     * KContainerLayout.
     */
    virtual bool eventFilter(QObject *, QEvent *ev);
    /**
     * Returns our size hint. The size necessary to display this container.
     */
    virtual QSize sizeHint() const;
    /**
     * Keeps the orientation of the container one of
     * KContainerLayout::Horizontal or KContainer::Vertical
     */
    int _orientation;
    /**
     * Should we split our size in equal parts by all the widgets?
     */ 
    bool _homogeneos;
    /**
     * Space to be used between widgets and between the first/last widget and
     * the container borders.
     */
    int _spacing;
    /**
     * Space between the starting border and the first widget
     */
    int _startOffset;
    /**
     * Space between the last widget and ending border
     */
    int _endOffset;
    /**
     * The list of all widgets packed on the start
     */
    QList<KContainerLayoutItem> _startWidgets;
    /**
     * The list of all widgets packed at the end
     */
    QList<KContainerLayoutItem> _endWidgets;
    /**
     * width or height we have for each widget
     */
    int _sizeForEach;	
    /**
     * our sizeHint that we will return on our implementation of sizeHint()
     */
    QSize _sizeHint;

};

#endif // __KCONTAINER_H__
