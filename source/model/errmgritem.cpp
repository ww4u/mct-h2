#include "errmgritem.h"
#include "mystd.h"

const static char *_headers[] =
{
    QT_TRANSLATE_NOOP("HEADER", "No."),
    QT_TRANSLATE_NOOP("HEADER", "Error Text"),
    QT_TRANSLATE_NOOP("HEADER", "Error"),
    QT_TRANSLATE_NOOP("HEADER", "Warning"),
    QT_TRANSLATE_NOOP("HEADER", "Information"),
    QT_TRANSLATE_NOOP("HEADER", "Reaction on error"),
    QT_TRANSLATE_NOOP("HEADER", "Output stage on"),
    QT_TRANSLATE_NOOP("HEADER", "Save diagnosis"),
};

const static char *_errorCodes[] =
{
    QT_TRANSLATE_NOOP("ERRORS", "Software error"),
    QT_TRANSLATE_NOOP("ERRORS", "Default parameter file invalid"),
    QT_TRANSLATE_NOOP("ERRORS", "Non-permitted hardware"),
    QT_TRANSLATE_NOOP("ERRORS", "Zero angle determination"),
    QT_TRANSLATE_NOOP("ERRORS", "Encoder"),
    QT_TRANSLATE_NOOP("ERRORS", "Parameter file invalid"),
    QT_TRANSLATE_NOOP("ERRORS", "Firmware update execution error"),
    QT_TRANSLATE_NOOP("ERRORS", "I²t malfunction motor"),
    QT_TRANSLATE_NOOP("ERRORS", "Softwarelimit positive"),
    QT_TRANSLATE_NOOP("ERRORS", "Softwarelimit negative"),
    QT_TRANSLATE_NOOP("ERRORS", "Logic voltage exceeded"),
    QT_TRANSLATE_NOOP("ERRORS", "Logic voltage too low"),
    QT_TRANSLATE_NOOP("ERRORS", "Temperature LM-CPU"),
    QT_TRANSLATE_NOOP("ERRORS", "Intermediate circuit voltage exceede"),
    QT_TRANSLATE_NOOP("ERRORS", "Intermediate circuit voltage too low"),
    QT_TRANSLATE_NOOP("ERRORS", "CAN Node Guarding, FB has overall control"),
    QT_TRANSLATE_NOOP("ERRORS", "CAN bus communication stopped by master, FB has overall control"),
    QT_TRANSLATE_NOOP("ERRORS", "Path calculation"),
    QT_TRANSLATE_NOOP("ERRORS", "CAN fieldbus parameters missing"),
    QT_TRANSLATE_NOOP("ERRORS", "Save parameters"),
    QT_TRANSLATE_NOOP("ERRORS", "Homing required"),
    QT_TRANSLATE_NOOP("ERRORS", "Target position behind negative software end position"),
    QT_TRANSLATE_NOOP("ERRORS", "Target position behind positive software end position"),
    QT_TRANSLATE_NOOP("ERRORS", "Firmware update, invalid firmware"),
    QT_TRANSLATE_NOOP("ERRORS", "Incorrect record number"),
    QT_TRANSLATE_NOOP("ERRORS", "I²t warning motor"),
    QT_TRANSLATE_NOOP("ERRORS", "Following error"),
    QT_TRANSLATE_NOOP("ERRORS", "Extern stop"),
    QT_TRANSLATE_NOOP("ERRORS", "CVE connection"),
    QT_TRANSLATE_NOOP("ERRORS", "MCT connection"),
    QT_TRANSLATE_NOOP("ERRORS", "Torque Off (TO)"),
    QT_TRANSLATE_NOOP("ERRORS", "CAN Node Guarding, warning, FB does not have overall control"),
    QT_TRANSLATE_NOOP("ERRORS", "CAN bus communication stopped by master, warning, FB does not have overall control"),
    QT_TRANSLATE_NOOP("ERRORS", "Standstill monitoring"),
    QT_TRANSLATE_NOOP("ERRORS", "Parameter file access"),
    QT_TRANSLATE_NOOP("ERRORS", "Homing Timeout"),
    QT_TRANSLATE_NOOP("ERRORS", "Start-up event"),
    QT_TRANSLATE_NOOP("ERRORS", "Diagnostic memory"),
    QT_TRANSLATE_NOOP("ERRORS", "Record invalid"),
    QT_TRANSLATE_NOOP("ERRORS", "System reset")
};


int ErrMgrItem::columns()
{
    return sizeof_array( _headers );
}
QString ErrMgrItem::header( int col )
{
    Q_ASSERT( col >= 0 && col <= sizeof_array(_headers) );

    return qApp->translate("HEADER", _headers[col]);
}

void ErrMgrItem::initErrorCodes()
{
    m_mapError.clear();
    int index = 0;
    m_mapError.insert(1,  qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(2,  qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(4,  qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(5,  qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(6,  qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(11, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(12, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(14, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(17, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(18, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(23, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(24, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(25, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(26, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(27, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(28, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(29, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(37, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(38, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(39, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(40, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(41, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(42, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(43, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(44, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(45, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(47, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(48, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(49, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(50, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(52, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(53, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(54, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(55, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(56, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(58, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(61, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(62, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(63, qApp->translate("ERRORS", _errorCodes[index++]) );
    m_mapError.insert(65, qApp->translate("ERRORS", _errorCodes[index++]) );
}


ErrMgrItem::ErrMgrItem()
{
    mNr = 0;
    mEventType = e_error;
    mAction = "";
    mbErrorAble = true;
    mbWarnAble = true;
    mbInfoAble = true;

    mbOutput = false;
    mbOutputAble = false;

    mbSaveDiagnosis = false;
    mbSaveAble = true;

    mbActionAble = true;

    initErrorCodes();
}

eColumnAttr ErrMgrItem::columnAttr( int col )
{
    return column_unk;
}

void ErrMgrItem::setEventType( e_event_type evt, bool b )
{
    //! \todo according the event attr
    if ( evt == e_error )
    {
        if ( b )
        { mEventType = e_error; }
        else
        { mEventType = e_warning; }
    }
    else if ( evt == e_warning )
    {
        if ( b )
        { mEventType = e_warning; }
        else
        { mEventType = e_info; }
    }
    else if ( evt == e_info )
    {
        if ( b )
        { mEventType = e_info; }
        else
        { mEventType = e_error; }
    }
    else
    { Q_ASSERT(false); }
}
