#ifndef MAINTASK_H
#define MAINTASK_H

#include <QObject>
#include <QImage>

class MainTask : public QObject {
    Q_OBJECT
public:
    explicit MainTask(QObject* parent=0);
    ~MainTask();

signals:
    void finished();

public slots:
    void run();
    void quit();

private:
    bool    isRunning_;
};

#endif//MAINTASK_H
