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

using namespace khtml;

RenderFormElement::RenderFormElement(QScrollView *view,
				     HTMLGenericFormElementImpl *element)
    : RenderWidget(view)
{
    m_element = element;
}

RenderFormElement::~RenderFormElement()
{
}

void RenderFormElement::layout(bool)
{
    if(m_widget) {
        m_widget->resize(m_width, m_height);
        m_widget->setEnabled(!m_element->disabled());
    }

}


void RenderFormElement::calcMinMaxWidth()
{
    layout(false);

    //kdDebug( 6040 ) << "inside RenderFormElement::calcMinMaxWidth()" << endl;

    m_minWidth = m_width;
    m_maxWidth = m_width;
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
}

// -------------------------------------------------------------------------

RenderButton::RenderButton(QScrollView *view,
                           HTMLGenericFormElementImpl *element)
    : RenderFormElement(view, element)
{
}

void RenderButton::layout(bool)
{
    QSize s(0, 0);

    if(m_widget)
        s = m_widget->sizeHint();

    m_height = s.height();
    m_width = s.width();

    RenderFormElement::layout(false);
}

// ------------------------------------------------------------------------------

RenderHiddenButton::RenderHiddenButton(QScrollView *view,
                                       HTMLInputElementImpl *element)
    : RenderButton(view, element)
{
}

// -------------------------------------------------------------------------------

RenderCheckBox::RenderCheckBox(QScrollView *view,
                               HTMLInputElementImpl *element)
    : RenderButton(view, element)
{
    CheckBoxWidget *b = new CheckBoxWidget(view->viewport());
    setQWidget(b);
    connect(b,SIGNAL(focused()),this,SLOT(slotFocused()));
    connect(b,SIGNAL(blurred()),this,SLOT(slotBlurred()));
    connect(b,SIGNAL(stateChanged(int)),this,SLOT(slotStateChanged(int)));
}

void RenderCheckBox::layout(bool deep)
{
    static_cast<QCheckBox*>(m_widget)->setChecked(static_cast<HTMLInputElementImpl*>(m_element)->checked());
    RenderButton::layout(deep);
}

void RenderCheckBox::slotStateChanged(int state)
{
    m_element->setAttribute(ATTR_CHECKED,state == 2 ? "" : 0);
}

// -------------------------------------------------------------------------------


RenderRadioButton::RenderRadioButton(QScrollView *view,
				     HTMLInputElementImpl *element)
    : RenderButton(view, element)
{
    RadioButtonWidget *b = new RadioButtonWidget(view->viewport());

    setQWidget(b);
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
    m_element->setAttribute(ATTR_CHECKED,"");
    if (m_element->ownerDocument()->isHTMLDocument())
	static_cast<HTMLDocumentImpl*>(m_element->ownerDocument())->updateRendering();
}

void RenderRadioButton::layout(bool deep)
{
    static_cast<QRadioButton*>(m_widget)->setChecked(static_cast<HTMLInputElementImpl*>(m_element)->checked());
    RenderButton::layout(deep);
}

// -------------------------------------------------------------------------------


RenderSubmitButton::RenderSubmitButton(QScrollView *view, HTMLInputElementImpl *element)
    : RenderButton(view, element)
{
    PushButtonWidget *p = new PushButtonWidget(view->viewport());
    setQWidget(p);
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
    static_cast<HTMLInputElementImpl*>(m_element)->mouseEventHandler( 0, DOM::NodeImpl::MouseClick, true );
    // ### if the above line calls some javascript which deletes us we will probably crash here
    if (m_element->form())
	m_element->form()->submit();
}

void RenderSubmitButton::layout(bool deep)
{
    QString value = static_cast<HTMLInputElementImpl*>(m_element)->value().isNull() ?
        defaultLabel() : static_cast<HTMLInputElementImpl*>(m_element)->value().string();
    static_cast<QPushButton*>(m_widget)->setText(value);
    RenderButton::layout(deep);
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
    static_cast<HTMLInputElementImpl*>(m_element)->mouseEventHandler( 0, DOM::NodeImpl::MouseClick, true );
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
    static_cast<HTMLInputElementImpl*>(m_element)->mouseEventHandler( 0, DOM::NodeImpl::MouseClick, true );
}


QString RenderPushButton::defaultLabel() {
    QString s;
    return s;
}

// -------------------------------------------------------------------------------


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

    setQWidget(edit);
}

void RenderLineEdit::slotReturnPressed()
{
    if (m_element->form())
	m_element->form()->maybeSubmit();
}

