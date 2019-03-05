#include "h2errmgr.h"
#include "ui_h2errmgr.h"

H2ErrMgr::H2ErrMgr(QWidget *parent) :
    XConfig(parent),
    ui(new Ui::H2ErrMgr)
{
    ui->setupUi(this);
    m_errorFileInfo = "";

    setFocuHelpName( "Error_management" );

    //! 建立错误响应通信和显示对应表
    m_mapRespStrToInt.insert( ACTION_FREEWHEEL, 1 );
    m_mapRespStrToInt.insert( ACTION_QSDEC, 2 );
    m_mapRespStrToInt.insert( ACTION_RECDEC, 3 );
    m_mapRespStrToInt.insert( ACTION_FINISH, 4 );

    m_mapRespIntToStr.insert( 1, ACTION_FREEWHEEL );
    m_mapRespIntToStr.insert( 2, ACTION_QSDEC );
    m_mapRespIntToStr.insert( 3, ACTION_RECDEC );
    m_mapRespIntToStr.insert( 4, ACTION_FINISH );


    //! 创建表格代理
    m_pCheckDelegate = new checkDelegate( shape_check, this );
    m_pRadioDelegate = new checkDelegate( shape_radio, this );
    m_pReactionCombox = new comboxDelegate( 1, this );

    //!

}

H2ErrMgr::~H2ErrMgr()
{
    delete m_pCheckDelegate ;
    delete m_pRadioDelegate ;
    delete m_pReactionCombox;
    delete ui;
}

int H2ErrMgr::readDeviceConfig()
{
    int isOk = 0;
    for(int i=0; i<ui->tvErr->model()->rowCount(); i++)
    {
        int code = mErrManager.items()->at(i)->mNr;;

        int type;
        int response;
        int diagnose;
        int enable; //默认使能,界面上没有对应的列


        //! 45 53 54 55 61 不是error类型

        int ret=  mrgErrorCodeConfigUpload(mViHandle, code, &type, &response, &diagnose, &enable);
        if(ret < 0){
            sysError("mrgErrorCodeConfigUpload", code);
            qDebug() << "mrgErrorCodeConfigUpload error" << code << ret;
            isOk = -1;
            continue;
        }
        qDebug() << "ErrorCodeUpload" << ret << ":" << code << type << response << diagnose << enable;

        mErrManager.mItems.at(i)->mEventType = (e_event_type)type;
        if(type == e_error)
        {
            int reaction = 0;
            int output = 0;
            parseResponse(response, &reaction, &output);

            mErrManager.mItems.at(i)->mAction =  m_mapRespIntToStr[reaction];
            mErrManager.mItems.at(i)->mbOutput = (output==1) ? true : false;
            mErrManager.mItems.at(i)->mbSaveDiagnosis = (diagnose==1) ? true : false;
        }
    }

    return isOk;
}

int H2ErrMgr::writeDeviceConfig()
{
    int isOk = 0;
    int ret = -1;
    bool bl = false;
    QString str;

    for(int i=0; i<ui->tvErr->model()->rowCount(); i++)
    {
        int code            = mErrManager.items()->at(i)->mNr;
        QString errorText   = mErrManager.items()->at(i)->mErr;
        int type            = mErrManager.items()->at(i)->mEventType;

        str                 = mErrManager.items()->at(i)->mAction;
        bl                  = mErrManager.items()->at(i)->mbOutput;
        int outputAble      = bl ? 1 : 0 ;

        int response        = calcResponse( m_mapRespStrToInt[str], outputAble);

        bl                  = mErrManager.items()->at(i)->mbSaveDiagnosis;
        int diagnose        = bl ? 1 : 0 ;

        int enable = 1; //默认使能,界面上没有对应的列

        ret = mrgErrorCodeConfigDownload(mViHandle, code, type, response, diagnose, enable);
//        qDebug() << "ErrorCodeDownload:" << code << type << response
//                 << diagnose << enable << "ret:" << ret;
        if(ret < 0){
            sysError("mrgErrorCodeConfigDownload", code);
            isOk = -1;
            continue;
        }

    }

    return isOk;
}

int H2ErrMgr::loadConfig()
{
    QString fileName = MCTHomeDIR() + "/dataset/" + mConfigFileName + ".xml";
    QFile file(fileName);
    if( !file.exists() )
        fileName = qApp->applicationDirPath() + "/dataset/errmgr_default.xml";

    //! load event from xml
    int ret = mErrManager.load( fileName );

    return ret;
}

int H2ErrMgr::saveConfig()
{
    //! save event to xml
    QString fileName = MCTHomeDIR() + "/dataset/" + mConfigFileName + ".xml";
    int ret = mErrManager.save( fileName );
    return ret;
}

void H2ErrMgr::updateShow()
{
    ui->tvErr->setModel( &mErrManager );

    ui->tvErr->setItemDelegateForColumn( 2, m_pRadioDelegate );
    ui->tvErr->setItemDelegateForColumn( 3, m_pRadioDelegate );
    ui->tvErr->setItemDelegateForColumn( 4, m_pRadioDelegate );

    ui->tvErr->setItemDelegateForColumn( 5, m_pReactionCombox );

    ui->tvErr->setItemDelegateForColumn( 6, m_pCheckDelegate );
    ui->tvErr->setItemDelegateForColumn( 7, m_pCheckDelegate );

    ui->tvErr->resizeColumnToContents(1);
    ui->tvErr->setColumnWidth(5,200);

}

void H2ErrMgr::translateUI()
{
    ui->retranslateUi(this);
}


/**
 * @brief calcResponse
 * @param reaction 1,2,3,4
 * @param outputAble 0,1
 * @return 小于零失败
 */
int H2ErrMgr::calcResponse(int reaction, int outputAble)
{
    if(reaction<1 || reaction>4 || outputAble<0 || outputAble>1 ) {
        return -1;
    }

    //! 1
    if(reaction == 1){
        return 1;
    }

    //! 2 3 4
    if(reaction != 1 && outputAble == 1){
        return reaction;
    }

    //! 5 6 7
    if(reaction != 1 && outputAble == 0){
        return reaction + 3;
    }
}

/**
 * @brief H2ErrMgr::parseResponse
 * @param response 1,2,3,4,5,6,7
 * @param reaction 输出1,2,3,4
 * @param outputAble 输出0或1
 * @return 小于零失败
 */
int H2ErrMgr::parseResponse(int response,int *reaction, int *outputAble)
{
    if(response == 1) {
        *reaction = 1;
        return 0;
    }

    if(response>=2 && response<=4) {
        *reaction = response;
        *outputAble = 1;
        return 0;
    }
    else if(response>=5 && response<=7) {
        *reaction = response - 3;
        *outputAble = 0;
        return 0;
    }
    else {
        return -1;
    }
}
