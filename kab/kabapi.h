/* -*- C++ -*-
 * Dialog widget using the addressbook,
 * provided for inclusion in other programs.
 *  
 * the Configuration Database library
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@hamburg.netsurf.de>
 *                          <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class, NANA
 * $Revision$
 */

#ifndef KABAPI_H
#define KABAPI_H

#include <qdialog.h>
#include "kab.h"

class QPushButton;
class QFrame;

/** The class KabAPI provides a public interface to access the 
  * users address database created using kab. The complete 
  * functionality regarding database operations provided by kab is
  * available using an object of this class.
  *
  * The class is derived from the class QDialog, thus objects
  * can be used as a dialog where the user may select an address
  * similar to the interface of kab. 
  * The following code may be used to let the user select an address:
  * <pre>
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
  * </pre>
  * Some methods declared here return keys of entries. This keys are of the 
  * datatype string declared in the (by now) standard C++ library. Every key
  * is (of course) unique and identifying. If you store it, you can access 
  * the entry it represents with it. Be careful that the entry may have been 
  * deleted by another program instance meanwhile!
  * Please note that the kab code assumes that a QImage-IO-handler for
  * jpeg pictures has been installed. You will get a message that the 
  * background image could not been loaded if you forget it. 
  * See kab/main.cc and kab/kabapi_test.cc for examples.<br>
  * <tt>Please be careful to test for the return code NotImplemented as 
  * long the kab API is not completely finished (it is not).</tt>
  * @short The class KabAPI defines the API to access user address databases.
  * @author Mirko Sucker <mirko.sucker@unibw-hamburg.de>
  * @version $Id$
  * @see #AddressBook #AddressWidget #QDialog
  */

class KabAPI : public QDialog
{
  Q_OBJECT
public:
  /** ErrorCode is used in all methods that may return more status 
    * codes than true or false.<br>
    * <tt> NoError </tt> The operations has been finished successful.<br>
    * <tt> NotImplemented </tt> The feature you requested is not 
    * implemented by KabAPI.<br>
    * <tt> PermDenied </tt> Not all permissions needed for the operation 
    * were available. Also returned if a database is locked.<br>
    * <tt> NoSuchFile </tt> No database file exists for this user.<br>
    * <tt> NoEntry </tt> You tried to retrieve an entry but there is none.<br>
    * <tt> InternError </tt> There was a program error.
    */
  enum ErrorCode {
    NoError,
    NotImplemented,
    PermDenied,
    NoSuchFile,
    NoEntry,
    Rejected,
    InternError };
  /** The constructor creates a KabAPI object, but it does not load the
    * database itselfes, as you could not query if this could be done 
    * without failures. Thus you have to call @ref #init before you can 
    * use the database.
    * @param parent The QWidget pointer to the parent widget.
    */
  KabAPI(QWidget* parent=0);
  /** You must call init before accessing the dialog. init opens the
    * database file (usually $HOME/.kde/share/apps/kab/addressbook.database)
    * and loads its contents. If the database is already opened by another 
    * instance of kab and readonly is NOT true, init returns 
    * PermDenied. 
    * You may try to open a database for reading and writing first,
    * and if it fails with PermDenied try again to open it readonly.
    * The dialog will not provide the capabilities to store the database or 
    * to edit an entry then.
    * @return NoError if all succeeded or a valid ErrorCode.
    * @see #ErrorCode
    */
  ErrorCode init(bool readonly=true);

  /** The method getEntry returns the selected entry. 
    * @return NoError if all succeeded or a valid ErrorCode.
    * @see #ErrorCode
    * @param entry Reference to an AddressBook::Entry -object.
    * @param key Reference to a string where the key of the entry is stored.
    */
  ErrorCode getEntry(AddressBook::Entry& entry, string& key);
  /** Using the method getEntries, the caller will get a copy of all entries in the 
    *  database. This might seem unneeded, but the address database can be 
    *  used by multiple instances of the kab API at the same time, so that, 
    *  if the programmer wants, for example, print a letter header for all 
    *  persons, the database might change during the operation. That is why 
    *  she can retrieve the whole database in one operation. 
    *  It is required that the referenced list is empty.
    *  Note that the function returns NoEntry if the database is empty.
    * @see #ErrorCode
    * @short Retrieves all entries out of the database.
    * @param entries Reference to a list of entries.
    * @return NoError or a valid error code. 
    */
  ErrorCode getEntries(list<AddressBook::Entry>& entries);
  /** The method requires that the database is not opened readonly.
    * A dialog will show up that allows the users to edit the entry
    * before it is inserted into the database.
    * If you do not want to open the dialog set edit to false.
    * @short Adds an entry to the users database.
    * @return NoError if all succeeded or a valid ErrorCode, especially PermDenied.
    * @param entry Reference to the entry to be added.
    * @param key Reference to a string where the key of the new entry is stored.
    * @param edit If true, the edit dialog is shown.
    */
  ErrorCode add(const AddressBook::Entry& entry, 
		string& key, bool edit=true);
  /** This method can be used to edit an Entry object used by your program.
    * The "entry" does not need to be in the database and is NOT added to 
    * it. Shortly, the database is not changed. Thus, you may call this 
    * method even if your database object is read only. 
    * @short Opens the kab edit dialog to edit the entry.
    * @param entry The entry to be edited.
    * @return NoError if the user accepted the dialog or Rejected if she rejeccted it.
    */
  ErrorCode edit(AddressBook::Entry& entry);
  /** In difference to the other edit method, this one will edit an entry 
    * in the database. A dialog will pop up where the user can edit it,
    * if the dialog is finished with "OK", the entry will be changed.
    * If called for a read only database, the method will return 
    * PermDenied.
    * @short Edit the database entry with this key.
    * @param key The key of the entry to be edited.
    * @return Rejected if the user cancelled the dialog, NoError if the entry has been changed or another ErrorCode.
    */
  ErrorCode edit(const string& key);
  /** If the preferences of kab say to query before deleting, the user has
    * to click "yes" on a message box that appeares.
    * If called for a read only database, the method will return 
    * PermDenied.
    * @short Deletes an entry in the database by its key.
    * @param key The key of the entry to delete.
    * @return NoEntry if there is no entry with this key  or another ErrorCode.
    */
  ErrorCode remove(const string& key);
  /** Use getEntryByName to find entries that look like the name given. 
    * The name might be incomplete or diffuse. 
    * @short This method delivers the closest matches to the given name.
    * @param name The name, containing "." for abbreviations.
    * @param entries Reference to a list of entries where matches are stored.
    * @param max Maximum number of returned entries.
    * @return NoError if an entry is found or NoEntry.
    */
  ErrorCode getEntryByName(const string& name, 
			   list<AddressBook::Entry>& entries,
			   const int max=5);
  /** This method also searches for close matches to the pattern,
    * but it compares the whole entry given. This way you can search for,
    * for example, nearly similar email addresses. Empty parts of the 
    * entry are not considered as criteria.
    * @short This method delivers the closest matches to the given entry.
    * @param name The name, containing "." for abbreviations.
    * @param entries Reference to a list of entries where matches are stored.
    * @param max Maximum number of returned entries.
    * @return NoError if an entry is found or NoEntry.
    */    
  ErrorCode getEntryByName(const AddressBook::Entry& pattern,
			   list<AddressBook::Entry>& entries,
			   const int max=5);
protected:
  AddressWidget* widget;
  QPushButton* buttonOK;
  QPushButton* buttonCancel;
  QFrame* frameLine;
public slots:
  void initializeGeometry();
};

#endif // KABAPI_H
