/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk A. Mueller (mueller@kde.org)
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
#include <kurl.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kapp.h>

#include <qcombobox.h>
#include <qstack.h>
#include <qlayout.h>

#include "dom_nodeimpl.h"
#include "dom_textimpl.h"
#include "dom_docimpl.h"

#include "html/html_formimpl.h"
#include "html/html_documentimpl.h"
#include "misc/htmlhashes.h"

#include "rendering/render_form.h"
#include "rendering/render_style.h"
#include <assert.h>

#include "khtmlview.h"

using namespace khtml;

RenderFormElement::RenderFormElement(QScrollView *view,
                                     HTMLGenericFormElementImpl *element)
    : RenderWidget(view)
{
    // init RenderObject attributes
    m_inline = true;   // our object is Inline

    m_element = element;
}

RenderFormElement::~RenderFormElement()
{
}

void RenderFormElement::applyLayout(int iWidth, int iHeight)
{
    if(m_widget) {
        m_widget->resize(iWidth, iHeight);
        m_widget->setEnabled(!m_element->disabled());
    }

    m_width  = iWidth  + borderLeft() + paddingLeft() + paddingRight() + borderRight();
    m_height = iHeight + borderTop() + paddingTop() + paddingBottom() + borderBottom();
}

void RenderFormElement::calcMinMaxWidth()
{
    layout();

    //kdDebug( 6040 ) << "inside RenderFormElement::calcMinMaxWidth()" << endl;

    m_minWidth = m_width;
    m_maxWidth = m_width;
}

void RenderFormElement::blur()
{
    disconnect(m_widget,SIGNAL(blurred()),this,SLOT(slotBlurred()));
    m_widget->clearFocus();
    connect(m_widget,SIGNAL(blurred()),this,SLOT(slotBlurred()));
}

void RenderFormElement::focus()
{
    disconnect(m_widget,SIGNAL(focused()),this,SLOT(slotFocused()));
    m_widget->setFocus();
    connect(m_widget,SIGNAL(focused()),this,SLOT(slotFocused()));
}

void RenderFormElement::slotBlurred()
{
    m_element->onBlur();
}

void RenderFormElement::slotFocused()
{
    m_element->onFocus();
}

void RenderFormElement::slotSelected()
{
}

void RenderFormElement::slotClicked()
{
    m_element->mouseEventHandler(0, DOM::NodeImpl::MouseClick, true);
}

// -------------------------------------------------------------------------

RenderButton::RenderButton(QScrollView *view,
                           HTMLGenericFormElementImpl *element)
    : RenderFormElement(view, element)
{
}

void RenderButton::layout()
{
    QSize s(0, 0);

    if(m_widget)
        s = m_widget->sizeHint();

    applyLayout(s.width(), s.height());

    setLayouted();
}

// ------------------------------------------------------------------------------

RenderHiddenButton::RenderHiddenButton(QScrollView *view,
                                       HTMLInputElementImpl *element)
    : RenderButton(view, element)
{
}

// -------------------------------------------------------------------------------

CheckBoxWidget::CheckBoxWidget(QWidget *parent)
    : QCheckBox(parent)
{
    setAutoMask(true);
}

void CheckBoxWidget::focusInEvent(QFocusEvent* e)
{
    QCheckBox::focusInEvent(e);
    emit focused();
}

void CheckBoxWidget::focusOutEvent(QFocusEvent* e)
{
    QCheckBox::focusOutEvent(e);
    emit blurred();
}

// -------------------------------------------------------------------------------

RenderCheckBox::RenderCheckBox(QScrollView *view,
                               HTMLInputElementImpl *element)
    : RenderButton(view, element)
{
    CheckBoxWidget *b = new CheckBoxWidget(view->viewport());
    setQWidget(b, false);
    connect(b,SIGNAL(focused()),this,SLOT(slotFocused()));
    connect(b,SIGNAL(blurred()),this,SLOT(slotBlurred()));
    connect(b,SIGNAL(stateChanged(int)),this,SLOT(slotStateChanged(int)));
    connect(b,SIGNAL(clicked()), this, SLOT(slotClicked()));
}

