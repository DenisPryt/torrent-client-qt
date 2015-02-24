#include "peerinfo.h"

void PeerInfo::clear()
{
    m_Address.clear();
    m_Port = 0;
    m_Id.clear();
    m_Intesting = false;
    m_Seed = false;
    m_LastVisited = 0;
    m_ConnectStart = 0;
    m_ConnectTime = 0;
    m_Pieces.clear();
    m_NumCompletedPieces = 0;
}
