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
#ifndef __KSPELL_H__
#define __KSPELL_H__

#include <qobject.h>
#include <qstrlist.h>

#include "kprocio.h"
#include "kspelldlg.h"
#include "ksconfig.h"

/**
 * KDE Spellchecker
 *
 * A KDE programmer's interface to International ISpell 3.1.  (GPL 1997)
 * A static method, @ref modalCheck() is provided for convenient
 *  access to the spellchecker.
 *
 * @author David Sweet <dsweet@kde.org>
 * @version $Id$
 * @see KSpellConfig
 */

class QTextCodec;

class KSpell : public QObject
{
  Q_OBJECT

public:

  /**
   * Possible states of the spell checker.
   *
   * @li @p Starting - After creation of @ref KSpell.
   * @li @p Running - After the ready signal has been emitted.
   * @li @p Cleaning - After @ref cleanUp() has been called.
   * @li @p Finished - After @ref cleanUp() has been completed.
   *
   * The following error states exist:
   *
   * @li @p Error -  An error occured in the @p Starting state.
   * @li @p Crashed - An error occured in the @p Running state.
   **/
  enum spellStatus { Starting = 0, Running, Cleaning, Finished, Error, Crashed };
  
  /**
   * Start spellchecker.
   *
   * @ref KSpell emits @ref ready() when it has verified that 
   * ISpell/ASpell is working properly. Pass the name of a slot -- do not pass zero!
   * Be sure to call @ref cleanUp() when you are done with @ref KSpell.
   *
   * If @ref KSpell could not be started correctly, @ref death() is emitted.
   *
   * @param _parent      Parent of @ref KSpellConfig dialg..
   * @param _caption     Caption of @ref KSpellConfig dialog.
   * @param _receiver    Receiver.

   * @param _slot        Receivers SLOT.
   * @param _kcs         Configuration for @ref KSpell.
   * @param _progressbar Indicates if progress bar should be shown.
   * @param _modal       Indicates modal or non-modal dialog.
   */
  KSpell(QWidget *_parent, QString _caption,
	 QObject *obj, const char *slot, KSpellConfig *_kcs=0,
	 bool _progressbar = TRUE, bool _modal = FALSE );

  /**
   * Retrieve the status of @ref KSpell.
   *
   * @see spellStatus()
   */
  spellStatus status() { return m_status; }

  /**
   * Clean up ISpell.
   *
   * Write out the personal dictionary and close ISpell's
   *  stdin.  A @ref death() signal will be emitted when the cleanup is
   *  complete, but this method will return immediately.
   **/
  virtual void cleanUp ();

  /** 
   * Auto-delete the @ref KSpell object after emitting @ref death().
   */
  void setAutoDelete(bool _autoDelete) { autoDelete = _autoDelete; }

  /**
   *  Spellcheck a buffer of many words in plain text 
   *  format.
   *
   * The @p _buffer is not modified.  The signal @ref done() will be
   *  sent when @ref check() is finished and the argument will be a 
   *  spell-corrected version of @p _buffer.  
   *
   * The spel check may be stopped by the user before the entire buffer
   *  has been checked.  You can check @ref lastPosition() to see how far
   *  in @p _buffer @ref check() reached before stopping.
   */

  virtual bool check (const QString &_buffer);

  /**
   * Retrieve the position (when using @ref check())  or word 
   * number (when using @ref checkList()) of
   * the last word checked.
   **/
  inline int lastPosition()
    { return lastpos;}

  /**
   * Spellcheck a list of words.
   *
   * @ref checkList() is more flexible than @ref check().  You could parse
   *  any type
   *  of document (HTML, TeX, etc.) into a list of spell-checkable words
   *  and send the list to @ref checkList().  Sending a marked-up document
   *  to @ref check() would result in the mark-up tags being
   *  spell checked.
   */
  virtual bool checkList (QStringList *_wordlist);

  /**
   * Spellcheck a single word.
   *
   * checkWord() is the most flexible function.  Some applications
   *  might need this flexibility but will sacrifice speed when
   *  checking large numbers of words.  Consider @ref checkList() for
   *  checking many words.
   *
   *  Use this method for implementing  "online" spellchecking (i.e.,
   *  spellcheck as-you-type).
   *
   * checkWord () returns @p false if @p buffer is not a word, otherwise it
   *  returns @p true;
   *
   * If @p usedialog is set to @p true, @ref KSpell will put up the standard
   *  dialog if the word is not found.  The dialog results can be queried
   *  by using  @ref dlgResult() and @ref replacement(). 
   *
   *  The signal @ref corrected() is emitted when the check is
   *  complete.  You can look at @ref suggestions() to see what the
   *  suggested replacements were. 
   */
  virtual bool checkWord (QString _buffer,  bool usedialog=FALSE);

