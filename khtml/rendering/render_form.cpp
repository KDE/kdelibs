/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */

#include <kdebug.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kcompletionbox.h>
#include <kcursor.h>

#include <qstyle.h>

#include "misc/helper.h"
#include "xml/dom2_eventsimpl.h"
#include "html/html_formimpl.h"
#include "misc/htmlhashes.h"

#include "rendering/render_form.h"
#include <assert.h>

#include "khtmlview.h"
#include "khtml_ext.h"
#include "xml/dom_docimpl.h"

#include <qpopupmenu.h>

using namespace khtml;

RenderFormElement::RenderFormElement(HTMLGenericFormElementImpl *element)
    : RenderWidget(element)
{
    // init RenderObject attributes
    setInline(true);   // our object is Inline

    m_state = 0;
    m_button = 0;
    m_isDoubleClick = false;
}

RenderFormElement::~RenderFormElement()
{
}

short RenderFormElement::baselinePosition( bool f ) const
{
    return RenderWidget::baselinePosition( f ) - 2 - style()->fontMetrics().descent();
}


void RenderFormElement::updateFromElement()
{
    m_widget->setEnabled(!element()->disabled());
    RenderWidget::updateFromElement();
}

void RenderFormElement::layout()
{
    KHTMLAssert( !layouted() );
    KHTMLAssert( minMaxKnown() );

    // minimum height
    m_height = 0;

    calcWidth();
    calcHeight();

    if ( m_widget )
        resizeWidget(m_width-borderLeft()-borderRight()-paddingLeft()-paddingRight(),
                     m_height-borderTop()-borderBottom()-paddingTop()-paddingBottom());

    if ( !style()->width().isPercent() )
        setLayouted();
}

void RenderFormElement::slotClicked()
{
    ref();
    QMouseEvent e2( QEvent::MouseButtonRelease, m_mousePos, m_button, m_state);

    element()->dispatchMouseEvent(&e2, EventImpl::CLICK_EVENT, m_isDoubleClick + 1);
    m_isDoubleClick = false;
    deref();
}

void RenderFormElement::slotPressed()
{
    ref();
    QMouseEvent e2( QEvent::MouseButtonPress, m_mousePos, m_button, m_state);
    element()->dispatchMouseEvent(&e2, EventImpl::MOUSEDOWN_EVENT, 1);
    deref();
}

void RenderFormElement::slotReleased()
{
    ref();
    QMouseEvent e2( QEvent::MouseButtonRelease, m_mousePos, m_button, m_state);
    element()->dispatchMouseEvent(&e2, EventImpl::MOUSEUP_EVENT, 1);
    deref();
}

// -------------------------------------------------------------------------

RenderButton::RenderButton(HTMLGenericFormElementImpl *element)
    : RenderFormElement(element)
{
}

short RenderButton::baselinePosition( bool f ) const
{
    return RenderWidget::baselinePosition( f ) - 2;
}

// -------------------------------------------------------------------------------

RenderCheckBox::RenderCheckBox(HTMLInputElementImpl *element)
    : RenderButton(element)
{
    QCheckBox* b = new QCheckBox(view()->viewport());
    b->setAutoMask(true);
    b->setMouseTracking(true);
    setQWidget(b);
    connect(b,SIGNAL(stateChanged(int)),this,SLOT(slotStateChanged(int)));
    connect(b, SIGNAL(clicked()), this, SLOT(slotClicked()));
    connect(b, SIGNAL(pressed()), this, SLOT(slotPressed()));
    connect(b, SIGNAL(released()), this, SLOT(slotReleased()));
}


void RenderCheckBox::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    QCheckBox *cb = static_cast<QCheckBox *>( m_widget );
    QSize s( cb->style().pixelMetric( QStyle::PM_IndicatorWidth ),
             cb->style().pixelMetric( QStyle::PM_IndicatorHeight ) );
    setIntrinsicWidth( s.width() );
    setIntrinsicHeight( s.height() );

    RenderButton::calcMinMaxWidth();
}

void RenderCheckBox::updateFromElement()
{
    widget()->setChecked(element()->checked());

    RenderButton::updateFromElement();
}

void RenderCheckBox::slotStateChanged(int state)
{
    element()->setChecked(state == 2);
}

// -------------------------------------------------------------------------------

RenderRadioButton::RenderRadioButton(HTMLInputElementImpl *element)
    : RenderButton(element)
{
    QRadioButton* b = new QRadioButton(view()->viewport());
    b->setAutoMask(true);
    b->setMouseTracking(true);
    setQWidget(b);
    connect(b, SIGNAL(clicked()), this, SLOT(slotClicked()));
    connect(b, SIGNAL(pressed()), this, SLOT(slotPressed()));
    connect(b, SIGNAL(released()), this, SLOT(slotReleased()));
}

void RenderRadioButton::updateFromElement()
{
    widget()->setChecked(element()->checked());

    RenderButton::updateFromElement();
}

