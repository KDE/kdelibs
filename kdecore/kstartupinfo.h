/****************************************************************************

 $Id$

 Copyright (C) 2001 Lubos Lunak        <l.lunak@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

****************************************************************************/

#ifndef __KSTARTUPINFO_H
#define __KSTARTUPINFO_H

#include <sys/types.h>
#include <qobject.h>
#include <qcstring.h>
#include <qstring.h>

class KStartupInfoId;
class KStartupInfoData;

class KStartupInfoPrivate;

/**
 * Description coming soon ... and you most probably don't want to use this anyway.
 */
class KStartupInfo
    : public QObject
    {
    Q_OBJECT
    public:
        KStartupInfo( bool clean_on_cantdetect, QObject* parent = 0, const char* name = 0 );
        virtual ~KStartupInfo();
        static bool sendStartup( const KStartupInfoId& id, const KStartupInfoData& data );
        static bool sendStartupX( Display* dpy, const KStartupInfoId& id_P,
            const KStartupInfoData& data_P );
        static bool sendFinish( const KStartupInfoId& id );
        static bool sendFinishX( Display* dpy, const KStartupInfoId& id_P );
        static KStartupInfoId currentStartupIdEnv();
        static void resetStartupEnv();
        enum startup_t { NoMatch, Match, CantDetect };
        startup_t checkStartup( WId w );
        startup_t checkStartup( WId w, KStartupInfoId& id );
        startup_t checkStartup( WId w, KStartupInfoData& data );
        startup_t checkStartup( WId w, KStartupInfoId& id, KStartupInfoData& data );
        void setTimeout( unsigned int secs );
        class Data; // internal
    signals:
        void gotNewStartup( const KStartupInfoId& id, const KStartupInfoData& data );
        void gotStartupChange( const KStartupInfoId& id, const KStartupInfoData& data );
        void gotRemoveStartup( const KStartupInfoId& id, const KStartupInfoData& data );
    private slots:
        void startups_cleanup();
        void startups_cleanup_no_age();
        void got_message( const QString& msg );
        void window_added( WId w );
    private:
        friend class KStartupInfoPrivate;
        void got_new_startup_info( const QString& msg_P );
        void got_remove_startup_info( const QString& msg_P );
        void new_startup_info_internal( const KStartupInfoId& id_P, Data& data_P );
        void remove_startup_info_internal( const KStartupInfoId& id_P );
        void remove_startup_pids( const QValueList< pid_t >& pids, const QString& hostname );
        startup_t check_startup_internal( WId w, KStartupInfoId* id, KStartupInfoData* data,
            bool remove );
        bool find_id( const KStartupInfoId& id_P, KStartupInfoId* id_O,
            KStartupInfoData* data_O, bool remove );
        bool find_pid( pid_t pid_P, const QString& hostname, KStartupInfoId* id_O,
            KStartupInfoData* data_O, bool remove );
        bool find_wclass( const QString& res_name_P, const QString& res_class_P,
            KStartupInfoId* id_O, KStartupInfoData* data_O, bool remove );
        static QString get_window_startup_id( WId w_P );
        static QString get_window_hostname( WId w_P );
        void startups_cleanup_internal( bool age_P );
        void clean_all_noncompliant();
        bool clean_on_cantdetect;
        unsigned int timeout;
        KStartupInfoPrivate* d;
    };

class KStartupInfoIdPrivate;
// CHECKME nejak pristupova prava ?
class KStartupInfoId
    {
    public:
        bool operator==( const KStartupInfoId& id ) const;
        bool operator!=( const KStartupInfoId& id ) const;
        bool valid() const;
        void initId();
        bool setupStartupEnv() const;
        KStartupInfoId();
        KStartupInfoId( const KStartupInfoId& data );
        ~KStartupInfoId();
        KStartupInfoId& operator=( const KStartupInfoId& data );
        bool operator<( const KStartupInfoId& id ) const;
    private:
        KStartupInfoId( const QString& txt );
        QString to_text() const;
        const QString& id() const;
        friend class KStartupInfo;
        KStartupInfoIdPrivate* d;
    };

class KStartupInfoDataPrivate;
class KStartupInfoData
    {
    public:
        void setBin( const QString& bin );
        const QString& bin() const;
        void setName( const QString& name );
        const QString& findName() const;
        const QString& name() const;
        void setIcon( const QString& icon );
        const QString& findIcon() const;
        const QString& icon() const;
        void setDesktop( int desktop );
        int desktop() const;
        void setCompliant( bool compliant );
        bool compliant() const;
        void addPid( pid_t pid );
        QValueList< pid_t > pids() const;
        bool is_pid( pid_t pid ) const;
        void setHostname( const QString& hostname = QString::null ); // adds current if null
        const QString& hostname() const;
        KStartupInfoData();
        KStartupInfoData( const KStartupInfoData& data );
        ~KStartupInfoData();
        KStartupInfoData& operator=( const KStartupInfoData& data );
        void update( const KStartupInfoData& data );
    private:
        KStartupInfoData( const QString& txt );
        QString to_text() const;
        void remove_pid( pid_t pid );
        friend class KStartupInfo;
        friend class KStartupInfo::Data;
        KStartupInfoDataPrivate* d;
    };

#endif
