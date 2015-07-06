#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <assert.h>

#include "ScreenWidget.h"

static MainWindow* instance_ = 0;

MainWindow::MainWindow(QWidget* parent)
:   QMainWindow(parent)
,   ui(new Ui::MainWindow) {
    instance_ = this;

    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
    instance_ = 0;
}

MainWindow* MainWindow::instance() {
    assert(instance_ != 0);
    return instance_;
}

void MainWindow::showEvent(QShowEvent* event) {
    QRect rect = centralWidget()->geometry();
    int dw = 640 - rect.width();
    int dh = 480 - rect.height();

    QWidget* w = new ScreenWidget(this, 640, 480);
    w->setGeometry(0, 0, 640, 480);
    setCentralWidget(w);

    rect.setWidth(geometry().width()+dw);
    rect.setHeight(geometry().height()+dh);
    this->setGeometry(rect);
    this->setFixedSize(rect.size());
}
