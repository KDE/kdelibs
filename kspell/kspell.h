#ifndef __KSPELL_H__
#define __KSPELL_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
 
#include <qobject.h>
#include <qstrlist.h>

#include "kprocio.h"
#include "kspelldlg.h"
#include "ksconfig.h"

/**
 *  ?/ ?/98 Entered into CVS
 *  1/ 2/98 KSpell 0.3.1
 * 11/25/97 KSpell 0.2.3
 * 11/  /97 KSpell 0.2.2
 *  9/21/97 KSpell 0.2
 *  9/11/97 KSpell 0.1
 * 
 * KSpell offers easy access to International ISpell 3.1 as well as a 
 *  typical spell-checker GUI ("Add", "Replace", etc.).  You can use
 *  KSpell to automatically spell-check an ASCII file as well as to implement
 *  online spell-checking and to spell-check proprietary format and
 *  marked up (e.g. HTML, TeX) documents.  The relevant methods for these
 *  three procedures are check(), checkWord(), and checkList().
 *
 * KSpellConfig holds configuration information about KSpell as well
 *  as acting as an options-setting dialog.
 * @short A KDE programmer's interface to International ISpell 3.1  (GPL 1997)
 * @author David Sweet dsweet@wheatcomputers.com
 * @version $Id$
 * @see KSpellConfig
 */


class KSpell : public QObject
{
  Q_OBJECT

public:

  /**
   * KSpell emits ready() when it has verified that ispell is
   *  working properly.  Pass the name of a slot -- do not pass zero!
   * Be sure to call cleanUp() when you are done with KSpell.
   */
  KSpell(QWidget *_parent, QString,
	 QObject *obj, const char *slot, KSpellConfig *_kcs=0,
	 bool _progressbar = TRUE);

  /**
   * Be sure your instance of KSpell isOk() before you use it.
   *  isOk()==TRUE would indicate that any memory that needed to be
   *  allocated was and that ispell is up and running.  If you find
   *  that isOk()==FALSE, it's probably b/c either ispell isn't installed
   *  (and in $PATH) or an invalid path was given for the dictionary
   *  or personal dictionary (see KSpellConfig).
   *
   *  If an instance of KSpell is not Ok, then it never will be.
   *   Delete it, reconfigure a KSpellConfig and try again (perhaps with
   *   the system defaults).
   **/
  inline bool isOk (void) { return ok; }

  /**
   * Clean up ISpell (write out the personal dictionary and close ispell's
   *  stdin).  A "death()" signal will be emitted when the cleanup is
   *  complete, but the cleanUp() method will return immediately.
   **/
  virtual void cleanUp (void);
  /**
   *  check() will spell check a buffer of many words in plain text 
   *  format
   * The _buffer is not modified.  The signal done(char *) will be
   *  sent when check() is finished and the argument will be a 
   *  spell-corrected version of _buffer.  (See done() for more information.)
   * The progress() signal is only sent when a misspelled word is found, so
   *  you may not get as fine a resolution as you requested; but you won't
   *  get a _finer_ resolution.
   * The spell check may be stopped by the user before the entire buffer
   *  has been checked.  You can check lastPosition() to see how far
   *  in _buffer check() reached before stopping.
   */

  virtual bool check (QString _buffer);

  /**
   * Returns the position (for check())  or word number (for checkList()) of
   * the last word checked.
   **/
  inline int lastPosition(void)
    { return lastpos;}

  /**
   * checkList() is more flexible than check().  You could parse any type
   *  of document (HTML, TeX, etc.) into a list of spell-checkable words
   *  and send the list to checkList().  Sending a marked-up document
   *  to check() would result in the mark-up tags being
   *  spell checked.
   * The progress() signals will be accurate here since words are
   *  checked one at a time.
   */
  virtual bool checkList (QStrList *_wordlist);

  /**
   * checkWord() is the most flexible function.  Some apps might need this
   *  flexibility but will sacrifice speed.  Consider checkList()
   *  for checking many words.
   *
   * checkWord () returns FALSE if buffer is not a word, otherwise it
   *  returns TRUE;
   *
   * If usedialog is set to TRUE, KSpell will put up the standard
   *  dialog if the word is not found.  The dialog results can be queried
   *  by using  dlgResult() and replacement().  The possible dlgResult()
   *  values are (from kspelldlg.h):
   *     KS_CANCEL      0 
   *     KS_REPLACE     1
   *     KS_REPLACEALL  2
   *     KS_IGNORE      3
   *     KS_IGNOREALL   4
   *     KS_ADD         5     
   *     KS_STOP        7
   *
   *  The signal corrected() is emitted when the check is complete.  You can
   *  look at suggestions() to see what the suggested replacements were.
   *   If the dialog box is not used, or the user chooses not to change
   *   the word, then new word is just word. pos=0 always.
   */
  virtual bool checkWord (QString _buffer,  bool usedialog=FALSE);

  /**
   * You can use this to manually hide the dialog box.  You only _need_ to
   *  do this when you are done with checkWord();
   */
  void hide (void)   { ksdlg->hide(); }

