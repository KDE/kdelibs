/* This file is part of the KDE libraries
   Copyright (C) 1997 David Sweet <dsweet@kde.org>
   Copyright (C) 2003 Zack Rusin <zack@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef K3SPELL_H
#define K3SPELL_H

#include <kde3support_export.h>

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QString>

class QTextCodec;
class KProcess;
class K3SpellConfig;
class K3SpellDlg;

/**
 * %KDE Spellchecker
 *
 * A %KDE programmer's interface to International ISpell 3.1, ASpell,
 * HSpell and ZPSpell..
 * A static method, modalCheck() is provided for convenient
 *  access to the spellchecker.
 *
 * @author David Sweet <dsweet@kde.org>
 * @see K3SpellConfig, KSyntaxHighlighter
 *
 * @deprecated, use sonnet instead
 */
class KDE3SUPPORT_EXPORT_DEPRECATED K3Spell : public QObject
{
  Q_OBJECT

public:

  /**
   * Possible states of the spell checker.
   *
   * @li @p Starting - After creation of K3Spell.
   * @li @p Running - After the ready signal has been emitted.
   * @li @p Cleaning - After cleanUp() has been called.
   * @li @p Finished - After cleanUp() has been completed.
   *
   * The following error states exist:
   *
   * @li @p Error -  An error occurred in the @p Starting state.
   * @li @p Crashed - An error occurred in the @p Running state.
   **/
  enum spellStatus { Starting = 0, Running, Cleaning, Finished, Error, Crashed, FinishedNoMisspellingsEncountered };

  /**
   * These are possible types of documents which the spell checker can check.
   *
   * @li @p Text  - The default type, checks every word
   * @li @p HTML  - For HTML/SGML/XML documents, will skip the tags,
   * @li @p TeX   - For TeX/LaTeX documents, will skip commands,
   * @li @p Nroff - For nroff/troff documents.
   *
   * Please note that not every option is supported on every type of
   * checker (e.g. ASpell doesn't support Nroff). In case a type
   * of a document is not supported the default Text option will
   * be used.
   */
  enum SpellerType { Text = 0, HTML, TeX, Nroff };

  /**
   * Starts the spellchecker.
   *
   * K3Spell emits ready() when it has verified that
   * ISpell/ASpell is working properly. Pass the name of a slot -- do not pass zero!
   * Be sure to call cleanUp() when you are done with K3Spell.
   *
   * If K3Spell could not be started correctly, death() is emitted.
   *
   * @param parent      Parent of K3SpellConfig dialog..
   * @param caption     Caption of K3SpellConfig dialog.
   * @param receiver    Receiver object for the ready(K3Spell *) signal.
   * @param slot        Receiver's slot, will be connected to the ready(K3Spell *) signal.
   * @param kcs         Configuration for K3Spell.
   * @param progressbar Indicates if progress bar should be shown.
   * @param modal       Indicates modal or non-modal dialog.
   */
  K3Spell(QWidget *parent, const QString &caption,
	 QObject *receiver, const char *slot, K3SpellConfig *kcs=0,
	 bool progressbar = true, bool modal = false);

  /**
   * Starts the spellchecker.
   *
   * K3Spell emits ready() when it has verified that
   * ISpell/ASpell is working properly. Pass the name of a slot -- do not pass zero!
   * Be sure to call cleanUp() when you are done with K3Spell.
   *
   * If K3Spell could not be started correctly, death() is emitted.
   *
   * @param parent      Parent of K3SpellConfig dialog..
   * @param caption     Caption of K3SpellConfig dialog.
   * @param receiver    Receiver object for the ready(K3Spell *) signal.
   * @param slot        Receiver's slot, will be connected to the ready(K3Spell *) signal.
   * @param kcs         Configuration for K3Spell.
   * @param progressbar Indicates if progress bar should be shown.
   * @param modal       Indicates modal or non-modal dialog.
   * @param type        Type of the document to check
   */
  K3Spell(QWidget *parent, const QString &caption,
	 QObject *receiver, const char *slot, K3SpellConfig *kcs,
	 bool progressbar, bool modal, SpellerType type);

  /**
   * Returns the status of K3Spell.
   *
   * @see spellStatus()
   */
  spellStatus status() const;

  /**
   * Cleans up ISpell.
   *
   * Write out the personal dictionary and close ISpell's
   *  stdin.  A death() signal will be emitted when the cleanup is
   *  complete, but this method will return immediately.
   */
  virtual void cleanUp ();

  /**
   * Sets the auto-delete flag. If this is set, the K3Spell object
   * is automatically deleted after emitting death().
   */
  void setAutoDelete(bool _autoDelete);

  /**
   *  Spellchecks a buffer of many words in plain text
   *  format.
   *
   * The @p _buffer is not modified.  The signal done() will be
   *  sent when check() is finished and the argument will be a
   *  spell-corrected version of @p _buffer.
   *
   * The spell check may be stopped by the user before the entire buffer
   *  has been checked.  You can check lastPosition() to see how far
   *  in @p _buffer check() reached before stopping.
   */

