#include "torrentserializer.h"

#include <QDebug>
#include <QSettings>

#include "torrentmodel.h"
#include "torrentmodelitem.h"

TorrentSerializer::TorrentSerializer(TorrentModel *model, QObject *parent)
    : QObject( parent )
    , m_model( model )
{
    Q_ASSERT( m_model != nullptr );
    m_settings = new QSettings( this );
}

TorrentSerializer::~TorrentSerializer()
{
}

void TorrentSerializer::save()
{
    int rowCount = m_model->rowCount();
    m_settings->beginWriteArray( "TorrentData", rowCount );
    for ( int i = 0; i < rowCount; ++i ){
        auto index       = m_model->index(i, 0);
        auto variantData = m_model->data( index, Qt::DisplayRole );
        auto torrentItem = variantData.value< TorrentModelItemStar >();
        m_settings->setArrayIndex( i );
        m_settings->setValue("torrentFilePath"  , torrentItem->torrentFilePath() );
        m_settings->setValue("destDir"          , torrentItem->destDir() );
        m_settings->setValue("uploadedBytes"    , torrentItem->client()->uploadedBytes() );
        m_settings->setValue("downloadedBytes"  , torrentItem->client()->downloadedBytes() );
        m_settings->setValue("dumpedState"      , torrentItem->client()->dumpedState() );

    }
    m_settings->endArray();
}

void TorrentSerializer::load()
{
    int rowCount = m_settings->beginReadArray( "TorrentData" );
    for ( int i = 0; i < rowCount; ++i ){
        m_settings->setArrayIndex( i );
        auto torFilePath     = m_settings->value("torrentFilePath").toUrl();
        auto destDir         = m_settings->value("destDir").toString();
        auto dumpedState     = m_settings->value("dumpedState").toByteArray();
        auto uploadedBytes   = m_settings->value("uploadedBytes" ).toLongLong();
        auto downloadedBytes = m_settings->value("downloadedBytes" ).toLongLong();

        int oldCount = m_model->rowCount();
        m_model->addTorrent( torFilePath, destDir, dumpedState );
        int newCount = m_model->rowCount();
        if ( oldCount == newCount )
            return;

        auto indexLast = m_model->index( m_model->rowCount() - 1, 0 );
        auto itemLast  = m_model->data( indexLast, Qt::DisplayRole ).value< TorrentModelItemStar >();
        auto addClient = itemLast->client();
        addClient->setUploadedBytes( uploadedBytes );
        addClient->setDownloadedBytes( downloadedBytes );
    }
    m_settings->endArray();
}

TorrentModel *TorrentSerializer::model() const
{
    return m_model;
}


