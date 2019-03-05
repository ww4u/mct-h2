#include "errmgrmodel.h"

#include "mystd.h"

ErrMgrModel::ErrMgrModel() : MegaTableModel()
{

}

ErrMgrModel::~ErrMgrModel()
{
    delete_all( mItems );
}

int ErrMgrModel::rowCount(const QModelIndex &parent) const
{ return mItems.count(); }
int ErrMgrModel::columnCount(const QModelIndex &parent) const
{ return ErrMgrItem::columns(); }

QVariant ErrMgrModel::data(const QModelIndex &index, int role) const
{
    if ( !index.isValid() )
    { return QVariant(); }

    //! index
    int col = index.column();
    int row = index.row();

    //! by role
    if ( role == Qt::DisplayRole || role == Qt::EditRole  )
    {  }
    else if ( role == Qt::UserRole )
    {
        return userRole_Visible( index );
    }
    else if ( role == (Qt::UserRole + 1) )
    {
        return QVariant( mItems[ row ]->mActionList );
    }
    else if ( role == Qt::BackgroundColorRole )
    { return backRole(index); }
    else if ( role == Qt::TextAlignmentRole )
    { return QVariant( Qt::AlignCenter ); }
    else
    { return QVariant(); }

    if ( col == 0 )
    {
        //! \note 0x
        if ( role == Qt::DisplayRole )
        {
            QString strCode = QString("0x%1(%2)")
                    .arg( mItems[row]->mNr, 2, 16, QChar('0') )
                    .arg(mItems[row]->mNr, 2, 10, QChar('0'));

            return QVariant( strCode.toUpper() );
        }
        else
        { return QVariant( mItems[ row ]->mNr ); }
    }
    else if ( col == 1 )
    {
        if ( role == Qt::DisplayRole )
        {
            mItems[row]->initErrorCodes();
            int code = mItems[row]->mNr;
            return QVariant( mItems[row]->m_mapError[code] );
        }
        else
        {
            return QVariant( ( mItems[ row ]->mErr ) );
        }
    }

    else if ( col == 2 )
    {
        return QVariant( ( mItems[ row ]->mEventType == e_error ) );
    }
    else if ( col == 3 )
    {
        return QVariant( ( mItems[ row ]->mEventType == e_warning ) );
    }
    else if ( col == 4 )
    {
        return QVariant( ( mItems[ row ]->mEventType == e_info ) );
    }

    else if ( col == 5 )
    {
        return QVariant( mItems[ row ]->mAction );
    }
    else if ( col == 6 )
    {
       return QVariant( mItems[ row ]->mbOutput );
    }
    else if ( col == 7 )
    {
        return QVariant( mItems[ row ]->mbSaveDiagnosis );
    }
    else
    { return QVariant(); }
}

bool ErrMgrModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int col = index.column();
    int row = index.row();

    if (index.isValid() && role == Qt::EditRole)
    {
        if ( col == 0 )
        { mItems[ row ]->mNr = value.toInt(); }
        else if ( index.column() == 1 )
        { mItems[ row ]->mErr = value.toString(); }

        else if ( index.column() == 2 )
        {
            if(mItems[ row ]->mEventType != e_error)
            {
                if(mItems[ row ]->mActionList.size() > 1)
                {
                    mItems[ row ]->mAction = mItems[ row ]->mActionList.at(0);
                    mItems[ row ]->mbActionAble = true;
                }else if(mItems[ row ]->mActionList.size() == 1){
                    mItems[ row ]->mAction = mItems[ row ]->mActionList.at(0);
                }

                if((mItems[ row ]->mActionList.size() == 1) && (mItems[ row ]->mActionList.at(0) == ACTION_FREEWHEEL)) {
                    mItems[ row ]->mbOutputAble = false;
                }else{
                    mItems[ row ]->mbOutputAble = true;
                }
            }

            if(mItems[ row ]->mEventType != e_error)
                mItems[ row ]->setEventType( e_error, value.toBool() );
        }
        else if ( index.column() == 3 )
        {
            if(mItems[ row ]->mEventType == e_error)
            {
                mItems[ row ]->mAction = "";
                mItems[ row ]->mbActionAble = false;
                mItems[ row ]->mbOutputAble = false;
            }

            if(mItems[ row ]->mEventType != e_warning)
                mItems[ row ]->setEventType( e_warning, value.toBool() );
        }
        else if( index.column() == 4 )
        {
            if(mItems[ row ]->mEventType == e_error)
            {
                mItems[ row ]->mAction = "";
                mItems[ row ]->mbActionAble = false;
                mItems[ row ]->mbOutputAble = false;
            }

            if(mItems[ row ]->mEventType != e_info)
                mItems[ row ]->setEventType( e_info, value.toBool() );
        }

        else if ( index.column() == 5 )
        { mItems[ row ]->mAction = value.toString(); }

        else if ( index.column() == 6 )
        { mItems[ row ]->mbOutput = value.toBool(); }
        else if ( index.column() == 7 )
        { mItems[ row ]->mbSaveDiagnosis = value.toBool(); }

        else
        {}

        emit dataChanged(index, index);

        return true;
    }
    else
    {
        return false;
    }
}

