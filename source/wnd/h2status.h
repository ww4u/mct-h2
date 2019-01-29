#ifndef H2STATUS_H
#define H2STATUS_H

#include <QTranslator>
#include <QWidget>

namespace Ui {
class H2Status;
}


class H2Status : public QWidget
{
    Q_OBJECT

public:
    enum ActionStatus_t{
        ACTION_EMPTY,
        ACTION_READY,
        ACTION_MC
    };

    enum ErrorStatus_t{
        STATUS_ERROR,
        STATUS_WARING,
        STATUS_EMPTY
    };

public:
    explicit H2Status(QWidget *parent = 0);
    ~H2Status();

    void changeLanguage(QString qmFile);

    void setEnable(bool isEnable);
    void setOpreateMode(QString mode);
    void setTorqueOff(bool isOFF);
    void setActionStatus(ActionStatus_t s);
    void setWarningError(ErrorStatus_t type, const QString &info);

signals:
    void signal_mct_checked(bool);
    void signal_power_checked(bool);
    void signal_btnAckError_clicked();

public slots:
    void on_chkMct_toggled(bool checked);
    void set_chkMct_enabled(bool isEnabled);

private slots:
    void on_chkPwr_toggled(bool checked);

private:
    Ui::H2Status *ui;
    QTranslator m_translator;
};

#endif // H2STATUS_H
