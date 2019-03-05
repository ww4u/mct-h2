#include "diagnosismodel.h"

#include "mystd.h"

DiagnosisModel::DiagnosisModel() : MegaTableModel()
{

}

DiagnosisModel::~DiagnosisModel()
{
    delete_all( mItems );
}

int DiagnosisModel::rowCount(const QModelIndex &parent) const
{ return mItems.count(); }
int DiagnosisModel::columnCount(const QModelIndex &parent) const
{ return DiagnosisItem::columns(); }

QVariant DiagnosisModel::data(const QModelIndex &index, int role) const
{
    if ( !index.isValid() )
    { return QVariant(); }

    //! index
    int col = index.column();
    int row = index.row();

    if ( role != Qt::DisplayRole && role != Qt::EditRole )
    { return QVariant(); }

    if ( col == 0 )
    { return QVariant( mItems[ row ]->mNr );        }
    else if ( col == 1 )
    { return QVariant( ( mItems[ row ]->mType ) ); }
    else if ( col == 2 )
    { return QVariant( ( mItems[ row ]->mTs ) ); }
    else if ( col == 3 )
    { return QVariant( mItems[ row ]->mAddInfo ); }
    else if ( col == 4 )
    { return QVariant( mItems[ row ]->mCounter ); }
    else if ( col == 5 )
    { return QVariant( mItems[ row ]->mMessage ); }
    else
    { return QVariant(); }
}

bool DiagnosisModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        int col = index.column();
        int row = index.row();
        if ( col == 0 )
        { mItems[ row ]->mNr = value.toInt(); }
        else if ( index.column() == 1 )
        { mItems[ row ]->mType = value.toString(); }
        else if ( index.column() == 2 )
        { mItems[ row ]->mTs = value.toString(); }
        else if ( index.column() == 3 )
        { mItems[ row ]->mAddInfo = value.toString(); }
        else if( index.column() == 4 )
        { mItems[ row ]->mCounter = value.toInt(); }
        else if ( index.column() == 5 )
        { mItems[ row ]->mMessage = value.toString(); }
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
Qt::ItemFlags DiagnosisModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
          return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) /*| Qt::ItemIsEditable*/;
}

bool DiagnosisModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    if ( position < 0 || rows < 0 )
    { return false; }

    beginInsertRows(QModelIndex(), position, position+rows-1);

    DiagnosisItem *pItem;
    for (int row = 0; row < rows; ++row)
    {
        pItem = new DiagnosisItem();
        mItems.insert( position+row, pItem );
    }

    endInsertRows();
    return true;
}
bool DiagnosisModel::removeRows(int position, int rows, const QModelIndex &parent)
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

QVariant DiagnosisModel::headerData(int section, Qt::Orientation orientation, int role ) const
{
    if ( role != Qt::DisplayRole )
    { return QVariant(); }

    if ( orientation == Qt::Horizontal )
    { return QVariant( DiagnosisItem::header(section)); }
    else
    { return QVariant(section);}
}

QList< DiagnosisItem *> *DiagnosisModel::items()
{
    return &mItems;
}

int DiagnosisModel::save( const QString &fileName )
{
    QFile fileOut( fileName );
    if ( !fileOut.open( QIODevice::WriteOnly) )
    { return -1; }

    QXmlStreamWriter writer( &fileOut );

    int ret;
    writer.writeStartDocument();
    writer.writeStartElement("diagnosis");
    ret = serialOut( writer );
    writer.writeEndElement();
    writer.writeEndDocument();
    fileOut.close();

    return ret;
}

int DiagnosisModel::load( const QString &fileName )
{
    //! check ver
    QFile fileIn(fileName);
    if ( !fileIn.open( QIODevice::ReadOnly) )
    { return -1; }

    QXmlStreamReader reader( &fileIn );

    int ret = 0;
    while( reader.readNextStartElement() )
    {
        if ( reader.name() == "diagnosis" )
        {
            ret = serialIn( reader );
        }
        else
        { reader.skipCurrentElement(); }
    }

    fileIn.close();

    return ret;
}

int DiagnosisModel::serialOut( QXmlStreamWriter & writer )
{
    foreach( DiagnosisItem *pAction, mItems )
    {
        Q_ASSERT( NULL != pAction );

        writer.writeStartElement( "item" );

        writer.writeTextElement( "nr", QString::number( pAction->mNr ) );
        writer.writeTextElement( "type", ( pAction->mType ) );
        writer.writeTextElement( "ts", ( pAction->mTs ) );
        writer.writeTextElement( "addinfo", (pAction->mAddInfo) );
        writer.writeTextElement( "counter", QString::number(pAction->mCounter) );
        writer.writeTextElement( "message", (pAction->mMessage) );

        writer.writeEndElement();
    }

    return 0;

}
int DiagnosisModel::serialIn( QXmlStreamReader & reader )
{
    //! item
    DiagnosisItem *pItem;
    QList< DiagnosisItem * > localItems;

    while( reader.readNextStartElement() )
    {
        if ( reader.name() == "item" )
        {
            pItem = new DiagnosisItem();
            Q_ASSERT( NULL != pItem );

            while( reader.readNextStartElement() )
            {
                if ( reader.name() == "nr" )
                { pItem->mNr = reader.readElementText().toInt(); }
                else if ( reader.name() == "type" )
                { pItem->mType = reader.readElementText(); }
                else if ( reader.name() == "ts" )
                { pItem->mTs = reader.readElementText(); }
                else if ( reader.name() == "addinfo" )
                { pItem->mAddInfo = reader.readElementText(); }
                else if ( reader.name() == "counter" )
                { pItem->mCounter = reader.readElementText().toInt(); }
                else if ( reader.name() == "message" )
                { pItem->mMessage = reader.readElementText(); }
                else
                { reader.skipCurrentElement(); }
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


void DiagnosisModel::appendOneItem(int nr,
                                QString type,
                                QString ts,
                                QString addInfo,
                                int counter,
                                QString message)
{
    DiagnosisItem *pItem;

    pItem = new DiagnosisItem();
    pItem->mNr = nr;

    if(type == "F")
        pItem->mType = tr("ERROR");
    else if(type == "W")
        pItem->mType = tr("WARNING");
    else if(type == "I")
        pItem->mType = tr("INFO");

    pItem->mTs = ts;
    pItem->mAddInfo = addInfo;
    pItem->mCounter = counter;
    pItem->mMessage = message;

    mItems.append( pItem );

    endResetModel();
}