void RenderRadioButton::slotClicked()
{
    element()->setChecked(true);

    // emit mouseClick event etc
    RenderButton::slotClicked();
}

void RenderRadioButton::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    QRadioButton *rb = static_cast<QRadioButton *>( m_widget );
    QSize s( rb->style().pixelMetric( QStyle::PM_ExclusiveIndicatorWidth ),
             rb->style().pixelMetric( QStyle::PM_ExclusiveIndicatorHeight ) );
    setIntrinsicWidth( s.width() );
    setIntrinsicHeight( s.height() );

    RenderButton::calcMinMaxWidth();
}

// -------------------------------------------------------------------------------


RenderSubmitButton::RenderSubmitButton(HTMLInputElementImpl *element)
    : RenderButton(element)
{
    QPushButton* p = new QPushButton(view()->viewport());
    setQWidget(p);
    p->setAutoMask(true);
    p->setMouseTracking(true);
    connect(p, SIGNAL(clicked()), this, SLOT(slotClicked()));
    connect(p, SIGNAL(pressed()), this, SLOT(slotPressed()));
    connect(p, SIGNAL(released()), this, SLOT(slotReleased()));
}

QString RenderSubmitButton::rawText()
{
    QString value = element()->value().isEmpty() ? defaultLabel() : element()->value().string();
    value = value.stripWhiteSpace();
    QString raw;
    for(unsigned int i = 0; i < value.length(); i++) {
        raw += value[i];
        if(value[i] == '&')
            raw += '&';
    }
    return raw;
}

void RenderSubmitButton::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    QString raw = rawText();
    QPushButton* pb = static_cast<QPushButton*>(m_widget);
    pb->setText(raw);
    pb->setFont(style()->font());

    bool empty = raw.isEmpty();
    if ( empty )
        raw = QString::fromLatin1(" ");
    QFontMetrics fm = pb->fontMetrics();
    QSize ts = fm.size( ShowPrefix, raw);
    QSize s(pb->style().sizeFromContents( QStyle::CT_PushButton, pb, ts )
            .expandedTo(QApplication::globalStrut()));
    int margin = pb->style().pixelMetric( QStyle::PM_ButtonMargin, pb) +
		 pb->style().pixelMetric( QStyle::PM_DefaultFrameWidth, pb ) * 2;
    int w = ts.width() + margin;
    int h = s.height();
    if (pb->isDefault() || pb->autoDefault()) {
	int dbw = pb->style().pixelMetric( QStyle::PM_ButtonDefaultIndicator, pb ) * 2;
	w += dbw;
    }

    // add 30% margins to the width (heuristics to make it look similar to IE)
    s = QSize( w*13/10, h ).expandedTo(QApplication::globalStrut());

    setIntrinsicWidth( s.width() );
    setIntrinsicHeight( s.height() );

    RenderButton::calcMinMaxWidth();
}

void RenderSubmitButton::updateFromElement()
{
    QString oldText = static_cast<QPushButton*>(m_widget)->text();
    QString newText = rawText();
    static_cast<QPushButton*>(m_widget)->setText(newText);
    if ( oldText != newText ) {
        setMinMaxKnown(false);
	setLayouted(false);
    }
    RenderFormElement::updateFromElement();
}

QString RenderSubmitButton::defaultLabel() {
    return i18n("Submit");
}

short RenderSubmitButton::baselinePosition( bool f ) const
{
    return RenderFormElement::baselinePosition( f );
}

// -------------------------------------------------------------------------------

RenderImageButton::RenderImageButton(HTMLInputElementImpl *element)
    : RenderImage(element)
{
    // ### support DOMActivate event when clicked
}


// -------------------------------------------------------------------------------

RenderResetButton::RenderResetButton(HTMLInputElementImpl *element)
    : RenderSubmitButton(element)
{
}

QString RenderResetButton::defaultLabel() {
    return i18n("Reset");
}


// -------------------------------------------------------------------------------

RenderPushButton::RenderPushButton(HTMLInputElementImpl *element)
    : RenderSubmitButton(element)
{
}

QString RenderPushButton::defaultLabel()
{
    return QString::null;
}

// -------------------------------------------------------------------------------

LineEditWidget::LineEditWidget(QWidget *parent)
        : KLineEdit(parent)
{
    setMouseTracking(true);
}

QPopupMenu *LineEditWidget::createPopupMenu()
{
    QPopupMenu *popup = KLineEdit::createPopupMenu();
    if ( !popup )
        return 0L;
    connect( popup, SIGNAL( activated( int ) ),
             this, SLOT( extendedMenuActivated( int ) ) );
    return popup;
}

void LineEditWidget::extendedMenuActivated( int id)
{
    switch ( id )
    {
    case ClearHistory:
        clearMenuHistory();
        break;
    default:
        break;
    }
}

void LineEditWidget::clearMenuHistory()
{
    emit clearCompletionHistory();
}


