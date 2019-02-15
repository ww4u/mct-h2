#include "h2measurement.h"
#include "ui_h2measurement.h"

H2Measurement::H2Measurement(QWidget *parent) :
    XConfig(parent),
    ui(new Ui::H2Measurement)
{
    ui->setupUi(this);
    m_ZeroPoint = 0;

    setFocuHelpName("Measurement");

    connect(ui->comboBox_AxesZeroPoint,SIGNAL(currentIndexChanged(int)),this,SLOT(slotChangeCornerPicture(int)));

}

H2Measurement::~H2Measurement()
{
    delete ui;
}

void H2Measurement::slotChangeCornerPicture(int index)
{
    QString strPixmap = ":/res/image/h2measurement/CornerPoint";
    strPixmap += QString::number(index+1);
    strPixmap += ".PNG";

    ui->label_picture->setPixmap(QPixmap(strPixmap));
    emit signal_AxesZeroPoint_currentTextChanged(ui->comboBox_AxesZeroPoint->currentText());
}

int H2Measurement::readDeviceConfig()
{
    int ret = mrgGetRobotCoordinateSystem(mViHandle, mRobotName);
    if(ret < 0)
    {
        qDebug() << "mrgGetRobotCoordinateSystem" << ret;
        sysError("mrgGetRobotCoordinateSystem", ret);
        return -1;
    }
    m_ZeroPoint = ret;

    double x, y, z;
    if(0 == mrgGetRobotProjectZero(mViHandle, mRobotName, &x, &y, &z) ){
        qDebug() << "mrgGetRobotProjectZero" << x << y;
        m_ProjectZeroPointX = x;
        m_ProjectZeroPointY = y;
    }else{
        sysError("mrgGetRobotProjectZero");
        return -1;
    }

    if(0 == mrgGetRobotSoftWareLimit(mViHandle, mRobotName, 0, &x, &y, &z) ){
        qDebug() << "mrgGetRobotSoftWareLimit Positive" << x << y;
        m_SWLimitPositiveX = x;
        m_SWLimitPositiveY = y;
    }else{
        sysError("mrgGetRobotSoftWareLimit Positive");
        return -1;
    }

    if(0 == mrgGetRobotSoftWareLimit(mViHandle, mRobotName, 1, &x, &y, &z) ){
        qDebug() << "mrgGetRobotSoftWareLimit Negative" << x << y;
        m_SWLimitNegativeX  = x;
        m_SWLimitNegativeY  = y;
    }else{
        sysError("mrgGetRobotSoftWareLimit Negative");
        return -1;
    }

    return 0;
}

int H2Measurement::writeDeviceConfig()
{
    int ret = 0;

    int value = ui->comboBox_AxesZeroPoint->currentIndex();
    ret = mrgSetRobotCoordinateSystem(mViHandle, mRobotName, value);
    qDebug() << "mrgSetRobotCoordinateSystem" << ret;
    if( 0 != ret) {
        sysError("mrgSetRobotCoordinateSystem");
        return -1;
    }

    ret = mrgSetRobotProjectZero(mViHandle, mRobotName, m_ProjectZeroPointX, m_ProjectZeroPointY, 0);
    qDebug() << "mrgSetRobotProjectZero" << ret;
    if( 0 != ret) {
        sysError("mrgSetRobotProjectZero");
        return -1;
    }

    ret = mrgSetRobotSoftWareLimit(mViHandle, mRobotName, 0, m_SWLimitPositiveX, m_SWLimitPositiveY, 0);
    qDebug() << "mrgSetRobotSoftWareLimit Positive" << ret;
    if( 0 != ret) {
        sysError("mrgSetRobotSoftWareLimit Positive");
        return -1;
    }

    ret = mrgSetRobotSoftWareLimit(mViHandle, mRobotName, 1, m_SWLimitNegativeX, m_SWLimitNegativeY, 0);
    qDebug() << "mrgSetRobotSoftWareLimit Negative" << ret;
    if( 0 != ret) {
        sysError("mrgSetRobotSoftWareLimit Negative");
        return -1;
    }

    return ret;
}

int H2Measurement::loadConfig()
{
    //! load xml
    MegaXML mXML;
    QString fileName = MCTHomeDIR() + "/robots/" + mConfigFileName + ".xml";
    QFile file(fileName);
    if( !file.exists() )
        fileName = qApp->applicationDirPath() + "/robots/default.xml";

    QMap<QString,QString> map = mXML.xmlRead(fileName);
    if(map.isEmpty()) return -1;

    m_ZeroPoint         = map["ZeroPoint"].toInt();
    m_ProjectZeroPointX = map["ProjectZeroPointX"].toDouble();
    m_ProjectZeroPointY = map["ProjectZeroPointY"].toDouble();
    m_SWLimitPositiveX  = map["SWLimitPositiveX"].toDouble();
    m_SWLimitPositiveY  = map["SWLimitPositiveY"].toDouble();
    m_SWLimitNegativeX  = map["SWLimitNegativeX"].toDouble();
    m_SWLimitNegativeY  = map["SWLimitNegativeY"].toDouble();

    return 0;
}

