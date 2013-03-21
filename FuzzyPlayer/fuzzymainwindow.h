#ifndef FUZZYMAINWINDOW_H
#define FUZZYMAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class FuzzyMainWindow;
}

class FuzzyMainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit FuzzyMainWindow(QWidget *parent = 0);
    ~FuzzyMainWindow();
    
private:
    Ui::FuzzyMainWindow *ui;
};

#endif // FUZZYMAINWINDOW_H
