/* This file is part of the KDE libraries
   Copyright (C) 1997 David Sweet <dsweet@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef KDELIBS_KSCONFIG_H
#define KDELIBS_KSCONFIG_H

#include <qwidget.h>
#include <qstring.h>
#include <qstringlist.h>

#include <kdelibs_export.h>

class QCheckBox;
class QComboBox;
class QLabel;

class KConfig;
class KSpellConfigPrivate;

/**
 * @brief Encoding for the spell checker
 * @note In the identifier names LATIN means ISO-8859, not ECMA Latin
 * @todo Should be replaced by the charset strings
 * because the config file would be more stable
 * when inserting entries in the list
 */
enum Encoding {
  KS_E_ASCII=0,
  KS_E_LATIN1=1,
  KS_E_LATIN2=2,
  KS_E_LATIN3=3,
  KS_E_LATIN4=4,
  KS_E_LATIN5=5, ///< ISO-8859-5 (Cyrillic)
  KS_E_LATIN7=6, ///< ISO-8859-6 (Arabic)
  KS_E_LATIN8=7, ///< ISO-8859-7 (Greek)
  KS_E_LATIN9=8, ///< ISO-8859-8 (Hebrew)
  KS_E_LATIN13=9, ///< ISO-8859-13 (Latin 7)
  KS_E_LATIN15=10, ///< ISO-8859-15 (Latin 9)
  KS_E_UTF8=11,
  KS_E_KOI8R=12,
  KS_E_KOI8U=13,
  KS_E_CP1251=14,
  KS_E_CP1255=15
};

enum KSpellClients {
  KS_CLIENT_ISPELL=0,
  KS_CLIENT_ASPELL=1,
  KS_CLIENT_HSPELL=2,
  KS_CLIENT_ZEMBEREK=3
};

/**
 * A configuration class/dialog for KSpell.
 *
 * It contains all of the options settings.The options are set to default
 * values by the constructor and can be reset either by using the
 * public interface or by using KSpellConfig as a widget in a dialog
 * (or, preferably a tabbed dialog using KDialogBase) and letting
 * the user change the settings. This way an application that uses
 * KSpell can either rely on the default settings (in the simplest
 * case), offer a dialog to configure KSpell, or offer a dialog to
 * configure KSpell <em>for this app only</em> (in which case, the application
 * should save the settings for use next time it is run).
 * This last option might be useful in an email program, for example, where
 * people may be writing in a language different from that used for
 * writing papers in their word processor.
 *
 * @author David Sweet <dsweet@kde.org>
 * @see KSpell
 */

class KDEUI_EXPORT KSpellConfig : public QWidget
{
  Q_OBJECT

  public:
    /**
     * Constructs a KSpellConfig with default or custom settings.
     *
     * @param parent Parent of the widget.
     * @param name Widget name.
     * @param spellConfig Predefined configuration. If this parameter
     *        is 0, a default configuration will be used.
     * @param addHelpButton Enabled or hides a help button. See
     * activateHelp for more information.
     *
     */
    KSpellConfig( QWidget *parent=0, 
		  KSpellConfig *spellConfig=0, bool addHelpButton = true );

    KSpellConfig( const KSpellConfig & );

    /**
     * Deconstructor.
     * Deletes private class.
     */
    virtual ~KSpellConfig();

    void operator=( const KSpellConfig &ksc );

    /**
     *
     * @short Get the translated dictionary names and, optionally, the
     * corresponding internal dictionary names.
     *
     * Fills @p box with the human readable, translated dictionary names and
     * selects the currently used dictionary (this will be the global
     * dictionary if you call @ref fillDicts directly after the constructor).
     * If @p dictionaries isn't 0 then @p dictionaries will be filled with the
     * corresponding internal dictionary names that are necessary to set the
     * dictionary with @ref setDictionary.
     *
     * @param box Will be filled with the translated dictionary names.
     * @param dictionaries Will be filled with the internal dictionary names.
     *
     * @since 3.2
     *
     */
    void fillDicts( QComboBox* box, QStringList* dictionaries = 0 );

    /**
     * @sect Options setting routines.
     **/

