#include "MainTask.h"

#include <QCoreApplication>
#include <QPainter>

#include "MainWindow.h"
#include "ScreenWidget.h"

#include "Engine.h"

MainTask::MainTask(QObject* parent)
:   QObject(parent)
,   isRunning_(false) {
}

MainTask::~MainTask() {
}

void MainTask::run() {
    ScreenWidget* screen = ScreenWidget::instance();
    Canvas canvas(screen->width(), screen->height(), screen->frameBuffer());

    this->isRunning_ = true;
    while (this->isRunning_) {
        ((QCoreApplication*)this->parent())->processEvents();

        for (int l=0; l < 32; ++l) {
            for (int i=0; i < 100; ++i) {
                DrawPixelFast(i, l, RGB565(255, 0, 255));
            }
            for (int i=0; i < 100; ++i) {
                DrawPixelAlpha(i, l, RGB565(0, 255, 0), l);
            }
        }

        DrawLine(0, 0, 300, 300, RGB565(0, 255, 0));
        DrawLineAlpha(200, 200, 400, 400, RGB565(255, 0, 0), 16);

        MainWindow::instance()->repaint();
    }
    emit finished();
}

void MainTask::quit() {
    this->isRunning_ = false;
}
