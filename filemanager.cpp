#include "filemanager.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QTimer>
#include <QTimerEvent>
#include <QCryptographicHash>

FileManager::FileManager(QObject *parent)
    : QThread(parent)
{
    m_quit = false;
    m_totalLength = 0;
    m_readId = 0;
    m_startVerification = false;
    m_wokeUp = false;
    m_newFile = false;
    m_numPieces = 0;
    m_verifiedPieces.fill(false);
}

FileManager::~FileManager()
{
    m_quit = true;
    m_cond.wakeOne();
    wait();

    foreach (QFile *file, m_files) {
        file->close();
        delete file;
    }
}

int FileManager::read(int pieceIndex, int offset, int length)
{
    ReadRequest request;
    request.pieceIndex = pieceIndex;
    request.offset = offset;
    request.length = length;

    QMutexLocker locker(&m_mutex);
    request.id = m_readId++;
    m_readRequests << request;

    if (!m_wokeUp) {
        m_wokeUp = true;
        QMetaObject::invokeMethod(this, "wakeUp", Qt::QueuedConnection);
    }

    return request.id;
}

void FileManager::write(int pieceIndex, int offset, const QByteArray &data)
{
    WriteRequest request;
    request.pieceIndex = pieceIndex;
    request.offset = offset;
    request.data = data;

    QMutexLocker locker(&m_mutex);
    m_writeRequests << request;

    if (!m_wokeUp) {
        m_wokeUp = true;
        QMetaObject::invokeMethod(this, "wakeUp", Qt::QueuedConnection);
    }
}

void FileManager::verifyPiece(int pieceIndex)
{
    QMutexLocker locker(&m_mutex);
    m_pendingVerificationRequests << pieceIndex;
    m_startVerification = true;

    if (!m_wokeUp) {
        m_wokeUp = true;
        QMetaObject::invokeMethod(this, "wakeUp", Qt::QueuedConnection);
    }
}

int FileManager::pieceLengthAt(int pieceIndex) const
{
    QMutexLocker locker(&m_mutex);
    return (m_sha1s.size() == pieceIndex + 1)
            ? (m_totalLength % m_pieceLength) : m_pieceLength;
}

QBitArray FileManager::completedPieces() const
{
    QMutexLocker locker(&m_mutex);
    return m_verifiedPieces;
}

void FileManager::setCompletedPieces(const QBitArray &pieces)
{
    QMutexLocker locker(&m_mutex);
    m_verifiedPieces = pieces;
}

QString FileManager::errorString() const
{
    return m_errString;
}

void FileManager::run()
{
    if (!generateFiles())
        return;

    do {
        {
            // Go to sleep if there's nothing to do.
            QMutexLocker locker(&m_mutex);
            if (!m_quit && m_readRequests.isEmpty() && m_writeRequests.isEmpty() && !m_startVerification)
                m_cond.wait(&m_mutex);
        }

        // Read pending read requests
        m_mutex.lock();
        QList<ReadRequest> newReadRequests = m_readRequests;
        m_readRequests.clear();
        m_mutex.unlock();
        while (!newReadRequests.isEmpty()) {
            ReadRequest request = newReadRequests.takeFirst();
            QByteArray block = readBlock(request.pieceIndex, request.offset, request.length);
            emit dataRead(request.id, request.pieceIndex, request.offset, block);
        }

        // Write pending write requests
        m_mutex.lock();
        QList<WriteRequest> newWriteRequests = m_writeRequests;
        m_writeRequests.clear();
        while (!m_quit && !newWriteRequests.isEmpty()) {
            WriteRequest request = newWriteRequests.takeFirst();
            writeBlock(request.pieceIndex, request.offset, request.data);
        }

        // Process pending verification requests
        if (m_startVerification) {
            m_newPendingVerificationRequests =  m_pendingVerificationRequests;
            m_pendingVerificationRequests.clear();
            verifyFileContents();
            m_startVerification = false;
        }
        m_mutex.unlock();
        m_newPendingVerificationRequests.clear();

    } while (!m_quit);

    // Write pending write requests
    m_mutex.lock();
    QList<WriteRequest> newWriteRequests = m_writeRequests;
    m_writeRequests.clear();
    m_mutex.unlock();
    while (!newWriteRequests.isEmpty()) {
        WriteRequest request = newWriteRequests.takeFirst();
        writeBlock(request.pieceIndex, request.offset, request.data);
    }
}

void FileManager::startDataVerification()
{
    QMutexLocker locker(&m_mutex);
    m_startVerification = true;
    m_cond.wakeOne();
}

