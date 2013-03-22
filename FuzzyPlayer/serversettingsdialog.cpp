#include "serversettingsdialog.h"
#include "ui_serversettingsdialog.h"

ServerSettingsDialog::ServerSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerSettingsDialog)
{
    ui->setupUi(this);
}

ServerSettingsDialog::~ServerSettingsDialog()
{
    delete ui;
}

void ServerSettingsDialog::on_buttonBox_accepted()
{
    int proto;

    if (ui->TCPRadio->isChecked())
        proto = TCP;
    else if (ui->UDPRadio->isChecked())
        proto = UDP;

    emit serverIgnite(proto);
}