void RenderCheckBox::layout()
{
    static_cast<QCheckBox*>(m_widget)->setChecked(static_cast<HTMLInputElementImpl*>(m_element)->checked());
    RenderButton::layout();
}

void RenderCheckBox::slotStateChanged(int state)
{
    m_element->setAttribute(ATTR_CHECKED,state == 2 ? "" : 0);
}

// -------------------------------------------------------------------------------

RadioButtonWidget::RadioButtonWidget(QWidget *parent)
    : QRadioButton(parent)
{
    setAutoMask(true);
}

void RadioButtonWidget::focusInEvent(QFocusEvent* e)
{
    QRadioButton::focusInEvent(e);
    emit focused();
}

void RadioButtonWidget::focusOutEvent(QFocusEvent* e)
{
    QRadioButton::focusOutEvent(e);
    emit blurred();
}

// -------------------------------------------------------------------------------

RenderRadioButton::RenderRadioButton(QScrollView *view,
                                     HTMLInputElementImpl *element)
    : RenderButton(view, element)
{
    RadioButtonWidget *b = new RadioButtonWidget(view->viewport());

    setQWidget(b, false);
    connect(b,SIGNAL(focused()),this,SLOT(slotFocused()));
    connect(b,SIGNAL(blurred()),this,SLOT(slotBlurred()));
    connect(b, SIGNAL(clicked()), this, SLOT(slotClicked()));
}

void RenderRadioButton::setChecked(bool checked)
{
    static_cast<QRadioButton *>(m_widget)->setChecked(checked);
}

void RenderRadioButton::slotClicked()
{
    // emit mouseClick event etc
    RenderButton::slotClicked();

    m_element->setAttribute(ATTR_CHECKED,"");

    if (m_element->ownerDocument()->isHTMLDocument())
        static_cast<HTMLDocumentImpl*>(m_element->ownerDocument())->updateRendering();
}

void RenderRadioButton::layout()
{
    static_cast<QRadioButton*>(m_widget)->setChecked(static_cast<HTMLInputElementImpl*>(m_element)->checked());
    RenderButton::layout();
}

// -------------------------------------------------------------------------------

void PushButtonWidget::focusInEvent(QFocusEvent* e)
{
    QPushButton::focusInEvent(e);
    emit focused();
}

void PushButtonWidget::focusOutEvent(QFocusEvent* e)
{
    QPushButton::focusOutEvent(e);
    emit blurred();
}

// -------------------------------------------------------------------------------


RenderSubmitButton::RenderSubmitButton(QScrollView *view, HTMLInputElementImpl *element)
    : RenderButton(view, element)
{
    PushButtonWidget *p = new PushButtonWidget(view->viewport());
    setQWidget(p, false);
    connect(m_widget,SIGNAL(focused()),this,SLOT(slotFocused()));
    connect(m_widget,SIGNAL(blurred()),this,SLOT(slotBlurred()));

    connect(p, SIGNAL(clicked()), this, SLOT(slotClicked()));
    m_clicked = false;
}

RenderSubmitButton::~RenderSubmitButton()
{
}

void RenderSubmitButton::slotClicked()
{
    m_clicked = true;
    RenderButton::slotClicked();

    // ### if the above line calls some javascript which deletes us we will probably crash here
    if (m_element->form())
        m_element->form()->prepareSubmit();
}

void RenderSubmitButton::layout()
{
    QString value = static_cast<HTMLInputElementImpl*>(m_element)->value().isEmpty() ?
        defaultLabel() : static_cast<HTMLInputElementImpl*>(m_element)->value().string();
    static_cast<PushButtonWidget*>(m_widget)->setText(value.stripWhiteSpace());
    static_cast<PushButtonWidget*>(m_widget)->setFont(m_style->font());

    RenderButton::layout();
}

