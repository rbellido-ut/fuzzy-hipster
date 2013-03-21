#ifndef CLIENTSETTINGSDIALOG_H
#define CLIENTSETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class ClientSettingsDialog;
}

class ClientSettingsDialog : public QDialog
{
    Q_OBJECT
signals:
    void clientIgnite(QString, QString);
public:
    explicit ClientSettingsDialog(QWidget *parent = 0);
    ~ClientSettingsDialog();
    
private slots:
    void on_buttonBox_accepted();

private:
    Ui::ClientSettingsDialog *ui;
};

#endif // CLIENTSETTINGSDIALOG_H
