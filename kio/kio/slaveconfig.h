// -*- c++ -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Waldo Bastian <bastian@kde.org>
 *
 * $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef KIO_SLAVE_CONFIG_H
#define KIO_SLAVE_CONFIG_H

#include <qobject.h>
#include <kio/global.h>

namespace KIO {

    class SlaveConfigPrivate;
    /**
     * SlaveConfig
     *
     * This class manages the configuration for io-slaves based on protocol
     * and host. The Scheduler makes use of this class to configure the slave
     * whenever it has to connect to a new host.
     *
     * You only need to use this class if you want to override specific
     * configuration items of an io-slave when the io-slave is used by
     * your application. 
     *
     * Normally io-slaves are being configured by "kio_<protocol>rc" 
     * configuration files. Groups defined in such files are treated as host 
     * or domain specification. Configuration items defined in a group are 
     * only applied when the slave is connecting with a host that matches with 
     * the host and/or domain specified by the group.
     */
    class SlaveConfig : public QObject
    {
	Q_OBJECT
    public:
        static SlaveConfig *self();
        ~SlaveConfig();
        /**
         * Configure slaves of type @p protocol by setting @p key to @p value.
         * If @p host is specified the configuration only applies when dealing
         * with @p host.
         *
         * Changes made to the slave configuration only apply to slaves
         * used by the current process.
         */
        void setConfigData(const QString &protocol, const QString &host, const QString &key, const QString &value );
        
        /**
         * Configure slaves of type @p protocol with @p config.
         * If @p host is specified the configuration only applies when dealing
         * with @p host.
         *
         * Changes made to the slave configuration only apply to slaves
         * used by the current process.
         */
        void setConfigData(const QString &protocol, const QString &host, const MetaData &config );
                
        /**
         * Query slave configuration for slaves of type @p protocol when
         * dealing with @p host.
         */
        MetaData configData(const QString &protocol, const QString &host);

        /**
         * Query a specific configuration key for slaves of type @p protocol when
         * dealing with @p host.
         */
        QString configData(const QString &protocol, const QString &host, const QString &key);

        /**
         * Undo any changes made by calls to @ref setConfigData.
         */
        void reset();
    signals:
        /**
         * This signal is raised when a slave of type @p protocol deals
         * with @p host for the first time.
         *
         * Your application can use this signal to make some last minute
         * configuration changes with @ref setConfigData based on the
         * host.
         */
        void configNeeded(const QString &protocol, const QString &host);
    protected:
        SlaveConfig();
        static SlaveConfig *_self;
        SlaveConfigPrivate *d;
    };
}

#endif
