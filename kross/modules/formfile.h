/***************************************************************************
 * formfile.h
 * This file is part of the KDE project
 * copyright (C)2007 by Sebastian Sauer (mail@dipe.org)
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

#ifndef KROSS_FORMFILE_H
#define KROSS_FORMFILE_H

#include <QString>
#include <QWidget>

//#include <kpagedialog.h>
#include <kfiledialog.h>

namespace Kross {

    /**
     * The FormFileWidget class provides a in a widget embedded KFileDialog.
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
            void setFilter(QString filter);

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

        private:
            virtual void showEvent(QShowEvent* event);
            virtual void hideEvent(QHideEvent* event);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

}

#endif

