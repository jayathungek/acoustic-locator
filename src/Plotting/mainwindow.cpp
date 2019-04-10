#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)

{
    ui->setupUi(this);
    MainWindow::plot();
}

MainWindow::~MainWindow()
{
    delete ui;
}

int MainWindow::plot(){

    //number of bits
    double bits = 16;
    // width of the graph
    int width = 1000;


    QFile file("D:/unblocked.csv");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << file.errorString();
        return 1;
    }
    bool ok;
    long hex_to_int;
    QVarLengthArray<long> data;
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        hex_to_int =  line.split(',').first().toLong(&ok, 16);
        data.append(hex_to_int);

    }
    // print integers from .csv
//    for (QVarLengthArray<long>::iterator i = data.begin(); i !=data.end(); i++){
//        qDebug() << *i;
//    }

    file.close();

    // print integers from .csv
//    for (QVarLengthArray<long>::iterator i = data.begin(); i !=data.end(); i++){
//        qDebug() << *i;
//    }


    int plotWidth = data.size();
    double plotHeight = std::pow(2, bits);

    if (plotWidth <width){
        width = plotWidth;
    }

    // split channels
    QVector<double> x(plotWidth), y(plotWidth);
    QVector<double> x2(plotWidth), y2(plotWidth);
    int count = 0;
    for (QVarLengthArray<long>::iterator it = data.begin(); it != data.end() && count < width; it = it + 2){
            x[count] = count; // x axis steps
            x2[count] = count;
            y[count] = *it;  // mic data
            y2[count] = *(it+1);
            count++;
    }



    //test code for parabola
//    for (int i=0; i<1000; ++i)
//    {
//      x[i] = i/50.0 - 1; // x goes from -1 to 1
//      y[i] = x[i]*x[i]; // let's plot a quadratic function
//    }




    // create graph and assign data to it:
    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setData(x, y);

    ui->customPlot->addGraph();
    ui->customPlot->graph(1)->setData(x2, y2);
    ui->customPlot->graph(1)->setPen(QPen(Qt::red)); // line color red for second graph


    // give the axes some labels:
    ui->customPlot->xAxis->setLabel("x");
    ui->customPlot->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    ui->customPlot->xAxis->setRange(0, width);
    ui->customPlot->yAxis->setRange(-5, plotHeight);
    ui->customPlot->replot();

    return 0;

}

