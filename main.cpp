#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QFile>
#include <QStyle>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "clamqt_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }



    QFile qss("Aqua.qss");
    qss.open(QFile::ReadOnly);
    a.setStyleSheet(qss.readAll());
    qss.close();



    MainWindow w;
    w.setWindowFlag(Qt::FramelessWindowHint);
    w.show();
    return a.exec();
}
