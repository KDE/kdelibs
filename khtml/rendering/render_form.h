/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000-2003 Dirk Mueller (mueller@kde.org)
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
#ifndef RENDER_FORM_H
#define RENDER_FORM_H

#include "rendering/render_replaced.h"
#include "rendering/render_image.h"
#include "rendering/render_flow.h"
#include "rendering/render_style.h"
#include "html/html_formimpl.h"

class QWidget;
class QLineEdit;
class QListboxItem;

#include <ktextedit.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qhbox.h>
#include <klistbox.h>
#include <kcombobox.h>
#include "dom/dom_misc.h"

class KHTMLPartBrowserExtension;
class KSpell;
class KFindDialog;
class KReplaceDialog;
class KFind;
class KReplace;
class KAction;
class KURLRequester;

namespace DOM {
    class HTMLFormElementImpl;
    class HTMLInputElementImpl;
    class HTMLSelectElementImpl;
    class HTMLGenericFormElementImpl;
    class HTMLTextAreaElementImpl;
}

namespace khtml {

class DocLoader;

// -------------------------------------------------------------------------

class RenderFormElement : public khtml::RenderWidget
{
public:
    RenderFormElement(DOM::HTMLGenericFormElementImpl* node);
    virtual ~RenderFormElement();

    virtual const char *renderName() const { return "RenderForm"; }

    virtual bool isFormElement() const { return true; }

    // form elements never have border/padding
    virtual int borderTop() const { return 0; }
    virtual int borderBottom() const { return 0; }
    virtual int borderLeft() const { return 0; }
    virtual int borderRight() const { return 0; }
    virtual int paddingTop() const { return 0; }
    virtual int paddingBottom() const { return 0; }
    virtual int paddingLeft() const { return 0; }
    virtual int paddingRight() const { return 0; }

    virtual void updateFromElement();

    virtual void layout();
    virtual short baselinePosition( bool ) const;

    DOM::HTMLGenericFormElementImpl *element() const
    { return static_cast<DOM::HTMLGenericFormElementImpl*>(RenderObject::element()); }

protected:
    virtual bool isRenderButton() const { return false; }
    virtual bool isEditable() const { return false; }

    QPoint m_mousePos;
    int m_state;
};

// -------------------------------------------------------------------------

// generic class for all buttons
class RenderButton : public RenderFormElement
{
    Q_OBJECT
public:
    RenderButton(DOM::HTMLGenericFormElementImpl* node);

    virtual const char *renderName() const { return "RenderButton"; }
    virtual short baselinePosition( bool ) const;

    // don't even think about making this method virtual!
    DOM::HTMLInputElementImpl* element() const
    { return static_cast<DOM::HTMLInputElementImpl*>(RenderObject::element()); }

protected:
    virtual bool isRenderButton() const { return true; }
};

// -------------------------------------------------------------------------

class RenderCheckBox : public RenderButton
{
    Q_OBJECT
public:
    RenderCheckBox(DOM::HTMLInputElementImpl* node);

    virtual const char *renderName() const { return "RenderCheckBox"; }
    virtual void updateFromElement();
    virtual void calcMinMaxWidth();

    virtual bool handleEvent(const DOM::EventImpl&) { return false; }

    QCheckBox *widget() const { return static_cast<QCheckBox*>(m_widget); }

public slots:
    virtual void slotStateChanged(int state);
};

// -------------------------------------------------------------------------

class RenderRadioButton : public RenderButton
{
public:
    RenderRadioButton(DOM::HTMLInputElementImpl* node);

    virtual const char *renderName() const { return "RenderRadioButton"; }

    virtual void calcMinMaxWidth();
    virtual void updateFromElement();

    virtual bool handleEvent(const DOM::EventImpl&) { return false; }

    QRadioButton *widget() const { return static_cast<QRadioButton*>(m_widget); }
};

// -------------------------------------------------------------------------

class RenderSubmitButton : public RenderButton
{
public:
    RenderSubmitButton(DOM::HTMLInputElementImpl *element);

    virtual const char *renderName() const { return "RenderSubmitButton"; }

    virtual QString defaultLabel();

