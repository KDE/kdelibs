#include <klocale.h>

#include <kdb/recordset.h>

#include "recordsetview.h"

#include "recordsetview.moc"

using namespace KDB;

RecordsetView::RecordsetView( QWidget *parent, const char *name )
    : QTable( parent, name )
    , m_editable( false )
{
}

RecordsetView::RecordsetView( Recordset *recordset, QWidget *parent, const char *name )
    : QTable( parent, name )
    , m_editable( false )
{
    horizontalHeader()->addLabel( i18n( "Column 1" ) );

    for( unsigned long index = 1 ; index <= recordset->count() ; index++ )
        verticalHeader()->addLabel( QString( "%1" ).arg( index ) );
}

RecordsetView::~RecordsetView()
{
}

void RecordsetView::setEditable( bool editable )
{
    m_editable = editable;
}

bool RecordsetView::editable()
{
    return m_editable;
}

void RecordsetView::setRecordset( Recordset *recordset )
{
    m_data = recordset;
}

Recordset * RecordsetView::recordset()
{
    return m_data;
}

void RecordsetView::addRecord( Record *record )
{
    //m_data->addRecord( record );
}