#define enable_flags( b )   b ? ( Qt::ItemIsSelectable \
                                  | Qt::ItemIsEditable \
                                  | Qt::ItemIsEnabled \
                                    ) : ( Qt::NoItemFlags )
Qt::ItemFlags ErrMgrModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
          return Qt::ItemIsEnabled;

    if ( index.isValid() )
    {}
    else
    { return Qt::NoItemFlags; }

    int col = index.column();
    int row = index.row();
    if ( col == 0 || col == 1 )
    { return Qt::NoItemFlags; }

    if ( col == 2 )
    {
        return enable_flags( mItems[row]->mbErrorAble );
    }
    else if ( col == 3 )
    {
        return enable_flags( mItems[row]->mbWarnAble );
    }
    else if ( col == 4 )
    {
        return enable_flags( mItems[row]->mbInfoAble );
    }
    else if ( col == 5 )
    {
//        return enable_flags( mItems[row]->mActionList.size() > 1 );
        return enable_flags( mItems[row]->mbActionAble );
    }
    else if ( col == 6 )
    {
        return enable_flags( mItems[row]->mbOutputAble );
    }
    else if ( col == 7 )
    {
        return enable_flags( mItems[row]->mbSaveAble );
    }
    else
    { return false; }

//    //! \todo for other columns
//    int flag = Qt::NoItemFlags;

//    //! get item
//    eColumnAttr attr;
//    attr = mItems.at( index.row() )->columnAttr( index.column() );
//    if ( is_bit1( attr, column_editable) )
//    { flag |= Qt::ItemIsEditable; }

////    return QAbstractItemModel::flags(index) | (Qt::ItemFlag)flag;
//    return QAbstractItemModel::flags(index) | (Qt::ItemIsEditable);
}

bool ErrMgrModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    if ( position < 0 || rows < 0 )
    { return false; }

    beginInsertRows(QModelIndex(), position, position+rows-1);

    ErrMgrItem *pItem;
    for (int row = 0; row < rows; ++row)
    {
        pItem = new ErrMgrItem();
        mItems.insert( position+row, pItem );
    }

    endInsertRows();
    return true;
}
bool ErrMgrModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    if ( position < 0 || rows < 1 )
    { return false; }

    beginRemoveRows(QModelIndex(), position, position+rows-1);

    for (int row = 0; row < rows; ++row)
    {
        delete mItems[position];
        mItems.removeAt(position);
    }

    endRemoveRows();
    return true;
}

QVariant ErrMgrModel::headerData(int section, Qt::Orientation orientation, int role ) const
{
    if ( role != Qt::DisplayRole )
    { return QVariant(); }

    if ( orientation == Qt::Horizontal )
    { return QVariant( ErrMgrItem::header(section)); }
    else
    { return QVariant(section);}
}

#define role_visible( b )   QVariant(b)
QVariant ErrMgrModel::userRole_Visible( const QModelIndex &index ) const
{
    if ( !index.isValid() )
    { return QVariant(); }

    //! index
    int col = index.column();
    int row = index.row();

    if ( col == 2 )
    { return role_visible(mItems[row]->mbErrorAble); }
    else if ( col == 3 )
    { return role_visible(mItems[row]->mbWarnAble); }
    else if ( col == 4 )
    { return role_visible(mItems[row]->mbInfoAble); }
    else if ( col == 5 )
    {
        return role_visible( mItems[row]->mbActionAble );
//        return role_visible(mItems[row]->mActionList.size() > 0 );
    }
    else if ( col == 6 )
    { return role_visible(mItems[row]->mbOutputAble); }
    else if ( col == 7 )
    {return role_visible(mItems[row]->mbSaveAble); }
    else
    { return QVariant(); }
}

#define back_role( b )  if ( b ){}else{ break; }
QVariant ErrMgrModel::backRole( const QModelIndex &index ) const
{
    if ( !index.isValid() )
    { return QVariant(); }

    //! index
    int col = index.column();
    int row = index.row();

    do
    {
        if ( col == 2 )
        {  back_role(mItems[row]->mbErrorAble); }
        else if ( col == 3 )
        {  back_role(mItems[row]->mbWarnAble); }
        else if ( col == 4 )
        {  back_role(mItems[row]->mbInfoAble); }
        else if ( col == 5 )
        {
            back_role(mItems[row]->mbActionAble );
//            back_role(mItems[row]->mActionList.size() > 0 );
        }
        else if ( col == 6 )
        {  back_role(mItems[row]->mbOutputAble); }
        else if ( col == 7 )
        {  back_role(mItems[row]->mbSaveAble); }
        else
        { return QVariant(); }

        //! default
        return QVariant();
    }while ( 0 );

    //! disabled
    return QVariant( QColor( Qt::lightGray ) );
}

QList< ErrMgrItem *> *ErrMgrModel::items()
{
    return &mItems;
}