QString RenderSubmitButton::defaultLabel() {
    return i18n("Submit");
}


// -------------------------------------------------------------------------------

RenderImageButton::RenderImageButton(HTMLInputElementImpl *element)
    : RenderImage()
{
    m_element = element;
}

RenderImageButton::~RenderImageButton()
{
}

// -------------------------------------------------------------------------------

RenderResetButton::RenderResetButton(QScrollView *view, HTMLInputElementImpl *element)
    : RenderSubmitButton(view, element)
{
}

RenderResetButton::~RenderResetButton()
{
}

void RenderResetButton::slotClicked()
{
    RenderSubmitButton::slotClicked();
    if (m_element->form())
        m_element->form()->reset();
}

QString RenderResetButton::defaultLabel() {
    return i18n("Reset");
}


// -------------------------------------------------------------------------------

RenderPushButton::RenderPushButton(QScrollView *view, HTMLInputElementImpl *element)
    : RenderSubmitButton(view, element)
{
}

RenderPushButton::~RenderPushButton()
{
}

void RenderPushButton::slotClicked()
{
    // DON't call RenderSubmitButton::slotClicked here!
    RenderButton::slotClicked();
}


QString RenderPushButton::defaultLabel() {
    QString s;
    return s;
}

// -------------------------------------------------------------------------------

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
    return QLineEdit::event( e );
}

void LineEditWidget::focusInEvent(QFocusEvent* e)
{
    QLineEdit::focusInEvent(e);
    emit focused();
}

void LineEditWidget::focusOutEvent(QFocusEvent* e)
{
    QLineEdit::focusOutEvent(e);
    emit blurred();
}


// -----------------------------------------------------------------------------

RenderLineEdit::RenderLineEdit(QScrollView *view, HTMLInputElementImpl *element)
    : RenderFormElement(view, element)
{
    LineEditWidget *edit = new LineEditWidget(view);
    connect(edit,SIGNAL(focused()),this,SLOT(slotFocused()));
    connect(edit,SIGNAL(blurred()),this,SLOT(slotBlurred()));
    connect(edit,SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));
    connect(edit,SIGNAL(textChanged(const QString &)),this,SLOT(slotTextChanged(const QString &)));

    if(element->inputType() == HTMLInputElementImpl::PASSWORD)
        edit->setEchoMode( QLineEdit::Password );

    setQWidget(edit, false);
}

void RenderLineEdit::slotReturnPressed()
{
    if (m_element->form())
        m_element->form()->maybeSubmit();
}

void RenderLineEdit::layout()
{
    QFontMetrics fm( m_widget->font() );
    QSize s;

    QLineEdit *edit = static_cast<QLineEdit*>(m_widget);
    HTMLInputElementImpl *input = static_cast<HTMLInputElementImpl*>(m_element);

    int size = input->size();

    edit->constPolish();

    int h = fm.height();
    int w = fm.width( 'x' ) * (size > 0 ? size : 17); // "some"
    if ( edit->frame() ) {
        h += 8;
        // ### this is not really portable between all styles.
        // I think one should try to find a generic solution which
        // works with all possible styles. Lars.
        // ### well, it is. it's the 1:1 copy of QLineEdit::sizeHint()
        // the only reason that made me including this thingie is
        // that I cannot get a sizehint for a specific number of characters
        // in the lineedit from it. It's not my fault, it's Qt's. Dirk
        if ( m_widget->style().guiStyle() == Qt::WindowsStyle && h < 26 )
            h = 22;
        s = QSize( w + 8, h ).expandedTo( QApplication::globalStrut() );
    } else
	s = QSize( w + 4, h + 4 ).expandedTo( QApplication::globalStrut() );

    edit->blockSignals(true);
    edit->setText(static_cast<HTMLInputElementImpl*>(m_element)->value().string());
    edit->blockSignals(false);

    // ### what if maxlength goes back to 0? can we unset maxlength in the widget?
    if (input->maxLength() > 0)
        edit->setMaxLength(input->maxLength());
    edit->setReadOnly(m_element->readOnly());

    applyLayout(s.width(), s.height());
    setLayouted();
}

