#ifndef __KSCONFIG_H__
#define __KSCONFIG_H__

#include <qwidget.h>
#include <qchkbox.h>
#include <qlayout.h>
#include <qstrlist.h>
#include <qradiobt.h>
#include <qbttngrp.h>
#include <qcombo.h>
#include <kconfig.h>
#include <klined.h>
#include <qpushbt.h>

/**
 *  KSpellConfig v 0.2
 *
 * KSpellConfig is config class for KSpell.  It contains all of the options
 *  settings.  The options are set to default values by the constructor
 *  and can be reset either by using the public interface or by using
 *  KSpellConfig as a widget in a dialog (or, preferably a tabbed dialog
 *  using QTabDialog or KTabCtl) and letting the user change the settings.
 *  This way an application that uses KSpell can either rely on the default
 *  settings (in the simplest case), or offer a dialog to configure
 *  KSpell, or offer a dialog to configure KSpell _for_this_app_only_ 
 *  (in which case, the application should save the settings for use
 *  next time it is run).
 *
 *  KSpellConfig's favorite size (when used as a widget) is 
 *   (width, height) = (490x317)
 *  @author David Sweet (GPL 1997)
 *  @short Configuration dialog for KSpell
 *  @version 0.2
 *  @see KSpell
 */

#define KS_E_LATIN1 0
#define KS_E_ASCII  1

class KSpellConfig : public QWidget
{

  Q_OBJECT;
  enum {sizehint_x=490, sizehint_y=317};
public:
  
  /**
   *  Create a KSpellConfig with the default options set if _ksc==0, or
   *   copy the options in _ksc.
   *  This constructor also sets up the dialog.
   */
  KSpellConfig (QWidget *parent=0, char *name=0,
		KSpellConfig *_ksc=0);

  KSpellConfig (const KSpellConfig &);

  void operator= (const KSpellConfig &ksc);

  /**
   *Options setting routines.
   * The ignorelist is a QStrList that contains words you'd like KSpell
   *  to ignore when it it spellchecking.  When you get a KSpellConfig
   *  object back from KSpell (using KSpell::kcConfig()), the ignorlist
   *  contains whatever was put in by you plus any words the user has
   *  chosen to ignore via the dialog box.  It might be useful to save
   *  this list with the document being edited to facilitate quicker
   *  future spellchecking.
   */
  void setNoRootAffix (bool);
  void setRunTogether(bool);
  void setDictionary (const char *);
  void setDictFromList (bool dfl);
  //  void setPersonalDict (const char *);
  void setEncoding (int enctype);
  void setIgnoreList (QStrList _ignorelist);

  /**
   * Options reading routines.
   */
  bool noRootAffix (void) const;
  bool runTogether(void) const;
  char *dictionary (void) const;
  bool dictFromList (void) const;
 //  char *personalDict (void) const;
  int encoding (void) const;
  QStrList ignoreList (void) const;

  /**
   * Call this method before this class is deleted  if you want the settings
   *  you have (or the user has) chosen to become the global, default settings.
   */
  bool writeGlobalSettings (void);

  /**
   * Tells you what a good size is for the ksconfig widget.
   */
  /*  QSize sizeHint(void) const 
    {return QSize (sizehint_x, sizehint_y);}
    */


private:

//The options
  int enc;   //1 ==> -Tlatin1			  
  bool bnorootaffix;            // -m
  bool bruntogether;            // -B
  bool dictfromlist;				  
  bool nodialog;
  QString qsdict;     // -d [dict]
  QString qspdict;   // -p [dict]
  QStrList ignorelist;

  KConfig *kc;			 
			  
  QCheckBox *cb1, *cb2;
  KLined *kle1; //, *kle2;
  QGridLayout *layout;			  
  QButtonGroup *dictgroup;
  QRadioButton *dictlistbutton, *dicteditbutton;
  QLabel *dictlist;
  QComboBox *dictcombo, *encodingcombo;
  QPushButton *browsebutton1;

  QStrList *langnames, *langfnames;
  void fillInDialog (void);
  bool readGlobalSettings (void);

  /**
   * This takes a dictionary file name (fname) and returns
   * a language abbreviation (lname; like de for German), appropriate for the
   * $LANG variable, and a human-readble name (hname; like "Deutsch").
   *
   * It also truncates ".aff" at the end of fname.
   * 
   * TRUE is returned if lname.data()==$LANG
   **/

  bool interpret (QString &fname, QString &lname,
		  QString &hname);
protected slots:
  void sHelp();
  void sBrowseDict();
//  void sBrowsePDict();
  void sNoAff(bool);
  void sRunTogether(bool);
  void sDictionary(bool);
  void sPathDictionary(bool);
  void sSetDictionary (int);
  void sChangeEncoding (int);
//  void textChanged1 (const char *);
//  void textChanged2 (const char *);

};

#endif
