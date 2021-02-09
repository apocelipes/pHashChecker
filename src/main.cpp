#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication a(argc, argv);
    QTranslator translator;
    // like zh_CN.qm
    if (translator.load(":/" + QLocale().name() + ".qm")) {
        QCoreApplication::installTranslator(&translator);
    }
    MainWindow w;
    w.show();
    return QApplication::exec();
}
