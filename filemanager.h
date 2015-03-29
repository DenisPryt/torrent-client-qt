#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QBitArray>
#include <QList>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QFile>

#include "macro.h"
#include "torrentfileinfo.h"

class FileManager : public QThread
{
    Q_OBJECT
public:
    explicit FileManager(QObject *parent = 0);
    virtual ~FileManager();

    inline void setMetaInfo(const TorrentFileInfo &info) { m_torrentFileInfo = info; }
    inline void setDestinationFolder(const QString &directory) { m_destinationPath = directory; }

    int read(int pieceIndex, int offset, int length);
    void write(int pieceIndex, int offset, const QByteArray &data);
    void verifyPiece(int pieceIndex);
    inline qint64 totalSize() const { return m_totalLength; }

    inline int pieceCount() const { return m_numPieces; }
    int pieceLengthAt(int pieceIndex) const;

    QBitArray completedPieces() const;
    void setCompletedPieces(const QBitArray &pieces);

    QString errorString() const;

public slots:
    void startDataVerification();

signals:
    void dataRead(int id, int pieceIndex, int offset, const QByteArray &data);
    void error();
    void verificationProgress(int percent);
    void verificationDone();
    void pieceVerified(int pieceIndex, bool verified);

protected:
    void run() Q_DECL_OVERRIDE;

private slots:
    bool verifySinglePiece(int pieceIndex);
    void wakeUp();

private:
    bool generateFiles();
    QByteArray readBlock(int pieceIndex, int offset, int length);
    bool writeBlock(int pieceIndex, int offset, const QByteArray &data);
    void verifyFileContents();

    struct WriteRequest {
        int pieceIndex;
        int offset;
        QByteArray data;
    };
    struct ReadRequest {
        int pieceIndex;
        int offset;
        int length;
        int id;
    };

    TorrentFileInfo m_torrentFileInfo;
    QString         m_destinationPath;

    QString         m_errString;

    QList<QFile *>      m_files;
    QList< QByteArray > m_sha1s;
    QBitArray           m_verifiedPieces;

    bool        m_newFile;
    int         m_pieceLength;
    qint64      m_totalLength;
    int         m_numPieces;
    int         m_readId;
    bool        m_startVerification;
    bool        m_quit;
    bool        m_wokeUp;

    QList< WriteRequest >   m_writeRequests;
    QList< ReadRequest >    m_readRequests;
    QList< int >            m_pendingVerificationRequests;
    QList< int >            m_newPendingVerificationRequests;
    QList< qint64 >         m_fileSizes;

    mutable QMutex          m_mutex;
    mutable QWaitCondition  m_cond;
};

#endif // FILEMANAGER_H
