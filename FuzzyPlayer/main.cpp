/*------------------------------------------------------------------------------------------------------------------
 -- SOURCE FILE: main.cpp - Entry point for the FuzzyPlayer app
 --
 -- PROGRAM: ychats
 --
 -- FUNCTIONS:
 -- main(int argc, char **argv)
 --
 -- DATE: March 20, 2013
 --
 -- REVISIONS: March 20, 2013 - Initial version
 --
 -- DESIGNER: Aaron Lee
 --
 -- PROGRAMMER: Aaron Lee
 --
 -- NOTES: instantiates a QT application window.
 ----------------------------------------------------------------------------------------------------------------------*/

#include "fuzzymainwindow.h"
#include <QApplication>


/*------------------------------------------------------------------------------------------------------------------
 -- FUNCTION: main
 --
 -- DATE: March 19, 2013
 --
 -- REVISIONS: March 19, 2013 -- Initial version.
 --
 -- DESIGNER: Aaron Lee
 --
 -- PROGRAMMER: Aaron Lee
 --
 -- INTERFACE: int main(int argc, char **argv)
 --             argc - number of arguments
 --             **argv - values of arguments
 --
 -- RETURNS: int - error type. 0 if success.
 --
 -- NOTES:
 -- Entry point.
----------------------------------------------------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FuzzyMainWindow w;
    w.show();
    
    return a.exec();
}
