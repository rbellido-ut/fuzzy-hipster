#include "fuzzymainwindow.h"
#include "ui_fuzzymainwindow.h"


FuzzyMainWindow::FuzzyMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FuzzyMainWindow)
{
    // setup GUI
    ui->setupUi(this);
    cDlg = new ClientSettingsDialog();

    this->statusBar()->showMessage("Idle");

    // slot-signal connections
    QObject::connect(&c, SIGNAL(statusChanged(QString)), this, SLOT(setStatus(const QString &)));
    QObject::connect(cDlg, SIGNAL(clientIgnite(QString,QString)), this, SLOT(startClient(const QString &, const QString &)));
}

FuzzyMainWindow::~FuzzyMainWindow()
{
    delete ui;
    delete cDlg;
}

void FuzzyMainWindow::on_actionE_xit_triggered()
{
    QApplication::exit(0);
}

void FuzzyMainWindow::on_action_Client_triggered()
{
    cDlg->show();
}


/*------------------------------------------------------------------------------------------------------------------
 -- FUNCTION: setStatus
 --
 -- DATE: March 19, 2013
 --
 -- REVISIONS: March 19, 2013 -- Initial version.
 --
 -- DESIGNER: Aaron Lee
 --
 -- PROGRAMMER: Aaron Lee
 --
 -- INTERFACE: void MainWindow::setStatus()
 --            QString s -- a message to be displayed
 --
 -- RETURNS: void
 --
 -- NOTES:
 -- Slot. Connected to the statusChanged() signal. Displays
 -- notification in status bar at the bottom of the window.
----------------------------------------------------------------------------------------------------------------------*/
void FuzzyMainWindow::setStatus(const QString& s)
{
    this->statusBar()->showMessage(s);
}

// slot function to start the TCP client engine
void FuzzyMainWindow::startClient(const QString& hostname, const QString& port)
{
    WSADATA wsadata;

    c.createTCPClient(&wsadata, hostname.toUtf8().constData(), port.toInt());
    c.startTCPClient();
}