int ErrMgrModel::save( const QString &fileName )
{
    QFile fileOut( fileName );

    if ( !fileOut.open( QIODevice::WriteOnly) )
    { return -1; }

    QXmlStreamWriter writer( &fileOut );

    int ret;

    writer.writeStartDocument();

    writer.writeStartElement("event");

    ret = serialOut( writer );

    writer.writeEndElement();

    writer.writeEndDocument();

    fileOut.close();

    return ret;
}

int ErrMgrModel::load( const QString &fileName )
{
    //! check ver
    QFile fileIn(fileName);
    if ( !fileIn.open( QIODevice::ReadOnly) )
    { return -1; }

    QXmlStreamReader reader( &fileIn );

    int ret = 0;
    while( reader.readNextStartElement() )
    {
        if ( reader.name() == "event" )
        {
            ret = serialIn( reader );
        }
        else
        { reader.skipCurrentElement(); }
    }

    fileIn.close();

    return ret;
}

int ErrMgrModel::serialOut( QXmlStreamWriter & writer )
{
    foreach( ErrMgrItem *pAction, mItems )
    {
        Q_ASSERT( NULL != pAction );

        writer.writeStartElement( "item" );

        writer.writeTextElement( "mNr", QString::number( pAction->mNr ) );
        writer.writeTextElement( "mErr", ( pAction->mErr ) );
        writer.writeTextElement( "mEventType", QString::number( pAction->mEventType ) );

        writer.writeTextElement( "mbErrorAble", QString::number( pAction->mbErrorAble ) );
        writer.writeTextElement( "mbWarnAble", QString::number( pAction->mbWarnAble ) );
        writer.writeTextElement( "mbInfoAble", QString::number( pAction->mbInfoAble ) );

        writer.writeTextElement( "mAction", ( pAction->mAction ) );
        writer.writeTextElement( "mActionList", pAction->mActionList.join(';') );

        writer.writeTextElement( "mbOutput", QString::number(pAction->mbOutput) );
        writer.writeTextElement( "mbOutputAble", QString::number(pAction->mbOutputAble) );
        writer.writeTextElement( "mbSaveDiagnosis", QString::number(pAction->mbSaveDiagnosis) );
        writer.writeTextElement( "mbSaveAble", QString::number(pAction->mbSaveAble) );

        writer.writeEndElement();
    }

    return 0;

}
int ErrMgrModel::serialIn( QXmlStreamReader & reader )
{
    //! item
    ErrMgrItem *pItem;
    QList< ErrMgrItem * > localItems;

    while( reader.readNextStartElement() )
    {
        if ( reader.name() == "item" )
        {
            pItem = new ErrMgrItem();
            Q_ASSERT( NULL != pItem );

            while( reader.readNextStartElement() )
            {
                if ( reader.name() == "mNr" )
                { pItem->mNr = reader.readElementText().toInt(); }
                else if ( reader.name() == "mErr" )
                { pItem->mErr = reader.readElementText(); }
                else if ( reader.name() == "mEventType" )
                { pItem->mEventType = (e_event_type)reader.readElementText().toInt(); }

                else if ( reader.name() == "mbErrorAble" )
                { pItem->mbErrorAble = reader.readElementText().toInt() > 0; }
                else if ( reader.name() == "mbWarnAble" )
                { pItem->mbWarnAble = reader.readElementText().toInt() > 0; }
                else if ( reader.name() == "mbInfoAble" )
                { pItem->mbInfoAble = reader.readElementText().toInt() > 0; }

                else if ( reader.name() == "mAction" )
                { pItem->mAction = reader.readElementText(); }
                else if ( reader.name() == "mActionList" )
                {
                    QStringList strList = reader.readElementText().split( ';', QString::SkipEmptyParts );
                    foreach( const QString &subStr, strList )
                    {
                        pItem->mActionList<< subStr.trimmed();
                    }
                }
                else if ( reader.name() == "mbOutput" )
                { pItem->mbOutput = reader.readElementText().toInt() > 0; }
                else if ( reader.name() == "mbOutputAble" )
                { pItem->mbOutputAble = reader.readElementText().toInt() > 0; }
                else if ( reader.name() == "mbSaveDiagnosis" )
                { pItem->mbSaveDiagnosis = reader.readElementText().toInt() > 0; }
                else if ( reader.name() == "mbSaveAble" )
                { pItem->mbSaveAble = reader.readElementText().toInt() > 0; }
                else
                { reader.skipCurrentElement(); }

                if(pItem->mActionList.size() > 1)
                {pItem->mbActionAble = true; }
                else
                {pItem->mbActionAble = false; }
            }

            localItems.append( pItem );
        }
        else
        { reader.skipCurrentElement(); }
    }

    //! assign
    delete_all( mItems );
    mItems = localItems;

    endResetModel();

    return 0;
}

void ErrMgrModel::createDebug()
{
    ErrMgrItem *pItem;

    pItem = new ErrMgrItem();
    pItem->mNr = 1;
    pItem->mErr = "hello";

    pItem->mEventType = e_error;

    pItem->mbOutput = true;
    pItem->mbOutputAble = true;
    pItem->mbSaveDiagnosis = true;

    mItems.append( pItem );
}
