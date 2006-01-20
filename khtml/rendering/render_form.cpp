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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include <kcompletionbox.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kfind.h>
#include <kfinddialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kreplace.h>
#include <kreplacedialog.h>
#include <kspell.h>
#include <kurlcompletion.h>
#include <kwin.h>

#include <QAbstractItemView>
#include <qstyle.h>
#include <QStyleOptionButton>

#include "misc/helper.h"
#include "xml/dom2_eventsimpl.h"
#include "html/html_formimpl.h"
#include "misc/htmlhashes.h"

#include "rendering/render_form.h"
#include <assert.h>

#include "khtmlview.h"
#include "khtml_ext.h"
#include "xml/dom_docimpl.h"

#include <q3popupmenu.h>
#include <qbitmap.h>

using namespace khtml;

RenderFormElement::RenderFormElement(HTMLGenericFormElementImpl *element)
    : RenderWidget(element)
{
    // init RenderObject attributes
    setInline(true);   // our object is Inline

    m_state = 0;
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
    KHTMLAssert( needsLayout() );
    KHTMLAssert( minMaxKnown() );

    // minimum height
    m_height = 0;

    calcWidth();
    calcHeight();

    if ( m_widget )
        resizeWidget(m_width-borderLeft()-borderRight()-paddingLeft()-paddingRight(),
                     m_height-borderTop()-borderBottom()-paddingTop()-paddingBottom());

    if ( !style()->width().isPercent() )
        setNeedsLayout(false);
}


Qt::AlignmentFlag RenderFormElement::textAlignment() const
{
    switch (style()->textAlign()) {
        case LEFT:
        case KHTML_LEFT:
            return Qt::AlignLeft;
        case RIGHT:
        case KHTML_RIGHT:
            return Qt::AlignRight;
        case CENTER:
        case KHTML_CENTER:
            return Qt::AlignHCenter;
        case JUSTIFY:
            // Just fall into the auto code for justify.
        case TAAUTO:
            return style()->direction() == RTL ? Qt::AlignRight : Qt::AlignLeft;
    }
    assert(false); // Should never be reached.
    return Qt::AlignLeft;
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
    b->setObjectName( "__khtml" );
    //b->setAutoMask(true);
    b->setMouseTracking(true);
    setQWidget(b);

    // prevent firing toggled() signals on initialization
    b->setChecked(element->checked());

    connect(b,SIGNAL(stateChanged(int)),this,SLOT(slotStateChanged(int)));
}


void RenderCheckBox::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    QCheckBox *cb = static_cast<QCheckBox *>( m_widget );
    QSize s( cb->style()->pixelMetric( QStyle::PM_IndicatorWidth ),
             cb->style()->pixelMetric( QStyle::PM_IndicatorHeight ) );
    setIntrinsicWidth( s.width() );
    setIntrinsicHeight( s.height() );

    RenderButton::calcMinMaxWidth();
}

void RenderCheckBox::updateFromElement()
{
    if (widget()->isChecked() != element()->checked())
        widget()->setChecked(element()->checked());

    RenderButton::updateFromElement();
}

void RenderCheckBox::slotStateChanged(int state)
{
    element()->setChecked(state == 2);

    ref();
    element()->onChange();
    deref();
}

// -------------------------------------------------------------------------------

RenderRadioButton::RenderRadioButton(HTMLInputElementImpl *element)
    : RenderButton(element)
{
    QRadioButton* b = new QRadioButton(view()->viewport());
    b->setObjectName( "__khtml" );
    b->setMouseTracking(true);
    b->setAutoExclusive(false);
    setQWidget(b);

    // prevent firing toggled() signals on initialization
    b->setChecked(element->checked());

    connect(b,SIGNAL(toggled(bool)),this,SLOT(slotToggled(bool)));
}

void RenderRadioButton::updateFromElement()
{
    widget()->setChecked(element()->checked());

    RenderButton::updateFromElement();
}

void RenderRadioButton::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    QRadioButton *rb = static_cast<QRadioButton *>( m_widget );
    QSize s( rb->style()->pixelMetric( QStyle::PM_ExclusiveIndicatorWidth ),
             rb->style()->pixelMetric( QStyle::PM_ExclusiveIndicatorHeight ) );
    setIntrinsicWidth( s.width() );
    setIntrinsicHeight( s.height() );

    RenderButton::calcMinMaxWidth();
}

void RenderRadioButton::slotToggled(bool activated)
{
    if(activated) {
      ref();
      element()->onChange();
      deref();
    }
}

// -------------------------------------------------------------------------------


RenderSubmitButton::RenderSubmitButton(HTMLInputElementImpl *element)
    : RenderButton(element)
{
    QPushButton* p = new QPushButton(view()->viewport());
    p->setObjectName( "__khtml" );
    setQWidget(p);
    //p->setAutoMask(true);
    p->setMouseTracking(true);
}

