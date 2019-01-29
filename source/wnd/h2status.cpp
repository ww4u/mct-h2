#include "h2status.h"
#include "ui_h2status.h"
#include <QThread>
#include <QTimer>

H2Status::H2Status(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::H2Status)
{
    ui->setupUi(this);

    connect(ui->chkMct,SIGNAL(toggled(bool)),
            this,SIGNAL(signal_mct_checked(bool)));

    connect(ui->chkPwr,SIGNAL(toggled(bool)),
            this,SIGNAL(signal_power_checked(bool)));

    connect(ui->btnAckError, SIGNAL(clicked(bool)),this, SIGNAL(signal_btnAckError_clicked()));
}

H2Status::~H2Status()
{
    delete ui;
}

void H2Status::changeLanguage(QString qmFile)
{
    //  翻译文件
    qApp->removeTranslator(&m_translator);
    m_translator.load(qmFile);
    qApp->installTranslator(&m_translator);
    ui->retranslateUi(this);
}

void H2Status::on_chkMct_toggled(bool checked)
{
    ui->chkMct->setChecked(checked);

    if(!checked){
        ui->chkPwr->setChecked(false);
        ui->chkPwr->setEnabled(false);
    }
    else{
        ui->chkPwr->setEnabled(true);
        ui->chkPwr->setChecked(false);
    }
}

void H2Status::set_chkMct_enabled(bool isEnabled)
{
    ui->chkMct->setEnabled(isEnabled);
}


void H2Status::on_chkPwr_toggled(bool checked)
{
    setEnable(checked);
}

void H2Status::setActionStatus(ActionStatus_t s)
{
    if(s == ACTION_READY)
    {
        ui->radReady->setChecked(true);
        ui->radMc->setChecked(false);
    }
    else if(s == ACTION_MC)
    {
        ui->radReady->setChecked(false);
        ui->radMc->setChecked(true);
        QTimer::singleShot(1000, [this](){
            ui->radReady->setChecked(true);
            ui->radMc->setChecked(false);
        });
    }
    else
    {
        ui->radReady->setChecked(false);
        ui->radMc->setChecked(false);
    }
}

void H2Status::setWarningError(ErrorStatus_t type, const QString &info)
{
    if(type == STATUS_ERROR)
    {
        ui->radError->setChecked(true);
        ui->radWarning->setChecked(false);
        ui->edtError->setText(info);
        ui->edtWarning->setText(" ");
    }
    else if(type == STATUS_WARING)
    {
        ui->radError->setChecked(false);
        ui->radWarning->setChecked(true);
        ui->edtError->setText(" ");
        ui->edtWarning->setText(info);
    }
    else if(type == STATUS_EMPTY)
    {
        ui->radError->setChecked(false);
        ui->radWarning->setChecked(false);
        ui->edtError->setText(" ");
        ui->edtWarning->setText(" ");
    }
}


void H2Status::setTorqueOff(bool isOFF)
{
    ui->radTo->setChecked(isOFF);
}

void H2Status::setEnable(bool isEnable)
{
    ui->radEn->setChecked(isEnable);
}

void H2Status::setOpreateMode(QString mode)
{
    ui->edtOpMode->setText(mode);
}
