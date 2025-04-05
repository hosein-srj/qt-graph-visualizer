#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    graph = new Graph();
    setCentralWidget(graph);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    graph->close();
}
