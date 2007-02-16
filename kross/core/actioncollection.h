/***************************************************************************
 * actioncollection.h
 * This file is part of the KDE project
 * copyright (C)2004-2006 by Sebastian Sauer (mail@dipe.org)
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

#ifndef KROSS_ACTIONCOLLECTION_H
#define KROSS_ACTIONCOLLECTION_H

#include "krossconfig.h"
#include "action.h"

#include <QString>
#include <QObject>
#include <QDir>

class QDomElement;
class QIODevice;

namespace Kross {

    /**
     * The ActionCollection class manages collections of \a Action
     * instances.
     */
    class KROSSCORE_EXPORT ActionCollection : public QObject
    {
            Q_OBJECT

        public:

           /**
             * Constructor.
             *
             * \param name The objectName the ActionCollection has.
             * \param parent The parent ActionCollection this
             * ActionCollection will be child of. If parent is not
             * NULL, this \a ActionCollection instance will register
             * itself as child of the parent \p parent by using the
             * \a registerCollection method.
             */
            explicit ActionCollection(const QString& name, ActionCollection* parent = 0);

            /**
             * Destructor.
             */
            virtual ~ActionCollection();

            /**
             * \return the objectName for this ActionCollection.
             */
            QString name() const;

            /**
             * \return the display text
             */
            QString text() const;

            /**
             * Set the display text to \p text .
             */
            void setText(const QString& text);

            /**
             * \return the optional description for this ActionCollection.
             */
            QString description() const;

            /**
             * Set the optional description for this ActionCollection.
             */
            void setDescription(const QString& description);

            /**
             * \return the name of the icon.
             */
            QString iconName() const;

            /**
             * Set the name of the icon to \p iconname .
             */
            void setIconName(const QString& iconname);

            /**
             * \return the icon defined with \p setIconName() .
             */
            QIcon icon() const;

            /**
             * Return the enable this ActionCollection has.
             */
            bool isEnabled() const;

            /**
             * Enable or disable this ActionCollection.
             */
            void setEnabled(bool enabled);

            /**
             * \return the parent \a ActionCollection instance this
             * \collection is child of or NULL oif this collection
             * does not have a parent.
             */
            ActionCollection* parentCollection() const;

            /**
             * \return true if this collection has a child \a ActionCollection
             * instance which objectName is \p name .
             */
            bool hasCollection(const QString& name) const;

            /**
             * \return the \a ActionCollection instance which objectName is
             * \p name or NULL if there exists no such \a ActionCollection .
             */
            ActionCollection* collection(const QString& name) const;

            /**
             * \return a list of names of child \a ActionCollection instances
             * this collection has
             */
            QStringList collections() const;

            QList<Action*> actions() const;
            Action* action(const QString& name) const;
            void addAction(const QString& name, Action* action);
            void removeAction(const QString& name);
            void removeAction(Action* action);

            /**
             * Load child \a Action and \a ActionCollection instances this
             * collection has from the \p element .
             *
             * \param element The QDomElement that contains the XML.
             * \param directory The current directory used for relative paths
             * defined within a script-tag for the file-attribute. If the
             * directory is QDir() relative paths are not resolved.
             * \return true on success else false.
             */
            bool readXml(const QDomElement& element, const QDir& directory = QDir());

            /**
             * Read XML from the QIODevice \p device .
             */
            bool readXml(QIODevice* device, const QDir& directory = QDir());

            /**
             * Read the XML from the file \p file .
             *
             * \param file The existing XML file that should be readed.
             * \return true if reading was successful else false.
             */
            bool readXmlFile(const QString& file);

            /**
             * Read the XML from the resource-type defined with \p resource
             * and use the filter \p filter .
             *
             * For example the \a GUIClient class uses this method to
             * restore the main \a Manager::actionCollection() collection.
             *
             * \param resource The type of resource to locate directories for.
             * \param filter Only accept filenames that fit to filter. The
             * filter may consist of an optional directory and a QRegExp
             * wildcard expression. E.g. "images\*.jpg". Use QString() if you
             * do not want a filter.
             * \return true if reading was complete successful (as in there
             * where no errors) else false is returned.
             */
            bool readXmlResource(const QByteArray& resource = "data", const QString& filer = QString());

            /**
             * \return a QDomElement that represents the child \a Action
             * and \a ActionCollection instances this collection has.
             */
            QDomElement writeXml();

            /**
             * Write XML to the QIODevice \p device and use a space-idention
             * of \p indent for the XML.
             */
            bool writeXml(QIODevice* device, int indent = 2);

        Q_SIGNALS:

            /**
             * This signal is emitted if the content of the ActionCollection
             * was changed.
             */
            void updated();

            //void inserted(ActionCollection* self, ActionCollection* into);
            //void updated(ActionCollection* self);
            //void removed(ActionCollection* self, ActionCollection* from);

        protected:
            void registerCollection(ActionCollection* collection);
            void unregisterCollection(const QString& name);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

}

#endif