  /**
   * Hide the dialog box.
   *
   * You'll need to do this when you are done with @ref checkWord();
   */
  void hide ()   { ksdlg->hide(); }

  /**
   * Retrieve list of suggested word replacements.
   *
   * After calling @ref checkWord() (an in response to
   *  a @ref misspelled() signal you can
   *  use this to get the list of
   *  suggestions (if any were available).
   */
  inline QStringList *suggestions ()	{ return &sugg; }

  /**
   * Get the result code of the dialog box.
   *
   * After calling checkWord, you can use this to get the dialog box's
   *  result code.
   * The possible
   *  values are (from kspelldlg.h):
   *    @li KS_CANCEL
   *    @li KS_REPLACE
   *    @li KS_REPLACEALL
   *    @li KS_IGNORE
   *    @li KS_IGNOREALL
   *    @li KS_ADD
   *    @li KS_STOP
   *
   */
  inline int dlgResult ()
    { return dlgresult; }

  /**
   * Move the dialog.
   *
   * If the dialog is not currently visible, it will
   *   be placed at this position when it becomes visible.
   * Use this to get the dialog out of the way of a highlighted
   * misspelled word in a document.
   */
  void moveDlg (int x, int y);

  /**
   * Retrieve the height of the dialog box.
   **/
  inline int heightDlg () {return ksdlg->height();}
  /**
   * Retrieve the width of the dialog box.
   **/
  inline int widthDlg () {return ksdlg->width();}

  /**
   * Retrieve the partially spellchecked buffer.
   *
   * You might want the full buffer in its partially-checked state.
   */
  const QString *intermediateBuffer () {return &newbuffer;}

  /**
   * Tell ISpell/ASpell to ignore this word for the life of this @ref KSpell instance.
   *
   *  @ref ignore() returns @p false if word is not a word or there was an error
   *  communicating with ISpell/ASpell.
   */
  virtual bool ignore (QString word);

  /**
   * Add a word to the user's personal dictionary. 
   *
   * Returns @p false if @p word
   *  is not a word or there was an error communicating with ISpell/ASpell.
   */
  virtual bool addPersonal (QString word);

  /**
   * Retrieve the @ref KSpellConfig object being used by this @ref KSpell instance.
   */
  KSpellConfig ksConfig () const;

  /**
   * Set the resolution (in percent) of the @ref progress() signals.
   *
   * E.g. @ref setProgressResolution (10) instructs @ref KSpell to send progress
   *  signals (at most) every 10% (10%, 20%, 30%...).
   * The default is 10%.
   */
  void setProgressResolution (unsigned res);

  /**
   * The destructor instructs ISpell/ASpell to write out the personal
   *  dictionary and then terminates ISpell/ASpell.
   */ 
  virtual ~KSpell();

  /**
   * Perform a sychronous spellcheck.
   *
   * This method does not return until spellchecking is done or canceled.
   * Your application's GUI will still be updated, however.
   */
  static int modalCheck( QString& text );

signals:

  /**
   * Emitted whenever a misspelled word is found by @ref check() or
   *   by @ref checkWord().
   *  If it is emitted by @ref checkWord(), @p pos=0.
   *  If it is emitted by @ref check(), then @p pos indicates the position of
   *   the misspelled word in the (original) @p _buffer.
   *   (The first position is zero.)
   *  If it is emitted by @ref checkList(), @p pos is the index to
   *  the misspelled
   *   word in the @ref QStringList passed to @ref checkList().
   *  Note, that @p originalword can be only a word part, if it's
   *  word with hyphens.
   *
   *  These are called _before_ the dialog is opened, so that the
   *   calling program's GUI may be updated. (e.g. the misspelled word may
   *   be highlighted).
   */
  void misspelling (QString originalword, QStringList *suggestions, 
		    unsigned pos);

  /**
   * Emitted after the "Replace" or "Replace All" buttons of the dialog
   * was pressed, or if the word was 
   * corrected without calling the dialog (i.e., the user previously chose
   * "Replace All" for this word). 
   *
   * Results from the dialog may be checked with @ref dlgResult()
   *  and @ref replacement().
   *
   * Note, that when using @ref checkList() this signal cann occure
   * more then once with same list position, when checking a word with
   * hyphens. In this case @p orignalword is the last replacement.
   *
   * @see check()
   */
  void corrected (QString originalword, QString newword, unsigned pos);

  /**
   * Emitted after @ref KSpell has verified that ISpell/ASpell is running
   * and working properly.  
   */
  void ready(KSpell *);

