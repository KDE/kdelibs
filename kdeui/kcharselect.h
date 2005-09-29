/* This file is part of the KDE libraries

   Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>

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

#ifndef kcharselect_h
#define kcharselect_h

#include <qgridview.h>
#include <qvbox.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qstring.h>
#include <qpoint.h>
#include <qstringlist.h>

#include <kdelibs_export.h>

class QFont;
class QFontDatabase;
class QMouseEvent;
class QSpinBox;
class KCharSelectTablePrivate;
class KCharSelectPrivate;

/**
 * @short Character selection table
 *
 * A table widget which displays the characters of a font. Internally
 * used by KCharSelect. See the KCharSelect documentation for further
 * details.
 *
 * @author Reginald Stadlbauer <reggie@kde.org>
 */

class KDEUI_EXPORT KCharSelectTable : public QGridView
{
    Q_OBJECT

public:
    KCharSelectTable( QWidget *parent, const char *name, const QString &_font,
		      const QChar &_chr, int _tableNum );

    virtual QSize sizeHint() const;
    virtual void resizeEvent( QResizeEvent * );

    virtual void setFont( const QString &_font );
    virtual void setChar( const QChar &_chr );
    virtual void setTableNum( int _tableNum );

    virtual QChar chr() { return vChr; }

protected:
    virtual void paintCell( class QPainter *p, int row, int col );

    virtual void mousePressEvent( QMouseEvent *e ) {  mouseMoveEvent( e ); }
    virtual void mouseDoubleClickEvent ( QMouseEvent *e ){  mouseMoveEvent( e ); emit doubleClicked();}
    virtual void mouseReleaseEvent( QMouseEvent *e ) { mouseMoveEvent( e ); emit activated( chr() ); emit activated(); }
    virtual void mouseMoveEvent( QMouseEvent *e );

    virtual void keyPressEvent( QKeyEvent *e );

    void gotoLeft();
    void gotoRight();
    void gotoUp();
    void gotoDown();

    QString vFont;
    QChar vChr;
    int vTableNum;
    QPoint vPos;
    QChar focusItem;
    QPoint focusPos;
    int temp;

signals:
    void highlighted( const QChar &c );
    void highlighted();
    void activated( const QChar &c );
    void activated();
    void focusItemChanged();
    void focusItemChanged( const QChar &c );
    void tableUp();
    void tableDown();
    void doubleClicked();

private:
    virtual void setFont(const QFont &f) { QGridView::setFont(f); }
    void setToolTips();
protected:
    virtual void virtual_hook( int id, void* data );
private:
    KCharSelectTablePrivate* const d;
};

/**
 * @short Character selection widget
 *
 * This widget allows the user to select a character of a
 * specified font in a table
 *
 * \image html kcharselect.png "Character Selection Widget"
 *
 * You can specify the font whose characters should be displayed via
 * setFont() or in the constructor. Using enableFontCombo() you can allow the
 * user to choose the font from a combob-box. As only 256 characters
 * are displayed at once in the table, using the spinbox on the top
 * the user can choose starting from which character the table
 * displays them. This spinbox also can be enabled or disabled using
 * enableTableSpinBox().
 *
 * KCharSelect supports keyboard and mouse navigation. Click+Move
 * always selects the character below the mouse cursor. Using the
 * arrow keys moves the focus mark around and pressing RETURN
 * or SPACE selects the cell which contains the focus mark.
 *
 * To get the current selected character, use the chr()
 * method. You can set the character which should be displayed with
 * setChar() and the table number which should be displayed with
 * setTableNum().
 *
 * @author Reginald Stadlbauer <reggie@kde.org>
 */

class KDEUI_EXPORT KCharSelect : public QVBox
{
    Q_OBJECT
    Q_PROPERTY( QString fontFamily READ font WRITE setFont )
    Q_PROPERTY( int tableNum READ tableNum WRITE setTableNum )
    Q_PROPERTY( bool fontComboEnabled READ isFontComboEnabled WRITE enableFontCombo )
    Q_PROPERTY( bool tableSpinBoxEnabled READ isTableSpinBoxEnabled WRITE enableTableSpinBox )

public:
    /**
     * Constructor. @p font specifies which font should be displayed, @p
     * chr which character should be selected and @p tableNum specifies
     * the number of the table which should be displayed.
     */
    KCharSelect( QWidget *parent, const char *name,
		 const QString &font = QString::null, const QChar &chr = ' ', int tableNum = 0 );
    ~KCharSelect();
    /**
     * Reimplemented.
     */
    virtual QSize sizeHint() const;

    /**
     * Sets the font which is displayed to @p font
     */
    virtual void setFont( const QString &font );

    /**
     * Sets the currently selected character to @p chr.
     */
    virtual void setChar( const QChar &chr );

    /**
     * Sets the currently displayed table to @p tableNum.
     */
    virtual void setTableNum( int tableNum );

    /**
     * Returns the currently selected character.
     */
    virtual QChar chr() const { return charTable->chr(); }

    /**
     * Returns the currently displayed font.
     */
    virtual QString font() const { return fontCombo->currentText(); }

    /**
     * Returns the currently displayed table
     */
    virtual int tableNum() const { return tableSpinBox->value(); }

    /**
     * If @p e is set to true, the combobox which allows the user to
     * select the font which should be displayed is enabled, else
     * disabled.
     */
    virtual void enableFontCombo( bool e ) { fontCombo->setEnabled( e ); }

    /**

     * If @p e is set to true, the spinbox which allows the user to
     * specify which characters of the font should be displayed, is
     * enabled, else disabled.
     */
    virtual void enableTableSpinBox( bool e ) { tableSpinBox->setEnabled( e ); }

    /**
     * Returns wether the font combobox on the top is enabled or
     * disabled.
     *
     * @see enableFontCombo()
     */
    virtual bool isFontComboEnabled() const { return fontCombo->isEnabled(); }

    /**
     * Returns wether the table spinbox on the top is enabled or
     * disabled.
     *
     * @see enableTableSpinBox()
     */
    virtual bool isTableSpinBoxEnabled() const { return tableSpinBox->isEnabled(); }

protected:
    virtual void fillFontCombo();
    static void cleanupFontDatabase();

    QComboBox *fontCombo;
    QSpinBox *tableSpinBox;
    KCharSelectTable *charTable;
    QStringList fontList;
    static QFontDatabase * fontDataBase;

protected slots:
    void fontSelected( const QString &_font );
    void tableChanged( int _value );
    void charHighlighted( const QChar &c ) { emit highlighted( c ); }
    void charHighlighted() { emit highlighted(); }
    void charActivated( const QChar &c ) { emit activated( c ); }
    void charActivated() { emit activated(); }
    void charFocusItemChanged() { emit focusItemChanged(); }
    void charFocusItemChanged( const QChar &c ) { emit focusItemChanged( c ); }
    void charTableUp() { if ( tableNum() < 255 ) setTableNum( tableNum() + 1 ); }
    void charTableDown() { if ( tableNum() > 0 ) setTableNum( tableNum() - 1 ); }
    void slotDoubleClicked() { emit doubleClicked(); }
    void slotUnicodeEntered();
    void slotUpdateUnicode( const QChar &c );
signals:
    void highlighted( const QChar &c );
    void highlighted();
    void activated( const QChar &c );
    void activated();
    void fontChanged( const QString &_font );
    void focusItemChanged();
    void focusItemChanged( const QChar &c );
    void doubleClicked();

private:
    virtual void setFont(const QFont &f) { QVBox::setFont(f); }
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KCharSelectPrivate;
    KCharSelectPrivate* const d;
};

#endif
