#include "fuzzymainwindow.h"
#include "ui_fuzzymainwindow.h"

FuzzyMainWindow::FuzzyMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FuzzyMainWindow)
{
    ui->setupUi(this);

    this->statusBar()->showMessage("Idle");

    // slot-signal connections
    QObject::connect(&c, SIGNAL(statusChanged(QString)), this, SLOT(setStatus(const QString &)));
}

FuzzyMainWindow::~FuzzyMainWindow()
{
    delete ui;
}

void FuzzyMainWindow::on_actionE_xit_triggered()
{
    QApplication::exit(0);
}

void FuzzyMainWindow::on_action_Client_triggered()
{
    WSADATA wsadata;

    c.createTCPClient(&wsadata);
    c.startTCPClient();
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