QString RenderSubmitButton::rawText()
{
    QString value = element()->valueWithDefault().string();
    value = value.trimmed();
    QString raw;
    for(int i = 0; i < value.length(); i++) {
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
        raw = QLatin1Char('X');
    QFontMetrics fm = pb->fontMetrics();
    QSize ts = fm.size( Qt::TextShowMnemonic, raw);
    //Oh boy.
    QStyleOptionButton butOpt;
    butOpt.init(pb);
    butOpt.text = raw;
    QSize s(pb->style()->sizeFromContents( QStyle::CT_PushButton, &butOpt, ts, pb )
            .expandedTo(QApplication::globalStrut()));
    int margin = pb->style()->pixelMetric( QStyle::PM_ButtonMargin) +
		 pb->style()->pixelMetric( QStyle::PM_DefaultFrameWidth ) * 2;
    int w = ts.width() + margin;
    int h = s.height();
    if (pb->isDefault() || pb->autoDefault()) {
	int dbw = pb->style()->pixelMetric( QStyle::PM_ButtonDefaultIndicator ) * 2;
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
    if ( oldText != newText )
        setNeedsLayoutAndMinMaxRecalc();
    RenderFormElement::updateFromElement();
}

short RenderSubmitButton::baselinePosition( bool f ) const
{
    return RenderFormElement::baselinePosition( f );
}

// -------------------------------------------------------------------------------

RenderResetButton::RenderResetButton(HTMLInputElementImpl *element)
    : RenderSubmitButton(element)
{
}

// -------------------------------------------------------------------------------

LineEditWidget::LineEditWidget(DOM::HTMLInputElementImpl* input, KHTMLView* view, QWidget* parent)
    : KLineEdit(parent), m_input(input), m_view(view), m_spell(0)
{
    setObjectName( "__khtml" );
    setMouseTracking(true);
    KActionCollection *ac = new KActionCollection(this);
    m_spellAction = KStdAction::spelling( this, SLOT( slotCheckSpelling() ), ac );
}

LineEditWidget::~LineEditWidget()
{
    delete m_spell;
    m_spell = 0L;
}

void LineEditWidget::slotCheckSpelling()
{
    if ( text().isEmpty() ) {
        return;
    }

    delete m_spell;
    m_spell = new KSpell( this, i18n( "Spell Checking" ), this, SLOT( slotSpellCheckReady( KSpell *) ), 0, true, true);

    connect( m_spell, SIGNAL( death() ),this, SLOT( spellCheckerFinished() ) );
    connect( m_spell, SIGNAL( misspelling( const QString &, const QStringList &, unsigned int ) ),this, SLOT( spellCheckerMisspelling( const QString &, const QStringList &, unsigned int ) ) );
    connect( m_spell, SIGNAL( corrected( const QString &, const QString &, unsigned int ) ),this, SLOT( spellCheckerCorrected( const QString &, const QString &, unsigned int ) ) );
}

void LineEditWidget::spellCheckerMisspelling( const QString &_text, const QStringList &, unsigned int pos)
{
    highLightWord( _text.length(),pos );
}

void LineEditWidget::highLightWord( unsigned int length, unsigned int pos )
{
    setSelection ( pos, length );
}

void LineEditWidget::spellCheckerCorrected( const QString &old, const QString &corr, unsigned int pos )
{
    if( old!= corr )
    {
        setSelection ( pos, old.length() );
        insert( corr );
        setSelection ( pos, corr.length() );
    }
}

void LineEditWidget::spellCheckerFinished()
{
}

void LineEditWidget::slotSpellCheckReady( KSpell *s )
{
    s->check( text() );
    connect( s, SIGNAL( done( const QString & ) ), this, SLOT( slotSpellCheckDone( const QString & ) ) );
}

void LineEditWidget::slotSpellCheckDone( const QString &s )
{
    if( s != text() )
        setText( s );
}


void LineEditWidget::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu* popup = createStandardContextMenu();

/*    menu->exec(e->globalPos());
            delete menu;*/
            
    if ( !popup ) {
        return;
    }

    if (m_input->autoComplete()) {
        popup->insertSeparator();
        QAction* act = popup->addAction( SmallIconSet("history_clear"), i18n("Clear &History"));
        act->setEnabled(compObj() && !compObj()->isEmpty());
        connect(act, SIGNAL(triggered()),
                this, SLOT(clearHistoryActivated()));
    }

    if (echoMode() == QLineEdit::Normal &&
        !isReadOnly()) {
        popup->insertSeparator();

        m_spellAction->plug(popup);
        m_spellAction->setEnabled( !text().isEmpty() );
    }
}


void LineEditWidget::clearHistoryActivated()
{
    m_view->clearCompletionHistory(m_input->name().string());
    if (compObj())
      compObj()->clear();
}

void LineEditWidget::paintEvent( QPaintEvent *pe )
{
    //Always paint our background color
    QRect r = rect();
    if (hasFrame()) {
        int margin = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
        r = QRect(QPoint(margin, margin), QSize(width() - 2*margin, height() - 2*margin));
    }

    QPainter p(this);
    p.fillRect(r, palette().brush(QPalette::Base));
    p.end();

    KLineEdit::paintEvent( pe );
}

bool LineEditWidget::event( QEvent *e )
{
    if (KLineEdit::event(e))
	return true;

    if ( e->type() == QEvent::AccelAvailable && isReadOnly() ) {
        QKeyEvent* ke = (QKeyEvent*) e;
        if ( ke->state() & Qt::ControlModifier ) {
            switch ( ke->key() ) {
                case Qt::Key_Left:
                case Qt::Key_Right:
                case Qt::Key_Up:
                case Qt::Key_Down:
                case Qt::Key_Home:
                case Qt::Key_End:
                    ke->accept();
                default:
                break;
            }
        }
    }
    return false;
}

void LineEditWidget::mouseMoveEvent(QMouseEvent *e)
{
    // hack to prevent Qt from calling setCursor on the widget
    setDragEnabled(false);
    KLineEdit::mouseMoveEvent(e);
    setDragEnabled(true);
}


// -----------------------------------------------------------------------------

RenderLineEdit::RenderLineEdit(HTMLInputElementImpl *element)
    : RenderFormElement(element)
{
    LineEditWidget *edit = new LineEditWidget(element, view(), view()->viewport());
    connect(edit,SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));
    connect(edit,SIGNAL(textChanged(const QString &)),this,SLOT(slotTextChanged(const QString &)));

    if(element->inputType() == HTMLInputElementImpl::PASSWORD)
        edit->setEchoMode( QLineEdit::Password );

    if ( element->autoComplete() ) {
        QStringList completions = view()->formCompletionItems(element->name().string());
        if (completions.count()) {
            edit->completionObject()->setItems(completions);
            edit->setContextMenuEnabled(true);
            edit->completionBox()->setTabHandling( false );
        }
    }

    setQWidget(edit);
}