void RenderLineEdit::layout(bool)
{
    QFontMetrics fm( m_widget->font() );
    QSize s;

    QLineEdit *edit = static_cast<QLineEdit*>(m_widget);
    HTMLInputElementImpl *input = static_cast<HTMLInputElementImpl*>(m_element);

    int size = input->size();

    int h = fm.height();
    int w = fm.width( 'x' ) * (size > 0 ? size : 17); // "some"
    if ( edit->frame() ) {
        h += 8;
	// ### this is not really portable between all styles.
	// I think one should try to find a generic solution which
	// works with all possible styles. Lars.
        if ( m_widget->style().guiStyle() == Qt::WindowsStyle && h < 26 )
            h = 22;
        s = QSize( w + 8, h );
    } else
        s = QSize( w + 4, h + 4 );

    edit->setText(static_cast<HTMLInputElementImpl*>(m_element)->value().string());
    // ### what if maxlength goes back to 0? can we unset maxlength in the widget?
    if (input->maxLength() > 0)
	edit->setMaxLength(input->maxLength());
    edit->setReadOnly(m_element->readOnly());

    m_height = s.height();
    m_width = s.width();

    RenderFormElement::layout(false);
}

void RenderLineEdit::slotTextChanged(const QString &string)
{
    static_cast<HTMLInputElementImpl*>(m_element)->setAttribute(ATTR_VALUE,DOMString(string));
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

    setQWidget(w);
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

void RenderFileButton::layout( bool )
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

    m_edit->setText(static_cast<HTMLInputElementImpl*>(m_element)->filename().string());
    m_edit->setMaxLength(input->maxLength());

    m_edit->setReadOnly(m_element->readOnly());

    m_height = s.height();
    m_width  = s.width();

    RenderFormElement::layout(false);
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
        setQWidget(w);
        connect(w,SIGNAL(highlighted(int)),this,SLOT(slotActivated(int)));
    }
    else {
        ComboBoxWidget *w = createComboBox();
        m_size = 1;
        setQWidget(w);
        connect(w,SIGNAL(activated(int)),this,SLOT(slotActivated(int)));
    }
    m_optionsChanged = true; // options will be inserted on first layout
    m_selectedIndex = element->selectedIndex();
}

