#include <memory>
#include <QApplication>
#include <QTimer>

#include "MainWindow.h"
#include "MainTask.h"

#include "Base.h"

int main(int argc, char** argv) {
    QApplication a(argc, argv);

    std::auto_ptr<MemPool>           pool_(new MemPool());

    MainWindow w;
    w.show();

    MainTask task(&a);
    QObject::connect(&a, SIGNAL(lastWindowClosed()), &task, SLOT(quit()));
    QObject::connect(&task, SIGNAL(finished()), &a, SLOT(quit()));
    QTimer::singleShot(0, &task, SLOT(run()));

    return a.exec();
}
