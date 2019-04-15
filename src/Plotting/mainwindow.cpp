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
    int width = 300;
    int offset = 1100;


    QFile file("D:/centre64.csv");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << file.errorString();
        return 1;
    }
    bool ok;
    int hex_to_int;
    QVarLengthArray<int> data;
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        hex_to_int =  line.split(',').first().toInt(&ok, 16);
        data.append(hex_to_int);
    }

    file.close();


    QFile file2("D:/left64.csv");
    if (!file2.open(QIODevice::ReadOnly)) {
        qDebug() << file2.errorString();
        return 1;
    }
    bool ok2;
    int hex_to_int2;
    QVarLengthArray<int> data2;
    while (!file2.atEnd()) {
        QByteArray line2 = file2.readLine();
        hex_to_int2 =  line2.split(',').first().toInt(&ok2, 16);
        data2.append(hex_to_int2);

    }

    file2.close();



    QFile file3("D:/right64.csv");
    if (!file3.open(QIODevice::ReadOnly)) {
        qDebug() << file3.errorString();
        return 1;
    }
    bool ok3;
    int hex_to_int3;
    QVarLengthArray<int> data3;
    while (!file3.atEnd()) {
        QByteArray line3 = file3.readLine();
        hex_to_int3 =  line3.split(',').first().toInt(&ok3, 16);
        data3.append(hex_to_int3);

    }

    file3.close();

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
    QVector<double> x3(plotWidth), y3(plotWidth);
    // this is for interleaved buffers (as they come from the microphone)
//    int count = 0;
//    for (QVarLengthArray<long>::iterator it = data.begin(); it != data.end() && count < width; it = it + 2){
//            x[count] = count; // x axis steps
//            x2[count] = count;
//            y[count] = *it;  // mic data
//            y2[count] = *(it+1);
//            count++;
//    }

    // this is for single buffers
    int count = 0;
    for (QVarLengthArray<int>::iterator it = data.begin()+offset; it != data.end() && count < width; it++){
            x[count] = count; // x axis steps
            y[count] = *it;  // mic data
            count++;
    }

    count = 0;
    for (QVarLengthArray<int>::iterator it2 = data2.begin()+offset; it2 != data2.end() && count < width; it2++){
            x2[count] = count; // x axis steps
            y2[count] = *it2;  // mic data
            count++;
    }

    count = 0;
    for (QVarLengthArray<int>::iterator it3 = data3.begin()+offset; it3 != data3.end() && count < width; it3++){
            x3[count] = count; // x axis steps
            y3[count] = *it3;  // mic data
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

    ui->customPlot->addGraph();
    ui->customPlot->graph(2)->setData(x3, y3);
    ui->customPlot->graph(2)->setPen(QPen(Qt::green));


    // give the axes some labels:
    ui->customPlot->xAxis->setLabel("x");
    ui->customPlot->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    ui->customPlot->xAxis->setRange(0, width);
    ui->customPlot->yAxis->setRange(-5, plotHeight);
    ui->customPlot->replot();

    return 0;

}

