#include "../include/PacketAnalyzer.h"

QString PacketAnalyzer::analyze(const QString& packet)
{
    QString result;
    if ( packet.contains("[F.]") ) {
        result = "Packet is intended to terminate a connection: ";
    }
    else if ( packet.contains("[S]") )
    {
        result = "first packet - first step of  TCP three-way handshake: ";
    }
    else if ( packet.contains("[S.]") )
    {
        result = "second packet - second step of  TCP three-way handshake: ";
    }
    else if ( packet.contains("[P.]") )
    {
        result = "data packet: ";
    }
    else
    {
        result = "unknown type of packet: ";
    }
    return result;
}

