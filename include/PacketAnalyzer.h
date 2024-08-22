#pragma once

#include <QString>
#include <vector>

struct Packet {
    std::string timestamp;
    std::string sourceIP;
    std::string source_Port;
    std::string destIP;
    std::string destination_Port;
    std::string flags;
    unsigned long long seqStart;
    unsigned long long seqEnd;
    unsigned long long ack;
    std::string payload;
    int length = -1;

    std::string flag_type;
    std::string packet_classification;

    bool isSet = false;
};

class PacketAnalyzer
{
public:
    QString analyze( QString packet );

private:
    std::vector<std::string> splitPackets( const std::string& packets );
    Packet classifyPacket( const std::string& input_packet );
    std::string fromPacketToString( Packet packet );
};
