/***************************************************************************
 * form.h
 * This file is part of the KDE project
 * copyright (C)2006-2007 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KROSS_FORM_H
#define KROSS_FORM_H

#include <QtGui/QWidget>
#include <QtCore/QUrl>
#include <QtGui/QListWidget>

#include <kpagedialog.h>
#include <kassistantdialog.h>
//#include <kfilewidget.h>

namespace Kross {

    /**
     * The FormListView class provides access to a ListView.
     */
    class FormListView : public QListWidget
    {
            Q_OBJECT
        public:
            explicit FormListView(QWidget* parent);
            virtual ~FormListView();
        public Q_SLOTS:
            void clear();
            void remove(int index);
            void addItem(const QString& text);
            int count();
            int current();
            void setCurrent(int row);
            QString text(int row);
    };

    /**
     * The FormFileWidget class provides access to a KFileWidget.
     */
    class FormFileWidget : public QWidget
    {
            Q_OBJECT
            Q_ENUMS(Mode)

        public:
            FormFileWidget(QWidget* parent, const QString& startDirOrVariable);
            virtual ~FormFileWidget();

            /**
             * The Mode the FormFileWidget could have.
             */
            enum Mode { Other = 0, Opening, Saving };

        public Q_SLOTS:

            /**
             * Set the \a Mode the FormFileWidget should have to \p mode .
             * Valid modes are "Other", "Opening" or "Saving".
             */
            void setMode(const QString& mode);

            /**
             * \return the current filter.
             */
            QString currentFilter() const;

            /**
             * Set the filter to \p filter .
             */
            void setFilter(const QString &filter);

            /**
             * \return the current mimetype filter.
             */
            QString currentMimeFilter() const;

            /**
             * Set the mimetype filter to \p filter .
             */
            void setMimeFilter(const QStringList& filter);

            /**
             * \return the currently selected file.
             */
            QString selectedFile() const;

            //QStringList selectedFiles() const { return KFileDialog::selectedFiles(); }
            //QString selectedUrl() const { return KFileDialog::selectedUrl().toLocalFile(); }

        Q_SIGNALS:

            /**
            * Emitted when the user selects a file. It is only emitted in single-
            * selection mode.
            */
            void fileSelected(const QString& file);

            /**
            * Emitted when the user highlights a file.
            */
            void fileHighlighted(const QString&);

            /**
            * Emitted when the user hilights one or more files in multiselection mode.
            */
            void selectionChanged();

            /**
            * Emitted when the filter changed, i.e. the user entered an own filter
            * or chose one of the predefined set via setFilter().
            */
            void filterChanged(const QString& filter);

        private Q_SLOTS:
            void slotFileSelected(const QString&);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

    /**
     * The FormProgressDialog class provides access to progressbar.
     *
     * Example (in Python) :
     * \code
     * import time, Kross
     * forms = Kross.module("forms")
     * progress = forms.showProgressDialog("My Title")
     * progress.setText("Some <i>detailed</i> text.")
     * for i in range(0,101):
     *     progress.setValue(i)
     *     progress.addText("%s" % i)
     *     time.sleep(1)
     * progress.reset()
     * \endcode
     */
    class FormProgressDialog : public KPageDialog
    {
            Q_OBJECT
        public:
            FormProgressDialog(const QString& caption, const QString& labelText);
            virtual ~FormProgressDialog();
            virtual void done(int r);
        public Q_SLOTS:
            /**
            * Set the value of the progressbar. If -1 the progressbar will be hidden.
            */
            void setValue(int progress);
            /**
            * Set the minimum and maximum range of the progressbar.
            */
            void setRange(int minimum, int maximum);
            /**
            * Set the HTML-text that is displayed as information to the text \p text .
            */
            void setText(const QString& text);
            /**
            * Add to the HTML-text that is displayed as information the text \p text .
            */
            void addText(const QString& text);
            /**
             * Shows the dialog as a modal dialog, blocking until the user
             * closes it and returns the execution result.
             *
             * \return >=1 if the dialog was accepted (e.g. "Ok" pressed) else
             * the user rejected the dialog (e.g. by pressing "Cancel" or just
             * closing the dialog by pressing the escape-key).
             */
            int exec();
            /**
             * Same as the \a exec() method above provided for Python-lovers (python
             * does not like functions named "exec" and PyQt named it "exec_loop", so
             * just let's do the same).
             */
            int exec_loop() { return exec(); }
            /**
             * Returns true if the user requested to cancel the operation.
             */
            bool isCanceled();
        Q_SIGNALS:
            /**
             * This signal got emitted if the user requests to cancel the operation.
             */
            void canceled();
        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

    /**
     * The FormDialog class provides access to KDialog objects as
     * top-level containers.
     *
     * Example (in Python) :
     * \code
     * import Kross
     * import sys,os
     * ourPath=(filter(lambda p: os.path.exists(p+'/mywidget.ui'),sys.path)+[''])[0]
     * forms = Kross.module("forms")
     * mydialog = forms.createDialog("MyDialog")
     * mydialog.setButtons("Ok|Cancel")
     * mydialog.setFaceType("Plain") #Auto Plain List Tree Tabbed
     * mypage = mydialog.addPage("name","header")
     * mywidget = forms.createWidgetFromUIFile(mypage, ourPath+'/mywidget.ui')
     * mywidget["lineEdit"].setText("some string")
     * if mydialog.exec_loop():
     *     if mydialog.result() == "Ok":
     *         print mywidget["lineEdit"].text
     * mydialog.deleteLater()
     * \endcode
     */
    class FormDialog: public KPageDialog
    {
            Q_OBJECT

        public:
            FormDialog(const QString& caption);
            virtual ~FormDialog();

        public Q_SLOTS:

            /**
             * Set the buttons.
             *
             * \param buttons string that defines the displayed buttons. For example the
             * string may look like "Ok" or "Ok|Cancel" or "Yes|No|Cancel".
             * \return true if the passed \p buttons string was valid and setting the
             * buttons was successfully else false is returned.
             */
            bool setButtons(const QString& buttons);

            /**
             * Set the text of a button.
             *
             * Sample how to change the buttons of a dialog;
             * \code
             * dialog.setButtons("Yes|No|Cancel")
             * dialog.setButtonText("Yes","Overwrite")
             * dialog.setButtonText("No","Skip")
             * dialog.setButtonText("Cancel","Abort")
             * \endcode
             *
             * \param button string that defines the button that should be changed.
             * \param text string that should be used as button text.
             * \return true if the passed \p buttons string was valid and setting the
             * button text was successfully else false is returned.
             */
            bool setButtonText(const QString& button, const QString& text);

            /**
             * Set the face type of the dialog.
             *
             * \param facetype the face type which could be "Auto", "Plain", "List",
             * "Tree" or "Tabbed" as defined in \a KPageView::FaceType .
             */
            bool setFaceType(const QString& facetype);

            /**
             * \return the name of the currently selected page. Use the \a page()
             * method to get the matching page QWidget instance.
             */
            QString currentPage() const;

            /**
             * Set the current page to \p name . If there exists no page with
             * such a pagename the method returns false else (if the page was
             * successfully set) true is returned.
             */
            bool setCurrentPage(const QString& name);

            /**
             * \return the QWidget page instance which has the pagename \p name
             * or NULL if there exists no such page.
             */
            QWidget* page(const QString& name) const;

            /**
             * Add and return a new page.
             *
             * \param name The name the page has. This name is for example returned
             * at the \a currentPage() method and should be unique. The name is also
             * used to display a short title for the page.
             * \param header The longer header title text used for display purposes.
             * \param iconname The name of the icon which the page have. This could
             * be for example "about_kde", "document-open", "configure" or any other
             * iconname known by KDE.
             * \return the new QWidget page instance.
             */
            QWidget* addPage(const QString& name, const QString& header = QString(), const QString& iconname = QString());

            /**
             * Set the \p newMainWidget QWidget as main widget. This is only needed if
             * you like to replace the KPageDialog page-widget with your own widget.
             */
            void setMainWidget(QWidget *newMainWidget);

            /**
             * Shows the dialog as a modal dialog, blocking until the user
             * closes it and returns the execution result.
             *
             * \return >=1 if the dialog was accepted (e.g. "Ok" pressed) else
             * the user rejected the dialog (e.g. by pressing "Cancel" or just
             * closing the dialog by pressing the escape-key).
             */
            int exec() { return KDialog::exec(); }

            /**
             * Same as the \a exec() method above provided for Python-lovers (python
             * does not like functions named "exec" and PyQt named it "exec_loop", so
             * just let's do the same).
             */
            int exec_loop() { return exec(); }

            /**
             * \return the result. The result may for example "Ok", "Cancel", "Yes" or "No".
             */
            QString result();

        private Q_SLOTS:
            virtual void slotButtonClicked(int button);
            void slotCurrentPageChanged(KPageWidgetItem* current);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };


    /**
     * The FormAssistant class provides access to KAssistantDialog objects as
     * top-level containers.
     *
     * Example (in Python) :
     * \code
     * import Kross
     * import sys,os
     * ourPath=(filter(lambda p: os.path.exists(p+'/mywidget.ui'),sys.path)+[''])[0]
     * forms = Kross.module("forms")
     * myassistant = forms.createAssistant("MyAssistant")
     * myassistant.showHelpButton(0)
     * mypage = myassistant.addPage("name","header")
     * mywidget = forms.createWidgetFromUIFile(mypage, ourPath+'/mywidget.ui')
     * mypage2 = myassistant.addPage("name2","header2")
     * mywidget2 = forms.createWidgetFromUIFile(mypage2, ourPath+'/mywidget.ui')
     * mypage3 = myassistant.addPage("name3","header3")
     * mywidget3 = forms.createWidgetFromUIFile(mypage3, ourPath+'/mywidget.ui')
     * mywidget["lineEdit"].setText("some string")
     *
     * def nextClicked():
     *     myassistant.setAppropriate("name2",0)
     * def finished():
     *     ...
     *     myassistant.deleteLater() #remember to cleanup
     *
     * myassistant.connect("nextClicked()",nextClicked)
     * myassistant.connect("finished()",finished)
     * myassistant.show()
     *
     * \endcode
     */
    class FormAssistant: public KAssistantDialog
    {
            Q_OBJECT
            Q_ENUMS(AssistantButtonCode)
        public:
            enum AssistantButtonCode
            {
                None    = 0x00000000,
                Help    = 0x00000001,
                Default = 0x00000002,
                Cancel  = 0x00000020,
                Finish  = 0x00001000,
                Next    = 0x00002000,
                Back    = 0x00004000,
                NoDefault = 0x00008000
            };
            Q_DECLARE_FLAGS(AssistantButtonCodes, AssistantButtonCode)

        public:
            FormAssistant(const QString& caption);
            virtual ~FormAssistant();

        public Q_SLOTS:

            void showHelpButton(bool);

            /**
             * \return the name of the currently selected page. Use the \a page()
             * method to get the matching page QWidget instance.
             */
            QString currentPage() const;

            /**
             * Set the current page to \p name . If there exists no page with
             * such a pagename the method returns false else (if the page was
             * successfully set) true is returned.
             */
            bool setCurrentPage(const QString& name);

            /**
             * \return the QWidget page instance which has the pagename \p name
             * or NULL if there exists no such page.
             */
            QWidget* page(const QString& name) const;

            /**
             * Add and return a new page.
             *
             * \param name The name the page has. This name is for example returned
             * at the \a currentPage() method and should be unique. The name is also
             * used to display a short title for the page.
             * \param header The longer header title text used for display purposes.
             * \param iconname The name of the icon which the page have. This could
             * be for example "about_kde", "document-open", "configure" or any other
             * iconname known by KDE.
             * \return the new QWidget page instance.
             */
            QWidget* addPage(const QString& name, const QString& header = QString(), const QString& iconname = QString());

            /**
             * @see KAssistantDialog::isAppropriate()
             */
            bool isAppropriate (const QString& name) const;
            /**
             * @see KAssistantDialog::setAppropriate()
             */
            void setAppropriate (const QString& name, bool appropriate);
            /**
             * @see KAssistantDialog::isValid()
             */
            bool isValid (const QString& name) const;
            /**
             * @see KAssistantDialog::setValid()
             */
            void setValid (const QString& name, bool enable);

            /**
             * Shows the dialog as a modal dialog, blocking until the user
             * closes it and returns the execution result.
             *
             * \return >=1 if the dialog was accepted (e.g. "Finished" pressed) else
             * the user rejected the dialog (e.g. by pressing "Cancel" or just
             * closing the dialog by pressing the escape-key).
             */
            int exec() { return KDialog::exec(); }

            /**
             * Same as the \a exec() method above provided for Python-lovers (python
             * does not like functions named "exec" and PyQt named it "exec_loop", so
             * just let's do the same).
             */
            int exec_loop() { return exec(); }

            /**
             * \return the result. The result may for example "Finish" or "Cancel".
             */
            QString result();

            /**
             * Force page switching. This will also emit backClicked()
             */
            void back();
            /**
             * Force page switching. This will also emit nextClicked()
             */
            void next();

        private Q_SLOTS:
            virtual void slotButtonClicked(int button);
            void slotCurrentPageChanged(KPageWidgetItem* current);

        signals:
            /**
             * use it to setAppropriate()
             */
            void nextClicked();
            void backClicked();

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };


    /**
     * The FormModule provides access to UI functionality like dialogs or widgets.
     *
     * Example (in Python) :
     * \code
     * import Kross
     * forms = Kross.module("forms")
     * dialog = forms.createDialog("My Dialog")
     * dialog.setButtons("Ok|Cancel")
     * page = dialog.addPage("Welcome","Welcome Page","document-open")
     * label = forms.createWidget(page,"QLabel")
     * label.text = "Hello World Label"
     * if dialog.exec_loop():
     *     forms.showMessageBox("Information", "Okay...", "The Ok-button was pressed")
     * \endcode
     */
    class FormModule: public QObject
    {
            Q_OBJECT

        public:
            explicit FormModule();
            virtual ~FormModule();

        public Q_SLOTS:

            /**
             * \return the active modal widget. Modal widgets are special top-level
             * widgets which are subclasses of QDialog and are modal.
             */
            QWidget* activeModalWidget();

            /**
             * \return the application top-level window that has the keyboard input
             * focus, or NULL if no application window has the focus.
             */
            QWidget* activeWindow();


            /**
             * \return i18n'ed version of the string
             */
            QString tr(const QString& str);

            /**
             * \return i18n'ed version of the string, differentiated by the comment text (like '\@title:window')
             */
            QString tr(const QString& str,const QString& comment);


            /**
             * Show a messagebox.
             *
             * \param dialogtype The type of the dialog which could be one
             * of the following;
             *      \li Sorry
             *      \li Error
             *      \li Information
             *      \li QuestionYesNo
             *      \li WarningYesNo
             *      \li WarningContinueCancel
             *      \li WarningYesNoCancel
             *      \li QuestionYesNoCancel
             * \param caption The caption the messagedialog displays.
             * \param message The message that is displayed in the messagedialog.
             * \param details The optional details
             * \return The buttoncode which chould be one of the following;
             *      \li Ok
             *      \li Cancel
             *      \li Yes
             *      \li No
             *      \li Continue
             */
            QString showMessageBox(const QString& dialogtype, const QString& caption, const QString& message, const QString& details = QString());

            /**
             * Show a progressdialog to provide visible feedback on the progress
             * of a slow operation.
             *
             * \param caption The caption the progressdialog displays.
             * \param labelText The displayed label.
             * \return The QProgressDialog widget instance.
             */
            QWidget* showProgressDialog(const QString& caption, const QString& labelText);

            /**
             * Create and return a new \a FormDialog instance.
             *
             * \param caption The displayed caption of the dialog.
             */
            QWidget* createDialog(const QString& caption);

            /**
             * Create and return a new \a FormAssistant instance.
             *
             * \param caption The displayed caption of the dialog.
             */
            QWidget* createAssistant(const QString& caption);

            /**
             * Create and return a new QWidget instance.
             *
             * \param parent the parent QWidget the new QWidget is a child of.
             * \param layout the layout style the widget has. This could be one
             * of the following strings;
             *        \li QVBoxLayout
             *        \li QHBoxLayout
             *        \li QStackedLayout
             * \return the new QLayout instance or NULL.
             */
            QObject* createLayout(QWidget* parent, const QString& layout);

            /**
             * Create and return a new QWidget instance.
             *
             * \param parent the parent QWidget the new QWidget is a child of.
             * \param className the name of the class that should be created
             * and returned. For example "QLabel" or "QForm".
             * \param name the objectName the new widget has.
             * \return the new QWidget instance or NULL.
             */
            QWidget* createWidget(const QString& className);

            /**
             * Create and return a new QWidget instance.
             *
             * \param parent the parent QWidget the new QWidget is a child of.
             * \param className the name of the class that should be created
             * and returned. For example "QLabel" or "QForm".
             * \param name the objectName the new widget has.
             * \return the new QWidget instance or NULL.
             */
            QWidget* createWidget(QWidget* parent, const QString& className, const QString& name = QString());

            /**
             * Create and return a new QWidget instance.
             *
             * \param parent the new QWidget is a child of parent.
             * \param xml the UI XML string used to construct the new widget.
             * \return the new QWidget instance or NULL.
             */
            QWidget* createWidgetFromUI(QWidget* parent, const QString& xml);

            /**
             * Create and return a new QWidget instance.
             *
             * \param parent the parent QWidget the new QWidget is a child of.
             * \param filename the full filename of the UI file which is readed
             * and its UI XML content is used to construct the new widget.
             */
            QWidget* createWidgetFromUIFile(QWidget* parent, const QString& filename);

            /**
             * Create and return a new \a FormFileWidget instance.
             *
             * \param parent the parent QWidget the new \a FormFileWidget instance
             * is a child of.
             * \param startDirOrVariable the start-directory or -variable.
             * \return the new \a FormFileWidget instance or NULL.
             */
            QWidget* createFileWidget(QWidget* parent, const QString& startDirOrVariable = QString());

            /**
             * Create and return a new \a FormListView instance.
             *
             * \param parent the parent QWidget the new \a FormListView instance
             * is a child of.
             * \return the new \a FormFileWidget instance or NULL.
             */
            QWidget* createListView(QWidget* parent);

            /**
             * Load and return a KPart component.
             * \param parent The parent QWidget the KPart's widget will be child of.
             * \param name The name of the KPart library like e.g. "libkhtmlpart".
             * \param url Optional Url that should be opened on load.
             */
            QObject* loadPart(QWidget* parent, const QString& name, const QUrl& url = QUrl());

            /**
             * Create and return a new \a QAction instance.
             *
             * \param parent the parent QObject the new \a QAction instance
             * is a child of.
             * \return the new \a QAction instance or NULL.
             */
            QAction* createAction(QObject* parent);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };
}

#endif

