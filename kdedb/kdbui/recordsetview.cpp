#include <klocale.h>

#include <kdb/recordset.h>

#include <kdebug.h>

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
    setRecordset(recordset);
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
    unsigned long row = 0;
    int col = 0;

    m_data = recordset;

    if (!m_data) {
        clear();
        return;
    }
        
    // recreate headers
    FieldList lst = m_data->fields();
    
    setNumCols(lst.count());
    setNumRows(m_data->count());
    
    FieldIterator it(lst);
    while (it.current()) {
        horizontalHeader()->setLabel(col, it.current()->name() );
        ++col;
        ++it;
    }

    for( row = 1 ; row <= m_data->count() ; row++ ) {
        verticalHeader()->setLabel( row - 1,  QString( "%1" ).arg( row ) );
    }
       
    // load data
    /*
      KDB::RecordsetIterator recs = m_data->begin();
      
      row = 0; 
      while (recs.current()) {
      col = 0;
      KDB::RecordPtr record = recs.current();
      KDB::FieldIterator fields = record->begin();
      while (fields.current()) {
      //kdDebug(20001) << "setText(" << row << "," << col << ","
      //               << fields.current()->value().toString() << ")" << endl;
      setText(row,col,fields.current()->value().toString());
      ++fields;
      col++;
      }
      row++;
      ++recs;
      }
    */
}

Recordset * RecordsetView::recordset()
{
    return m_data;
}

void RecordsetView::addRecord( Record * )
{
    //m_data->addRecord( record );
}

void
RecordsetView::clear()
{
    m_data = 0L;
    setNumCols(0);
    setNumRows(0);
}


void
RecordsetView::paintCell(QPainter *p, int row, int col, const QRect &cr, bool selected)
{
    // first, clear the rect, but leave the 1x1 border around (faster and flicker free)
    p->eraseRect(1, 1,
                 columnWidth(col), rowHeight(row));

    if (!m_data) {
        return;
    }
    
    KDB::RecordsetIterator it = m_data->begin();
    it.moveTo(row);
    if (!it.current())
        return; // drawing past the end of the recordset, just clear and exit
    
    p->moveTo(0,rowHeight(row));
    p->lineTo(columnWidth(col), rowHeight(row));
    p->lineTo(columnWidth(col), 0);
    // draw the text leaving the 1x1 margin
    p->drawText(1, 1,
                columnWidth(col) - 1, rowHeight(row) - 1,
                Qt::AlignVCenter | Qt::AlignLeft, 
                it.current()->field(col)->toString());

    //TODO: handle selection
    
}
