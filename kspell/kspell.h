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
 *  1/ 2/98 KSpell 0.3.1
 * 11/25/97 KSpell 0.2.3
 * 11/  /97 KSpell 0.2.2
 *  9/21/97 KSpell 0.2
 *  9/11/97 KSpell 0.1
 * @short a KDE programmer's interface to International ISpell 3.1  (GPL 1997)
 * @author David Sweet
 * @see KSpellConfig
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
 */


class KSpell : public QObject
{
  Q_OBJECT;


  bool ok, personaldict;
  
  KProcIO *proc;
  QString ispellID, caption;
  QWidget *parent;
  KSpellConfig *ksconfig;
  KSpellDlg *ksdlg;
  QStrList ignorelist, replacelist;
  QStrList *wordlist, sugg;
  QString orig;
  bool usedialog, texmode;
  QString buffer, newbuffer, cwword, dlgorigword;

  QString dlgreplacement, dialog3slot;
  int dlgresult, trystart, maxtrystart;
  bool dlgon;
  
  unsigned int lastpos, totalpos, lastline, posinline, lastlastline, offset;
  int progres, curprog;
  char *temp;

  int parseOneResponse (char *_buffer, char *word, QStrList *sugg);
  char *funnyWord (char *word);
  void dialog (char *word, QStrList *sugg, char *_slot);

public:

  /**
   * KSpell emits ready() when it has verified that ispell is
   *  working properly.  Pass the name of a slot -- do not pass zero!
   */

  KSpell(QWidget *_parent, const char *caption,
	 QObject *obj, char *slot, KSpellConfig *_kcs=0);
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
  bool isOk (void) { return ok; }

  /**
   *  check() will spellcheck a buffer of many words in plaintext 
   *  format
   * The _buffer is not modified.  The signal done(char *) will be
   *  sent when check() is finished and the argument will be a 
   *  spell-corrected version of _buffer.  (See done() for more information.)
   * The progress() signal is only sent when a misspelled word is found, so
   *  you may not get as fine a resolution as you requested; but you won't
   *  get a _finer_ resolution.
   * The spellcheck may be stopped by the user before the entire buffer
   *  has been checked.  You can check lastPosition() to see how far
   *  in _buffer check() reached before stopping.
   */

  bool check (char *_buffer);
  int lastPosition(void)
    { return lastpos;}

  /**
   * checkList() is more flexible than check().  You could parse any type
   *  of document (HTML, TeX, etc.) into a list of spell-checkable words
   *  and send the list to checkList().  Sending a marked-up document
   *  to check() would result in the mark-up tags being
   *  spellchecked.
   * The progress() signals will be accurate here since words are
   *  checked one at a time.
   */
  bool checkList (QStrList *_wordlist);

  /**
   * checkWord() is the most flexible function.  Some apps might need this
   *  flexibility.
   *
   * checkWord () returns FALSE if buffer is not a word, otherwise it
   *  returns TRUE;
   *
   * If usedialog is set to TRUE, KSpell will put up the standard
   *  dialog if the word is not found.  The dialog results can be queried
   *  by using  dlgResult() and replacement().  The possible dlgResult()
   *  values are:  (from kspelldlg.h)
   *     #define KS_CANCEL      0 
   *     #define KS_REPLACE     1
   *     #define KS_REPLACEALL  2
   *     #define KS_IGNORE      3
   *     #define KS_IGNOREALL   4
   *     #define KS_ADD         5     
   *     #define KS_STOP        7
   *
   *  The signal corrected() is emitted when the check is complete.  You can
   *  look at suggestions() to see what the suggested replacements were.
   *   If the dialogc box is not used, or the user chooses not to change
   *   the word, then newword is just word. pos=0 always.
   */
  bool checkWord (char *_buffer,  bool usedialog=FALSE);

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
  QStrList *suggestions (void)
    { return &sugg; }
  /**
   * After calling checkWord, you can use this to get the dialog box's
   *  result code.
   */
  int dlgResult (void)
    { return dlgresult; }
  /**
   * Moves the dialog.  If the dialog is not currently visible, it will
   *   be placed at this position when it becomes visible.
   */
  void moveDlg (int x, int y);
  int heightDlg (void) {return ksdlg->height();}
  int widthDlg (void) {return ksdlg->width();}

  /**
   * You might want the full buffer in its partially-checked state.
   */
  const char *intermediateBuffer (void) {return buffer;}

  /**
   * Tell ispell to ignore this word for the life of this KSpell instance.
   *  ignore() returns FALSE if word is not a word or there was an error
   *  communicating with ispell.
   */
  bool ignore (char *word);
  /**
   * Add a word to the user's personal dictionary.  Returns FALSE if word
   *  is not a word or there was an error communicating with ispell.
   */
  bool addPersonal (char *word);

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
  void setProgressResolution (int res);

  /**
   * The destructor instructs ispell to write out the personal
   *  dictionary and then terminates ispell.
   */ 
  ~KSpell();

signals:
  /**
   * This is emitted whenever a misspelled word is found by check() or
   *   by checkWord().
   *  If it is emitted by checkWord(), pos=0.
   *  If it is emitted by check(), the pos indicates the position of
   *   the misspelled word in the _buffer.  (The first position is zero.)
   *  If it is emitted by checkList(), pos is the index to the misspelled
   *   word in the QStrList passed to checkList().
   *
   *  These are called _before_ the dialog is opened, so that the
   *   calling program's GUI may be updated. (e.g. the misspelled word may
   *   be highlighted).
   */
  void misspelling (char *originalword, QStrList *suggestions, int  pos);
  /**
   * This is emitted after the dialog is closed, or if the word was 
   * corrected without calling the dialog (i.e., the user previously chose
   * "Replace All" for this word). 
   * Results from the dialog may be checked with dlgResult() and replacement()
   * (see notes for check() for more information).
   */
  void corrected (char *originalword, char *newword, int pos);


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
  void progress (int i);
  /**
   * emitted when check() is done
   * Copy the results of buffer if you need them.  You can only rely
   *  on the contents of buffer for the life of the slot which was signalled
   *  by done(char *).
   */
  void done (char *buffer);
  /**
   * emitted when checkList() is done.  If the argument is
   * <i>TRUE</i>, then you should update your text from the
   * wordlist, otherwise, don't.
   */
  void done(bool);

  /**
   * emitted on teminal errors
   */
  void death(KSpell *);

protected slots:
    /* All of those signals from KProcIO get sent here. */
  void KSpell2 (KProcIO *);
  void checkWord2 (KProcIO *);
  void checkWord3 ();
  void check2 (KProcIO *);
  void check2a ();
  void checkList2 ();
  void checkList3a (KProcIO *);
  void checkList3 ();
  void checkList4 ();
  void dialog2 (int dlgresult);
  void check3 ();
		   
  void ispellExit (KProcess *);

signals:
  void dialog3 ();
  void ez ();
  void eza ();

protected:
  char *replacement (void)
    { return dlgreplacement.data(); }
  bool isBadIspellChar (char);
  void  emitProgress (void);
  bool cleanFputs (const char *s, bool appendCR=TRUE);
  bool cleanFputsWord (const char *s, bool appendCR=TRUE);
  void startIspell(void);
};

#endif
