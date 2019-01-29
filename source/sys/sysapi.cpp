#include "sysapi.h"
#include "mainwindow.h"
#include <QProcess>

void sysInfo( const QString &str )
{ MainWindow::requestLogout( str, eLogInfo ); }

void sysInfo( const QString &info, const QString &str2 )
{
    QString str = QString("%1 %2").arg(info).arg(str2);
    MainWindow::requestLogout( str, eLogInfo );
}

void sysInfo( const QString &info, const int &val )
{
    QString str = QString("%1 : %2").arg(info).arg(val);
    MainWindow::requestLogout( str, eLogInfo );
}

void sysWarning( const QString &str )
{ MainWindow::requestLogout( str, eLogWarning ); }

void sysError( const QString &str )
{ MainWindow::requestLogout( str, eLogError ); }

void sysError( const QString &info , const int &val)
{
    QString str = QString("%1 : %2").arg(info).arg(val);
    MainWindow::requestLogout( str, eLogError );
}

void sysShowStatus(const QString &statusInfo)
{
    MainWindow::showStatus(statusInfo);
}

void sysShowProgressBar(const bool isRunning)
{
    MainWindow::showProgressBar(isRunning);
}

bool copyFileToPath(QString sourceDir ,QString toDir, bool coverFileIfExist)
{
    toDir.replace("\\","/");
    if (sourceDir == toDir){
        return true;
    }
    if (!QFile::exists(sourceDir)){
        return false;
    }
    QDir *createfile     = new QDir;
    bool exist = createfile->exists(toDir);
    if (exist){
        if(coverFileIfExist){
            createfile->remove(toDir);
        }
    }//end if

    if(!QFile::copy(sourceDir, toDir))
    {
        return false;
    }
    return true;
}

QString readFile(QString fileName)
{
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << QString("Can't ReadOnly open the file: %1").arg(fileName);
        return "";
    }

    QByteArray array = file.readAll();
    file.close();
    return QString(array);
}

int writeFile(QString fileName, QString text)
{
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << QString("Can't WriteOnly open the file: %1").arg(fileName);
        return -1;
    }

    file.write(text.toUtf8());
    file.close();
    return 0;
}

bool QtPing(const QString ip)
{
    // #Linux指令 "ping -s 1 -c 1 IP"
#ifndef _WIN32
    QString cmdstr = QString("ping -s 1 -c 1 %1").arg(ip);
#else
    // #Windows指令 "ping IP -n 1 -w 超时(ms)"
    QString cmdstr = QString("ping %1 -n 1 -w %2").arg(ip).arg(1000);
#endif
    QProcess cmd;
    cmd.start(cmdstr);
    cmd.waitForReadyRead(1000);
    cmd.waitForFinished(1000);

    QString res = cmd.readAll();
    if (res.indexOf("TTL") == -1){
        return false;
    }
    else{
        return true;
    }
}

