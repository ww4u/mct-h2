
#include <QApplication>
#include "../source/wnd/mainwindow.h"

#define MCTVERSION "00.00.01.06"

int main(int argc, char *argv[])
{
//    QApplication a(argc, argv);

//    //解决中文字符乱码的问题，使用下面转化方式
//    //! QString->char * ==> str.toLocal8Bit().data()
//    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));
//    a.setApplicationVersion(MCTVERSION);

//    MainWindow w;
//    w.showMaximized();

//    return a.exec();

    printf("version: %08x,_WIN32_WINNT:%08x\n", NTDDI_VERSION, _WIN32_WINNT);

}



