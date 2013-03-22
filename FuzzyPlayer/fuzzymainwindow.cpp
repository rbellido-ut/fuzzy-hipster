#include "fuzzymainwindow.h"
#include "ui_fuzzymainwindow.h"


FuzzyMainWindow::FuzzyMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FuzzyMainWindow)
{
    // setup GUI
    ui->setupUi(this);
    cDlg = new ClientSettingsDialog();
    sDlg = new ServerSettingsDialog();

    populateFileTree(ui->localTree, new QDir("/"));

    this->statusBar()->showMessage("Idle");

    // slot-signal connections
    QObject::connect(&c, SIGNAL(statusChanged(QString)), this, SLOT(setStatus(const QString &)));
    QObject::connect(cDlg, SIGNAL(clientIgnite(QString,QString)), this, SLOT(startClient(const QString &, const QString &)));
}

FuzzyMainWindow::~FuzzyMainWindow()
{
    delete ui;
    delete cDlg;
    delete sDlg;
}

void FuzzyMainWindow::populateFileTree(QTreeWidget *tree, QDir *dir)
{
    // remove all items from tree
    tree->clear();


    // columns are not quite right yet...
    //localDir = new QDir("/");
    QFileInfoList filesList = dir->entryInfoList();
    foreach(QFileInfo fileInfo, filesList)
    {
      QTreeWidgetItem* item = new QTreeWidgetItem();
      item->setText(0,fileInfo.fileName());

      if(fileInfo.isFile())
      {
        item->setText(1,QString::number(fileInfo.size()));
        item->setIcon(0,*(new QIcon("file.jpg")));
      }

      if(fileInfo.isDir())
      {
        item->setIcon(0,*(new QIcon("folder.jpg")));
        addChildren(item,fileInfo.filePath());
      }

      item->setText(2,fileInfo.filePath());
      tree->addTopLevelItem(item);
    }
}

// populateFileTree helper
void FuzzyMainWindow::addChildren(QTreeWidgetItem* item,QString filePath)
{
    QDir* rootDir = new QDir(filePath);
    QFileInfoList filesList = rootDir->entryInfoList();

    foreach(QFileInfo fileInfo, filesList)
    {
        QTreeWidgetItem* child = new QTreeWidgetItem();
        child->setText(0,fileInfo.fileName());


        if(fileInfo.isFile())
        {
          child->setText(1,QString::number(fileInfo.size()));
        }

        if(fileInfo.isDir())
        {
          child->setIcon(0,*(new QIcon("folder.jpg")));
          child->setText(2,fileInfo.filePath());
        }

        item->addChild(child);
    }
}

void FuzzyMainWindow::on_actionE_xit_triggered()
{
    QApplication::exit(0);
}

void FuzzyMainWindow::on_action_Client_triggered()
{
    cDlg->show();
}

void FuzzyMainWindow::on_action_Server_triggered()
{
    sDlg->show();
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

void FuzzyMainWindow::on_action_Open_Local_Directory_triggered()
{
    // create browse folder dialog
    QString directory = QFileDialog::getExistingDirectory(this, tr("Open Local Directory"),
          "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    // populate qlistwidget with file names from above folder
    populateFileTree(ui->localTree, new QDir(directory));
}