  /**
   * After calling checkWord (an in response to a misspelled() signal you can
   *  use this to get the list of
   *  suggestions (if any were available)
   */
  inline QStrList *suggestions (void)	{ return &sugg; }

  /**
   * After calling checkWord, you can use this to get the dialog box's
   *  result code.
   */
  inline int dlgResult (void)
    { return dlgresult; }

  /**
   * Moves the dialog.  If the dialog is not currently visible, it will
   *   be placed at this position when it becomes visible.
   */
  void moveDlg (int x, int y);

  inline int heightDlg (void) {return ksdlg->height();}
  inline int widthDlg (void) {return ksdlg->width();}

  /**
   * You might want the full buffer in its partially-checked state.
   */
  const QString *intermediateBuffer (void) {return &newbuffer;}

  /**
   * Tell ispell to ignore this word for the life of this KSpell instance.
   *  ignore() returns FALSE if word is not a word or there was an error
   *  communicating with ispell.
   */
  virtual bool ignore (QString word);

  /**
   * Add a word to the user's personal dictionary.  Returns FALSE if word
   *  is not a word or there was an error communicating with ispell.
   */
  virtual bool addPersonal (QString word);

  /**
   * Returns the KSpellConfig object being used by this KSpell.
   */
  KSpellConfig ksConfig (void) const;

  /**
   * Set the resolution (in percent) of the progress() signals.
   * E.g. setProgressResolution (10) instructs KSpell to send progress
   *  signals (at most) every 10% (10%, 20%, 30%...).
   * The default is 10%.
   */
  void setProgressResolution (unsigned res);

  /**
   * The destructor instructs ispell to write out the personal
   *  dictionary and then terminates ispell.
   */ 
  virtual ~KSpell();

signals:

  /**
   * This is emitted whenever a misspelled word is found by check() or
   *   by checkWord().
   *  If it is emitted by checkWord(), pos=0.
   *  If it is emitted by check(), the pos indicates the position of
   *   the misspelled word in the (original) _buffer.  (The first position is zero.)
   *  If it is emitted by checkList(), pos is the index to the misspelled
   *   word in the QStrList passed to checkList().
   *
   *  These are called _before_ the dialog is opened, so that the
   *   calling program's GUI may be updated. (e.g. the misspelled word may
   *   be highlighted).
   */
  void misspelling (QString originalword, QStrList *suggestions, 
		    unsigned pos);

  /**
   * This is emitted after the dialog is closed, or if the word was 
   * corrected without calling the dialog (i.e., the user previously chose
   * "Replace All" for this word). 
   * Results from the dialog may be checked with dlgResult() and replacement()
   * (see notes for check() for more information).
   */
  void corrected (QString originalword, QString newword, unsigned pos);

  /**
   * This is emitted after KSpell has verified that ispell is running
   *  and working properly.  The calling application should check
   *  isOk() in response to this signal before attempting to use any of
   *  the spell-checking methods (check(), et at).  Those methods _don't_
   *  check isOk() for you.
   */
  void ready(KSpell *);

  /**
   * i is between 1 and 100 -- emitted only during a check ()
   */
  void progress (unsigned int i);

  /**
   * emitted when check() is done
   * Copy the results of buffer if you need them.  You can only rely
   *  on the contents of buffer for the life of the slot which was signaled
   *  by done(const char *).
   */
  void done (const char *buffer);

  /**
   * emitted when checkList() is done.  If the argument is
   * <i>TRUE</i>, then you should update your text from the
   * wordlist, otherwise, don't.
   */
  void done(bool);

  /**
   * emitted when cleanUp() is done
   **/
  void cleanDone();

  /**
   * emitted on terminal errors
   */
  void death(KSpell *);

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
  void ispellErrors (KProcess *, char *, int);

signals:
  void dialog3 ();
  void ez ();
  void eza ();

protected:

  KProcIO *proc;
  QWidget *parent;
  KSpellConfig *ksconfig;
  KSpellDlg *ksdlg;
  QStrList *wordlist, ignorelist, replacelist, sugg;
  
  char *temp;

  bool cleaning, usedialog, texmode, dlgon, ok, personaldict, dialogwillprocess;
  bool progressbar, dialogsetup;

  QString caption, orig;
  QString origbuffer, newbuffer, cwword, dlgorigword;
  QString dlgreplacement, dialog3slot;

  int dlgresult, trystart, maxtrystart;
  unsigned int lastpos, totalpos, lastline, posinline, lastlastline;
  unsigned int offset, progres, curprog;

  int parseOneResponse (const QString &_buffer, QString &word, QStrList *sugg);
  QString funnyWord (QString word);
  void dialog (QString word, QStrList *sugg, const char* _slot);
  inline QString replacement (void)
    { return dlgreplacement; }

  void setUpDialog ( bool reallyusedialogbox = TRUE);

  void emitProgress (void);
  bool cleanFputs (QString s, bool appendCR=TRUE);
  bool cleanFputsWord (QString s, bool appendCR=TRUE);
  void startIspell(void);
  bool writePersonalDictionary (void);
};

#endif