    virtual void calcMinMaxWidth();
    virtual void updateFromElement();
    virtual short baselinePosition( bool ) const;
private:
    QString rawText();
};

// -------------------------------------------------------------------------

class RenderImageButton : public RenderImage
{
public:
    RenderImageButton(DOM::HTMLInputElementImpl *element)
        : RenderImage(element) {}

    virtual const char *renderName() const { return "RenderImageButton"; }
};


// -------------------------------------------------------------------------

class RenderResetButton : public RenderSubmitButton
{
public:
    RenderResetButton(DOM::HTMLInputElementImpl *element);

    virtual const char *renderName() const { return "RenderResetButton"; }

    virtual QString defaultLabel();
};

// -------------------------------------------------------------------------

class RenderPushButton : public RenderSubmitButton
{
public:
    RenderPushButton(DOM::HTMLInputElementImpl *element)
        : RenderSubmitButton(element) {}

    virtual QString defaultLabel();
};

// -------------------------------------------------------------------------

class RenderLineEdit : public RenderFormElement
{
    Q_OBJECT
public:
    RenderLineEdit(DOM::HTMLInputElementImpl *element);

    virtual void calcMinMaxWidth();

    virtual const char *renderName() const { return "RenderLineEdit"; }
    virtual void updateFromElement();
    virtual void setStyle(RenderStyle *style);

    void select();

    KLineEdit *widget() const { return static_cast<KLineEdit*>(m_widget); }
    DOM::HTMLInputElementImpl* element() const
    { return static_cast<DOM::HTMLInputElementImpl*>(RenderObject::element()); }
    void highLightWord( unsigned int length, unsigned int pos );

public slots:
    void slotReturnPressed();
    void slotTextChanged(const QString &string);
protected:
    virtual void handleFocusOut();

private:
    virtual bool isEditable() const { return true; }
};

// -------------------------------------------------------------------------

class LineEditWidget : public KLineEdit
{
    Q_OBJECT
public:
    LineEditWidget(DOM::HTMLInputElementImpl* input,
                   KHTMLView* view, QWidget* parent);
    ~LineEditWidget();
    void highLightWord( unsigned int length, unsigned int pos );

protected:
    virtual bool event( QEvent *e );
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual QPopupMenu *createPopupMenu();
private slots:
    void extendedMenuActivated( int id);
    void slotCheckSpelling();
    void slotSpellCheckReady( KSpell *s );
    void slotSpellCheckDone( const QString &s );
    void spellCheckerMisspelling( const QString &text, const QStringList &, unsigned int pos);
    void spellCheckerCorrected( const QString &, const QString &, unsigned int );
    void spellCheckerFinished();

private:
    enum LineEditMenuID {
        ClearHistory
    };
    DOM::HTMLInputElementImpl* m_input;
    KHTMLView* m_view;
    KSpell *m_spell;
    KAction *m_spellAction;
};

// -------------------------------------------------------------------------

class RenderFieldset : public RenderBlock
{
public:
    RenderFieldset(DOM::HTMLGenericFormElementImpl *element);

    virtual const char *renderName() const { return "RenderFieldSet"; }
protected:
    virtual void paintBoxDecorations(PaintInfo& pI, int _tx, int _ty);
    void paintBorderMinusLegend(QPainter *p, int _tx, int _ty, int w,
                                  int h, const RenderStyle *style, int lx, int lw);
    bool findLegend( int &lx, int &ly, int &lw, int &lh);
};

// -------------------------------------------------------------------------

class RenderFileButton : public RenderFormElement
{
    Q_OBJECT
public:
    RenderFileButton(DOM::HTMLInputElementImpl *element);

    virtual const char *renderName() const { return "RenderFileButton"; }
    virtual void calcMinMaxWidth();
    virtual void updateFromElement();
    void select();

    KURLRequester *widget() const { return static_cast<KURLRequester*>(m_widget); }

    DOM::HTMLInputElementImpl *element() const
    { return static_cast<DOM::HTMLInputElementImpl*>(RenderObject::element()); }

public slots:
    void slotReturnPressed();
    void slotTextChanged(const QString &string);

protected:
    virtual void handleFocusOut();