  virtual bool check (const QString &_buffer, bool usedialog = true);

  /**
   * Returns the position (when using check())  or word
   * number (when using checkList()) of
   * the last word checked.
   */
  int lastPosition() const;

  /**
   * Spellchecks a list of words.
   *
   * checkList() is more flexible than check().  You could parse
   *  any type
   *  of document (HTML, TeX, etc.) into a list of spell-checkable words
   *  and send the list to checkList().  Sending a marked-up document
   *  to check() would result in the mark-up tags being
   *  spell checked.
   */
  virtual bool checkList (QStringList *_wordlist, bool usedialog = true);

  /**
   * Spellchecks a single word.
   *
   * checkWord() is the most flexible function.  Some applications
   *  might need this flexibility but will sacrifice speed when
   *  checking large numbers of words.  Consider checkList() for
   *  checking many words.
   *
   *  Use this method for implementing  "online" spellchecking (i.e.,
   *  spellcheck as-you-type).
   *
   * checkWord() returns @p false if @p buffer is not a single word (e.g.
   *  if it contains white space), otherwise it returns @p true;
   *
   * If @p usedialog is set to @p true, K3Spell will open the standard
   *  dialog if the word is not found.  The dialog results can be queried
   *  by using dlgResult() and replacement().
   *
   *  The signal corrected() is emitted when the check is
   *  complete.  You can look at suggestions() to see what the
   *  suggested replacements were.
   */
  virtual bool checkWord (const QString &_buffer, bool usedialog = false);
  bool checkWord( const QString & buffer, bool _usedialog, bool suggest );

  /**
   * Hides the dialog box.
   *
   * You'll need to do this when you are done with checkWord();
   */
  void hide ();

  /**
   * Returns list of suggested word replacements.
   *
   * After calling checkWord() (an in response to
   *  a misspelled() signal you can
   *  use this to get the list of
   *  suggestions (if any were available).
   */
  QStringList suggestions () const;

  /**
   * Gets the result code of the dialog box.
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
  int dlgResult () const;

  /**
   * Moves the dialog.
   *
   * If the dialog is not currently visible, it will
   *   be placed at this position when it becomes visible.
   * Use this to get the dialog out of the way of a highlighted
   * misspelled word in a document.
   */
  void moveDlg (int x, int y);

  /**
   * Returns the height of the dialog box.
   */
  int heightDlg () const;
  /**
   * Returns the width of the dialog box.
   */
  int widthDlg () const;

  /**
   * Returns the partially spellchecked buffer.
   *
   * You might want the full buffer in its partially-checked state.
   */
  QString intermediateBuffer () const;

  /**
   * Tells ISpell/ASpell to ignore this word for the life of this K3Spell instance.
   *
   *  @return false if @p word is not a word or there was an error
   *  communicating with ISpell/ASpell.
   */
  virtual bool ignore (const QString & word);

  /**
   * Adds a word to the user's personal dictionary.
   *
   * @return false if @p word
   *  is not a word or there was an error communicating with ISpell/ASpell.
   */
  virtual bool addPersonal (const QString & word);

  /**
   * @return the K3SpellConfig object being used by this K3Spell instance.
   */
  K3SpellConfig ksConfig () const;

  /**
   * Sets the resolution (in percent) of the progress() signals.
   *
   * E.g. setProgressResolution (10) instructs K3Spell to send progress
   *  signals (at most) every 10% (10%, 20%, 30%...).
   * The default is 10%.
   */
  void setProgressResolution (unsigned int res);

  /**
   * The destructor instructs ISpell/ASpell to write out the personal
   *  dictionary and then terminates ISpell/ASpell.
   */
  virtual ~K3Spell();

  /**
   * @deprecated
   * Performs a synchronous spellcheck.
   *
   * This method does not return until spellchecking is done or canceled.
   * Your application's GUI will still be updated, however.
   *
   */
  static KDE_DEPRECATED int modalCheck( QString& text );

  /**
   * Performs a synchronous spellcheck.
   *
   * This method does not return until spellchecking is done or canceled.
   * Your application's GUI will still be updated, however.
   *
   * This overloaded method uses the spell-check configuration passed as parameter.
   */
  static int modalCheck( QString& text, K3SpellConfig * kcs );

  /**
   * Call setIgnoreUpperWords(true) to tell the spell-checker to ignore
   * words that are completely uppercase. They are spell-checked by default.
   */
  void setIgnoreUpperWords(bool b);

  /**
   * Call setIgnoreTitleCase(true) to tell the spell-checker to ignore
   * words with a 'title' case, i.e. starting with an uppercase letter.
   * They are spell-checked by default.
   */
  void setIgnoreTitleCase(bool b);

Q_SIGNALS:

  /**
   * Emitted whenever a misspelled word is found by check() or
   *   by checkWord().
   *  If it is emitted by checkWord(), @p pos=0.
   *  If it is emitted by check(), then @p pos indicates the position of
   *   the misspelled word in the (original) @p _buffer.
   *   (The first position is zero.)
   *  If it is emitted by checkList(), @p pos is the index to
   *  the misspelled
   *   word in the QStringList passed to checkList().
   *  Note, that @p originalword can be only a word part, if it's a
   *  word with hyphens.
   *
   *  These are called _before_ the dialog is opened, so that the
   *   calling program's GUI may be updated. (e.g. the misspelled word may
   *   be highlighted).
   */
  void misspelling (const QString & originalword, const QStringList & suggestions,
		    unsigned int pos);

