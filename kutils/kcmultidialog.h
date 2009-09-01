/*
   Copyright (c) 2000 Matthias Elter <elter@kde.org>
   Copyright (c) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (c) 2003,2006 Matthias Kretz <kretz@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/

#ifndef KCMULTIDIALOG_H
#define KCMULTIDIALOG_H

#include <kcmoduleinfo.h>
#include <kpagedialog.h>

class KCMultiDialogPrivate;

/**
 * @short A class that offers a KPageDialog containing arbitrary
 *        KControl Modules.
 *
 * @author Matthias Elter <elter@kde.org>, Daniel Molkentin <molkentin@kde.org>
 */
class KUTILS_EXPORT KCMultiDialog : public KPageDialog
{
  Q_OBJECT
    Q_DECLARE_PRIVATE(KCMultiDialog)

  public:
    /**
     * Constructs a new KCMultiDialog
     *
     * @param parent The parent widget
     **/
    KCMultiDialog( QWidget *parent = 0 );


    /**
     * Destructor
     **/
   virtual ~KCMultiDialog();

    /**
     * Add a module.
     *
     * The module is added according to its KCModuleInfo::weight(). The weight determines where in the list
     * the module will appear. Lighter modules on top, heavier modules at the bottom.
     *
     * @param module Specify the name of the module that is to be added
     *               to the list of modules the dialog will show.
     *
     * @param args The arguments that should be given to the KCModule when it is created
     *
     * @returns The @see KPageWidgetItem associated with the new dialog page.
     **/
    KPageWidgetItem* addModule( const QString& module, const QStringList& 
                                args = QStringList() );

    /**
     * Add a module.
     *
     * The module is added according to its KCModuleInfo::weight(). The weight determines where in the list
     * the module will appear. Lighter modules on top, heavier modules at the bottom.
     *
     * @param moduleinfo Pass a KCModuleInfo object which will be
     *                   used for creating the module. It will be added
     *                   to the list of modules the dialog will show.
     *
     * @param parent The @see KPageWidgetItem that should appear as parents
     *               in the tree view or a 0 pointer if there is no parent.
     *
     * @param args The arguments that should be given to the KCModule when it is created
     **/
    KPageWidgetItem* addModule( const KCModuleInfo& moduleinfo, KPageWidgetItem *parent = 0, 
                                const QStringList& args = QStringList() );

    /**
     * Removes all modules from the dialog.
     */
    void clear();

  Q_SIGNALS:
    /**
     * Emitted after all KCModules have been told to save their configuration.
     *
     * The applyClicked and okClicked signals are emitted before the
     * configuration is saved.
     */
    void configCommitted();

    /**
     * Emitted after the KCModules have been told to save their configuration.
     * It is emitted once for every instance the KCMs that were changed belong
     * to.
     *
     * You can make use of this if you have more than one component in your
     * application. componentName tells you the instance that has to reload its
     * configuration.
     *
     * The applyClicked and okClicked signals are emitted before the
     * configuration is saved.
     *
     * @param componentName The name of the instance that needs to reload its
     *                     configuration.
     */
    void configCommitted( const QByteArray & componentName );

    protected:
        /**
         * This constructor can be used by subclasses to provide a custom KPageWidget.
         */
        KCMultiDialog(KPageWidget *pageWidget, QWidget *parent, Qt::WFlags flags = 0);
        KCMultiDialog(KCMultiDialogPrivate &dd, KPageWidget *pageWidget, QWidget *parent, Qt::WFlags flags = 0);

  protected Q_SLOTS:
    /**
     * This slot is called when the user presses the "Default" Button.
     * You can reimplement it if needed.
     *
     * @note Make sure you call the original implementation.
     **/
    void slotDefaultClicked();

    /**
     * This slot is called when the user presses the "Reset" Button.
     * You can reimplement it if needed.
     *
     * @note Make sure you call the original implementation.
     */
    void slotUser1Clicked();

    /**
     * This slot is called when the user presses the "Apply" Button.
     * You can reimplement it if needed.
     *
     * @note Make sure you call the original implementation.
     **/
    void slotApplyClicked();

    /**
     * This slot is called when the user presses the "OK" Button.
     * You can reimplement it if needed.
     *
     * @note Make sure you call the original implementation.
     **/
    void slotOkClicked();

    /**
     * This slot is called when the user presses the "Help" Button.
     * It reads the X-DocPath field of the currently selected KControl
     * module's .desktop file to find the path to the documentation,
     * which it then attempts to load.
     *
     * You can reimplement this slot if needed.
     *
     * @note Make sure you call the original implementation.
     **/
    void slotHelpClicked();

    private:
        Q_PRIVATE_SLOT(d_func(), void _k_slotCurrentPageChanged(KPageWidgetItem *))
        Q_PRIVATE_SLOT(d_func(), void _k_clientChanged())
        Q_PRIVATE_SLOT(d_func(), void _k_dialogClosed())
        Q_PRIVATE_SLOT(d_func(), void _k_updateHeader(bool use, const QString &message))
};

#endif
