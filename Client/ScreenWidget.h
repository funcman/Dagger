#ifndef SCREENWIDGET_H
#define SCREENWIDGET_H

#include <QWidget>

class ScreenWidget : public QWidget {
    Q_OBJECT
public:
    explicit ScreenWidget(QWidget* parent=0, int width=640, int height=480);
    ~ScreenWidget();

    static ScreenWidget* instance();

    void* frameBuffer();

protected:
    void paintEvent(QPaintEvent* event);

signals:

public slots:

private:
    int     width_;
    int     height_;
    QImage* screen_;
};

#endif//SCREENWIDGET_H