bool LineEditWidget::event( QEvent *e )
{
    if ( e->type() == QEvent::AccelAvailable && isReadOnly() ) {
        QKeyEvent* ke = (QKeyEvent*) e;
        if ( ke->state() & ControlButton ) {
            switch ( ke->key() ) {
                case Key_Left:
                case Key_Right:
                case Key_Up:
                case Key_Down:
                case Key_Home:
                case Key_End:
                    ke->accept();
                default:
                break;
            }
        }
    }
    else if ( e->type() == QEvent::MouseButtonPress )
        emit pressed();
    else if ( e->type() == QEvent::MouseButtonRelease )
        emit released();
    return KLineEdit::event( e );
}

// -----------------------------------------------------------------------------

RenderLineEdit::RenderLineEdit(HTMLInputElementImpl *element)
    : RenderFormElement(element)
{
    LineEditWidget *edit = new LineEditWidget(view()->viewport());
    connect(edit,SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));
    connect(edit,SIGNAL(textChanged(const QString &)),this,SLOT(slotTextChanged(const QString &)));
    connect(edit,SIGNAL(pressed()), this, SLOT(slotPressed()));
    connect(edit,SIGNAL(released()), this, SLOT(slotReleased()));
    connect(edit, SIGNAL(clearCompletionHistory()), this, SLOT( slotClearCompletionHistory()));
    if(element->inputType() == HTMLInputElementImpl::PASSWORD)
        edit->setEchoMode( QLineEdit::Password );

    if ( element->autoComplete() ) {
        QStringList completions = view()->formCompletionItems(element->name().string());
        if (completions.count()) {
            edit->completionObject()->setItems(completions);
            edit->setContextMenuEnabled(true);
        }
    }

    setQWidget(edit);
}

void RenderLineEdit::slotClearCompletionHistory()
{
    if ( element()->autoComplete() ) {
        view()->clearCompletionHistory(element()->name().string());
        static_cast<LineEditWidget*>(m_widget)->completionObject()->clear();
    }
}

void RenderLineEdit::slotReturnPressed()
{
    // don't submit the form when return was pressed in a completion-popup
    KCompletionBox *box = widget()->completionBox(false);
    if ( box && box->isVisible() && box->currentItem() != -1 )
	return;

    // Emit onChange if necessary
    // Works but might not be enough, dirk said he had another solution at
    // hand (can't remember which) - David
    handleFocusOut();

    HTMLFormElementImpl* fe = element()->form();
    if ( fe )
        fe->submitFromKeyboard();
}

void RenderLineEdit::handleFocusOut()
{
    if ( widget() && widget()->edited() ) {
        element()->onChange();
        widget()->setEdited( false );
    }
}

void RenderLineEdit::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    const QFontMetrics &fm = style()->fontMetrics();
    QSize s;

    int size = element()->size();

    int h = fm.lineSpacing();
    int w = fm.width( 'x' ) * (size > 0 ? size+1 : 17); // "some"
    s = QSize(w + 2 + 2*widget()->frameWidth(),
              QMAX(h, 14) + 2 + 2*widget()->frameWidth())
        .expandedTo(QApplication::globalStrut());

    setIntrinsicWidth( s.width() );
    setIntrinsicHeight( s.height() );

    RenderFormElement::calcMinMaxWidth();
}

void RenderLineEdit::updateFromElement()
{
    int ml = element()->maxLength();
    if ( ml < 0 || ml > 1024 )
        ml = 1024;
    if ( widget()->maxLength() != ml )
        widget()->setMaxLength( ml );

    if (element()->value().string() != widget()->text()) {
        widget()->blockSignals(true);
        int pos = widget()->cursorPosition();
        widget()->setText(element()->value().string());

        widget()->setEdited( false );

        widget()->setCursorPosition(pos);
        widget()->blockSignals(false);
    }
    widget()->setReadOnly(element()->readOnly());

    RenderFormElement::updateFromElement();
}

void RenderLineEdit::slotTextChanged(const QString &string)
{
    // don't use setValue here!
    element()->m_value = string;
}

void RenderLineEdit::select()
{
    static_cast<LineEditWidget*>(m_widget)->selectAll();
}

// ---------------------------------------------------------------------------

RenderFieldset::RenderFieldset(HTMLGenericFormElementImpl *element)
    : RenderFlow(element)
{
}

