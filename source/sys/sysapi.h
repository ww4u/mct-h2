#ifndef SYSAPI_H
#define SYSAPI_H

#include <QtCore>

QString MCTHomeDIR();

void sysInfo( const QString &str );
void sysInfo(const QString &info, const QString &str2 );
void sysInfo( const QString &info, const int &val );

void sysWarning( const QString &str );

void sysError( const QString &str );
void sysError( const QString &info , const int &val);

void sysShowStatus(const QString &statusInfo);

void sysShowProgressBar(const bool isRunning);

bool copyFileToPath(QString sourceDir ,QString toDir, bool coverFileIfExist);

QString readFile(QString fileName);

int writeFile(QString fileName, QString text);

bool updateConfigFile(QString configFileName, QString item, QString value);

bool QtPing(const QString ip);
#endif // SYSAPI_H