void RenderLineEdit::slotTextChanged(const QString &string)
{
    static_cast<HTMLInputElementImpl*>(m_element)->setValue(DOMString(string));
}

void RenderLineEdit::select()
{
    static_cast<LineEditWidget*>(m_widget)->selectAll();
}


// ---------------------------------------------------------------------------

RenderFieldset::RenderFieldset(QScrollView *view,
                               HTMLGenericFormElementImpl *element)
    : RenderFormElement(view, element)
{
}

RenderFieldset::~RenderFieldset()
{
}


// -------------------------------------------------------------------------


RenderFileButton::RenderFileButton(QScrollView *view, HTMLInputElementImpl *element)
    : RenderFormElement(view, element)
{
    QWidget *w = new QWidget(view->viewport());
    QHBoxLayout *layout = new QHBoxLayout(w);

    m_edit = new LineEditWidget(w);
    connect(m_edit, SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));
    connect(m_edit, SIGNAL(textChanged(const QString &)),this,SLOT(slotTextChanged(const QString &)));
    connect(m_edit,SIGNAL(focused()),this,SLOT(slotFocused()));
    connect(m_edit,SIGNAL(blurred()),this,SLOT(slotBlurred()));
    m_button = new PushButtonWidget(i18n("Browse..."), w);
    connect(m_button, SIGNAL(clicked()), this, SLOT(slotClicked()));
    connect(m_button,SIGNAL(focused()),this,SLOT(slotFocused()));
    connect(m_button,SIGNAL(blurred()),this,SLOT(slotBlurred()));

    if (element->maxLength() > 0) m_edit->setMaxLength(element->maxLength());

    layout->addWidget(m_edit);
    layout->addWidget(m_button);

    setQWidget(w, false);
    m_haveFocus = false;
}

RenderFileButton::~RenderFileButton()
{
}

void RenderFileButton::slotClicked()
{
    QString file_name = KFileDialog::getOpenFileName(QString::null, QString::null, 0, i18n("Browse..."));
    if (!file_name.isNull()) {
        // ### truncate if > maxLength
        static_cast<HTMLInputElementImpl*>(m_element)->setFilename(DOMString(file_name));
        m_edit->setText(file_name);
    }
}

void RenderFileButton::layout( )
{
    // this is largely taken from the RenderLineEdit layout
    QFontMetrics fm( m_edit->font() );
    QSize s;
    HTMLInputElementImpl *input = static_cast<HTMLInputElementImpl*>(m_element);
    int size = input->size();

    int h = fm.height();
    int w = fm.width( 'x' ) * (size > 0 ? size : 17);
    w += m_button->sizeHint().width();

    if ( m_edit->frame() ) {
        h += 8;
        if ( m_widget->style().guiStyle() == Qt::WindowsStyle && h < 26 )
            h = 22;
        s = QSize( w + 8, h );
    } else
        s = QSize( w + 4, h + 4 );

    m_edit->blockSignals(true);
    m_edit->setText(static_cast<HTMLInputElementImpl*>(m_element)->filename().string());
    m_edit->blockSignals(false);
    m_edit->setMaxLength(input->maxLength());

    m_edit->setReadOnly(m_element->readOnly());

    applyLayout(s.width(), s.height());
    setLayouted();
}

void RenderFileButton::slotReturnPressed()
{
    if (m_element->form())
        m_element->form()->maybeSubmit();
}

void RenderFileButton::slotTextChanged(const QString &string)
{
    static_cast<HTMLInputElementImpl*>(m_element)->setFilename(DOMString(string));
}

void RenderFileButton::slotBlurred()
{
    if (sender() != m_edit && sender() != m_button)
        return;

    if ((sender() == m_edit && m_button->hasFocus()) ||
        (sender() == m_button && m_edit->hasFocus()))
        m_haveFocus = true;
    else {
        m_haveFocus = false;
        RenderFormElement::slotBlurred();
    }
}