bool RenderFieldset::findLegend( int &lx, int &ly, int &lw, int &lh)
{
    RenderObject *r = this, *ref = 0;
    int minx = 0, curx = 0, maxw = 0;
    if( r->firstChild() && r->firstChild()->element() &&
        r->firstChild()->element()->id() == ID_LEGEND)
            r = r->firstChild();
    else
        return false;
    if(!r->firstChild() || r->isSpecial())
        return false;
    ly = r->yPos();
    minx = r->width();
    curx = r->xPos();
    lh = r->height();
    ref = r;

    while(r) {
        if(r->firstChild())
            r = r->firstChild();
        else if(r->nextSibling())
            r = r->nextSibling();
        else {
            RenderObject *next = 0;
            while(!next) {
                r = r->parent();
                if(!r || r == (RenderObject *)ref ) goto end;
                next = r->nextSibling();
            }
            r = next;
        }
        if(r->isSpecial())
            continue;
        curx += r->xPos();
        if(r->width() && curx<minx)
             minx = curx;
        if(curx-minx+r->width() > maxw) {
                maxw = curx-minx+r->width();
        }
        if(!r->childrenInline())
            curx -= r->xPos();
    }
    end:
        lx = minx - ref->paddingLeft();
        lw = maxw + ref->paddingLeft() + ref->paddingRight();
        if(lx < 0 || lx+lw > width())
            return false;
        return !!maxw;
}

void RenderFieldset::paintBoxDecorations(QPainter *p,int, int _y,
                                       int, int _h, int _tx, int _ty)
{
    //kdDebug( 6040 ) << renderName() << "::paintDecorations()" << endl;

    int w = width();
    int h = height() + borderTopExtra() + borderBottomExtra();
    int lx = 0, ly = 0, lw = 0, lh = 0;
    bool legend = findLegend(lx, ly, lw, lh);

    if(legend) {
        int yOff = ly + lh/2 - borderTop()/2;
        h -= yOff;
        _ty += yOff;
    }
    _ty -= borderTopExtra();

    int my = QMAX(_ty,_y);
    int end = QMIN( _y + _h,  _ty + h );
    int mh = end - my;

    paintBackground(p, style()->backgroundColor(), style()->backgroundImage(), my, mh, _tx, _ty, w, h);

    if ( style()->hasBorder() ) {
	if ( legend )
	    paintBorderMinusLegend(p, _tx, _ty, w, h, style(), lx, lw);
	else
	    paintBorder(p, _tx, _ty, w, h, style());
    }
}

void RenderFieldset::paintBorderMinusLegend(QPainter *p, int _tx, int _ty, int w, int h,
                                            const RenderStyle* style, int lx, int lw)
{

    const QColor& tc = style->borderTopColor();
    const QColor& bc = style->borderBottomColor();

    EBorderStyle ts = style->borderTopStyle();
    EBorderStyle bs = style->borderBottomStyle();
    EBorderStyle ls = style->borderLeftStyle();
    EBorderStyle rs = style->borderRightStyle();

    bool render_t = ts > BHIDDEN;
    bool render_l = ls > BHIDDEN;
    bool render_r = rs > BHIDDEN;
    bool render_b = bs > BHIDDEN;

    if(render_t) {
        drawBorder(p, _tx, _ty, _tx + lx, _ty +  style->borderTopWidth(), BSTop, tc, style->color(), ts,
                   (render_l && ls<=DOUBLE?style->borderLeftWidth():0), 0);
        drawBorder(p, _tx+lx+lw, _ty, _tx + w, _ty +  style->borderTopWidth(), BSTop, tc, style->color(), ts,
                   0, (render_r && rs<=DOUBLE?style->borderRightWidth():0));
    }

    if(render_b)
        drawBorder(p, _tx, _ty + h - style->borderBottomWidth(), _tx + w, _ty + h, BSBottom, bc, style->color(), bs,
                   (render_l && ls<=DOUBLE?style->borderLeftWidth():0),
		   (render_r && rs<=DOUBLE?style->borderRightWidth():0));

    if(render_l)
    {
	const QColor& lc = style->borderLeftColor();

	bool ignore_top =
	  (tc == lc) &&
	  (ls <= OUTSET) &&
	  (ts == DOTTED || ts == DASHED || ts == SOLID || ts == OUTSET);

	bool ignore_bottom =
	  (bc == lc) &&
	  (ls <= OUTSET) &&
	  (bs == DOTTED || bs == DASHED || bs == SOLID || bs == INSET);

        drawBorder(p, _tx, _ty, _tx + style->borderLeftWidth(), _ty + h, BSLeft, lc, style->color(), ls,
		   ignore_top?0:style->borderTopWidth(),
		   ignore_bottom?0:style->borderBottomWidth());
    }

    if(render_r)
    {
	const QColor& rc = style->borderRightColor();

	bool ignore_top =
	  (tc == rc) &&
	  (rs <= SOLID || rs == INSET) &&
	  (ts == DOTTED || ts == DASHED || ts == SOLID || ts == OUTSET);

	bool ignore_bottom =
	  (bc == rc) &&
	  (rs <= SOLID || rs == INSET) &&
	  (bs == DOTTED || bs == DASHED || bs == SOLID || bs == INSET);

        drawBorder(p, _tx + w - style->borderRightWidth(), _ty, _tx + w, _ty + h, BSRight, rc, style->color(), rs,
		   ignore_top?0:style->borderTopWidth(),
		   ignore_bottom?0:style->borderBottomWidth());
    }
}

