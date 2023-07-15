#ifndef COMMANDRUNNER_H
#define COMMANDRUNNER_H

#include <QObject>
#include <QProcess>

class CommandRunner : public QObject
{
    Q_OBJECT

public:
    explicit CommandRunner(QObject *parent = nullptr);

    int sudo(const QStringList& command, const bool waitForCompletion = false, QByteArray* output = nullptr);
    QByteArray readFile(const QString& absolutePath);
    bool writeFile(const QString& absolutePath, const QByteArray& value);
    bool rm(const QString& path);

public slots:
    bool validatePassword();
    void providePassword(const QString& password);

private:
    QProcess* m_process = nullptr;

signals:
    void passwordRequested();
};

#endif // COMMANDRUNNER_H
