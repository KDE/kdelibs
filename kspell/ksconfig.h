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
#ifndef __KSCONFIG_H__
#define __KSCONFIG_H__

#include <qwidget.h>

class QCheckBox;
class QComboBox;
class QLabel;

class KConfig;

// Should be replaced by the charset strings
// because the config file would be more stable
// when inserting entries in the list
enum Encoding {
  KS_E_ASCII=0,
  KS_E_LATIN1=1,
  KS_E_LATIN2=2,
  KS_E_LATIN3=3,
  KS_E_LATIN4=4,
  KS_E_LATIN5=5,
  KS_E_LATIN7=6,
  KS_E_LATIN8=7,
  KS_E_LATIN9=8,
  KS_E_LATIN15=9,
  KS_E_UTF8=10,
  KS_E_KOI8R=11,
  KS_E_KOI8U=12, 
#if QT_VERSION >= 224
  KS_E_CP1251=13,
#endif
  KS_E_LATIN13=14
};

enum KSpellClients {
  KS_CLIENT_ISPELL=0,
  KS_CLIENT_ASPELL=1
};

/**
 * A configuration class/dialog for @ref KSpell.
 *
 * It contains all of the options settings.The options are set to default
 * values by the constructor and can be reset either by using the 
 * public interface or by using @ref KSpellConfig as a widget in a dialog 
 * (or, preferably a tabbed dialog using @ref KDialogBase) and letting 
 * the user change the settings. This way an application that uses 
 * @ref KSpell can either rely on the default settings (in the simplest 
 * case), offer a dialog to configure @ref KSpell, or offer a dialog to 
 * configure @ref KSpell _for_this_app_only_ (in which case, the application 
 * should save the settings for use next time it is run).
 * This last option might be useful in an email program, for example, where
 * people may be writing in a language different from that used for
 * writing papers in their word processor.
 *
 * @author David Sweet <dsweet@kde.org>
 * @version $Id$
 * @see KSpell
 */

class KSpellConfig : public QWidget
{
  Q_OBJECT

  public:
    /**
     * Create a @ref KSpellConfig with default or custom settings. 
     *
     * @param parent Parent of the widget.
     * @param name Widget name.
     * @param spellConfig Predefined configuration. If this parameter 
     *        is 0, a default configuration will be used.
     * @param addHelpButton Enabled or hides a help button. See
     *        @ref activateHelp for more information.
     *
     */
    KSpellConfig( QWidget *parent=0, const char *name=0,
		  KSpellConfig *spellConfig=0, bool addHelpButton = true );

    KSpellConfig (const KSpellConfig &);

    virtual ~KSpellConfig ();

    void operator= (const KSpellConfig &ksc);

    /**
     * @sect Options setting routines.
     **/

    /**
     *
     * The @p ignorelist contains words you'd like @ref KSpell
     * to ignore when it is spellchecking.  When you get a @ref KSpellConfig
     * object back from @ref KSpell (using @ref KSpell::kcConfig()),
     * the @ref ignorelist contains whatever was put in by you plus 
     * any words the user has chosen to ignore via the dialog box.  
     * It may be useful to save this list with the document being 
     * edited to facilitate quicker future spellchecking.
     */
    void setIgnoreList (QStringList _ignorelist);

    /**
     * Set an ISpell option.
     *
     * If @p true, don't create root-affix combinations.
     */
    void setNoRootAffix (bool);

    /**
     * Set an ISpell option.
     *
     * If @p true, treat run-together words a valid.
     */
    void setRunTogether(bool);

    /**
     * Set the name of the dictionary to use.
     */
    void setDictionary (const QString qs);
    void setDictFromList (bool dfl);
    //void setPersonalDict (const char *s);

    /**
     *
     */
    void setEncoding (int enctype);
    void setClient (int client);

    /**
     * Options reading routines.
     */
    bool noRootAffix () const;
    bool runTogether() const;
    const QString dictionary () const;
    bool dictFromList () const;
    //QString personalDict () const;
    int encoding () const;
    QStringList ignoreList () const;
    int client () const; //see enums at top of file

    /**
     * Call this method before this class is deleted  if you want 
     * the settings you have (or the user has) chosen to become the 
     * global, default settings.
     */
    bool writeGlobalSettings ();
    
  protected:
    void fillInDialog();
    bool readGlobalSettings();

    /**
     * This takes a dictionary file name (fname) and returns a language 
     * abbreviation (lname; like de for German), appropriate for the
     * $LANG variable, and a human-readble name (hname; like "Deutsch").
     *
     * It also truncates ".aff" at the end of fname.
     * 
     * TRUE is returned if lname.data()==$LANG
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
    void sHelp();
    //void sBrowseDict();
    //void sBrowsePDict();
    void sNoAff(bool);
    void sRunTogether(bool);
    void sDictionary(bool);
    void sPathDictionary(bool);
    void sSetDictionary (int);
    void sChangeEncoding (int);
    void sChangeClient (int);
    //void textChanged1 (const char *);
    //void textChanged2 (const char *);

  protected:
    // The options
    int enc;			//1 ==> -Tlatin1			  
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
    class KSpellConfigPrivate;
    KSpellConfigPrivate *d;
    void getAvailDictsIspell();
    void getAvailDictsAspell();
};

#endif