// -------------------------------------------------------------------------

RenderFileButton::RenderFileButton(HTMLInputElementImpl *element)
    : RenderFormElement(element)
{
    QHBox *w = new QHBox(view()->viewport());

    m_edit = new LineEditWidget(w);

    connect(m_edit, SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));
    connect(m_edit, SIGNAL(textChanged(const QString &)),this,SLOT(slotTextChanged(const QString &)));

    m_button = new QPushButton(i18n("Browse..."), w);
    m_button->setFocusPolicy(QWidget::ClickFocus);
    connect(m_button,SIGNAL(clicked()), this, SLOT(slotClicked()));
    connect(m_button, SIGNAL(pressed()), this, SLOT(slotPressed()));
    connect(m_button, SIGNAL(released()), this, SLOT(slotReleased()));

    w->setStretchFactor(m_edit, 2);
    w->setFocusProxy(m_edit);

    setQWidget(w);
    m_haveFocus = false;
}

void RenderFileButton::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    const QFontMetrics &fm = style()->fontMetrics();
    QSize s;
    int size = element()->size();

    int h = fm.lineSpacing();
    int w = fm.width( 'x' ) * (size > 0 ? size : 17); // "some"
    w += 6 + fm.width( m_button->text() ) + 2*fm.width( ' ' );
    s = QSize(w + 2 + 2*m_edit->frameWidth(),
              QMAX(h, 14) + 2 + 2*m_edit->frameWidth())
        .expandedTo(QApplication::globalStrut());

    setIntrinsicWidth( s.width() );
    setIntrinsicHeight( s.height() );

    RenderFormElement::calcMinMaxWidth();
}

void RenderFileButton::handleFocusOut()
{
    if ( m_edit && m_edit->edited() ) {
        element()->onChange();
        m_edit->setEdited( false );
    }
}

void RenderFileButton::slotClicked()
{
    QString file_name = KFileDialog::getOpenFileName(QString::null, QString::null, 0, i18n("Browse"));
    if (!file_name.isNull()) {
        element()->m_value = DOMString(file_name);
        m_edit->setText(file_name);
    }
}

void RenderFileButton::updateFromElement()
{
    m_edit->blockSignals(true);
    m_edit->setText(element()->value().string());
    m_edit->blockSignals(false);
    int ml = element()->maxLength();
    if ( ml < 0 || ml > 1024 )
        ml = 1024;
    m_edit->setMaxLength( ml );
    m_edit->setEdited( false );

    RenderFormElement::updateFromElement();
}

void RenderFileButton::slotReturnPressed()
{
    if (element()->form())
	element()->form()->submitFromKeyboard();
}

void RenderFileButton::slotTextChanged(const QString &string)
{
   element()->m_value = DOMString(string);
}

void RenderFileButton::select()
{
    m_edit->selectAll();
}

// -------------------------------------------------------------------------

RenderLabel::RenderLabel(HTMLGenericFormElementImpl *element)
    : RenderFormElement(element)
{

}

// -------------------------------------------------------------------------

RenderLegend::RenderLegend(HTMLGenericFormElementImpl *element)
    : RenderFlow(element)
{
    setInline(false);
}

// -------------------------------------------------------------------------------

ComboBoxWidget::ComboBoxWidget(QWidget *parent)
    : KComboBox(false, parent)
{
    setAutoMask(true);
    if (listBox()) listBox()->installEventFilter(this);
    setMouseTracking(true);
}

bool ComboBoxWidget::event(QEvent *e)
{
    if (e->type()==QEvent::KeyPress)
    {
	QKeyEvent *ke = static_cast<QKeyEvent *>(e);
	switch(ke->key())
	{
	case Key_Return:
	case Key_Enter:
	    popup();
	    ke->accept();
	    return true;
	default:
	    return KComboBox::event(e);
	}
    }
    return KComboBox::event(e);
}

bool ComboBoxWidget::eventFilter(QObject *dest, QEvent *e)
{
    if (dest==listBox() &&  e->type()==QEvent::KeyPress)
    {
	QKeyEvent *ke = static_cast<QKeyEvent *>(e);
	bool forward = false;
	switch(ke->key())
	{
	case Key_Tab:
	    forward=true;
	case Key_BackTab:
	    // ugly hack. emulate popdownlistbox() (private in QComboBox)
	    // we re-use ke here to store the reference to the generated event.
	    ke = new QKeyEvent(QEvent::KeyPress, Key_Escape, 0, 0);
	    QApplication::sendEvent(dest,ke);
	    focusNextPrevChild(forward);
	    delete ke;
	    return true;
	default:
	    return KComboBox::eventFilter(dest, e);
	}
    }
    return KComboBox::eventFilter(dest, e);
}

// -------------------------------------------------------------------------