bool FileManager::generateFiles()
{
    QMutexLocker locker(&m_mutex);

    m_numPieces = -1;
    QDir dir;
    QString prefix;

    if ( !m_destinationPath.isEmpty() ){
        prefix = m_destinationPath;
        if ( !prefix.endsWith("/") )
            prefix += "/";
    }

    if ( !m_torrentFileInfo.GetRootDirName().isEmpty() ){
        prefix += m_torrentFileInfo.GetRootDirName();
        if ( !prefix.endsWith("/") )
            prefix += "/";
    }

    if ( !dir.mkpath(prefix) ){
        m_errString = tr("Failed to create directory %1").arg(prefix);
        emit error();
        return false;
    }

    for ( uint fileIndex = 0; fileIndex < m_torrentFileInfo.GetFilesCount(); ++fileIndex ) {
        auto &entry = m_torrentFileInfo.GetFileInfo( fileIndex );
        QString filePath = QFileInfo(prefix + entry.FilePath).path();
        if ( !QFile::exists(filePath) ){
            if ( !dir.mkpath(filePath) ){
                m_errString = tr("Failed to create directory %1").arg(filePath);
                emit error();
                return false;
            }
        }

        auto filePtr = new QFile( prefix + entry.FilePath );
        if ( !filePtr->open(QFile::ReadWrite) ){
            m_errString = tr("Failed to open/create file %1: %2")
                          .arg( filePtr->fileName() ).arg( filePtr->errorString() );
            delete filePtr;
            emit error();
            return false;
        }

        if ( filePtr->size() != entry.FileSize ){
            m_newFile = true;
            if ( !filePtr->resize(entry.FileSize) ){
                m_errString = tr("Failed to resize file %1: %2")
                              .arg( filePtr->fileName() ).arg( filePtr->errorString() );
                delete filePtr;
                emit error();
                return false;
            }
        }
        m_fileSizes << filePtr->size();
        m_files     << filePtr;
        filePtr->close();
    }

    m_totalLength   = m_torrentFileInfo.GetTotalFilesSize();
    m_sha1s         = m_torrentFileInfo.GetPieces();
    m_pieceLength   = m_torrentFileInfo.GetPieceLength();
    m_numPieces     = m_sha1s.size();

    return true;
}

QByteArray FileManager::readBlock(int pieceIndex, int offset, int length)
{
    QByteArray block;
    qint64 startReadIndex = (quint64(pieceIndex) * m_pieceLength) + offset;
    qint64 currentIndex = 0;

    for (int i = 0; !m_quit && i < m_files.size() && length > 0; ++i) {
        QFile *file = m_files[i];
        qint64 currentFileSize = m_fileSizes.at(i);
        if ((currentIndex + currentFileSize) > startReadIndex) {
            if (!file->isOpen()) {
                if (!file->open(QFile::ReadWrite)) {
                    m_errString = tr("Failed to read from file %1: %2")
                            .arg(file->fileName()).arg(file->errorString());
                    emit error();
                    break;
                }
            }

            file->seek(startReadIndex - currentIndex);
            QByteArray chunk = file->read(qMin<qint64>(length, currentFileSize - file->pos()));
            file->close();

            block += chunk;
            length -= chunk.size();
            startReadIndex += chunk.size();
            if (length < 0) {
                m_errString = tr("Failed to read from file %1 (read %3 bytes): %2")
                        .arg(file->fileName()).arg(file->errorString()).arg(length);
                emit error();
                break;
            }
        }
        currentIndex += currentFileSize;
    }
    return block;
}

bool FileManager::writeBlock(int pieceIndex, int offset, const QByteArray &data)
{
    qint64 startWriteIndex = (qint64(pieceIndex) * m_pieceLength) + offset;
    qint64 currentIndex = 0;
    int bytesToWrite = data.size();
    int written = 0;

    for (int i = 0; !m_quit && i < m_files.size(); ++i) {
        QFile *file = m_files[i];
        qint64 currentFileSize = m_fileSizes.at(i);

        if ((currentIndex + currentFileSize) > startWriteIndex) {
            if (!file->isOpen()) {
                if (!file->open(QFile::ReadWrite)) {
                    m_errString = tr("Failed to write to file %1: %2")
                            .arg(file->fileName()).arg(file->errorString());
                    emit error();
                    break;
                }
            }

            file->seek(startWriteIndex - currentIndex);
            qint64 bytesWritten = file->write(data.constData() + written,
                                              qMin<qint64>(bytesToWrite, currentFileSize - file->pos()));
            file->close();

            if (bytesWritten <= 0) {
                m_errString = tr("Failed to write to file %1: %2")
                        .arg(file->fileName()).arg(file->errorString());
                emit error();
                return false;
            }

            written += bytesWritten;
            startWriteIndex += bytesWritten;
            bytesToWrite -= bytesWritten;
            if (bytesToWrite == 0)
                break;
        }
        currentIndex += currentFileSize;
    }
    return true;
}

void FileManager::verifyFileContents()
{
    // Verify all pieces the first time
    if (m_newPendingVerificationRequests.isEmpty()) {
        if (m_verifiedPieces.count(true) == 0) {
            m_verifiedPieces.resize(m_sha1s.size());

            int oldPercent = 0;
            if (!m_newFile) {
                int numPieces = m_sha1s.size();

                for (int index = 0; index < numPieces; ++index) {
                    verifySinglePiece(index);

                    int percent = ((index + 1) * 100) / numPieces;
                    if (oldPercent != percent) {
                        emit verificationProgress(percent);
                        oldPercent = percent;
                    }
                }
            }
        }
        emit verificationDone();
        return;
    }

    // Verify all pending pieces
    for (int index : m_newPendingVerificationRequests)
        emit pieceVerified(index, verifySinglePiece(index));
}

bool FileManager::verifySinglePiece(int pieceIndex)
{
    QByteArray block = readBlock(pieceIndex, 0, m_pieceLength);
    QByteArray sha1Sum = QCryptographicHash::hash(block, QCryptographicHash::Sha1);

    if (sha1Sum != m_sha1s.at(pieceIndex))
        return false;

    m_verifiedPieces.setBit(pieceIndex);
    return true;
}

void FileManager::wakeUp()
{
    QMutexLocker locker(&m_mutex);
    m_wokeUp = false;
    m_cond.wakeOne();
}
