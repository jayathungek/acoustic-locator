#include "mainwindow.h"
#include <QApplication>

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <fstream>

#include <QDebug>
#include <QString>
#include <QFile>
#include <QStringList>
#include <QDir>
#include <QVarLengthArray>

int main(int argc, char *argv[])
{



    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
