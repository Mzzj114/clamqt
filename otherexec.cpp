#include "otherexec.h"
#include <QDebug>

OtherExec::OtherExec(QObject *parent)
    : QObject{parent}
{
    p = new QProcess(parent);

    connect(p, &QProcess::readyReadStandardOutput, this, [=]{
        QString str = p->readAllStandardOutput();
        qDebug() << "otherexec" << str;
        emit readyRead(str);
    });
    connect(p, &QProcess::readyReadStandardError, this, [=]{
        emit readyReadError(p->readAllStandardError());
    });
    connect(p, &QProcess::stateChanged, this, [=](QProcess::ProcessState state){
        if (state==QProcess::NotRunning)
            emit exit();
    });
}

void OtherExec::start(const QString &path, QStringList args)
{
    p->start(path,args);
}

void OtherExec::stop()
{
    p->terminate();
}
