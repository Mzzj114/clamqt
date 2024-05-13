#ifndef OTHEREXEC_H
#define OTHEREXEC_H

#include <QObject>
#include <QProcess>

class OtherExec : public QObject
{
    Q_OBJECT
public:
    explicit OtherExec(QObject *parent = nullptr);

    void start(const QString &path, QStringList args = {});
    void stop();

private:
    QProcess *p;



signals:
    void readyRead(QString);
    void readyReadError(QString);
    void exit();
};

#endif // OTHEREXEC_H