RenderSelect::RenderSelect(HTMLSelectElementImpl *element)
    : RenderFormElement(element)
{
    m_ignoreSelectEvents = false;
    m_multiple = element->multiple();
    m_size = element->size();
    m_useListBox = (m_multiple || m_size > 1);
    m_selectionChanged = true;
    m_optionsChanged = true;

    if(m_useListBox)
        setQWidget(createListBox());
    else
        setQWidget(createComboBox());
}

void RenderSelect::updateFromElement()
{
    m_ignoreSelectEvents = true;

    // change widget type
    bool oldMultiple = m_multiple;
    unsigned oldSize = m_size;
    bool oldListbox = m_useListBox;

    m_multiple = element()->multiple();
    m_size = element()->size();
    m_useListBox = (m_multiple || m_size > 1);

    if (oldMultiple != m_multiple || oldSize != m_size) {
        if (m_useListBox != oldListbox) {
            // type of select has changed
            if(m_useListBox)
                setQWidget(createListBox());
            else
                setQWidget(createComboBox());
        }

        if (m_useListBox && oldMultiple != m_multiple) {
            static_cast<KListBox*>(m_widget)->setSelectionMode(m_multiple ? QListBox::Extended : QListBox::Single);
        }
        m_selectionChanged = true;
        m_optionsChanged = true;
    }

    // update contents listbox/combobox based on options in m_element
    if ( m_optionsChanged ) {
        if (element()->m_recalcListItems)
            element()->recalcListItems();
        QMemArray<HTMLGenericFormElementImpl*> listItems = element()->listItems();
        int listIndex;

        if(m_useListBox) {
            static_cast<KListBox*>(m_widget)->clear();
        }

        else
            static_cast<KComboBox*>(m_widget)->clear();

        for (listIndex = 0; listIndex < int(listItems.size()); listIndex++) {
            if (listItems[listIndex]->id() == ID_OPTGROUP) {
                DOMString text = listItems[listIndex]->getAttribute(ATTR_LABEL);
                if (text.isNull())
                    text = "";

                if(m_useListBox) {
                    QListBoxText *item = new QListBoxText(QString(text.implementation()->s, text.implementation()->l));
                    static_cast<KListBox*>(m_widget)
                        ->insertItem(item, listIndex);
                    item->setSelectable(false);
                }
                else
                    static_cast<KComboBox*>(m_widget)
                        ->insertItem(QString(text.implementation()->s, text.implementation()->l), listIndex);
            }
            else if (listItems[listIndex]->id() == ID_OPTION) {
                HTMLOptionElementImpl* optElem = static_cast<HTMLOptionElementImpl*>(listItems[listIndex]);
                QString text = optElem->text().string();
                if (optElem->parentNode()->id() == ID_OPTGROUP)
                {
                    // Prefer label if set
                    DOMString label = optElem->getAttribute(ATTR_LABEL);
                    if (!label.isEmpty())
                        text = label.string();
                    text = QString::fromLatin1("    ")+text;
                }

                if(m_useListBox)
                    static_cast<KListBox*>(m_widget)->insertItem(text, listIndex);
                else
                    static_cast<KComboBox*>(m_widget)->insertItem(text, listIndex);
            }
            else
                KHTMLAssert(false);
            m_selectionChanged = true;
        }
        setMinMaxKnown(false);
        setLayouted(false);
        m_optionsChanged = false;
    }

    // update selection
    if (m_selectionChanged) {
        updateSelection();
    }


    m_ignoreSelectEvents = false;

    RenderFormElement::updateFromElement();
}

void RenderSelect::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    if (m_optionsChanged)
        updateFromElement();

    // ### ugly HACK FIXME!!!
    setMinMaxKnown();
    if ( !layouted() )
        layout();
    setLayouted( false );
    setMinMaxKnown( false );
    // ### end FIXME

    RenderFormElement::calcMinMaxWidth();
}

void RenderSelect::layout( )
{
    KHTMLAssert(!layouted());
    KHTMLAssert(minMaxKnown());

    // ### maintain selection properly between type/size changes, and work
    // out how to handle multiselect->singleselect (probably just select
    // first selected one)

    // calculate size
    if(m_useListBox) {
        KListBox* w = static_cast<KListBox*>(m_widget);

        QListBoxItem* p = w->firstItem();
        int width = 0;
        int height = 0;
        while(p) {
            width = QMAX(width, p->width(p->listBox()));
            height = QMAX(height, p->height(p->listBox()));
            p = p->next();
        }

        int size = m_size;
        // check if multiple and size was not given or invalid
        // Internet Exploder sets size to QMIN(number of elements, 4)
        // Netscape seems to simply set it to "number of elements"
        // the average of that is IMHO QMIN(number of elements, 10)
        // so I did that ;-)
        if(size < 1)
            size = QMIN(static_cast<KListBox*>(m_widget)->count(), 10);

        width += 2*w->frameWidth() + w->verticalScrollBar()->sizeHint().width();
        height = size*height + 2*w->frameWidth();

        setIntrinsicWidth( width );
        setIntrinsicHeight( height );
    }
    else {
        QSize s(m_widget->sizeHint());
        setIntrinsicWidth( s.width() );
        setIntrinsicHeight( s.height() );
    }

    /// uuh, ignore the following line..
    setLayouted( false );
    RenderFormElement::layout();

    // and now disable the widget in case there is no <option> given
    QMemArray<HTMLGenericFormElementImpl*> listItems = element()->listItems();

    bool foundOption = false;
    for (uint i = 0; i < listItems.size() && !foundOption; i++)
	foundOption = (listItems[i]->id() == ID_OPTION);

    m_widget->setEnabled(foundOption && ! element()->disabled());
}