    /**
     *
     * The @p _ignorelist contains words you'd like KSpell
     * to ignore when it is spellchecking.  When you get a KSpellConfig
     * object back from KSpell (using KSpell::kcConfig()),
     * the @p _ignorelist contains whatever was put in by you plus
     * any words the user has chosen to ignore via the dialog box.
     * It may be useful to save this list with the document being
     * edited to facilitate quicker future spellchecking.
     */
    void setIgnoreList( const QStringList &_ignorelist );

    /**
     * The @p _replaceAllList contains word you like that replace
     * word. Be careful that this list contains word which is replaced
     * and new word.
     */
    void setReplaceAllList( const QStringList &_replaceAllList );

    /**
     * Set an ISpell option.
     *
     * If @p true, don't create root-affix combinations.
     */
    void setNoRootAffix( bool );

    /**
     * Set an ISpell option.
     *
     * If @p true, treat run-together words a valid.
     */
    void setRunTogether( bool );

    /**
     * Set the name of the dictionary to use.
     */
    void setDictionary( const QString qs );
    void setDictFromList( bool dfl );
    //void setPersonalDict (const char *s);

    /**
     *
     */
    void setEncoding( int enctype );
    void setClient( int client );

    /**
     * Options reading routines.
     */
    bool noRootAffix() const;
    bool runTogether() const;
    const QString dictionary() const;
    bool dictFromList() const;
    //QString personalDict () const;
    int encoding() const;
    QStringList ignoreList() const;
    QStringList replaceAllList() const;

    int client() const; ///< Spell checker client, @see KSpellClients

    /**
     * Call this method before this class is deleted  if you want
     * the settings you have (or the user has) chosen to become the
     * global, default settings.
     */
    bool writeGlobalSettings();

    bool readGlobalSettings();
  protected:
    void fillInDialog();

    /**
     * This takes a dictionary file name and provides both a language
     * abbreviation appropriate for the $LANG variable, and a
     * human-readable name.
     *
     * It also truncates ".aff" at the end of fname.
     *
     * @param fname the dictionary name file (input)
     * @param lname the language abbreviation, such as de for German (output)
     * @param hname the human-readable name, such as Deutsch (output)
     *
     * @return true if <pre>lname.data()==$LANG</pre>
     */
    bool interpret( QString &fname, QString &lname, QString &hname );


  public slots:
    /**
     * Use this function to activate the help information for this
     * widget. The function is particulary useful if the help button is
     * not displayed as specified by the constructor. Normally you want
     * to hide the help button if this widget is embedded into a larger
     * dialog box that has its own help button. See kedit
     * (optiondialog.cpp) for an example
     */
    void activateHelp( void );


  protected slots:
    /**
     * Invokes the help documentation for kspell
     */
    void sHelp();
    //void sBrowseDict();
    //void sBrowsePDict();
    void sNoAff( bool );
    void sRunTogether( bool );
    void sDictionary( bool );
    void sPathDictionary( bool );
    void sSetDictionary( int );
    void sChangeEncoding( int );
    void sChangeClient( int );
    //void textChanged1 (const char *);
    //void textChanged2 (const char *);

  protected:
    // The options
    int enc;			// 1 ==> -Tlatin1
    bool bnorootaffix;		// -m
    bool bruntogether;		// -B
    bool dictfromlist;
    bool nodialog;
    QString qsdict;		// -d [dict]
    QString qspdict;		// -p [dict]
    QStringList ignorelist;
    enum {rdictlist=3, rencoding=4, rhelp=6};
    KConfig *kc;
    int iclient;            // defaults to ispell, may be aspell, too

    QCheckBox *cb1, *cb2;
    //KLineEdit *kle1, *kle2;
    //QButtonGroup *dictgroup;
    //QRadioButton *dictlistbutton, *dicteditbutton;
    QLabel *dictlist;
    QComboBox *dictcombo, *encodingcombo, *clientcombo;
    //QPushButton *browsebutton1;
    QStringList langfnames;

signals:
    void configChanged();

private:
    KSpellConfigPrivate *d;
    void getAvailDictsIspell();
    void getAvailDictsAspell();
};

#endif // KDELIBS_KSCONFIG_H





