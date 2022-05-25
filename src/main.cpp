// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2022 apocelipes

#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator translator;
    // like zh_CN.qm
    if (translator.load(":/" + QLocale().name() + ".qm")) {
        QCoreApplication::installTranslator(&translator);
    }
    MainWindow w;
    w.setWindowTitle(QObject::tr("pHashChecker"));
    w.show();
    return QApplication::exec();
}
