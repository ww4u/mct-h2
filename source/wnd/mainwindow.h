#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <QDockWidget>

#include "helppanel.h"
#include "aboutdlg.h"
#include "h2ops.h"
#include "roboconfig.h"

namespace Ui {
class MainWindow;
}

enum {
    LANG_EN,
    LANG_CN,
};

enum {
    STYLE_MEGAROBO,
    STYLE_CLASSIC,
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static MainWindow *_pBackendProxy;
    static void requestLogout( const QString &str, LogLevel lev );

protected:
    void closeEvent(QCloseEvent *event);
//    void showEvent(QShowEvent *event);

signals:
    void signal_logout_request( QString str, LogLevel lev );

private slots:
    void on_actionAbout_triggered();
    void on_actionChinese_triggered();
    void on_actionEnglish_triggered();
    void on_actionMega_triggered();
    void on_actionClassic_triggered();
    void slotSetDockOpsName(QString strDevInfo, int visa, int deviceName, int roboName);
    void slot_logout( const QString &str, LogLevel lev = eLogInfo );
    void slot_focus_in( const QString &name );
//    void slot_post_startup();

    void on_actionExit_triggered();

private:
    Ui::MainWindow *ui;

    QLabel *m_pLabStatus, *m_pLabMctVer, *m_pLabConVer;

    QDockWidget *m_pDockOps;
    H2Ops *m_pOps;

    QDockWidget *m_pDockHelp;
    HelpPanel *m_pHelpPanel;

    RoboConfig *m_roboConfig;

    int m_style;
    int m_language;
    QTranslator m_translator;

    void setupWorkArea();
    void setupToolBar();
    void setupStatusBar();
    void buildConnection();
    void loadConfig();
    void changeLanguage();
    void setUiStyle(const QString &styleFile);
};

#endif // MAINWINDOW_H
