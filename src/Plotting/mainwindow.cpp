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

void MainWindow::plot(){
    // generate some data:
    QVector<double> x(101), y(101); // initialize with entries 0..100
    for (int i=0; i<101; ++i)
    {
      x[i] = i/50.0 - 1; // x goes from -1 to 1
      y[i] = x[i]*x[i]; // let's plot a quadratic function
    }

    QVector<double> x2(101), y2(101); // initialize with entries 0..100
    for (int i=0; i<101; ++i)
    {
      x2[i] = i/50.0 - 1; // x goes from -1 to 1
      y2[i] = x2[i]*x2[i]*-1; // let's plot a quadratic function
    }

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
    ui->customPlot->xAxis->setRange(-1, 1);
    ui->customPlot->yAxis->setRange(-1, 1);
    ui->customPlot->replot();

}

