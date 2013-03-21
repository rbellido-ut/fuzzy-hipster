#ifndef FUZZYMAINWINDOW_H
#define FUZZYMAINWINDOW_H

#include <QMainWindow>
#include "client.h"

namespace Ui {
class FuzzyMainWindow;
}

class FuzzyMainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit FuzzyMainWindow(QWidget *parent = 0);
    ~FuzzyMainWindow();
    
private slots:
    void on_actionE_xit_triggered();

    void on_action_Client_triggered();

private:
    Ui::FuzzyMainWindow *ui;
    Client c;
};

#endif // FUZZYMAINWINDOW_H