    virtual bool isEditable() const { return true; }

    bool m_clicked;
    bool m_haveFocus;
};


// -------------------------------------------------------------------------

class RenderLabel : public RenderFormElement
{
public:
    RenderLabel(DOM::HTMLGenericFormElementImpl *element);

    virtual const char *renderName() const { return "RenderLabel"; }
};


// -------------------------------------------------------------------------

class RenderLegend : public RenderBlock
{
public:
    RenderLegend(DOM::HTMLGenericFormElementImpl *element);

    virtual const char *renderName() const { return "RenderLegend"; }
};

// -------------------------------------------------------------------------

class ComboBoxWidget : public KComboBox
{
public:
    ComboBoxWidget(QWidget *parent);

protected:
    virtual bool event(QEvent *);
    virtual bool eventFilter(QObject *dest, QEvent *e);
};

// -------------------------------------------------------------------------

class RenderSelect : public RenderFormElement
{
    Q_OBJECT
public:
    RenderSelect(DOM::HTMLSelectElementImpl *element);

    virtual const char *renderName() const { return "RenderSelect"; }

    virtual void calcMinMaxWidth();
    virtual void layout();

    void setOptionsChanged(bool _optionsChanged);

    bool selectionChanged() { return m_selectionChanged; }
    void setSelectionChanged(bool _selectionChanged) { m_selectionChanged = _selectionChanged; }
    virtual void updateFromElement();

    void updateSelection();

    DOM::HTMLSelectElementImpl *element() const
    { return static_cast<DOM::HTMLSelectElementImpl*>(RenderObject::element()); }

protected:
    KListBox *createListBox();
    ComboBoxWidget *createComboBox();

    unsigned  m_size;
    bool m_multiple;
    bool m_useListBox;
    bool m_selectionChanged;
    bool m_ignoreSelectEvents;
    bool m_optionsChanged;

protected slots:
    void slotSelected(int index);
    void slotSelectionChanged();
};

// -------------------------------------------------------------------------
class TextAreaWidget : public KTextEdit
{
    Q_OBJECT
public:
    TextAreaWidget(int wrap, QWidget* parent);
    virtual ~TextAreaWidget();

protected:
    virtual bool event (QEvent *e );
    virtual QPopupMenu *createPopupMenu(const QPoint& pos);
    virtual QPopupMenu* createPopupMenu() { return KTextEdit::createPopupMenu(); }
private slots:
    void slotFind();
    void slotDoFind();
    void slotFindNext();
    void slotReplace();
    void slotDoReplace();
    void slotReplaceNext();
    void slotReplaceText(const QString&, int, int, int);
    void slotFindHighlight(const QString&, int, int);
private:
    KFindDialog *m_findDlg;
    KFind *m_find;
    KReplaceDialog *m_repDlg;
    KReplace *m_replace;
    KAction *m_findAction;
    KAction *m_findNextAction;
    KAction *m_replaceAction;
    int m_findIndex, m_findPara;
    int m_repIndex, m_repPara;
};


// -------------------------------------------------------------------------

class RenderTextArea : public RenderFormElement
{
    Q_OBJECT
public:
    RenderTextArea(DOM::HTMLTextAreaElementImpl *element);
    ~RenderTextArea();

    virtual const char *renderName() const { return "RenderTextArea"; }
    virtual void calcMinMaxWidth();
    virtual void layout();
    virtual void setStyle(RenderStyle *style);

    virtual void close ( );
    virtual void updateFromElement();

    // don't even think about making this method virtual!
    TextAreaWidget *widget() const { return static_cast<TextAreaWidget*>(m_widget); }
    DOM::HTMLTextAreaElementImpl* element() const
    { return static_cast<DOM::HTMLTextAreaElementImpl*>(RenderObject::element()); }

    QString text();
    void highLightWord( unsigned int length, unsigned int pos );

    void select();

protected slots:
    void slotTextChanged();

protected:
    virtual void handleFocusOut();

    virtual bool isEditable() const { return true; }

    bool scrollbarsStyled;
};

// -------------------------------------------------------------------------

} //namespace

#endif
