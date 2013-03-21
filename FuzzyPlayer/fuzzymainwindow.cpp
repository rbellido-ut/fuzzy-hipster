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
