#include "torrent.h"

//// class DownloadingInfo

void DownloadingInfo::clear()
{
    Downloaded = 0;
    Left = 0;
    Uploaded = 0;
    Key = 0;
    NumWant = -1;
}

//// class Torrent

Torrent::Torrent(QObject *parent) : QObject(parent)
{

}

Torrent::~Torrent()
{

}