void RenderFileButton::slotFocused()
{
    if (sender() != m_edit && sender() != m_button)
        return;

    if (!m_haveFocus)
        RenderFormElement::slotFocused();
    m_haveFocus = true;
}

void RenderFileButton::select()
{
    m_edit->selectAll();
}


// -------------------------------------------------------------------------

RenderLabel::RenderLabel(QScrollView *view,
                         HTMLGenericFormElementImpl *element)
    : RenderFormElement(view, element)
{

}

RenderLabel::~RenderLabel()
{
}


// -------------------------------------------------------------------------

RenderLegend::RenderLegend(QScrollView *view,
                           HTMLGenericFormElementImpl *element)
    : RenderFormElement(view, element)
{
}

RenderLegend::~RenderLegend()
{
}


// -------------------------------------------------------------------------------

ListBoxWidget::ListBoxWidget(QWidget *parent)
    : KListBox(parent)
{
    // ### looks broken
    //setAutoMask(true);
}

void ListBoxWidget::focusInEvent(QFocusEvent* e)
{
    KListBox::focusInEvent(e);
    emit focused();
}

void ListBoxWidget::focusOutEvent(QFocusEvent* e)
{
    KListBox::focusOutEvent(e);
    emit blurred();
}


// -------------------------------------------------------------------------------

ComboBoxWidget::ComboBoxWidget(QWidget *parent)
    : QComboBox(parent)
{
    setAutoMask(true);
}

void ComboBoxWidget::focusInEvent(QFocusEvent* e)
{
    QComboBox::focusInEvent(e);
    emit focused();
}

void ComboBoxWidget::focusOutEvent(QFocusEvent* e)
{
    QComboBox::focusOutEvent(e);
    emit blurred();
}


// -------------------------------------------------------------------------

RenderSelect::RenderSelect(QScrollView *view, HTMLSelectElementImpl *element)
    : RenderFormElement(view, element)
{
    m_multiple = element->multiple();
    m_size = element->size();

    m_listBox = (m_multiple || m_size > 1);

    if(m_listBox) {
        ListBoxWidget *w = createListBox();
        w->setSelectionMode(m_multiple ? QListBox::Multi : QListBox::Single);
        setQWidget(w, false);
        connect(w,SIGNAL(highlighted(int)),this,SLOT(slotActivated(int)));
    }
    else {
        ComboBoxWidget *w = createComboBox();
        m_size = 1;
        setQWidget(w, false);
        connect(w,SIGNAL(activated(int)),this,SLOT(slotActivated(int)));
    }
    m_ignoreSelectEvents = false;
}