void RenderSelect::layout( bool )
{
    // ### maintain selection properly between type/size changes, and work
    // out how to handle multiselect->singleselect (probably just select
    // first selected one)

    HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_element);
    bool forceSelectionRecalc = false;

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
	
	    if(oldListbox)
		disconnect(static_cast<KListBox*>(m_widget),SIGNAL(highlighted(int)),this,SLOT(slotActivated(int)));
	    else
		disconnect(static_cast<QComboBox*>(m_widget),SIGNAL(activated(int)),this,SLOT(slotActivated(int)));
	    delete m_widget;

	    if(m_listBox) {
		ListBoxWidget *w = createListBox();
		w->setSelectionMode(m_multiple ? QListBox::Multi : QListBox::Single);
		setQWidget(w);
		connect(w,SIGNAL(highlighted(int)),this,SLOT(slotActivated(int)));
	    }
	    else {
		ComboBoxWidget *w = createComboBox();
		m_size = 1;
		setQWidget(w);
		connect(w,SIGNAL(activated(int)),this,SLOT(slotActivated(int)));
	    }
	}

	if (m_listBox && oldMultiple != m_multiple) {
	    static_cast<KListBox*>(m_widget)->setSelectionMode(m_multiple ? QListBox::Multi : QListBox::Single);
	}
    }

    if (m_optionsChanged) {
	forceSelectionRecalc = true;
	// insert all given <option>'s

	NodeImpl* current = f->firstChild();
	int i = 0;
	bool inOptGroup = false;

	listOptions.clear();
	if(m_listBox)
	    static_cast<KListBox*>(m_widget)->clear();
	else
	    static_cast<QComboBox*>(m_widget)->clear();

	while(current) {
	    if (!inOptGroup && current->id() == ID_OPTGROUP && current->firstChild()) {
		if (m_listBox) {
		    // put the optgroup label in the list - ### make this work for combo boxes
		    DOMString text = static_cast<HTMLElementImpl*>(current)->getAttribute(ATTR_LABEL);
		    if (text.isNull())
			text = "";	
		    QListBoxText *optGroupItem = new QListBoxText(QString(text.implementation()->s, text.implementation()->l));

		    static_cast<KListBox*>(m_widget)->insertItem(optGroupItem, i);
		    optGroupItem->setSelectable(false);
		    i++;
		}
	
		current = current->firstChild();
		inOptGroup = true;
	    }
	    if (current->id() == ID_OPTION) {
		DOMString text = static_cast<HTMLElementImpl*>(current)->getAttribute(ATTR_LABEL);
		if (text.isNull() && current->firstChild() && current->firstChild()->id() == ID_TEXT)
		    text = static_cast<TextImpl *>(current->firstChild())->string();
		if (text.isNull())
		    text = "";
		if (inOptGroup)
		    text = DOMString("    ")+text;

		if(m_listBox)
		    static_cast<KListBox*>(m_widget)
			->insertItem(QString(text.implementation()->s, text.implementation()->l), i);
		else
		    static_cast<QComboBox*>(m_widget)
			->insertItem(QString(text.implementation()->s, text.implementation()->l), i);
		listOptions.insert(i,static_cast<HTMLOptionElementImpl*>(current));
	    }
	    NodeImpl *parent = current->parentNode();
	    current = current->nextSibling();
	    i++;
	    if (!current) {
		if (inOptGroup) {
		    current = parent->nextSibling();
		    inOptGroup = false;
		}
	    }
	}
	m_optionsChanged = false;
    }

    if (m_selectedIndex != static_cast<HTMLSelectElementImpl*>(m_element)->selectedIndex() ||
	forceSelectionRecalc) {
	m_selectedIndex = static_cast<HTMLSelectElementImpl*>(m_element)->selectedIndex();
	// selectedIndex changed since last update
	if (m_listBox) {
	    // if multi-select, we select only the new selected index
	    int listIndex;
	    KListBox *listBox = static_cast<KListBox*>(m_widget);
	    for (listIndex = 0; listIndex < int(listBox->count()); listIndex++)
		listBox->setSelected(listIndex,0);
	    listBox->setSelected(optionToListIndex(m_selectedIndex),true);
	}
	else {
	    int listIndex = optionToListIndex(m_selectedIndex);
	    static_cast<QComboBox*>(m_widget)->setCurrentItem(listIndex >= 0 ? listIndex : 0);
	}
    }

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
        QComboBox* w = static_cast<QComboBox*>(m_widget);
        if(!w->count())
            w->setEnabled(false);
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

        m_width = width;
        m_height = height;
    }
    else
    {
        QSize s(m_widget->sizeHint());
        m_width = s.width();
        m_height = s.height();
    }

    RenderFormElement::layout(false);
}

void RenderSelect::close()
{
    HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_element);

    // Restore state
    QString state = f->ownerDocument()->registerElement(f);
    if ( !state.isEmpty())
    {
        kdDebug( 6040 ) << "Restoring SelectElem state=" << state << endl;
        restoreState( state );
    }
    setLayouted(false);

    RenderFormElement::close();
}


void RenderSelect::reset()
{
    // ### move this to HTMLSelectElementImpl
    HTMLSelectElementImpl* f = static_cast<HTMLSelectElementImpl*>(m_element);

    NodeImpl* current = f->firstChild();
    int i = 0;

    while(current) {
        if(current->id() == ID_OPTION &&
           current->firstChild() && current->firstChild()->id() == ID_TEXT) {

            HTMLOptionElementImpl* p = static_cast<HTMLOptionElementImpl*>(current);

            if (m_listBox) {
                // we only honor the first option that is selected
                // in case there is more than one selected
                static_cast<KListBox*>(m_widget)
                    ->setSelected(i, p->selected());
		static_cast<HTMLSelectElementImpl*>(m_element)->setSelectedIndex(i);
	    }
            else if(p->selected()) {
                static_cast<QComboBox*>(m_widget)->setCurrentItem(i);
		static_cast<HTMLSelectElementImpl*>(m_element)->setSelectedIndex(i);
                break;
            }
        }
        current = current->nextSibling();
        i++;
    }
}

