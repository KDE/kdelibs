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
#ifndef RENDER_FORM_H
#define RENDER_FORM_H

// TODO:
//
// ### we need a generic event handling mechanism for elements replaced by widgets
// ### this is just a hack and does only work for onclick :-(
//
// ### tabbing order
// ### return == submit in some cases

#include "render_replaced.h"

class QWidget;
class QMultiLineEdit;
class QScrollView;
class QLineEdit;

#include <qpixmap.h>

namespace DOM {
    class HTMLFormElementImpl;
    class HTMLInputElementImpl;
};

namespace khtml {

// -------------------------------------------------------------------------

class RenderFormElement : public khtml::RenderWidget
{
    Q_OBJECT
public:
    RenderFormElement(QScrollView *view,
                      HTMLFormElementImpl *form);
    virtual ~RenderFormElement();

    virtual const char *renderName() const { return "RenderForm"; }

    HTMLFormElementImpl *form() { return m_form; }
    void setGenericFormElement(HTMLGenericFormElementImpl *gform)
    { m_gform = gform; }

    virtual bool isRendered() { return true; }

    // enum to get the type of form element
    enum Type {
	ResetButton,
	SubmitButton,
	RadioButton,
        HiddenButton,
	CheckBox,
	PushButton,
	File,
	Hidden,
	LineEdit,
        Select,
        MultiLineEdit
    };
    virtual Type type() = 0;

    /*
     * override in derived classes to get the encoded name=value pair
     * for submitting
     */
    virtual QString encoding() = 0;

    /*
     * state() and restoreState() are complimentary functions.
     */
    virtual QString state() { return QString::null; }
    virtual void restoreState(const QString &) { };

    virtual void reset() = 0;
    virtual void setChecked(bool) {}
    void setReadonly(bool ro)  { m_readonly = ro; }
    bool readonly() { return m_readonly; }

    const DOMString &name() const { return m_name; }
    void setName(const DOMString &name) { m_name = name; }

    const DOMString &value() const { return m_value; }
    virtual void setValue(const DOMString &value) { m_value = value; }

    virtual void calcMinMaxWidth();
    virtual void layout(bool);

    virtual bool isInline() const { return true; }

protected:
    QString encodeString( QString e );
    QString decodeString( QString e );

    DOMString m_name;
    DOMString m_value;
    HTMLFormElementImpl *m_form;
    HTMLGenericFormElementImpl *m_gform;
    bool m_readonly;
};


// -------------------------------------------------------------------------

// generic class for all buttons
class RenderButton : public RenderFormElement
{
    Q_OBJECT
public:
    RenderButton(QScrollView *view,
                 HTMLFormElementImpl *form);

    virtual const char *renderName() const { return "RenderButton"; }

    virtual void layout(bool);
};	


// -------------------------------------------------------------------------

class RenderHiddenButton : public RenderButton
{
public:
    RenderHiddenButton(QScrollView *view,
                       HTMLFormElementImpl *form);

    virtual const char *renderName() const { return "RenderHiddenButton"; }
    virtual Type type() { return HiddenButton; }

    virtual QString encoding();
    virtual void reset() { };
};


// -------------------------------------------------------------------------

class RenderCheckBox : public RenderButton
{
public:
    RenderCheckBox(QScrollView *view,
                   HTMLFormElementImpl *form);

    virtual const char *renderName() const { return "RenderCheckBox"; }
    virtual Type type() { return CheckBox; }

    virtual QString encoding();
    virtual void reset();
    virtual void setChecked(bool);

    virtual QString state();
    virtual void restoreState(const QString &);
};


// -------------------------------------------------------------------------

class RenderRadioButton : public RenderButton
{
    Q_OBJECT
public:
    RenderRadioButton(QScrollView *view, HTMLFormElementImpl *form);

    virtual const char *renderName() const { return "RenderRadioButton"; }
    virtual Type type() { return RadioButton; }

    virtual void reset();
    virtual void setChecked(bool);
    virtual QString encoding();

    virtual QString state();
    virtual void restoreState(const QString &);

 public slots:	
    void slotClicked();
};


// -------------------------------------------------------------------------

class RenderSubmitButton : public RenderButton
{
    Q_OBJECT
public:
    RenderSubmitButton(QScrollView *view, HTMLFormElementImpl *form);
    virtual ~RenderSubmitButton();

    virtual const char *renderName() const { return "RenderButton"; }

    virtual Type type() { return SubmitButton; }

    virtual QString encoding();
    virtual void setValue(const DOMString &value);

    virtual void reset();

public slots:	
    virtual void slotClicked();

protected:
    bool m_clicked;
};

// -------------------------------------------------------------------------

class RenderImageButton : public RenderSubmitButton
{
public:
    RenderImageButton(QScrollView *view, HTMLFormElementImpl *form);
    virtual ~RenderImageButton();

    virtual const char *renderName() const { return "RenderSubmitButton"; }

    virtual void setValue(const DOMString &value)
        { RenderButton::setValue(value); }

