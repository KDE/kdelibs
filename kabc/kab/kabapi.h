/* -*- C++ -*-
   Dialog widget using the addressbook,
   provided for inclusion in other programs.

   the KDE addressbook

   $ Author: Mirko Boehm $
   $ Copyright: (C) 1996-2001, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org $
   $ License: GPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $
*/

#ifndef KABAPI_H
#define KABAPI_H

#include "addressbook.h"
#include <kdialogbase.h>

class QPushButton;
class KListBox;
class Q3ListBoxItem;

/**
  * The class KabAPI provides a public interface to access the
  * users address database created using kab. The complete
  * functionality regarding database operations provided by kab is
  * available using an object of this class.
  *
  * The class is derived from the class KDialogBase, thus objects
  * can be used as a KDE dialog where the user may select a person
  * out of the entries in his personal database.
  * The following code may be used to let the user select an address:
  * \code
  * KabAPI kabapi(this);
  * if(dialog.init()!=KabAPI::NoError)
  *   {
  *     ... error handling
  *   }
  * AddressBook::Entry entry;
  * if(kabapi.exec())
  *   {
  *     if(!kabapi.getEntry(entry))
  *       {
  *         // ... the database is empty
  *       } else {
  *         // ... use the entry
  *       }
  *   }
  * ...
  * \endcode
  * Some methods declared here return keys of entries. The keys are of the
  * datatype KabKey. Every key
  * is (of course) unique and identifying. If you store it, you can access
  * the entry it represents with it. Be careful that the entry may have been
  * deleted by another program instance meanwhile!
  * <tt>Please be careful to test for the return code NotImplemented as
  * long the kab API is not completely finished.</tt>
  * @short The class KabAPI defines the API to access user address databases.
  * @author Mirko Boehm <mirko@kde.org>
  * @see AddressBook #KDialogBase
  */

class KabAPI : public KDialogBase
{
  // ############################################################################
  Q_OBJECT
  // ----------------------------------------------------------------------------
public:
  /**
   * The constructor creates a KabAPI object, but it does not load the
   * database itselfes, as you could not query if this could be done
   * without failures. Thus you have to call init before you can
   * use the database.
   * @param parent The QWidget pointer to the parent widget.
   * @param name   The name of the object (deprecated).
   */
  KabAPI(QWidget* parent=0, const char* name=0);
  /**
   * You must call init before accessing the database. init opens the
   * database file (usually $HOME/.kde/share/apps/kab/addressbook.database)
   * and loads its contents.
   * @return NoError if all succeeded or a valid ErrorCode.
   * @see AddressBook::ErrorCode
   */
  AddressBook::ErrorCode init();
  /**
   * Get the addressbook object of this API. This is probably the most powerful
   * method in the KabAPI since it allows you to access the database backend
   * itselfes.
   * If the API has not been initialized (using #init) before, zero is returned.
   * @see init
   */
  AddressBook* addressbook();
  /**
   * Save the database to the file.
   * This method is used to force the database to save its contents.
   * If force is true, the method will try to get writing permissions to
   * the file if the database is opened readonly. After finishing saving,
   * the r/o state is reset. This allows easier file sharing, since by default,
   * all files are opened readonly aand closed after all read accesses.
   */
  AddressBook::ErrorCode save(bool force=false);
  /**
   * The method getEntry returns the selected entry.
   * @return NoError if all succeeded or a valid ErrorCode.
   * @see AddressBook::ErrorCode
   * @param entry Reference to an AddressBook::Entry -object.
   * @param key Reference to a KabKey where the key of the entry is stored.
   */
  AddressBook::ErrorCode getEntry(AddressBook::Entry& entry, KabKey& key);
  /**
   * Using the method getEntries, the caller will get a copy of all entries
   * in the database. This might seem unneeded, but the address database can be
   * used by multiple instances of the kab API at the same time, so that,
   * if the programmer wants, for example, print a letter header for all
   * persons, the database might change during the operation. That is why
   * she can retrieve the whole database in one operation.
   * It is required that the referenced list is empty.
   * Note that the function returns NoEntry if the database is empty.
   * @see AddressBook::ErrorCode
   * @short Retrieves all entries out of the database.
   * @param entries Reference to a list of entries.
   * @return NoError or a valid error code.
   */
  AddressBook::ErrorCode getEntries(std::list<AddressBook::Entry>& entries);
  /**
   * The method requires that the database is not opened readonly.
   * @short Adds an entry to the users default database.
   * @return NoError if all succeeded or a valid ErrorCode, especially PermDenied.
   * @param entry Reference to the entry to be added.
   * @param key Reference to a KabKey where the key of the new entry is stored.
   * @param update Whether to update the mirror map or not.
   * Note: The functionality to edit an entry herein has been removed.
   */
  AddressBook::ErrorCode add(const AddressBook::Entry& entry, KabKey& key,
			     bool update=true);
  /**
   * If the preferences of kab say to query before deleting, the user has
   * to click "yes" on a message box that appeares.
   * If called for a read only database, the method will return
   * PermDenied.
   * @short Deletes an entry in the database by its key.
   * @param key The key of the entry to delete.
   * @return NoEntry if there is no entry with this key  or another ErrorCode.
   */
  AddressBook::ErrorCode remove(const KabKey& key);
  /**
   * Use getEntryByName to find entries that look like the name given.
   * The name might be incomplete or diffuse.
   * @short This method delivers the closest matches to the given name.
   * @param name The name, containing "." for abbreviations.
   * @param entries Reference to a list of entries where matches are stored.
   * @param max Maximum number of returned entries.
   * @return NoError if an entry is found or NoEntry.
   */
  AddressBook::ErrorCode getEntryByName(const QString& name,
			   std::list<AddressBook::Entry>& entries,
			   const int max=5);
  /**
   * This method also searches for close matches to the pattern,
   * but it compares the whole entry given. This way you can search for,
   * for example, nearly similar email addresses. Empty parts of the
   * entry are not considered as criteria.
   * @short This method delivers the closest matches to the given entry.
   * @param pattern Entry that serves as a pattern to look for.
   * @param entries Reference to a list of entries where matches are stored.
   * @param max Maximum number of returned entries.
   * @return NoError if an entry is found or NoEntry.
   */
  AddressBook::ErrorCode getEntryByName(const AddressBook::Entry& pattern,
			   std::list<AddressBook::Entry>& entries,
			   const int max=5);
  /**
   * Execute this dialog. This overloads QDialog::exec to fill the list box
   * before showing.
   */
  int exec();
  // ----------------------------------------------------------------------------
protected:
  /**
   * This is our backend to the users address database.
   */
  AddressBook* book;
  /**
   * This displays the overview over the addresses in the dialog.
   */
  KListBox* listbox;
  /**
   * The index of the selected entry. This value is only valid after the
   * KabAPI dialog has been executed and accepted by the user.
   */
  int selection;
protected slots:
  /**
   * Capture selections in the dialog (listbox).
   */
  void entrySelected(int);
  /**
   * Capture status messages from book.
   */
  void setStatusSlot(const QString&);
  /**
   * Double clicked on an item. @reimp
   */
  void slotDoubleClicked ( Q3ListBoxItem * );
  signals:
  /**
   * Send status messages.
   */
  void setStatus(const QString&);
  // ############################################################################
private:
  class KAbAPIPrivate;
  KAbAPIPrivate *d;
};

#endif // KABAPI_H