bool RenderSelect::encoding(khtml::encodingList& encoding)
{
    bool successful = false;

    if (m_listBox) {
        KListBox* w = static_cast<KListBox*>(m_widget);
        uint i;
        for (i = 0; i < w->count(); i++)
        {
	    HTMLOptionElementImpl* p = listOptions[i];
            if (w->isSelected(i) && p)
            {
                if(p->value().isNull())
                {
                    if(w->item(i))
                    {
                        encoding += w->item(i)->text().local8Bit();
                        successful = true;
                    }
                }
                else
                {
                    encoding += p->value().string().local8Bit();
                    successful = true;
                }
            }
        }
    }
    else
    {
        QComboBox* w = static_cast<QComboBox*>(m_widget);
	HTMLOptionElementImpl* p = listOptions[w->currentItem()];
	if (p) {
	    if(p->value().isNull())
            {
		encoding += w->currentText().local8Bit();
                successful = true;
            }
	    else
            {
		encoding += p->value().string().local8Bit();
                successful = true;
            }
	}
    }

    return successful;
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
    static_cast<HTMLSelectElementImpl*>(m_element)->setSelectedIndex(listToOptionIndex(index));
}

int RenderSelect::optionToListIndex(int optionIndex)
{
    if (optionIndex < 0 || optionIndex >= int(listOptions.count()))
	return -1;
	
    int listIndex = 0;
    int optionIndex2 = 0;
    for (;
	 optionIndex2 < int(listOptions.count()) && optionIndex2 <= optionIndex;
	 listIndex++) { // not a typo!
	if (listOptions[listIndex])
	    optionIndex2++;
    }
    listIndex--;
    return listIndex;
}

int RenderSelect::listToOptionIndex(int listIndex)
{
    if (!listOptions[listIndex])
	return -1;

    int optionIndex = 0; // actual index of option not counting OPTGROUP entries that may be in list
    int i;
    for (i = 0; i < listIndex; i++)
	if (listOptions[i])
	    optionIndex++;
    return optionIndex;
}


void RenderSelect::setOptionsChanged(bool _optionsChanged)
{
    m_optionsChanged = _optionsChanged;
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
}


// -------------------------------------------------------------------------

// ### allow contents to be manipulated via DOM - will require updating
// of text node child

RenderTextArea::RenderTextArea(QScrollView *view, HTMLTextAreaElementImpl *element)
    : RenderFormElement(view, element)
{
    TextAreaWidget *edit = new TextAreaWidget(element->wrap(), view);
    setQWidget(edit);
    connect(edit,SIGNAL(textChanged()),this,SLOT(slotTextChanged()));
    connect(edit,SIGNAL(blurred()),this,SLOT(slotBlurred()));
    connect(edit,SIGNAL(focused()),this,SLOT(slotFocused()));
}

void RenderTextArea::layout( bool )
{
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    HTMLTextAreaElementImpl* f = static_cast<HTMLTextAreaElementImpl*>(m_element);

    w->setReadOnly(m_element->readOnly());
    w->setText(static_cast<HTMLTextAreaElementImpl*>(m_element)->value().string());

    QFontMetrics m = w->fontMetrics();
    QSize size( QMAX(f->cols(), 1)*m.maxWidth() + w->frameWidth()*5 +
                w->verticalScrollBar()->sizeHint().width(),
                QMAX(f->rows(), 1)*m.height() + w->frameWidth()*3 +
                (w->wordWrap() == QMultiLineEdit::NoWrap ?
                 w->horizontalScrollBar()->sizeHint().height() : 0)
        );

    m_width  = size.width();
    m_height = size.height();

    RenderFormElement::layout(false);
}

void RenderTextArea::close( )
{
    HTMLTextAreaElementImpl *f = static_cast<HTMLTextAreaElementImpl*>(m_element);

    if(f->firstChild() && f->firstChild()->id() == ID_TEXT) {
        f->setValue(static_cast<TextImpl*>(f->firstChild())->string());
	f->saveDefaults();
    }

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

void RenderTextArea::blur()
{
    disconnect(m_widget,SIGNAL(blurred()),this,SLOT(slotBlurred()));
    static_cast<TextAreaWidget *>(m_widget)->clearFocus();
    connect(m_widget,SIGNAL(blurred()),this,SLOT(slotBlurred()));
}

void RenderTextArea::focus()
{
    disconnect(m_widget,SIGNAL(focused()),this,SLOT(slotFocused()));
    static_cast<TextAreaWidget *>(m_widget)->setFocus();
    connect(m_widget,SIGNAL(focused()),this,SLOT(slotFocused()));
}

void RenderTextArea::slotBlurred()
{
    m_element->onBlur();
}

void RenderTextArea::slotFocused()
{
    m_element->onFocus();
}

void RenderTextArea::select()
{
    static_cast<TextAreaWidget *>(m_widget)->selectAll();
}


// ---------------------------------------------------------------------------

#include "render_form.moc"
