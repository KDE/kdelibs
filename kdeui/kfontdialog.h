/*
    $Id$
    
    Requires the Qt widget libraries, available at no cost at 
    http://www.troll.no
    
    Copyright (C) 1997 Bernd Johannes Wuebben <wuebben@kde.org>
    Copyright (c) 1999 Preston Brown <pbrown@kde.org>
    Copyright (c) 1999 Mario Weilguni <mweilguni@kde.org>

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


#ifndef _K_FONT_DIALOG_H_
#define _K_FONT_DIALOG_H_

#include <qdialog.h>
#include <qlineedit.h>

#include <kdialogbase.h>

class QLabel;
class QListBox;
class QComboBox;
class QGroupBox;
class QFont;
class QStringList;

/**
 * Widget for interactive font selection.
 *
 * @author Preston Brown <pbrown@kde.org>, Bernd Wuebben <wuebben@kde.org>
 * @version $Id$
 */
class KFontChooser : public QWidget
{
  Q_OBJECT
  
public:
  /**
   * Constructor.  Creates a new font picker dialog.
   *
   * @param parent The parent widget of the dialog, if any.
   * @param name The name of the dialog.
   * @param modal Specifies whether the dialog is modal or not.
   * @param onlyFixedFonts only display fonts which have fixed-width
   *        character sizes.
   * @param fontList a list of fonts to display, in XLFD format.  If
   *        no list is formatted, the internal KDE font list is used.
   *        If that has not been created, X is queried, and all fonts
   *        available on the system are displayed.
   * @param visibleListSize The minimum number of visible entries in the 
   *        fontlists.
   */
  KFontChooser(QWidget *parent = 0L, const char *name = 0L, 
	       bool onlyFixed = false,
	       const QStringList &fontList = QStringList(),
	       bool makeFrame = true, int visibleListSize=8 );

  /**
   * Sets the currently selected font in the chooser.
   * 
   * @param font The font to select.
   * @param onlyFixed readjust the font list to display only fixed
   *        width fonts if true, or vice-versa
   */
  void setFont( const QFont &font, bool onlyFixed = false );

  
  /**
   * @return The currently selected font in the chooser.
   */
  QFont font() { return selFont; }
  
  /**
   * @return The current text in the sample text input area
   */
  QString sampleText() { return sampleEdit->text(); }

  /**
   * This function converts a QFont into the corresponding X Logical Font 
   * Description.
   *
   * @param theFont the font to convert.
   * @return a string representing the given font in XLFD format.
   */
  static QString getXLFD( const QFont &theFont )
    { return theFont.rawName(); }

  /**
   * Constructs a list of font strings that matches the pattern.
   * 
   * @param list The list is returned here
   * @param pattern The font pattern
   */
  static void getFontList( QStringList &list, const char *pattern );

  virtual QSize sizeHint( void ) const;

private slots:
  void family_chosen_slot(const QString&);
  void size_chosen_slot(const QString&);
  void style_chosen_slot(const QString&);
  void displaySample(const QFont &font);
  void charset_chosen_slot(const QString&);
  void showXLFDArea(bool);

signals:
  /**
   * connect to this to monitor the font as it is selected.
   */
  void fontSelected( const QFont &font );

protected:
  void fillFamilyListBox(bool onlyFixedFonts = false);
  void fillCharsetsCombo();
  void getFontList( QStringList &list, bool fixed );
  // This one must be static since getFontList( QStringList, char*) is so
  static void addFont( QStringList &list, const char *xfont );

  void setupDisplay();
    
  // pointer to an optinally supplied list of fonts to 
  // inserted into the fontdialog font-family combo-box
  QStringList  fontList; 
  
  QGroupBox    *xlfdBox;

  QLineEdit    *sampleEdit;
  QLineEdit    *xlfdEdit;

  QListBox     *familyListBox;
  QListBox     *styleListBox;
  QListBox     *sizeListBox;
  QComboBox    *charsetsCombo;
  
  QFont        selFont;

  bool usingFixed;
};

/**
 * Dialog for interactive font selection.  Provides a wrapper around
 * KFontChooser.
 *
 * @author Preston Brown <pbrown@kde.org>, Bernd Wuebben <wuebben@kde.org>
 * @version $Id$
 */
class KFontDialog : public KDialogBase  {
    Q_OBJECT

public:
  /**
   * Constructor.  Creates a new font picker dialog.
   *
   * @param parent The parent widget of the dialog, if any.
   * @param name The name of the dialog.
   * @param modal Specifies whether the dialog is modal or not.
   * @param onlyFixed only display fonts which have fixed-width
   *        character sizes.
   * @param fontlist a list of fonts to display, in XLFD format.  If
   *        no list is formatted, the internal KDE font list is used.
   *        If that has not been created, X is queried, and all fonts
   *        available on the system are displayed.
   * @param makeFrame Draws a frame with titles around the contents.
   *
   */
  KFontDialog( QWidget *parent = 0L, const char *name = 0,
	       bool modal = false, bool onlyFixed = false,
	       const QStringList &fontlist = QStringList(),
	       bool makeFrame = true );

  /**
   * Sets the currently selected font in the dialog.
   * 
   * @param font The font to select.
   * @param onlyFixed readjust the font list to display only fixed
   *        width fonts if true, or vice-versa
   */
  void setFont( const QFont &font, bool onlyFixed = false )
    { chooser->setFont(font, onlyFixed); }
  
  /**
   * @return The currently selected font in the dialog.
   */
  QFont font() { return chooser->font(); }
  
  /**
   * This is probably the function you are looking for.
   * Just call this to pop up a dialog to get the selected font.
   *
   * @param theFont a reference to the font to write the chosen font
   *        into.
   * @param onlyFixed if true, only select from fixed-width fonts.
   * @param parent Parent widget of the dialog. Specifying a widget different
   *        from 0 (Null) improves centering (looks better).
   * @param makeFrame Draws a frame with titles around the contents.
   * @return The result of the dialog.
   * 
   * @see QDialog::result
   */
  static int getFont( QFont &theFont, bool onlyFixed = false, 
		      QWidget *parent = 0L, bool makeFrame = true );
  
  /**
   * When you are not only interested in the font selected, but also
   * in the example string typed in, you can call this method.
   *
   * @param theFont a reference to the font to write the chosen font
   *        into.
   * @param theString a reference to the example text that was typed.
   * @param onlyFixed if true, only select from fixed-width fonts.
   * @param parent Parent widget of the dialog. Specifying a widget different
   *        from 0 (Null) improves centering (looks better).
   * @param makeFrame Draws a frame with titles around the contents.
   * @return The result of the dialog.
   */
  static int getFontAndText( QFont &theFont, QString &theString,
			     bool onlyFixed = false, QWidget *parent = 0L, 
			     bool makeFrame = true );

signals:
  /**
   * connect to this to monitor the font as it is selected if you are
   * not running modal.
   */
  void fontSelected( const QFont &font );

protected:
  KFontChooser *chooser;
};

#endif