void RenderLineEdit::setStyle(RenderStyle* _style)
{
    RenderFormElement::setStyle( _style );

    widget()->setAlignment(textAlignment());
}

void RenderLineEdit::highLightWord( unsigned int length, unsigned int pos )
{
    LineEditWidget* w = static_cast<LineEditWidget*>(m_widget);
    if ( w )
        w->highLightWord( length, pos );
}


void RenderLineEdit::slotReturnPressed()
{
    // don't submit the form when return was pressed in a completion-popup
    KCompletionBox *box = widget()->completionBox(false);

    if ( box && box->isVisible() && box->currentItem() != -1 ) {
      box->hide();
      return;
    }

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

    QStyleOption opt;
    opt.init(widget());

    int margin = static_cast<LineEditWidget*>(widget())->hasFrame() ? 4 : 0;
    s = QSize(w + margin, qMax(h, 14) + margin);
    s = widget()->style()->sizeFromContents(QStyle::CT_LineEdit, &opt, s);
    s = s.expandedTo(QApplication::globalStrut());

    setIntrinsicWidth( s.width() );
    setIntrinsicHeight( s.height() );

    RenderFormElement::calcMinMaxWidth();
}

void RenderLineEdit::updateFromElement()
{
    int ml = element()->maxLength();
    if ( ml < 0 || ml > 1024 )
        ml = 1024;

     if ( widget()->maxLength() != ml )  {
         widget()->setMaxLength( ml );
     }

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
    element()->m_unsubmittedFormChange = true;
}

void RenderLineEdit::select()
{
    static_cast<LineEditWidget*>(m_widget)->selectAll();
}

// ---------------------------------------------------------------------------

RenderFieldset::RenderFieldset(HTMLGenericFormElementImpl *element)
    : RenderBlock(element)
{
}

RenderObject* RenderFieldset::layoutLegend(bool relayoutChildren)
{
    RenderObject* legend = findLegend();
    if (legend) {
        if (relayoutChildren)
            legend->setNeedsLayout(true);
        legend->layoutIfNeeded();

        int xPos = borderLeft() + paddingLeft() + legend->marginLeft();
        if (style()->direction() == RTL)
            xPos = m_width - paddingRight() - borderRight() - legend->width() - legend->marginRight();
        int b = borderTop();
        int h = legend->height();
        legend->setPos(xPos, qMax((b-h)/2, 0));
        m_height = qMax(b,h) + paddingTop();
    }
    return legend;
}

RenderObject* RenderFieldset::findLegend()
{
    for (RenderObject* legend = firstChild(); legend; legend = legend->nextSibling()) {
      if (!legend->isFloatingOrPositioned() && legend->element() &&
          legend->element()->id() == ID_LEGEND)
        return legend;
    }
    return 0;
}

