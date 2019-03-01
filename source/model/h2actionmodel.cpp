#include "h2actionmodel.h"
#include "mystd.h"
#include "../mrp/mdataset.h"

H2ActionModel::H2ActionModel() : MegaTableModel()
{

}

H2ActionModel::~H2ActionModel()
{
    delete_all( mItems );
}

int H2ActionModel::rowCount(const QModelIndex &parent) const
{ return mItems.count(); }
int H2ActionModel::columnCount(const QModelIndex &parent) const
{ return H2ActionItem::columns(); }

QVariant H2ActionModel::data(const QModelIndex &index, int role) const
{
    if ( !index.isValid() )
    { return QVariant(); }

    //! index
    int col = index.column();
    int row = index.row();

    if ( role == Qt::DisplayRole || role == Qt::EditRole )
    {}
    else if ( role == Qt::TextAlignmentRole )
    { return QVariant( Qt::AlignCenter ); }
    else
    { return QVariant(); }


    if ( col == 0 )
    { return QVariant( mItems[ row ]->mType ); }
    else if ( col == 1 )
    { return QVariant( ( mItems[ row ]->mX ) ); }
    else if ( col == 2 )
    { return QVariant( ( mItems[ row ]->mY ) ); }
    else if ( col == 3 )
    { return QVariant( mItems[ row ]->mVel ); }
    else if ( col == 4 )
    { return QVariant( mItems[ row ]->mAcc ); }
    else if ( col == 5 )
    { return QVariant( mItems[ row ]->mComment ); }
    else
    { return QVariant(); }
}

bool H2ActionModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        beginResetModel();

        int col = index.column();
        int row = index.row();
        if ( col == 0 )
        { mItems[ row ]->mType = value.toString(); }
        else if ( index.column() == 1 )
        { mItems[ row ]->mX = value.toDouble(); }
        else if ( index.column() == 2 )
        { mItems[ row ]->mY = value.toDouble(); }
        else if ( index.column() == 3 )
        { mItems[ row ]->mVel = value.toDouble(); }
        else if( index.column() == 4 )
        { mItems[ row ]->mAcc = value.toDouble(); }
        else if ( index.column() == 5 )
        { mItems[ row ]->mComment = value.toString(); }
        else
        {}

        endResetModel();
//        emit dataChanged(index, index);

        return true;
    }
    else
    {
        return false;
    }
}
Qt::ItemFlags H2ActionModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
          return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool H2ActionModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    if ( position < 0 || rows < 0 )
    { return false; }

    beginInsertRows(QModelIndex(), position, position+rows-1);

    H2ActionItem *pItem;
    for (int row = 0; row < rows; ++row)
    {
        pItem = new H2ActionItem();
        mItems.insert( position+row, pItem );
    }

    endInsertRows();
    return true;
}
bool H2ActionModel::removeRows(int position, int rows, const QModelIndex &parent)
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

QVariant H2ActionModel::headerData(int section, Qt::Orientation orientation, int role ) const
{
    if ( role != Qt::DisplayRole )
    { return QVariant(); }

    if ( orientation == Qt::Horizontal )
    { return QVariant( H2ActionItem::header(section)); }
    else if ( orientation == Qt::Vertical )
    { return QVariant(section+1); }
    else
    { return QVariant(section);}
}

QList< H2ActionItem *> *H2ActionModel::items()
{
    return &mItems;
}

int H2ActionModel::save( const QString &fileName )
{
    QFile fileOut( fileName );

    if ( !fileOut.open( QIODevice::WriteOnly) )
    { return -1; }

    QXmlStreamWriter writer( &fileOut );

    int ret;

    writer.writeStartDocument();

    writer.writeStartElement("action");

    ret = serialOut( writer );

    writer.writeEndElement();

    writer.writeEndDocument();

    fileOut.close();

    return ret;
}

int H2ActionModel::load( const QString &fileName )
{
    //! check ver
    QFile fileIn(fileName);
    if ( !fileIn.open( QIODevice::ReadOnly) )
    { return -1; }

    QXmlStreamReader reader( &fileIn );

    int ret = 0;
    while( reader.readNextStartElement() )
    {
        if ( reader.name() == "action" )
        {
            ret = serialIn( reader );
        }
        else
        { reader.skipCurrentElement(); }
    }

    fileIn.close();

    return ret;
}

