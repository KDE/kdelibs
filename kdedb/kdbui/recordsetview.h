#ifndef __RECORDSETVIEW_H__
#define __RECORDSETVIEW_H__

#include <qtable.h>

#include <kdb/recordset.h>

namespace KDB
{

    class RecordsetView : public QTable
    {
        Q_OBJECT

    public:
        RecordsetView( QWidget *parent = 0L, const char *name = 0L );
        RecordsetView( Recordset* recordset, QWidget *parent = 0L, const char *name = 0L );
        ~RecordsetView( );

        void setEditable( bool editable = false );
        bool editable();

        void setRecordset( Recordset *recordset );
        Recordset * recordset();

        void addRecord( Record *record );

    private:
        bool         m_editable;
        RecordsetPtr m_data;
    };

};

#endif
