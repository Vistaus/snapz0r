#include "commandrunner.h"

#include <QDebug>

CommandRunner::CommandRunner(QObject *parent) :
    QObject(parent),
    m_process(new QProcess(this))
{
    connect(this->m_process, &QProcess::stateChanged, this, [=](QProcess::ProcessState state) {
        if (state == QProcess::NotRunning) {
            qDebug() << "Command stopped";
        }
    });
    connect(this->m_process, &QProcess::readyReadStandardError,
            this, [=]() {
        const QByteArray stdErrContent = this->m_process->readAllStandardError();
        qDebug() << stdErrContent;
        if (stdErrContent.contains("userpasswd")) {
            emit passwordRequested();
        }
    });
}

int CommandRunner::shell(const QStringList &command, const bool waitForCompletion, QByteArray* output)
{
    QStringList cmd = QStringList{"-c", command.join(" ")};

    this->m_process->start("bash", cmd, QProcess::ReadWrite);
    if (waitForCompletion) {
        this->m_process->waitForFinished();
        if (output) {
            *output = this->m_process->readAllStandardOutput();
        }
        qDebug() << this->m_process->exitCode();
        const int ret = this->m_process->exitCode();
        this->m_process->kill();
        return ret;
    }
    return -1;
}

int CommandRunner::sudo(const QStringList &command, const bool waitForCompletion, QByteArray* output)
{
    QStringList cmd = QStringList{"-S", "-p", "userpasswd"} + command;
    qDebug() << "running" << cmd.join(" ");

    this->m_process->start("sudo", cmd, QProcess::ReadWrite);
    if (waitForCompletion) {
        this->m_process->waitForFinished();
        if (output) {
            *output = this->m_process->readAllStandardOutput();
        }
        qDebug() << this->m_process->exitCode();
        const int ret = this->m_process->exitCode();
        this->m_process->kill();
        return ret;
    }
    return -1;
}

bool CommandRunner::sudo(const QStringList &command)
{
    return sudo(command, true, nullptr);
}

QByteArray CommandRunner::readFile(const QString &absolutePath)
{
    sudo(QStringList{"cat" , absolutePath});
    this->m_process->waitForFinished();
    const QByteArray value = this->m_process->readAllStandardOutput();
    qDebug() << absolutePath << "=" << value;
    return value;
}

bool CommandRunner::writeFile(const QString &absolutePath, const QByteArray &value)
{
    const QStringList writeCommand {
        QStringLiteral("/bin/sh"), QStringLiteral("-c"),
        QStringLiteral("echo '%1' | tee %2").arg(value, absolutePath)
    };
    sudo(writeCommand);
    this->m_process->waitForFinished();
    return (this->m_process->exitCode() == 0);
}

bool CommandRunner::rm(const QString& path)
{
    const QStringList writeCommand {
        QStringLiteral("/bin/sh"), QStringLiteral("-c"),
        QStringLiteral("/bin/rm '%1'").arg(path)
    };
    sudo(writeCommand);
    this->m_process->waitForFinished();
    return (this->m_process->exitCode() == 0);
}

void CommandRunner::providePassword(const QString& password)
{
    this->m_process->write(password.toUtf8());
    this->m_process->write("\n");
}

bool CommandRunner::validatePassword()
{
    const QStringList idCommand {
        QStringLiteral("id"), QStringLiteral("-u")
    };
    sudo(idCommand);
    this->m_process->waitForFinished();
    const QByteArray output = this->m_process->readAllStandardOutput();
    return (output.trimmed() == "0");
}

void CommandRunner::cancel()
{
    m_process->kill();
    m_process->waitForFinished();
}