int H2ActionModel::serialOut( QXmlStreamWriter & writer )
{
    foreach( H2ActionItem *pAction, mItems )
    {
        Q_ASSERT( NULL != pAction );

        writer.writeStartElement( "item" );

        writer.writeTextElement( "type", ( pAction->mType ) );
        writer.writeTextElement( "x", QString::number( pAction->mX ) );
        writer.writeTextElement( "y", QString::number( pAction->mY ) );
        writer.writeTextElement( "a", QString::number(pAction->mAcc) );
        writer.writeTextElement( "v", QString::number(pAction->mVel) );
        writer.writeTextElement( "comment", (pAction->mComment) );

        writer.writeEndElement();
    }

    return 0;

}
int H2ActionModel::serialIn( QXmlStreamReader & reader )
{
    //! item
    H2ActionItem *pItem;
    QList< H2ActionItem * > localItems;

    while( reader.readNextStartElement() )
    {
        if ( reader.name() == "item" )
        {
            pItem = new H2ActionItem();
            Q_ASSERT( NULL != pItem );

            while( reader.readNextStartElement() )
            {

                if ( reader.name() == "type" )
                { pItem->mType = reader.readElementText(); }
                else if ( reader.name() == "x" )
                { pItem->mX = reader.readElementText().toDouble(); }
                else if ( reader.name() == "y" )
                { pItem->mY = reader.readElementText().toDouble(); }
                else if ( reader.name() == "a" )
                { pItem->mAcc = reader.readElementText().toDouble(); }
                else if ( reader.name() == "v" )
                { pItem->mVel = reader.readElementText().toDouble(); }
                else if ( reader.name() == "comment" )
                { pItem->mComment = reader.readElementText(); }
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

//! output to csv
int H2ActionModel::output( const QString &fileName )
{
    MDataSet dataSet;

    dataSet.setModel( "MRX-H2-M");
    QStringList headers;
    headers << "type" << "x" << "y" << "v" << "a" << "comment";
    dataSet.setHeaders( headers );

    MDataSection *pSection;
    pSection = dataSet.addSection();
    if ( NULL == pSection )
    { return -1; }

    //! export data
    QString fmtStr = fmtString( headers );

    bool bRet;
    foreach( H2ActionItem *pItem, mItems )
    {
        Q_ASSERT( NULL != pItem );

        bRet = pSection->addRow( QString("%1").arg( fmtStr )
                                               .arg( pItem->mType )
                                               .arg( pItem->mX )
                                               .arg( pItem->mY )
                                               .arg( pItem->mVel )
                                               .arg( pItem->mAcc )
                                               .arg( pItem->mComment ) );
        if ( !bRet )
        { return -1; }
    }

    //! save
    return dataSet.save( fileName );
}

//! load from csv
int H2ActionModel::input( const QString &fileName )
{
    int ret;

    //! load
    MDataSet dataSet;
    ret = dataSet.load( fileName );
    if ( ret != 0 )
    { return ret; }

//    if ( dataSet.isEmpty() )
//    { return -1; }

    if ( dataSet.verifyHeader( "x", "y" ) )
    {}
    else
    { return -1; }

    MDataSection *pSec;
    pSec = dataSet.section( 0 );
    if ( NULL == pSec )
    { return -2; }

    deparse_column_index( type, "type" );
    deparse_column_index( x, "x" );
    deparse_column_index( y, "y" );

    deparse_column_index( v, "v" );
    deparse_column_index( a, "a" );
    deparse_column_index( comment, "comment" );

    removeRows( 0, mItems.count(), QModelIndex() );

    beginResetModel();
    //! data
    H2ActionItem item;
    for ( int i = 0; i < pSec->rows(); i++ )
    {
        if ( !pSec->cellValue( i, c_x, item.mX, 0 ) )
        { continue; }
        if ( !pSec->cellValue( i, c_y, item.mY, 0 ) )
        { continue; }

        pSec->cellValue( i, c_type, item.mType, "PA" );
        item.mType = item.mType.toUpper();

        pSec->cellValue( i, c_v, item.mVel, 0 );
        pSec->cellValue( i, c_a, item.mAcc, 0 );
        pSec->cellValue( i, c_comment, item.mComment, "" );

        //! append the item
        insertRow( mItems.size() );
        *mItems[ mItems.size()- 1 ] = item;
    }
    endResetModel();

    return 0;
}
