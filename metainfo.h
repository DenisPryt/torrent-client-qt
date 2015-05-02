/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef METAINFO_H
#define METAINFO_H

#include <QByteArray>
#include <QDateTime>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>

struct MetaInfoSingleFile
{
    qint64              Length;
    QByteArray          MD5sum;
    QString             Name;
    int                 PieceLength;
    //QList<QByteArray>   SHA1SumList;
};

struct MetaInfoMultiFile
{
    qint64      Length;
    QByteArray  MD5sum;
    QString     Path;
};

class MetaInfo : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QString metaInfoName READ name )
public:

    enum FileForm {
        SingleFileForm,
        MultiFileForm
    };

    MetaInfo();
    MetaInfo( const MetaInfo &other );
    MetaInfo(       MetaInfo &&other );
    MetaInfo& operator =( const MetaInfo &other );
    MetaInfo& operator =(       MetaInfo &&other );

    void clear();
    void copy( const MetaInfo &src );

    bool parse(const QByteArray &data);
    QString errorString() const;

    QByteArray infoValue() const;

    FileForm fileForm() const;
    const QStringList &announceList() const;
    const QDateTime &creationDate() const;
    const QString &comment() const;
    const QString &createdBy() const;

    // For single file form
    const MetaInfoSingleFile &singleFile() const;

    // For multifile form
    const QList<MetaInfoMultiFile> &multiFiles() const;
    const QString &name() const;
    int pieceLength() const;
    const QList<QByteArray> &sha1Sums() const;

    // Total size
    qint64 totalSize() const;

private:
    QString errString;
    QByteArray content;
    QByteArray infoData;

    FileForm metaInfoFileForm;
    MetaInfoSingleFile metaInfoSingleFile;
    QList<MetaInfoMultiFile> metaInfoMultiFiles;
    QStringList metaInfoAnnounceList;
    QDateTime metaInfoCreationDate;
    QString metaInfoComment;
    QString metaInfoCreatedBy;
    QString metaInfoName;
    int metaInfoPieceLength;
    QList<QByteArray> metaInfoSha1Sums;
};

Q_DECLARE_METATYPE( MetaInfo )

#endif
