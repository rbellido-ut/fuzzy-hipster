#ifndef FUZZYMAINWINDOW_H
#define FUZZYMAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QTreeWidgetItem>
#include "client.h"
#include "server.h"
#include "clientsettingsdialog.h"
#include "serversettingsdialog.h"

namespace Ui {
    class FuzzyMainWindow;
}

class FuzzyMainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit FuzzyMainWindow(QWidget *parent = 0);
    ~FuzzyMainWindow();
    
public slots:
    void setStatus(const QString &);
    void startClient(const QString &, const QString &);
    void startServer(int protocol);

private slots:
    void on_actionE_xit_triggered();

    void on_action_Client_triggered();

    void on_action_Open_Local_Directory_triggered();

    void on_action_Server_triggered();

private:
    // methods
    void populateFileTree(QTreeWidget*, QDir*);
    void addChildren(QTreeWidgetItem*, QString);

    // members
    QDir* localDir;

    // GUI Objects
    Ui::FuzzyMainWindow *ui;
    ClientSettingsDialog *cDlg;
    ServerSettingsDialog *sDlg;

    // Network objects
    Client client_;
    Server server_;
};

#endif // FUZZYMAINWINDOW_H
