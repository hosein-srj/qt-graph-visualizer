#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Graph.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setCentralWidget(new Graph());

}

MainWindow::~MainWindow()
{
    delete ui;
}
