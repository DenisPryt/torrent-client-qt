#include "peerwireclient.h"
#include "ratecontroller.h"

#include <QtCore>
#include <QDebug>

Q_GLOBAL_STATIC(RateController, rateController)

RateController *RateController::instance()
{
    return rateController();
}

RateController::RateController(QObject *parent)
    : QObject( parent )
    , transferScheduled( false )
{ }

void RateController::addSocket(PeerWireClient *socket)
{
    connect(socket, &PeerWireClient::readyToTransfer, this, &RateController::scheduleTransfer);
    socket->setReadBufferSize(m_downloadLimit * 4);
    sockets << socket;
    scheduleTransfer();
}

void RateController::removeSocket(PeerWireClient *socket)
{
    disconnect(socket, &PeerWireClient::readyToTransfer, this, &RateController::scheduleTransfer);
    socket->setReadBufferSize(0);
    sockets.remove(socket);
}

qint64 RateController::uploadLimit() const
{
    return m_uploadLimit;
}

qint64 RateController::downloadLimit() const
{
    return m_downloadLimit;
}

void RateController::setUploadLimit(qint64 bytesPerSecond)
{
    qDebug() << bytesPerSecond;
    if ( bytesPerSecond == m_uploadLimit )
        return;

    emit uploadLimitChanged( m_uploadLimit = bytesPerSecond );
}

void RateController::setDownloadLimit(qint64 bytesPerSecond)
{
    if ( m_downloadLimit == bytesPerSecond )
        return;

    for (auto socket : sockets)
        socket->setReadBufferSize( bytesPerSecond * 4 );

    emit downloadLimitChanged( m_downloadLimit = bytesPerSecond );
}

void RateController::scheduleTransfer()
{
    if (transferScheduled)
        return;

    transferScheduled = true;
    QTimer::singleShot(50, this, &RateController::transfer);
}

void RateController::transfer()
{
    transferScheduled = false;
    if (sockets.isEmpty())
        return;

    int msecs = 1000;
    if ( !stopWatch.isNull() )
        msecs = qMin(msecs, stopWatch.elapsed());

    qint64 bytesToWrite = (m_uploadLimit   * msecs) / 1000;
    qint64 bytesToRead  = (m_downloadLimit * msecs) / 1000;
    if (bytesToWrite == 0 && bytesToRead == 0) {
        scheduleTransfer();
        return;
    }

    QSet<PeerWireClient *> pendingSockets;
    for (auto client : sockets) {
        if ( client->canTransferMore() )
            pendingSockets << client;
    }
    if ( pendingSockets.isEmpty() )
        return;

    stopWatch.start();

    bool canTransferMore;
    do {
        canTransferMore   = false;
        qint64 writeChunk = qMax<qint64>(1, bytesToWrite / pendingSockets.size());
        qint64 readChunk  = qMax<qint64>(1, bytesToRead / pendingSockets.size());

        QSetIterator<PeerWireClient *> it(pendingSockets);
        while (it.hasNext() && (bytesToWrite > 0 || bytesToRead > 0)) {
            PeerWireClient *socket = it.next();
            if (socket->state() != QAbstractSocket::ConnectedState) {
                pendingSockets.remove(socket);
                continue;
            }

            bool dataTransferred = false;
            qint64 available = qMin<qint64>(socket->socketBytesAvailable(), readChunk);
            if (available > 0) {
                qint64 readBytes = socket->readFromSocket(qMin<qint64>(available, bytesToRead));
                if (readBytes > 0) {
                    bytesToRead -= readBytes;
                    dataTransferred = true;
                }
            }

            if (m_uploadLimit * 2 > socket->bytesToWrite()) {
                qint64 chunkSize = qMin<qint64>(writeChunk, bytesToWrite);
                qint64 toWrite   = qMin(m_uploadLimit * 2 - socket->bytesToWrite(), chunkSize);
                if (toWrite > 0) {
                    qint64 writtenBytes = socket->writeToSocket(toWrite);
                    if (writtenBytes > 0) {
                        bytesToWrite -= writtenBytes;
                        dataTransferred = true;
                    }
                }
            }

            if (dataTransferred && socket->canTransferMore())
                canTransferMore = true;
            else
                pendingSockets.remove(socket);
        }
    } while (canTransferMore && (bytesToWrite > 0 || bytesToRead > 0) && !pendingSockets.isEmpty());

    if (canTransferMore || bytesToWrite == 0 || bytesToRead == 0)
        scheduleTransfer();
}
