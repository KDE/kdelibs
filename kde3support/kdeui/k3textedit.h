/* This file is part of the KDE libraries
    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

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

#ifndef K3TEXTEDIT_H
#define K3TEXTEDIT_H

#include <kde3support_export.h>

#include <Qt3Support/Q3TextEdit>

class K3Spell;

/**
 * @short A KDE'ified Q3TextEdit
 *
 * This is just a little subclass of Q3TextEdit, implementing
 * some standard KDE features, like Cursor auto-hiding, configurable
 * wheelscrolling (fast-scroll or zoom) and deleting of entire
 * words with Ctrl-Backspace or Ctrl-Delete.
 *
 * Basic rule: whenever you want to use Q3TextEdit, use K3TextEdit!
 *
 * @see Q3TextEdit
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KDE3SUPPORT_EXPORT K3TextEdit : public Q3TextEdit
{
    Q_OBJECT

public:
    /**
     * Constructs a K3TextEdit object. See Q3TextEdit::Q3TextEdit
     * for details.
     */
    K3TextEdit( const QString& text,
                const QString& context = QString(),
                QWidget * parent = 0, const char *name = 0 );
    /**
     * Constructs a K3TextEdit object. See Q3TextEdit::Q3TextEdit
     * for details.
     */

    K3TextEdit( QWidget *parent = 0L, const char *name = 0 );
    /**
     * Destroys the K3TextEdit object.
     */
    ~K3TextEdit();

    /**
     * Reimplemented to set a proper "deactivated" background color.
     */
    virtual void setReadOnly (bool readOnly);

    /**
     * Reimplemented for tracking custom palettes.
     */
    virtual void setPalette( const QPalette& palette );

    /**
     * Turns spell checking for this text edit on or off.
     *
     * @see checkSpellingEnabled()
     */
    void setCheckSpellingEnabled( bool check );

    /**
     * Returns true if spell checking is enabled for this text edit.
     *
     * @see setCheckSpellingEnabled()
     */
    bool checkSpellingEnabled() const;

    void highLightWord( unsigned int length, unsigned int pos );


public Q_SLOTS:
    /**
     * Create a modal dialog to check the spelling.  This slot will not return
     * until spell checking has been completed.
     *
     */
    void checkSpelling();

protected:
    /**
     * Reimplemented to catch "delete word" key events.
     */
    virtual void keyPressEvent( QKeyEvent * );

    /**
     * Reimplemented to instantiate a KDictSpellingHighlighter, if
     * spellchecking is enabled.
     */
    virtual void focusInEvent( QFocusEvent * );

    /**
     * Reimplemented to allow fast-wheelscrolling with Ctrl-Wheel
     * or zoom.
     */
    virtual void contentsWheelEvent( QWheelEvent * );

    /**
     * Deletes a word backwards from the current cursor position,
     * if available.
     */
    virtual void deleteWordBack();

    /**
     * Deletes a word forwards from the current cursor position,
     * if available.
     */
    virtual void deleteWordForward();

    /**
     * Reimplemented from Q3TextEdit to add spelling related items
     * when appropriate.
     */
    virtual Q3PopupMenu *createPopupMenu( const QPoint &pos );

    /**
     * This is just a reimplementation of a deprecated method from Q3TextEdit and
     * is just here to keep source compatibility.  This should not be used in
     * new code.  Specifically reimplementing this method will probably not do
     * what you expect.  See the method above.
     *
     * @deprecated
     */
    virtual Q3PopupMenu *createPopupMenu();

protected:
    virtual void virtual_hook( int id, void* data );

private Q_SLOTS:
    void slotSpellCheckReady( K3Spell *s );
    void slotSpellCheckDone( const QString &s );
    void spellCheckerMisspelling( const QString &text, const QStringList &, unsigned int pos);
    void spellCheckerCorrected( const QString &, const QString &, unsigned int );
    void spellCheckerFinished();
    void toggleAutoSpellCheck();
    void slotAllowTab();

private:
    void posToRowCol( unsigned int pos, unsigned int &line, unsigned int &col );

    class K3TextEditPrivate;
    K3TextEditPrivate *d;
};

#endif // KTEXTEDIT_H
