
#include <QApplication>
#include "../source/wnd/mainwindow.h"

#define MCTVERSION "0.0.1.5"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //解决中文字符乱码的问题，使用下面转化方式
    //! QString->char * ==> str.toLocal8Bit().data()
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));
    a.setApplicationVersion(MCTVERSION);

    MainWindow w;
    w.showMaximized();

    return a.exec();
}



