#include "connectionwidget.h"
#include "dblistview.h"

#include <kdb/dbengine.h>
#include <kdb/plugin.h>
#include <kdb/connection.h>
#include <kdb/database.h>

#include <kcombobox.h>
#include <klineedit.h>
#include <kcompletion.h>
#include <kmessagebox.h>
#include <kiconloader.h>

#include <qlistview.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qcheckbox.h>

#include "connectionwidget.moc"

using namespace KDB;

ConnectionWidget::ConnectionWidget(QWidget * parent, const char * name, bool showDatabase)
    : ConnectionBase(parent, name), conn(0L), databaseList(0L)
{
    pluginCombo->insertStringList(DBENGINE->pluginNames());

    // hide database list and create button
    if ( showDatabase ) {
        databaseList = new DBListView( this, "databaseList", true, DBListView::SemiAuto );
        databaseList->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7 ) );
        ConnectionBaseLayout->addMultiCellWidget( databaseList, 6, 6, 0, 2 );
        connect(databaseList, SIGNAL(databaseSelected(KDB::Database *)), this, SIGNAL(databaseSelected()));
	connect(databaseList, SIGNAL(nonDatabaseSelected()), this, SIGNAL(nonDatabaseSelected()));
    } else {
        Line2->hide();
        createButton->hide();
        saveConnection->hide();
        saveConnection->setChecked(true);
    }
    // TODO: load the completitions for host & user
    
}

ConnectionWidget::~ConnectionWidget()
{
    //TODO: save the completitions for host & user
}

//TODO: handle signals from KLineEdits

Database *
ConnectionWidget::database()
{
    return databaseList->selectedDatabase();
}

void
ConnectionWidget::setConnection(Connection *connection)
{
    conn = connection;

    pluginCombo->setCompletedText( conn->plugin() );
    m_host->setText( conn->host() );
    m_port->setText( QString( "%1" ).arg( conn->port() ) );
    m_user->setText( conn->user() );
    m_password->setText( conn->password() );
}

Connection *
ConnectionWidget::connection()
{
    return conn;
}

void 
ConnectionWidget::slotDatabaseSelected()
{
    emit databaseSelected();
}

void 
ConnectionWidget::createConnection()
{
    QString plugin = pluginCombo->currentText();
    QString host = m_host->text();
    int port = m_port->text().toInt();
    QString user = m_user->text();
    QString pwd = m_password->text();

    if (pwd.isEmpty()) {
        pwd = QString::null;
    }
    
    conn = DBENGINE->addConnection(plugin, host, port, user);

    
    if (!conn) {
        Exception *e = DBENGINE->popError();
        QString msg = "Cannot create connection\nReason:";
        msg += e->description();
        KMessageBox::sorry(this,msg);
        delete e;
    } else {
        conn->setPassword(pwd,remember->isChecked());
        conn->setSaveStatus(saveConnection->isChecked());
        //load the list if it is available
        if ( databaseList ) {
            conn->open();
            databaseList->clear();
            databaseList->addConnection(conn);
        }
        createButton->setEnabled(false);
    } 
}


void
ConnectionWidget::slotCreateConnection()
{
    createConnection();
}
