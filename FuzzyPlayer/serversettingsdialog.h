#ifndef SERVERSETTINGSDIALOG_H
#define SERVERSETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class ServerSettingsDialog;
}

class ServerSettingsDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ServerSettingsDialog(QWidget *parent = 0);
    ~ServerSettingsDialog();
    
private:
    Ui::ServerSettingsDialog *ui;
};

#endif // SERVERSETTINGSDIALOG_H