  /**
   * Emitted during a @ref check().
   * @p i is between 1 and 100.
   */
  void progress (unsigned int i);

  /**
   * Emitted when @ref check() is done.
   *
   * Be sure to copy the results of @p buffer if you need them.
   *  You can only rely
   *  on the contents of buffer for the life of the slot which was signaled
   *  by @ref done().
   */
  void done (const QString &buffer);

  /**
   * Emitted when @ref checkList() is done.
   *
   * If the argument is
   * @p true, then you should update your text from the
   * wordlist, otherwise not.
   */
  void done(bool);

  /**
   * Emitted on terminal errors and after clean up.
   *
   * You can delete the @ref KSpell object in this signal.
   *
   * You can check @ref status() to see what caused the death:
   * @li @p Error - @ref KSpell could not start.
   * @li @p Crashed - @ref KSpell encountered an unexpected error during execution.
   * @li @p Finished - Clean up finished.
   */
  void death( );
  

protected slots:
  /* All of those signals from KProcIO get sent here. */
  void KSpell2 (KProcIO *);
  void checkWord2 (KProcIO *);
  void checkWord3 ();
  void check2 (KProcIO *);
  void checkList2 ();
  void checkList3a (KProcIO *);
  void checkList3 ();
  void checkList4 ();
  void dialog2 (int dlgresult);
  void check3 ();
		   
  void slotStopCancel (int);
  void ispellExit (KProcess *);
  void emitDeath();
  void ispellErrors (KProcess *, char *, int);

private slots:
  /**
   * Used for @ref modalCheck().
   */
  void slotModalReady();
 /**
   * Used for @ref modalCheck().
   */
  void slotModalDone( const QString & );
    
signals:
  void dialog3 ();
  void ez ();
  void eza ();

protected:

  KProcIO *proc;
  QWidget *parent;
  KSpellConfig *ksconfig;
  KSpellDlg *ksdlg;
  QStringList *wordlist;
  QStringList::Iterator wlIt;
  QStringList ignorelist;
  QStringList replacelist;
  QStringList sugg;
  QTextCodec* codec;

  spellStatus m_status;
  
  bool usedialog;
  bool texmode;
  bool dlgon;
  bool personaldict;
  bool dialogwillprocess;
  bool progressbar;
  bool dialogsetup;
  bool autoDelete;

  QString caption;
  QString orig;
  QString origbuffer;
  QString newbuffer;
  QString cwword;
  QString dlgorigword;
  QString dlgreplacement;
  QString dialog3slot;

  int dlgresult;
  int trystart;
  int maxtrystart;
  unsigned int lastpos;
  unsigned int totalpos;
  unsigned int lastline;
  unsigned int posinline;
  unsigned int lastlastline;
  unsigned int offset;
  unsigned int progres;
  unsigned int curprog;

  /**
   * Used for @ref #modalCheck.
   */
  bool modaldlg;
  static QString modaltext;
  static int modalreturn;
  static QWidget* modalWidgetHack;
    
  int parseOneResponse (const QString &_buffer, QString &word, QStringList *sugg);
  QString funnyWord (QString word);
  void dialog (QString word, QStringList *sugg, const char* _slot);
  inline QString replacement ()
    { return dlgreplacement; }

  void setUpDialog ( bool reallyusedialogbox = TRUE);

  void emitProgress ();
  bool cleanFputs (QString s, bool appendCR=TRUE);
  bool cleanFputsWord (QString s, bool appendCR=TRUE);
  void startIspell();
  bool writePersonalDictionary ();

private:
  class KSpellPrivate;
  KSpellPrivate *d;
};

/**
 * @libdoc Spelling Checker Library
 *
 * @ref KSpell offers easy access to International ISpell or ASpell 
 *  (at the user's option) as well as a spell-checker GUI
 *  ("Add", "Replace", etc.).  
 *
 * You can use @ref KSpell to
 *  automatically spell-check an ASCII file as well as to implement
 *  online spell-checking and to spell-check proprietary format and
 *  marked up (e.g. HTML, TeX) documents.  The relevant methods for
 *  these three procedures are @ref check(), @ref checkWord(), and
 *  @ref checkList(), respectively.
 *
 * @ref KSpellConfig holds configuration information about @ref KSpell as well
 *  as acting as an options-setting dialog.
 *
 * KSpell usually works asynchronously. If you do not need that, you should
 * simply use @ref KSpell::modalCheck(). It won't return until the
 *  passed string
 * is processed or the spell checking canceled. During modal spell checking
 * your GUI is still repainted, but the user may only interact with the
 * @ref KSpell dialog.
 *
 * @see KSpell, KSpellConfig
 **/

#endif
