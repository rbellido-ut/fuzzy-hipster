#include "clientsettingsdialog.h"
#include "ui_clientsettingsdialog.h"

ClientSettingsDialog::ClientSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ClientSettingsDialog)
{
    ui->setupUi(this);
}

ClientSettingsDialog::~ClientSettingsDialog()
{
    delete ui;
}

// start fuzzymainwindow's client engine
void ClientSettingsDialog::on_buttonBox_accepted()
{
    // pass hostname, port textbox data with signal to end slot
    emit clientIgnite(ui->ipEdit->text(), ui->portEdit->text());
}