void RenderSelect::layout( )
{
    // ### maintain selection properly between type/size changes, and work
    // out how to handle multiselect->singleselect (probably just select
    // first selected one)

    HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_element);
    m_ignoreSelectEvents = true;

    // change widget type

    bool oldMultiple = m_multiple;
    unsigned oldSize = m_size;
    bool oldListbox = m_listBox;

    m_multiple = f->multiple();
    m_size = f->size();
    m_listBox = (m_multiple || m_size > 1);

    if (oldMultiple != m_multiple || oldSize != m_size) {
        if (m_listBox != oldListbox) {
            // type of select has changed
            delete m_widget;

            if(oldListbox)
                disconnect(m_widget,SIGNAL(highlighted(int)),this,SLOT(slotActivated(int)));
            else
                disconnect(m_widget,SIGNAL(activated(int)),this,SLOT(slotActivated(int)));

            if(m_listBox) {
                ListBoxWidget *w = createListBox();
                w->setSelectionMode(m_multiple ? QListBox::Multi : QListBox::Single);
                setQWidget(w, false);
                connect(m_widget,SIGNAL(highlighted(int)),this,SLOT(slotActivated(int)));
            }
            else {
                setQWidget(createComboBox(), false);
                m_size = 1;
                connect(m_widget,SIGNAL(activated(int)),this,SLOT(slotActivated(int)));
            }
        }

        if (m_listBox && oldMultiple != m_multiple) {
            static_cast<KListBox*>(m_widget)->setSelectionMode(m_multiple ? QListBox::Multi : QListBox::Single);
        }
        m_selectionChanged = true;
    }

    HTMLSelectElementImpl *select = static_cast<HTMLSelectElementImpl*>(m_element);

    // update contents listbox/combobox based on options in m_element
    // ### check if we actually need to do this
    QArray<HTMLGenericFormElementImpl*> listItems = select->listItems();
    int listIndex;

    if(m_listBox)
        static_cast<KListBox*>(m_widget)->clear();
    else
        static_cast<QComboBox*>(m_widget)->clear();

    for (listIndex = 0; listIndex < int(listItems.size()); listIndex++) {
        if (listItems[listIndex]->id() == ID_OPTGROUP) {
            DOMString text = listItems[listIndex]->getAttribute(ATTR_LABEL);
            if (text.isNull())
                text = "";

            if(m_listBox) {
                QListBoxText *item = new QListBoxText(QString(text.implementation()->s, text.implementation()->l));
                static_cast<KListBox*>(m_widget)
                    ->insertItem(item, listIndex);
                item->setSelectable(false);
            }
            else
                static_cast<QComboBox*>(m_widget)
                    ->insertItem(QString(text.implementation()->s, text.implementation()->l), listIndex);
        }
        else if (listItems[listIndex]->id() == ID_OPTION) {
            DOMString text = static_cast<HTMLOptionElementImpl*>(listItems[listIndex])->text();
            if (text.isNull())
                text = "";
            if (listItems[listIndex]->parentNode()->id() == ID_OPTGROUP)
                text = DOMString("    ")+text;

            if(m_listBox)
                static_cast<KListBox*>(m_widget)
                    ->insertItem(QString(text.implementation()->s, text.implementation()->l), listIndex);
            else
                static_cast<QComboBox*>(m_widget)
                    ->insertItem(QString(text.implementation()->s, text.implementation()->l), listIndex);
        }
        else
            assert(false);
        m_selectionChanged = true;
    }

    // update selection
    if (m_selectionChanged)
        updateSelection();
    m_selectionChanged = false;

    if (m_listBox) {
        // check if multiple and size was not given or invalid
        // Internet Exploder sets size to QMIN(number of elements, 4)
        // Netscape seems to simply set it to "number of elements"
        // the average of that is IMHO QMIN(number of elements, 15)
        // so I did that ;-)
        if(m_multiple && m_size < 1)
            m_size = QMIN(static_cast<KListBox*>(m_widget)->count(), 15);
    }
    else {
        // and now disable the widget in case there is no <option> given
        // ### do the same if there is only optgroups
        QComboBox* w = static_cast<QComboBox*>(m_widget);
        if(!w->count())
            w->setEnabled(false);
        // ### select the first option (unless another specified), in case the first item is an optgroup
    }

//    reset();


    // calculate size

    if(m_listBox) {
        KListBox* w = static_cast<KListBox*>(m_widget);

        QListBoxItem* p = w->firstItem();
        int width = 0;
        int height = 0;
        while(p) {
            width = QMAX(width, p->width(p->listBox()));
            height = QMAX(height, p->height(p->listBox()));
            p = p->next();
        }

        width += 2*w->frameWidth() + w->verticalScrollBar()->sizeHint().width();
        height = QMAX(m_size, 1)*height + 2*w->frameWidth();

        applyLayout(width, height);
    }
    else
    {
        QSize s(m_widget->sizeHint());

        applyLayout(s.width(), s.height());
    }

    setLayouted();

    m_ignoreSelectEvents = false;

}

void RenderSelect::close()
{
    HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_element);

    // Restore state
    QString state = f->ownerDocument()->registerElement(f);
    if ( !state.isEmpty())
    {
        restoreState( state );
    }
    setLayouted(false);
    static_cast<HTMLSelectElementImpl*>(m_element)->recalcListItems();

    RenderFormElement::close();
}