int H2Measurement::saveConfig()
{
    MegaXML mXML;
    QString fileName = MCTHomeDIR() + "/robots/" + mConfigFileName + ".xml";
    QMap<QString,QString> map;

    map.insert("ZeroPoint"  , QString::number(m_ZeroPoint));
    map.insert("Direction"  , ui->comboBox_AxesZeroPoint->currentText() );
    map.insert("ProjectZeroPointX" , QString::number( m_ProjectZeroPointX, 10, 2));
    map.insert("ProjectZeroPointY" , QString::number( m_ProjectZeroPointY, 10, 2));
    map.insert("SWLimitPositiveX"  , QString::number( m_SWLimitPositiveX, 10, 2 ));
    map.insert("SWLimitPositiveY"  , QString::number( m_SWLimitPositiveY, 10, 2 ));
    map.insert("SWLimitNegativeX"  , QString::number( m_SWLimitNegativeX, 10, 2 ));
    map.insert("SWLimitNegativeY"  , QString::number( m_SWLimitNegativeY, 10, 2 ));

    mXML.xmlNodeRemove(fileName,"H2Measurement");
    mXML.xmlNodeAppend(fileName, "H2Measurement", map);

    return 0;
}

void H2Measurement::updateShow()
{
    ui->comboBox_AxesZeroPoint->setCurrentIndex(m_ZeroPoint);
    ui->doubleSpinBox_pzpX->setValue( m_ProjectZeroPointX );
    ui->doubleSpinBox_pzpY->setValue( m_ProjectZeroPointY );
    ui->doubleSpinBox_swlp_X->setValue( m_SWLimitPositiveX );
    ui->doubleSpinBox_swlp_Y->setValue( m_SWLimitPositiveY );
    ui->doubleSpinBox_swln_X->setValue( m_SWLimitNegativeX );
    ui->doubleSpinBox_swln_Y->setValue( m_SWLimitNegativeY );
}

void H2Measurement::on_comboBox_AxesZeroPoint_currentIndexChanged(int index)
{
    m_ZeroPoint = index;
    emit signalModelDataChanged(true);
}

void H2Measurement::on_doubleSpinBox_pzpX_valueChanged(double arg1)
{
    m_ProjectZeroPointX = arg1;
    emit signalModelDataChanged(true);

    ui->doubleSpinBox_pzpX->setRange(0, m_WorkStrokeX);
    ui->doubleSpinBox_pzpX->setValue(arg1);

    ui->doubleSpinBox_swlp_X->setRange(arg1, m_WorkStrokeX);
    ui->doubleSpinBox_swlp_X->setToolTip(QString("%1<->%2").arg(arg1).arg(m_WorkStrokeX));
    ui->doubleSpinBox_swlp_X->setValue(m_WorkStrokeX);

    ui->doubleSpinBox_swln_X->setRange(0, arg1);
    ui->doubleSpinBox_swln_X->setToolTip(QString("%1<->%2").arg(0).arg(arg1));
    ui->doubleSpinBox_swln_X->setValue(0);
}

void H2Measurement::on_doubleSpinBox_pzpY_valueChanged(double arg1)
{
    m_ProjectZeroPointY = arg1;
    emit signalModelDataChanged(true);

    ui->doubleSpinBox_pzpY->setRange(0, m_WorkStrokeY);
    ui->doubleSpinBox_pzpY->setValue(arg1);

    ui->doubleSpinBox_swlp_Y->setRange(arg1, m_WorkStrokeY);
    ui->doubleSpinBox_swlp_Y->setToolTip(QString("%1<->%2").arg(arg1).arg(m_WorkStrokeY));
    ui->doubleSpinBox_swlp_Y->setValue(m_WorkStrokeY);

    ui->doubleSpinBox_swln_Y->setRange(0, arg1);
    ui->doubleSpinBox_swln_Y->setToolTip(QString("%1<->%2").arg(0).arg(arg1));
    ui->doubleSpinBox_swln_Y->setValue(0);
}

void H2Measurement::on_doubleSpinBox_swlp_X_valueChanged(double arg1)
{
    m_SWLimitPositiveX = arg1;
    emit signalModelDataChanged(true);
}

void H2Measurement::on_doubleSpinBox_swlp_Y_valueChanged(double arg1)
{
    m_SWLimitPositiveY = arg1;
    emit signalModelDataChanged(true);
}

void H2Measurement::on_doubleSpinBox_swln_X_valueChanged(double arg1)
{
    m_SWLimitNegativeX = arg1;
    emit signalModelDataChanged(true);
}

void H2Measurement::on_doubleSpinBox_swln_Y_valueChanged(double arg1)
{
    m_SWLimitNegativeY = arg1;
    emit signalModelDataChanged(true);
}

void H2Measurement::setWorkStrokeY(double WorkStrokeY)
{
    m_WorkStrokeY = WorkStrokeY;
    on_doubleSpinBox_pzpY_valueChanged(0);
}

void H2Measurement::setWorkStrokeX(double WorkStrokeX)
{
    m_WorkStrokeX = WorkStrokeX;
    on_doubleSpinBox_pzpX_valueChanged(0);
}

void H2Measurement::translateUI()
{
    ui->retranslateUi(this);
}
