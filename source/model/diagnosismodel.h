#ifndef DIAGNOSISMODEL_H
#define DIAGNOSISMODEL_H

#include "megatablemodel.h"
#include "diagnosisitem.h"

class DiagnosisModel : public MegaTableModel
{
    Q_OBJECT
public:
    DiagnosisModel();
    ~DiagnosisModel();

public:
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;

    virtual QVariant data(const QModelIndex &index, int role) const;

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    virtual bool insertRows(int position, int rows, const QModelIndex &parent);
    virtual bool removeRows(int position, int rows, const QModelIndex &parent);

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

public:
    QList< DiagnosisItem *> *items();

    int save( const QString &fileName );
    int load( const QString &fileName );

    int serialOut( QXmlStreamWriter & writer );
    int serialIn( QXmlStreamReader & reader );

public:
    QList< DiagnosisItem *> mItems;
    void appendOneItem(int nr, QString type, QString ts, QString addInfo, int counter, QString message);
};

#endif // DIAGNOSISMODEL_H