void RenderSelect::slotSelected(int index) // emitted by the combobox only
{
    if ( m_ignoreSelectEvents ) return;

    KHTMLAssert( !m_useListBox );

    QMemArray<HTMLGenericFormElementImpl*> listItems = element()->listItems();
    if(index >= 0 && index < int(listItems.size()))
    {
        bool found = ( listItems[index]->id() == ID_OPTION );

        if ( !found ) {
            // this one is not selectable,  we need to find an option element
            while ( ( unsigned ) index < listItems.size() ) {
                if ( listItems[index]->id() == ID_OPTION ) {
                    found = true;
                    break;
                }
                ++index;
            }

            if ( !found ) {
                while ( index >= 0 ) {
                    if ( listItems[index]->id() == ID_OPTION ) {
                        found = true;
                        break;
                    }
                    --index;
                }
            }
        }

        if ( found ) {
            bool changed = false;

            for ( unsigned int i = 0; i < listItems.size(); ++i )
                if ( listItems[i]->id() == ID_OPTION && i != (unsigned int) index )
                {
                    HTMLOptionElementImpl* opt = static_cast<HTMLOptionElementImpl*>( listItems[i] );
                    changed |= (opt->m_selected == true);
                    opt->m_selected = false;
                }

            HTMLOptionElementImpl* opt = static_cast<HTMLOptionElementImpl*>(listItems[index]);
            changed |= (opt->m_selected == false);
            opt->m_selected = true;

            if ( index != static_cast<ComboBoxWidget*>( m_widget )->currentItem() )
                static_cast<ComboBoxWidget*>( m_widget )->setCurrentItem( index );

            // When selecting an optgroup item, and we move forward to we
            // shouldn't emit onChange. Hence this bool, the if above doesn't do it.
            if ( changed )
            {
                ref();
                element()->onChange();
                deref();
            }
        }
    }
}


void RenderSelect::slotSelectionChanged() // emitted by the listbox only
{
    if ( m_ignoreSelectEvents ) return;

    // don't use listItems() here as we have to avoid recalculations - changing the
    // option list will make use update options not in the way the user expects them
    QMemArray<HTMLGenericFormElementImpl*> listItems = element()->m_listItems;
    for ( unsigned i = 0; i < listItems.count(); i++ )
        // don't use setSelected() here because it will cause us to be called
        // again with updateSelection.
        if ( listItems[i]->id() == ID_OPTION )
            static_cast<HTMLOptionElementImpl*>( listItems[i] )
                ->m_selected = static_cast<KListBox*>( m_widget )->isSelected( i );

    ref();
    element()->onChange();
    deref();
}

void RenderSelect::setOptionsChanged(bool _optionsChanged)
{
    m_optionsChanged = _optionsChanged;
}

KListBox* RenderSelect::createListBox()
{
    KListBox *lb = new KListBox(view()->viewport());
    lb->setSelectionMode(m_multiple ? QListBox::Extended : QListBox::Single);
    // ### looks broken
    //lb->setAutoMask(true);
    connect( lb, SIGNAL( selectionChanged() ), this, SLOT( slotSelectionChanged() ) );
    connect( lb, SIGNAL( clicked( QListBoxItem * ) ), this, SLOT( slotClicked() ) );
    m_ignoreSelectEvents = false;
    lb->setMouseTracking(true);

    return lb;
}

ComboBoxWidget *RenderSelect::createComboBox()
{
    ComboBoxWidget *cb = new ComboBoxWidget(view()->viewport());
    connect(cb, SIGNAL(activated(int)), this, SLOT(slotSelected(int)));
    return cb;
}

void RenderSelect::updateSelection()
{
    QMemArray<HTMLGenericFormElementImpl*> listItems = element()->listItems();
    int i;
    if (m_useListBox) {
        // if multi-select, we select only the new selected index
        KListBox *listBox = static_cast<KListBox*>(m_widget);
        for (i = 0; i < int(listItems.size()); i++)
            listBox->setSelected(i,listItems[i]->id() == ID_OPTION &&
                                static_cast<HTMLOptionElementImpl*>(listItems[i])->selected());
    }
    else {
        bool found = false;
        unsigned firstOption = listItems.size();
        i = listItems.size();
        while (i--)
            if (listItems[i]->id() == ID_OPTION) {
                if (found)
                    static_cast<HTMLOptionElementImpl*>(listItems[i])->m_selected = false;
                else if (static_cast<HTMLOptionElementImpl*>(listItems[i])->selected()) {
                    static_cast<KComboBox*>( m_widget )->setCurrentItem(i);
                    found = true;
                }
                firstOption = i;
            }

        Q_ASSERT(firstOption == listItems.size() || found);
    }

    m_selectionChanged = false;
}


