/* -*- c++ -*-
 * (C) Daniel M. Duley 1999 KDE Artistic
 */


#ifndef __KRECENTDOCUMENT_H
#define __KRECENTDOCUMENT_H

#include <qstring.h>

/**
 * Manage the "Recent Document Menu" entries displayed by
 * applications such as Kicker and Konqueror.
 *
 * These entries are automatically
 * generated .desktop files pointing to the current application and document.
 * You should call the static @ref add() method whenever the user opens or saves a
 * new document if you want it to show up in the menu.
 *
 * You don't have to worry about this if you are using any @ref KFileBaseDialog
 * derived class to open and save documents, as it already calls this class.
 * User defined limits on the maximum number of documents to save, etc... are
 * all automatically handled.
 *
 * @author Daniel M. Duley <mosfet@kde.org>
 */
class KRecentDocument
{
public:
    /**
     * Add a new item to the Recent Document menu.
     *
     * @param documentStr The full path to the document or URL to add.
     *
     * @param isURL Local files and URL .desktop files are encoded differently
     * in order for the filename to be easier for Konqueror users to read. Set
     * to @p true if @p documentStr is an URL and not a local file path.
     */
    static void add(const QString &documentStr, bool isURL = false);
    /**
     * Clear the recent document menu of all entries.
     */
    static void clear();
    /**
     * Retrieve the maximum amount of recent document entries allowed.
     */
    static int maximumItems();
};

#endif
