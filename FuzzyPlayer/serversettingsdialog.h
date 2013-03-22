#ifndef SERVERSETTINGSDIALOG_H
#define SERVERSETTINGSDIALOG_H

#include <QDialog>
#include "util.h"

namespace Ui {
class ServerSettingsDialog;
}

class ServerSettingsDialog : public QDialog
{
    Q_OBJECT

signals:
    void serverIgnite(int);
public:
    explicit ServerSettingsDialog(QWidget *parent = 0);
    ~ServerSettingsDialog();

private slots:
    void on_buttonBox_accepted();
    
private:
    Ui::ServerSettingsDialog *ui;
};

#endif // SERVERSETTINGSDIALOG_H
