#ifndef FUZZYMAINWINDOW_H
#define FUZZYMAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QTreeWidgetItem>
#include "client.h"
#include "clientsettingsdialog.h"

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

private slots:
    void on_actionE_xit_triggered();

    void on_action_Client_triggered();

    void on_action_Open_Local_Directory_triggered();

private:
    // methods
    void populateFileTree(QTreeWidget*, QDir*) ;
    void addChildren(QTreeWidgetItem*, QString);

    // members
    QDir* localDir;

    // GUI Objects
    Ui::FuzzyMainWindow *ui;
    ClientSettingsDialog *cDlg;

    // Network objects
    Client c;
};

#endif // FUZZYMAINWINDOW_H