void RenderFieldset::paintBoxDecorations(PaintInfo& pI, int _tx, int _ty)
{
    //kdDebug( 6040 ) << renderName() << "::paintDecorations()" << endl;

    RenderObject* legend = findLegend();
    if (!legend)
        return RenderBlock::paintBoxDecorations(pI, _tx, _ty);

    int w = width();
    int h = height() + borderTopExtra() + borderBottomExtra();
    int yOff = (legend->yPos() > 0) ? 0 : (legend->height()-borderTop())/2;
    h -= yOff;
    _ty += yOff - borderTopExtra();

    int my = qMax(_ty,pI.r.y());
    int end = qMin( pI.r.y() + pI.r.height(),  _ty + h );
    int mh = end - my;

    paintBackground(pI.p, style()->backgroundColor(), style()->backgroundLayers(), my, mh, _tx, _ty, w, h);

    if ( style()->hasBorder() )
	    paintBorderMinusLegend(pI.p, _tx, _ty, w, h, style(), legend->xPos(), legend->width());
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
                   (render_l && (ls == DOTTED || ls == DASHED || ls == DOUBLE)?style->borderLeftWidth():0), 0);
        drawBorder(p, _tx+lx+lw, _ty, _tx + w, _ty +  style->borderTopWidth(), BSTop, tc, style->color(), ts,
                   0, (render_r && (rs == DOTTED || rs == DASHED || rs == DOUBLE)?style->borderRightWidth():0));
    }

    if(render_b)
        drawBorder(p, _tx, _ty + h - style->borderBottomWidth(), _tx + w, _ty + h, BSBottom, bc, style->color(), bs,
                   (render_l && (ls == DOTTED || ls == DASHED || ls == DOUBLE)?style->borderLeftWidth():0),
                   (render_r && (rs == DOTTED || rs == DASHED || rs == DOUBLE)?style->borderRightWidth():0));

    if(render_l)
    {
	const QColor& lc = style->borderLeftColor();

	bool ignore_top =
	  (tc == lc) &&
	  (ls >= OUTSET) &&
	  (ts == DOTTED || ts == DASHED || ts == SOLID || ts == OUTSET);

	bool ignore_bottom =
	  (bc == lc) &&
	  (ls >= OUTSET) &&
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
	  (rs >= DOTTED || rs == INSET) &&
	  (ts == DOTTED || ts == DASHED || ts == SOLID || ts == OUTSET);

	bool ignore_bottom =
	  (bc == rc) &&
	  (rs >= DOTTED || rs == INSET) &&
	  (bs == DOTTED || bs == DASHED || bs == SOLID || bs == INSET);

        drawBorder(p, _tx + w - style->borderRightWidth(), _ty, _tx + w, _ty + h, BSRight, rc, style->color(), rs,
		   ignore_top?0:style->borderTopWidth(),
		   ignore_bottom?0:style->borderBottomWidth());
    }
}

void RenderFieldset::setStyle(RenderStyle* _style)
{
    RenderBlock::setStyle(_style);

    // WinIE renders fieldsets with display:inline like they're inline-blocks.  For us,
    // an inline-block is just a block element with replaced set to true and inline set
    // to true.  Ensure that if we ended up being inline that we set our replaced flag
    // so that we're treated like an inline-block.
    if (isInline())
        setReplaced(true);
}

// -------------------------------------------------------------------------

RenderFileButton::RenderFileButton(HTMLInputElementImpl *element)
    : RenderFormElement(element)
{
    KUrlRequester* w = new KUrlRequester( view()->viewport() );
    w->setObjectName( "__khtml" );

    w->setMode(KFile::File | KFile::ExistingOnly);
    w->completionObject()->setDir(KGlobalSettings::documentPath());

    connect(w->lineEdit(), SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));
    connect(w->lineEdit(), SIGNAL(textChanged(const QString &)),this,SLOT(slotTextChanged(const QString &)));

    setQWidget(w);
    m_haveFocus = false;
}



void RenderFileButton::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    const QFontMetrics &fm = style()->fontMetrics();
    int size = element()->size();

    int h = fm.lineSpacing();
    int w = fm.width( 'x' ) * (size > 0 ? size : 17); // "some"
    KLineEdit* edit = static_cast<KUrlRequester*>( m_widget )->lineEdit();

    QStyleOption opt;
    opt.init(edit);
    QSize s = edit->style()->sizeFromContents(QStyle::CT_LineEdit,
                                             &opt,
          QSize(w + 2 + 2, qMax(h, 14) + 2 + 2), edit)
        .expandedTo(QApplication::globalStrut());
    QSize bs = static_cast<KUrlRequester*>( m_widget )->sizeHint();

    setIntrinsicWidth( s.width() + bs.width() );
    setIntrinsicHeight( qMax(s.height(), bs.height()) );

    RenderFormElement::calcMinMaxWidth();
}

void RenderFileButton::handleFocusOut()
{
    if ( widget()->lineEdit() && widget()->lineEdit()->edited() ) {
        element()->onChange();
        widget()->lineEdit()->setEdited( false );
    }
}

void RenderFileButton::updateFromElement()
{
    KLineEdit* edit = widget()->lineEdit();
    edit->blockSignals(true);
    edit->setText(element()->value().string());
    edit->blockSignals(false);
    int ml = element()->maxLength();
    if ( ml < 0 || ml > 1024 )
        ml = 1024;
    edit->setMaxLength( ml );
    edit->setEdited( false );

    RenderFormElement::updateFromElement();
}

void RenderFileButton::slotReturnPressed()
{
    handleFocusOut();

    if (element()->form())
	element()->form()->submitFromKeyboard();
}

void RenderFileButton::slotTextChanged(const QString &/*string*/)
{
   element()->m_value = KUrl( widget()->url() ).prettyURL( 0, KUrl::StripFileProtocol );
}

void RenderFileButton::select()
{
    widget()->lineEdit()->selectAll();
}

// -------------------------------------------------------------------------

RenderLabel::RenderLabel(HTMLGenericFormElementImpl *element)
    : RenderFormElement(element)
{

}

// -------------------------------------------------------------------------

RenderLegend::RenderLegend(HTMLGenericFormElementImpl *element)
    : RenderBlock(element)
{
}