// -------------------------------------------------------------------------

TextAreaWidget::TextAreaWidget(int wrap, QWidget* parent)
    : KTextEdit(parent)
{
    if(wrap != DOM::HTMLTextAreaElementImpl::ta_NoWrap) {
        setWordWrap(QTextEdit::WidgetWidth);
        setHScrollBarMode( AlwaysOff );
        setVScrollBarMode( AlwaysOn );
    }
    else {
        setWordWrap(QTextEdit::NoWrap);
        setHScrollBarMode( Auto );
        setVScrollBarMode( Auto );
    }
    KCursor::setAutoHideCursor(viewport(), true);
    setTextFormat(QTextEdit::PlainText);
    setAutoMask(true);
    setMouseTracking(true);
}

bool TextAreaWidget::event( QEvent *e )
{
    if ( e->type() == QEvent::AccelAvailable && isReadOnly() ) {
        QKeyEvent* ke = (QKeyEvent*) e;
        if ( ke->state() & ControlButton ) {
            switch ( ke->key() ) {
                case Key_Left:
                case Key_Right:
                case Key_Up:
                case Key_Down:
                case Key_Home:
                case Key_End:
                    ke->accept();
                default:
                break;
            }
        }
    }
    return KTextEdit::event( e );
}

// -------------------------------------------------------------------------

RenderTextArea::RenderTextArea(HTMLTextAreaElementImpl *element)
    : RenderFormElement(element)
{
    TextAreaWidget *edit = new TextAreaWidget(element->wrap(), view());
    setQWidget(edit);

    connect(edit,SIGNAL(textChanged()),this,SLOT(slotTextChanged()));
}

RenderTextArea::~RenderTextArea()
{
    if ( element()->m_dirtyvalue ) {
        element()->m_value = text();
        element()->m_dirtyvalue = false;
    }
}

void RenderTextArea::handleFocusOut()
{
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    if ( w && element()->m_dirtyvalue ) {
        element()->m_value = text();
        element()->m_dirtyvalue = false;
        element()->onChange();
    }
}

void RenderTextArea::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    const QFontMetrics &m = style()->fontMetrics();
    w->setTabStopWidth(8 * m.width(" "));
    QSize size( QMAX(element()->cols(), 1)*m.width('x') + w->frameWidth() +
                w->verticalScrollBar()->sizeHint().width(),
                QMAX(element()->rows(), 1)*m.height() + w->frameWidth()*2 +
                (w->wordWrap() == QTextEdit::NoWrap ?
                 w->horizontalScrollBar()->sizeHint().height() : 0)
        );

    setIntrinsicWidth( size.width() );
    setIntrinsicHeight( size.height() );

    RenderFormElement::calcMinMaxWidth();
}

void RenderTextArea::updateFromElement()
{
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    w->setReadOnly(element()->readOnly());
    QString elementText = element()->value().string();
    if ( elementText != text() )
    {
        w->blockSignals(true);
        int line, col;
        w->getCursorPosition( &line, &col );
        w->setText( elementText );
        w->setCursorPosition( line, col );
        w->blockSignals(false);
    }
    element()->m_dirtyvalue = false;

    RenderFormElement::updateFromElement();
}

void RenderTextArea::close( )
{
    element()->setValue( element()->defaultValue() );

    RenderFormElement::close();
}

QString RenderTextArea::text()
{
    QString txt;
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);

    if(element()->wrap() == DOM::HTMLTextAreaElementImpl::ta_Physical) {
        // yeah, QTextEdit has no accessor for getting the visually wrapped text
        for (int p=0; p < w->paragraphs(); ++p) {
            int pl = w->paragraphLength(p);
            int ll = 0;
            int lindex = w->lineOfChar(p, 0);
            QString paragraphText = w->text(p);
            for (int l = 0; l < pl; ++l) {
                if (lindex != w->lineOfChar(p, l)) {
                    paragraphText.insert(l+ll++, QString::fromLatin1("\n"));
                    lindex = w->lineOfChar(p, l);
                }
            }
            txt += paragraphText;
            if (p < w->paragraphs() - 1)
                txt += QString::fromLatin1("\n");
        }
    }
    else
        txt = w->text();

    return txt;
}

void RenderTextArea::slotTextChanged()
{
    element()->m_dirtyvalue = true;
}

void RenderTextArea::select()
{
    static_cast<TextAreaWidget *>(m_widget)->selectAll();
}

// ---------------------------------------------------------------------------

#include "render_form.moc"