    void setImageUrl(DOM::DOMString url, DOM::DOMString baseUrl);
    virtual void setPixmap( const QPixmap &, CachedObject * );

    CachedObject *image;
};


// -------------------------------------------------------------------------

class RenderResetButton : public RenderSubmitButton
{
public:
    RenderResetButton(QScrollView *view, HTMLFormElementImpl *form);
    virtual ~RenderResetButton();

    virtual void slotClicked();
};

// -------------------------------------------------------------------------

// these define <Input type=button>, and can only work with scripts
// so we need to know the corresponding HTMLInputElementImpl, to be able to trigger
// execution of the script there

class RenderPushButton : public RenderSubmitButton
{
public:
    RenderPushButton(QScrollView *view, HTMLFormElementImpl *form,
		      HTMLInputElementImpl *domParent);
    virtual ~RenderPushButton();

    virtual Type type() { return PushButton; }

    virtual void slotClicked();

protected:
    HTMLInputElementImpl *m_domParent;
};


// -------------------------------------------------------------------------

class RenderLineEdit : public RenderFormElement
{
    Q_OBJECT
public:
    RenderLineEdit(QScrollView *view,
                   HTMLFormElementImpl *form, int maxLen, int size,
                   bool passwd = false);

    virtual Type type() { return LineEdit; }
    virtual QString encoding();

    virtual void layout(bool);

    virtual void reset();

    virtual void setValue(const DOMString &value);
    virtual QString state();
    virtual void restoreState(const QString &);

    virtual const char *renderName() const { return "RenderLineEdit"; }

protected:

    int m_size;

public slots:	
    void slotReturnPressed();
};


// -------------------------------------------------------------------------

class ccRenderButton : public RenderFormElement
{
public:
    ccRenderButton(QScrollView *view,
                   HTMLFormElementImpl* form);

    virtual const char *renderName() const { return "RenderFormElement"; }

    virtual ~ccRenderButton();

    enum typeEnum {
	SUBMIT,
	RESET,
	BUTTON
    };

    virtual QString encoding();

    virtual void reset();
    virtual void print(QPainter *, int, int, int, int, int, int);

    //public slots:
    void slotSubmit();

protected:
    DOMString _value;
    bool _clicked;
    typeEnum _type;
    QString currValue;
    QPixmap pixmap;
    bool dirty;
};


// -------------------------------------------------------------------------

class RenderFieldset : public RenderFormElement
{
public:
    RenderFieldset(QScrollView *view,
                   HTMLFormElementImpl* form);
    virtual ~RenderFieldset();

    virtual const char *renderName() const { return "RenderFieldSet"; }
};


// -------------------------------------------------------------------------

class RenderFileButton : public RenderFormElement
{
    Q_OBJECT
public:
    RenderFileButton(QScrollView *view, HTMLFormElementImpl *form, 
                     int maxLen, int size);
    virtual ~RenderFileButton();

    virtual const char *renderName() const { return "RenderFileButton"; }

    virtual void layout( bool deep = false );

    virtual QString encoding();

    virtual Type type() { return File; }

    virtual void reset();

    virtual void setValue(const DOMString &value);
    virtual QString state();
    virtual void restoreState(const QString &);

public slots:	
    virtual void slotClicked();
    virtual void slotReturnPressed();

protected:
    bool m_clicked;
    QLineEdit   *m_edit;
    QPushButton *m_button;
    int m_size;
};


// -------------------------------------------------------------------------

class RenderLabel : public RenderFormElement
{
public:
    RenderLabel(QScrollView *view,
                HTMLFormElementImpl *form);
    virtual ~RenderLabel();

    virtual const char *renderName() const { return "RenderLabel"; }
};


// -------------------------------------------------------------------------

class RenderLegend : public RenderFormElement
{
public:
    RenderLegend(QScrollView *view,
        HTMLFormElementImpl *form);
    virtual ~RenderLegend();

    virtual const char *renderName() const { return "RenderLegend"; }
};


// -------------------------------------------------------------------------

class RenderSelect : public RenderFormElement
{
public:
    RenderSelect(int size, bool multiple, QScrollView *view,
                 HTMLFormElementImpl *form);

    virtual const char *renderName() const { return "RenderSelect"; }

    virtual void layout( bool deep = false );
    virtual void close( );

    virtual Type type() { return Select; }

    virtual void reset();
    virtual QString encoding();

    virtual QString state();
    virtual void restoreState(const QString &);

protected:
    unsigned  m_size;
    bool m_multiple;
};


// -------------------------------------------------------------------------

class RenderTextArea : public RenderFormElement
{
public:
    RenderTextArea(int wrap, QScrollView *view,
                   HTMLFormElementImpl *form);

    virtual const char *renderName() const { return "RenderTextArea"; }

    virtual void layout( bool deep = false );
    virtual void close ( );

    virtual Type type() { return MultiLineEdit; }

    virtual void reset();
    virtual QString encoding();

    virtual QString state();
    virtual void restoreState(const QString &);
};

// -------------------------------------------------------------------------

}; //namespace

#endif