// -------------------------------------------------------------------------------

ComboBoxWidget::ComboBoxWidget(QWidget *parent)
    : KComboBox(false, parent)
{
    setObjectName( "__khtml" );
    //setAutoMask(true);
    if (view()) view()->installEventFilter(this);
    setMouseTracking(true);
}

bool ComboBoxWidget::event(QEvent *e)
{
    if (KComboBox::event(e))
	return true;
    if (e->type()==QEvent::KeyPress)
    {
	QKeyEvent *ke = static_cast<QKeyEvent *>(e);
	switch(ke->key())
	{
	case Qt::Key_Return:
	case Qt::Key_Enter:
	    popup();
	    ke->accept();
	    return true;
	default:
	    return false;
	}
    }
    return false;
}

bool ComboBoxWidget::eventFilter(QObject *dest, QEvent *e)
{
    if (dest==view() &&  e->type()==QEvent::KeyPress)
    {
	QKeyEvent *ke = static_cast<QKeyEvent *>(e);
	bool forward = false;
	switch(ke->key())
	{
	case Qt::Key_Tab:
	    forward=true;
	case Qt::Key_Backtab:
	    // ugly hack. emulate popdownlistbox() (private in QComboBox)
	    // we re-use ke here to store the reference to the generated event.
	    ke = new QKeyEvent(QEvent::KeyPress, Qt::Key_Escape, 0, 0);
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
            static_cast<KListBox*>(m_widget)->setSelectionMode(m_multiple ? Q3ListBox::Extended : Q3ListBox::Single);
        }
        m_selectionChanged = true;
        m_optionsChanged = true;
    }

    // update contents listbox/combobox based on options in m_element
    if ( m_optionsChanged ) {
        if (element()->m_recalcListItems)
            element()->recalcListItems();
        QVector<HTMLGenericFormElementImpl*> listItems = element()->listItems();
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
                    Q3ListBoxText *item = new Q3ListBoxText(QString(text.implementation()->s, text.implementation()->l));
                    static_cast<KListBox*>(m_widget)
                        ->insertItem(item, listIndex);
                    item->setSelectable(false);
                }
                else {
                    static_cast<KComboBox*>(m_widget)
                        ->insertItem(QString(text.implementation()->s, text.implementation()->l), listIndex);
#ifdef __GNUC__
  #warning "This needs fixing (though did it work in 3?)"
#endif
#if 0
		    static_cast<KComboBox*>(m_widget)->listBox()->item(listIndex)->setSelectable(false);
#endif
		}
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
                    text = QLatin1String("    ")+text;
                }

                if(m_useListBox) {
                    KListBox *l = static_cast<KListBox*>(m_widget);
                    l->insertItem(text, listIndex);
                    DOMString disabled = optElem->getAttribute(ATTR_DISABLED);
                    if (!disabled.isNull() && l->item( listIndex )) {
                        l->item( listIndex )->setSelectable( false );
                    }
                }  else
                    static_cast<KComboBox*>(m_widget)->insertItem(text, listIndex);
            }
            else
                KHTMLAssert(false);
            m_selectionChanged = true;
        }

        // QComboBox caches the size hint unless you call setFont (ref: TT docu)
        if(!m_useListBox) {
            KComboBox *that = static_cast<KComboBox*>(m_widget);
            that->setFont( that->font() );
        }
        setNeedsLayoutAndMinMaxRecalc();
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
    layoutIfNeeded();
    setNeedsLayoutAndMinMaxRecalc();
    // ### end FIXME

    RenderFormElement::calcMinMaxWidth();
}