  /**
   * Emitted after the "Replace" or "Replace All" buttons of the dialog
   * was pressed, or if the word was
   * corrected without calling the dialog (i.e., the user previously chose
   * "Replace All" for this word).
   *
   * Results from the dialog may be checked with dlgResult()
   *  and replacement().
   *
   * Note, that when using checkList() this signal can occur
   * more than once with same list position, when checking a word with
   * hyphens. In this case @p originalword is the last replacement.
   *
   * @see check()
   */
  void corrected (const QString & originalword, const QString & newword, unsigned int pos);

  /**
   * Emitted when the user pressed "Ignore All" in the dialog.
   * This could be used to make an application or file specific
   * user dictionary.
   *
   */
  void ignoreall (const QString & originalword);

  /**
   * Emitted when the user pressed "Ignore" in the dialog.
   * Don't know if this could be useful.
   *
   */
  void ignoreword (const QString & originalword);

  /**
   * Emitted when the user pressed "Add" in the dialog.
   * This could be used to make an external user dictionary
   * independent of the ISpell personal dictionary.
   *
   */
  void addword (const QString & originalword);

  /**
   * Emitted when the user pressed "ReplaceAll" in the dialog.
   */
  void replaceall( const QString & origword ,  const QString &replacement );

  /**
   * Emitted after K3Spell has verified that ISpell/ASpell is running
   * and working properly.
   */
  void ready(K3Spell *);

  /**
   * Emitted during a check().
   * @p i is between 1 and 100.
   */
  void progress (unsigned int i);

  /**
   * Emitted when check() is done.
   *
   * Be sure to copy the results of @p buffer if you need them.
   *  You can only rely
   *  on the contents of buffer for the life of the slot which was signaled
   *  by done().
   */
  void done (const QString &buffer);

  /**
   * Emitted when checkList() is done.
   *
   * If the argument is
   * @p true, then you should update your text from the
   * wordlist, otherwise not.
   */
  void done(bool);

  /**
   * Emitted on terminal errors and after clean up.
   *
   * You can delete the K3Spell object in this signal.
   *
   * You can check status() to see what caused the death:
   * @li @p Error - K3Spell could not start.
   * @li @p Crashed - K3Spell encountered an unexpected error during execution.
   * @li @p Finished - Clean up finished.
   */
  void death( );


protected Q_SLOTS:
  /* All of those signals from KProcess get sent here. */
  void K3Spell2 ();
  void checkWord2 ();
  void checkWord3 ();
  void check2 ();
  void checkList2 ();
  void checkList3a ();
  void checkListReplaceCurrent ();
  void checkList4 ();
  void dialog2 (int dlgresult);
  void check3 ();
  void suggestWord();

  void slotStopCancel (int);
  void ispellExit ();
  void emitDeath();
  void ispellErrors ();
  void checkNext();

private Q_SLOTS:
  /**
   * Used for modalCheck().
   */
  void slotModalReady();

  /**
   * Used for modalCheck().
   */
  void slotModalDone( const QString & );

  /**
   * Used for modalCheck().
   */
  void slotSpellCheckerCorrected( const QString & oldText, const QString & newText, unsigned int );

  /**
   * Used for modalCheck().
   */
  void  slotModalSpellCheckerFinished( );

Q_SIGNALS:
  void dialog3 ();

protected:

  KProcess *proc;
  QWidget *parent;
  K3SpellConfig *ksconfig;
  K3SpellDlg *ksdlg;
  QStringList *wordlist;
  QStringList::Iterator wlIt;
  QStringList ignorelist;
  QStringList replacelist;
  QStringList sugg;

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
  int lastpos;
  unsigned int totalpos;
  int lastline;
  unsigned int posinline;
  unsigned int lastlastline;
  unsigned int offset;
  unsigned int progres;
  unsigned int curprog;

  /**
   * Used for modalCheck.
   */
  bool modaldlg;
  static QString modaltext;
  static int modalreturn;
  static QWidget* modalWidgetHack;

  int parseOneResponse (const QString &_buffer, QString &word, QStringList &sugg);
  QString funnyWord (const QString & word);
  void dialog (const QString & word, QStringList & sugg, const char* _slot);
  QString replacement () const;

  void setUpDialog ( bool reallyusedialogbox = true);

  void emitProgress ();
  bool cleanFputs (const QString & s );
  bool cleanFputsWord (const QString & s );
  void startIspell();
  bool writePersonalDictionary();
  void initialize( QWidget *_parent, const QString &_caption,
                   QObject *obj, const char *slot, K3SpellConfig *_ksc,
                   bool _progressbar, bool _modal, SpellerType type );

private:
  class K3SpellPrivate;
  K3SpellPrivate *d;
};

#endif
