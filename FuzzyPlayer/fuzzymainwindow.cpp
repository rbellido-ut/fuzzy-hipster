#include "fuzzymainwindow.h"
#include "ui_fuzzymainwindow.h"

FuzzyMainWindow::FuzzyMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FuzzyMainWindow)
{
    ui->setupUi(this);
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