void RenderSelect::layout( )
{
    KHTMLAssert(needsLayout());
    KHTMLAssert(minMaxKnown());

    // ### maintain selection properly between type/size changes, and work
    // out how to handle multiselect->singleselect (probably just select
    // first selected one)

    // calculate size
    if(m_useListBox) {
        KListBox* w = static_cast<KListBox*>(m_widget);

        Q3ListBoxItem* p = w->firstItem();
        int width = 0;
        int height = 0;
        while(p) {
            width = qMax(width, p->width(p->listBox()));
            height = qMax(height, p->height(p->listBox()));
            p = p->next();
        }
        if ( !height )
            height = w->fontMetrics().height();
        if ( !width )
            width = w->fontMetrics().width( 'x' );

        int size = m_size;
        // check if multiple and size was not given or invalid
        // Internet Exploder sets size to qMin(number of elements, 4)
        // Netscape seems to simply set it to "number of elements"
        // the average of that is IMHO qMin(number of elements, 10)
        // so I did that ;-)
        if(size < 1)
            size = qMin(static_cast<KListBox*>(m_widget)->count(), 10u);

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
    setNeedsLayout(true);
    RenderFormElement::layout();

    // and now disable the widget in case there is no <option> given
    QVector<HTMLGenericFormElementImpl*> listItems = element()->listItems();

    bool foundOption = false;
    for (int i = 0; i < listItems.size() && !foundOption; i++)
	foundOption = (listItems[i]->id() == ID_OPTION);

    m_widget->setEnabled(foundOption && ! element()->disabled());
}

void RenderSelect::slotSelected(int index) // emitted by the combobox only
{
    if ( m_ignoreSelectEvents ) return;

    KHTMLAssert( !m_useListBox );

    QVector<HTMLGenericFormElementImpl*> listItems = element()->listItems();
    if(index >= 0 && index < int(listItems.size()))
    {
        bool found = ( listItems[index]->id() == ID_OPTION );

        if ( !found ) {
            // this one is not selectable,  we need to find an option element
            while ( index < listItems.size() ) {
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

            for ( int i = 0; i < listItems.size(); ++i )
                if ( listItems[i]->id() == ID_OPTION && i !=  index )
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
    QVector<HTMLGenericFormElementImpl*> listItems = element()->m_listItems;
    for ( int i = 0; i < listItems.count(); i++ )
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
    lb->setObjectName( "__khtml" );
    lb->setSelectionMode(m_multiple ? Q3ListBox::Extended : Q3ListBox::Single);
    // ### looks broken
    //lb->setAutoMask(true);
    connect( lb, SIGNAL( selectionChanged() ), this, SLOT( slotSelectionChanged() ) );
//     connect( lb, SIGNAL( clicked( QListBoxItem * ) ), this, SLOT( slotClicked() ) );
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
    QVector<HTMLGenericFormElementImpl*> listItems = element()->listItems();
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
        int firstOption = listItems.size();
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
    : K3TextEdit(parent), m_findDlg(0), m_find(0), m_repDlg(0), m_replace(0)
{
    setObjectName( "__khtml" );
    if(wrap != DOM::HTMLTextAreaElementImpl::ta_NoWrap) {
        setWordWrap(Q3TextEdit::WidgetWidth);
        setHScrollBarMode( AlwaysOff );
        setVScrollBarMode( AlwaysOn );
    }
    else {
        setWordWrap(Q3TextEdit::NoWrap);
        setHScrollBarMode( Auto );
        setVScrollBarMode( Auto );
    }
    KCursor::setAutoHideCursor(viewport(), true);
    setTextFormat(Qt::PlainText);
    //setAutoMask(true);
    setMouseTracking(true);

    KActionCollection *ac = new KActionCollection(this);
    m_findAction = KStdAction::find( this, SLOT( slotFind() ), ac );
    m_findNextAction = KStdAction::findNext( this, SLOT( slotFindNext() ), ac );
    m_replaceAction = KStdAction::replace( this, SLOT( slotReplace() ), ac );
}


TextAreaWidget::~TextAreaWidget()
{
    delete m_replace;
    m_replace = 0L;
    delete m_find;
    m_find = 0L;
    delete m_repDlg;
    m_repDlg = 0L;
    delete m_findDlg;
    m_findDlg = 0L;
}


Q3PopupMenu *TextAreaWidget::createPopupMenu(const QPoint& pos)
{
    Q3PopupMenu *popup = K3TextEdit::createPopupMenu(pos);

    if ( !popup ) {
        return 0L;
    }

    if (!isReadOnly()) {
        popup->insertSeparator();

        m_findAction->plug(popup);
        m_findAction->setEnabled( !text().isEmpty() );

        m_findNextAction->plug(popup);
        m_findNextAction->setEnabled( m_find != 0 );

        m_replaceAction->plug(popup);
        m_replaceAction->setEnabled( !text().isEmpty() );
    }

    return popup;
}


void TextAreaWidget::slotFindHighlight(const QString& text, int matchingIndex, int matchingLength)
{
    Q_UNUSED(text)
    //kdDebug() << "Highlight: [" << text << "] mi:" << matchingIndex << " ml:" << matchingLength << endl;
    if (sender() == m_replace) {
        setSelection(m_repPara, matchingIndex, m_repPara, matchingIndex + matchingLength);
        setCursorPosition(m_repPara, matchingIndex);
    } else {
        setSelection(m_findPara, matchingIndex, m_findPara, matchingIndex + matchingLength);
        setCursorPosition(m_findPara, matchingIndex);
    }
    ensureCursorVisible();
}


void TextAreaWidget::slotReplaceText(const QString &text, int replacementIndex, int /*replacedLength*/, int matchedLength) {
    Q_UNUSED(text)
    //kdDebug() << "Replace: [" << text << "] ri:" << replacementIndex << " rl:" << replacedLength << " ml:" << matchedLength << endl;
    setSelection(m_repPara, replacementIndex, m_repPara, replacementIndex + matchedLength);
    removeSelectedText();
    insertAt(m_repDlg->replacement(), m_repPara, replacementIndex);
    if (m_replace->options() & KReplaceDialog::PromptOnReplace) {
        ensureCursorVisible();
    }
}


void TextAreaWidget::slotDoReplace()
{
    if (!m_repDlg) {
        // Should really assert()
        return;
    }

    delete m_replace;
    m_replace = new KReplace(m_repDlg->pattern(), m_repDlg->replacement(), m_repDlg->options(), this);
    if (m_replace->options() & KFind::FromCursor) {
        getCursorPosition(&m_repPara, &m_repIndex);
    } else if (m_replace->options() & KFind::FindBackwards) {
        m_repPara = paragraphs() - 1;
        m_repIndex = paragraphLength(m_repPara) - 1;
    } else {
        m_repPara = 0;
        m_repIndex = 0;
    }

    // Connect highlight signal to code which handles highlighting
    // of found text.
    connect(m_replace, SIGNAL(highlight(const QString &, int, int)),
            this, SLOT(slotFindHighlight(const QString &, int, int)));
    connect(m_replace, SIGNAL(findNext()), this, SLOT(slotReplaceNext()));
    connect(m_replace, SIGNAL(replace(const QString &, int, int, int)),
            this, SLOT(slotReplaceText(const QString &, int, int, int)));

    m_repDlg->close();
    slotReplaceNext();
}


void TextAreaWidget::slotReplaceNext()
{
    if (!m_replace) {
        // assert?
        return;
    }

    if (!(m_replace->options() & KReplaceDialog::PromptOnReplace)) {
        viewport()->setUpdatesEnabled(false);
    }

    KFind::Result res = KFind::NoMatch;
    while (res == KFind::NoMatch) {
        // If we're done.....
        if (m_replace->options() & KFind::FindBackwards) {
            if (m_repIndex == 0 && m_repPara == 0) {
                break;
            }
        } else {
            if (m_repPara == paragraphs() - 1 &&
                m_repIndex == paragraphLength(m_repPara) - 1) {
                break;
            }
        }

        if (m_replace->needData()) {
            m_replace->setData(text(m_repPara), m_repIndex);
        }

        res = m_replace->replace();

        if (res == KFind::NoMatch) {
            if (m_replace->options() & KFind::FindBackwards) {
                if (m_repPara == 0) {
                    m_repIndex = 0;
                } else {
                    m_repPara--;
                    m_repIndex = paragraphLength(m_repPara) - 1;
                }
            } else {
                if (m_repPara == paragraphs() - 1) {
                    m_repIndex = paragraphLength(m_repPara) - 1;
                } else {
                    m_repPara++;
                    m_repIndex = 0;
                }
            }
        }
    }

    if (!(m_replace->options() & KReplaceDialog::PromptOnReplace)) {
        viewport()->setUpdatesEnabled(true);
        repaintChanged();
    }

    if (res == KFind::NoMatch) { // at end
        m_replace->displayFinalDialog();
        delete m_replace;
        m_replace = 0;
        ensureCursorVisible();
        //or           if ( m_replace->shouldRestart() ) { reinit (w/o FromCursor) and call slotReplaceNext(); }
    } else {
        //m_replace->closeReplaceNextDialog();
    }
}


void TextAreaWidget::slotDoFind()
{
    if (!m_findDlg) {
        // Should really assert()
        return;
    }

    delete m_find;
    m_find = new KFind(m_findDlg->pattern(), m_findDlg->options(), this);
    if (m_find->options() & KFind::FromCursor) {
        getCursorPosition(&m_findPara, &m_findIndex);
    } else if (m_find->options() & KFind::FindBackwards) {
        m_findPara = paragraphs() - 1;
        m_findIndex = paragraphLength(m_findPara) - 1;
    } else {
        m_findPara = 0;
        m_findIndex = 0;
    }

    // Connect highlight signal to code which handles highlighting
    // of found text.
    connect(m_find, SIGNAL(highlight(const QString &, int, int)),
            this, SLOT(slotFindHighlight(const QString &, int, int)));
    connect(m_find, SIGNAL(findNext()), this, SLOT(slotFindNext()));

    m_findDlg->close();
    m_find->closeFindNextDialog();
    slotFindNext();
}


void TextAreaWidget::slotFindNext()
{
    if (!m_find) {
        // assert?
        return;
    }

    KFind::Result res = KFind::NoMatch;
    while (res == KFind::NoMatch) {
        // If we're done.....
        if (m_find->options() & KFind::FindBackwards) {
            if (m_findIndex == 0 && m_findPara == 0) {
                break;
            }
        } else {
            if (m_findPara == paragraphs() - 1 &&
                m_findIndex == paragraphLength(m_findPara) - 1) {
                break;
            }
        }

        if (m_find->needData()) {
            m_find->setData(text(m_findPara), m_findIndex);
        }

        res = m_find->find();

        if (res == KFind::NoMatch) {
            if (m_find->options() & KFind::FindBackwards) {
                if (m_findPara == 0) {
                    m_findIndex = 0;
                } else {
                    m_findPara--;
                    m_findIndex = paragraphLength(m_findPara) - 1;
                }
            } else {
                if (m_findPara == paragraphs() - 1) {
                    m_findIndex = paragraphLength(m_findPara) - 1;
                } else {
                    m_findPara++;
                    m_findIndex = 0;
                }
            }
        }
    }

    if (res == KFind::NoMatch) { // at end
        m_find->displayFinalDialog();
        delete m_find;
        m_find = 0;
        //or           if ( m_find->shouldRestart() ) { reinit (w/o FromCursor) and call slotFindNext(); }
    } else {
        //m_find->closeFindNextDialog();
    }
}


void TextAreaWidget::slotFind()
{
    if( text().isEmpty() )  // saves having to track the text changes
        return;

    if ( m_findDlg ) {
#ifdef Q_WS_WIN
      m_findDlg->activateWindow();
#else
      KWin::activateWindow( m_findDlg->winId() );
#endif
    } else {
      m_findDlg = new KFindDialog(false, this, "KHTML Text Area Find Dialog");
      connect( m_findDlg, SIGNAL(okClicked()), this, SLOT(slotDoFind()) );
    }
    m_findDlg->show();
}


void TextAreaWidget::slotReplace()
{
    if( text().isEmpty() )  // saves having to track the text changes
        return;

    if ( m_repDlg ) {
#ifdef Q_WS_WIN
      m_repDlg->activateWindow();
#else
      KWin::activateWindow( m_repDlg->winId() );
#endif
    } else {
      m_repDlg = new KReplaceDialog(this, "KHTMLText Area Replace Dialog", 0,
                                    QStringList(), QStringList(), false);
      connect( m_repDlg, SIGNAL(okClicked()), this, SLOT(slotDoReplace()) );
    }
    m_repDlg->show();
}


bool TextAreaWidget::event( QEvent *e )
{
    if ( e->type() == QEvent::AccelAvailable && isReadOnly() ) {
        QKeyEvent* ke = (QKeyEvent*) e;
        if ( ke->state() & Qt::ControlModifier ) {
            switch ( ke->key() ) {
                case Qt::Key_Left:
                case Qt::Key_Right:
                case Qt::Key_Up:
                case Qt::Key_Down:
                case Qt::Key_Home:
                case Qt::Key_End:
                    ke->accept();
                default:
                break;
            }
        }
    }
    return K3TextEdit::event( e );
}

// -------------------------------------------------------------------------

RenderTextArea::RenderTextArea(HTMLTextAreaElementImpl *element)
    : RenderFormElement(element)
{
    scrollbarsStyled = false;

    TextAreaWidget *edit = new TextAreaWidget(element->wrap(), view());
    setQWidget(edit);
    const KHTMLSettings *settings = view()->part()->settings();
    edit->setCheckSpellingEnabled( settings->autoSpellCheck() );
    edit->setTabChangesFocus( ! settings->allowTabulation() );

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
    QSize size( qMax(element()->cols(), 1L)*m.width('x') + w->frameWidth() +
                w->verticalScrollBar()->sizeHint().width(),
                qMax(element()->rows(), 1L)*m.lineSpacing() + w->frameWidth()*4 +
                (w->wordWrap() == Q3TextEdit::NoWrap ?
                 w->horizontalScrollBar()->sizeHint().height() : 0)
        );

    setIntrinsicWidth( size.width() );
    setIntrinsicHeight( size.height() );

    RenderFormElement::calcMinMaxWidth();
}

void RenderTextArea::setStyle(RenderStyle* _style)
{
    bool unsubmittedFormChange = element()->m_unsubmittedFormChange;

    RenderFormElement::setStyle(_style);

    widget()->blockSignals(true);
    widget()->setAlignment(textAlignment());
    widget()->blockSignals(false);

    scrollbarsStyled = false;

    element()->m_unsubmittedFormChange = unsubmittedFormChange;
}

void RenderTextArea::layout()
{
    KHTMLAssert( needsLayout() );

    RenderFormElement::layout();

    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);

    if (!scrollbarsStyled) {
        w->horizontalScrollBar()->setPalette(style()->palette());
        w->verticalScrollBar()->setPalette(style()->palette());
        scrollbarsStyled=true;
    }
}

void RenderTextArea::updateFromElement()
{
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    w->setReadOnly(element()->readOnly());
    QString elementText = element()->value().string();
    if ( elementText != w->text() )
    {
        w->blockSignals(true);
        int line, col;
        w->getCursorPosition( &line, &col );
        int cx = w->contentsX();
        int cy = w->contentsY();
        w->setText( elementText );
        w->setCursorPosition( line, col );
        w->scrollBy( cx, cy );
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

static QString expandLF(const QString& s)
{
    // LF -> CRLF
    unsigned crs = s.count( '\n' );
    if (crs == 0)
	return s;
    unsigned len = s.length();

    QString r;
    r.reserve(len + crs + 1);
    unsigned pos2 = 0;
    for(unsigned pos = 0; pos < len; pos++)
    {
       QChar c = s.at(pos);
       switch(c.unicode())
       {
         case '\n':
           r[pos2++] = '\r';
           r[pos2++] = '\n';
           break;

         case '\r':
           break;

         default:
           r[pos2++]= c;
           break;
       }
    }
    r.squeeze();
    return r;
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
                    paragraphText.insert(l+ll++, QLatin1String("\n"));
                    lindex = w->lineOfChar(p, l);
                }
            }
            txt += paragraphText;
            if (p < w->paragraphs() - 1)
                txt += QLatin1String("\n");
        }
    }
    else
        txt = w->text();

    return expandLF(txt);
}


void RenderTextArea::highLightWord( unsigned int length, unsigned int pos )
{
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    if ( w )
        w->highLightWord( length, pos );
}


void RenderTextArea::slotTextChanged()
{
    element()->m_dirtyvalue = true;
    if (element()->m_value != text())
        element()->m_unsubmittedFormChange = true;
}

void RenderTextArea::select()
{
    static_cast<TextAreaWidget *>(m_widget)->selectAll();
}

// ---------------------------------------------------------------------------

#include "render_form.moc"
