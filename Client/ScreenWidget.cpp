#include "ScreenWidget.h"

#include <assert.h>
#include <QPainter>

static ScreenWidget* instance_ = 0;

ScreenWidget::ScreenWidget(QWidget* parent, int width, int height)
:   QWidget(parent)
,   width_(width)
,   height_(height) {
    instance_ = this;
    screen_ = new QImage(width_, height_, QImage::Format_RGB16);
}

ScreenWidget::~ScreenWidget() {
    delete screen_;
    instance_ = 0;
}

ScreenWidget* ScreenWidget::instance() {
    assert(instance_ != 0);
    return instance_;
}

void ScreenWidget::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.drawImage(0, 0, *screen_);
}

void* ScreenWidget::frameBuffer() {
    return screen_->bits();
}
