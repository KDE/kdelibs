/*****************************************************************

Copyright (c) 2000, 2001 Matthias Hoelzer-Kluepfel
                         Tobias Koenig <tokoe82@yahoo.de>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef __K_TIP_H__
#define __K_TIP_H__

#include <qstringlist.h>

#include <kdialog.h>

class QCheckBox;

class KTextBrowser;

/**
 * A database for tips-of-the-day.
 *
 * This class provides convenient access to a database containing
 * tips of the day. The database is stored in a XML file and parsed
 * when a KTipDatabase object is created.
 *
 * Once the file is read in, you can access the tips to display
 * them in the tip of the day dialog.
 *
 * The state of the tipdialog is saved to the applications's config file
 * in the group "TipOfDay" with a bool entry "RunOnStart". Check this value
 * if you want to allow the user to enable/disable the tipdialog in the
 * application's configuration dialog.
 *
 * @author Matthias Hoelzer-Kluepfel <mhk@caldera.de>
 *
 */
class KTipDatabase
{
public:
    /**
     * The constructor.
     *
     * This reads in the tips from a file with the given name. If no name is
     * given, a file called 'application-name/tips' will be loaded.
     *
     * @param tipFile The name of the tips file.
     */
    KTipDatabase(const QString &tipFile = QString::null);

    /**
     * Returns the current tip.
     */
    QString tip() const;

    /**
     * The next tip will become the current one.
     */
    void nextTip();

    /**
     * The previous tip will become the current one.
     */
    void prevTip();

private:
    void loadTips(const QString &tipFile);

    QStringList tips;

    int current;
    class KTipDatabasePrivate;
    KTipDatabasePrivate *d;
};

/**
 * A Tip-of-the-Day dialog.
 *
 * This dialog class presents a tip-of-the-day.
 *
 * @author Matthias Hoelzer-Kluepfel <mhk@caldera.de>
 */
class KTipDialog : public KDialog
{
    Q_OBJECT
public:
    /**
     * Construct a tip dialog.
     *
     * @param db TipDatabase that should be used by the TipDialog.
     * @param parent Parent widget of TipDialog.
     * @param name The object name.
     */
    KTipDialog(KTipDatabase *db, QWidget *parent = 0, const char *name = 0);

    /**
     * Shows a tip.
     *
     * This static method is all that is needed to add a tip-of-the-day
     * dialog to an application. It will pop up the dialog, unless the
     * user has asked that the dialog does not pop up on startup.
     *
     * Note that you probably want an item in the help menu calling
     * this method with force=true.
     *
     * @param parent Parent widget of TipDialog.
     * @param tipFile The name of the tip file. It has be relative to the "apps"
     *                resource of KStandardDirs
     * @param force If true, the dialog is show, even when the users
     *              disabled it.
     */
    static void showTip(QWidget *parent, const QString &tipFile = QString::null, bool force=false);

    /**
     * Shows a tip.
     *
     * This methods calls showTip() with the applications main window as parent.
     *
     */
    static void showTip(const QString &tipFile = QString::null, bool force = false);

    /**
     * Toggles the start behaviour.
     *
     * Normally, the user can disable the display of the tip in the dialog.
     * This is just a way to change this setting from outside.
     */
    static void setShowOnStart(bool show);

protected:
    bool eventFilter(QObject *, QEvent *);
    virtual void virtual_hook( int id, void* data );

private slots:
    void nextTip();
    void prevTip();
    void showOnStart(bool);

private:
    KTipDatabase *_database;

    QCheckBox *_tipOnStart;
    KTextBrowser *_tipText;

    class KTipDialogPrivate;
    KTipDialogPrivate *d;

    static KTipDialog *_instance;

	QColor _baseColor, _blendedColor, _textColor;
};

#endif