QString RenderSelect::state()
{
    QString state;

    if(m_listBox) {

        KListBox* w = static_cast<KListBox*>(m_widget);

        HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_element);

        NodeImpl* current = f->firstChild();
        int i = 0;
        while(current) {
            if (current->id() == ID_OPTION)
            {
                if( w->isSelected(i))
                    state += 'X';
                else
                    state += 'O';
            }
            current = current->nextSibling();
            i++;
        }
    }
    else
    {
        QComboBox* w = static_cast<QComboBox*>(m_widget);
        HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_element);

        NodeImpl* current = f->firstChild();
        int i = 0;
        while(current) {
            if (current->id() == ID_OPTION )
            {
                if( i == w->currentItem())
                    state += 'X';
                else
                    state += 'O';
            }
            current = current->nextSibling();
            i++;
        }
    }

    return state;
}

void RenderSelect::restoreState(const QString &state)
{
    if (m_listBox) {

        KListBox* w = static_cast<KListBox*>(m_widget);

        HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_element);

        NodeImpl* current = f->firstChild();
        int i = 0;
        while(current) {
            if (current->id() == ID_OPTION)
            {
                w->setSelected(i, (state[i] == 'X'));
            }
            current = current->nextSibling();
            i++;
        }
    }
    else
    {
        QComboBox* w = static_cast<QComboBox*>(m_widget);
        HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_element);

        NodeImpl* current = f->firstChild();
        int i = 0;
        while(current) {
            if (current->id() == ID_OPTION )
            {
                if (state[i] == 'X')
                    w->setCurrentItem(i);
            }
            current = current->nextSibling();
            i++;
        }
    }

}

void RenderSelect::slotActivated(int index)
{
    if (m_ignoreSelectEvents)
        return;

    m_ignoreSelectEvents = true;

    QArray<HTMLGenericFormElementImpl*> listItems = static_cast<HTMLSelectElementImpl*>(m_element)->listItems();
    if (index < 0 || index >= int(listItems.size()))
        return;

    if (listItems[index]->id() == ID_OPTION) {
        if (m_listBox)
            static_cast<HTMLOptionElementImpl*>(listItems[index])->setSelected(static_cast<KListBox*>(m_widget)->isSelected(index));
        else
            static_cast<HTMLOptionElementImpl*>(listItems[index])->setSelected(true);
    }
    else {
        // nope, sorry, can't select this
        if (m_listBox)
            static_cast<KListBox*>(m_widget)->setCurrentItem(-1);
        else {
            // ###
        }
    }

    m_ignoreSelectEvents = false;

    static_cast<HTMLSelectElementImpl*>(m_element)->onChange();

}

void RenderSelect::setOptionsChanged(bool /*ptionsChanged*/)
{
    // ###
//    m_optionsChanged = _optionsChanged;
}

ListBoxWidget *RenderSelect::createListBox()
{
    ListBoxWidget *lb = new ListBoxWidget(m_view);
    connect(lb,SIGNAL(focused()),this,SLOT(slotFocused()));
    connect(lb,SIGNAL(blurred()),this,SLOT(slotBlurred()));
    return lb;
}

ComboBoxWidget *RenderSelect::createComboBox()
{
    ComboBoxWidget *cb = new ComboBoxWidget(m_view);
    connect(cb,SIGNAL(focused()),this,SLOT(slotFocused()));
    connect(cb,SIGNAL(blurred()),this,SLOT(slotBlurred()));
    return cb;
}

