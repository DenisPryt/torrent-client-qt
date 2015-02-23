#ifndef REQUESTTOSERVERMANAGER_H
#define REQUESTTOSERVERMANAGER_H

#include <QObject>

class RequestToServerManager : public QObject
{
    Q_OBJECT
public:
    explicit RequestToServerManager(QObject *parent = 0);
    ~RequestToServerManager();

signals:

public slots:
};

#endif // REQUESTTOSERVERMANAGER_H