void RenderSelect::updateSelection()
{
    QArray<HTMLGenericFormElementImpl*> listItems = static_cast<HTMLSelectElementImpl*>(m_element)->listItems();
    int i;
    if (m_listBox) {
        // if multi-select, we select only the new selected index
        KListBox *listBox = static_cast<KListBox*>(m_widget);
        for (i = 0; i < int(listItems.size()); i++)
            listBox->setSelected(i,listItems[i]->id() == ID_OPTION &&
                                static_cast<HTMLOptionElementImpl*>(listItems[i])->selected());
    }
    else {
        for (i = 0; i < int(listItems.size()); i++)
            if (listItems[i]->id() == ID_OPTION && static_cast<HTMLOptionElementImpl*>(listItems[i])->selected()) {
                static_cast<QComboBox*>(m_widget)->setCurrentItem(i);
                return;
            }
        static_cast<QComboBox*>(m_widget)->setCurrentItem(0); // ### wrong if item 0 is an optgroup
    }
}


// -------------------------------------------------------------------------

TextAreaWidget::TextAreaWidget(int wrap, QWidget* parent)
    : QMultiLineEdit(parent)
{
    if(wrap != DOM::HTMLTextAreaElementImpl::ta_NoWrap) {
        setWordWrap(QMultiLineEdit::WidgetWidth);
        clearTableFlags(Tbl_autoScrollBars);
        setTableFlags(Tbl_vScrollBar);
    }
    else {
        clearTableFlags(Tbl_autoScrollBars);
        setTableFlags(Tbl_vScrollBar | Tbl_hScrollBar);
    }
    setAutoMask(true);
}

void TextAreaWidget::focusInEvent(QFocusEvent* e)
{
    QMultiLineEdit::focusInEvent(e);
    emit focused();
}

void TextAreaWidget::focusOutEvent(QFocusEvent* e)
{
    QMultiLineEdit::focusOutEvent(e);
    emit blurred();
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
    return QMultiLineEdit::event( e );
}


// -------------------------------------------------------------------------

// ### allow contents to be manipulated via DOM - will require updating
// of text node child

RenderTextArea::RenderTextArea(QScrollView *view, HTMLTextAreaElementImpl *element)
    : RenderFormElement(view, element)
{
    TextAreaWidget *edit = new TextAreaWidget(element->wrap(), view);
    setQWidget(edit, false);
    connect(edit,SIGNAL(textChanged()),this,SLOT(slotTextChanged()));
    connect(edit,SIGNAL(blurred()),this,SLOT(slotBlurred()));
    connect(edit,SIGNAL(focused()),this,SLOT(slotFocused()));
}

void RenderTextArea::layout( )
{
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    HTMLTextAreaElementImpl* f = static_cast<HTMLTextAreaElementImpl*>(m_element);

    w->setReadOnly(m_element->readOnly());
    w->blockSignals(true);
    w->setText(static_cast<HTMLTextAreaElementImpl*>(m_element)->value().string());
    w->blockSignals(false);

    QFontMetrics m = w->fontMetrics();
    QSize size( QMAX(f->cols(), 1)*m.maxWidth() + w->frameWidth()*5 +
                w->verticalScrollBar()->sizeHint().width(),
                QMAX(f->rows(), 1)*m.height() + w->frameWidth()*3 +
                (w->wordWrap() == QMultiLineEdit::NoWrap ?
                 w->horizontalScrollBar()->sizeHint().height() : 0)
        );

    applyLayout(size.width(), size.height());

    setLayouted();
}

void RenderTextArea::close( )
{
    HTMLTextAreaElementImpl *f = static_cast<HTMLTextAreaElementImpl*>(m_element);

    if(f->firstChild() && f->firstChild()->id() == ID_TEXT)
        f->setValue(static_cast<TextImpl*>(f->firstChild())->string());

    layout();

    RenderFormElement::close();
}

QString RenderTextArea::text()
{
    return static_cast<TextAreaWidget *>(m_widget)->text();
}

void RenderTextArea::slotTextChanged()
{
    static_cast<HTMLTextAreaElementImpl*>(m_element)->setValue(static_cast<TextAreaWidget *>(m_widget)->text());
}

void RenderTextArea::select()
{
    static_cast<TextAreaWidget *>(m_widget)->selectAll();
}


// ---------------------------------------------------------------------------

#include "render_form.moc"
