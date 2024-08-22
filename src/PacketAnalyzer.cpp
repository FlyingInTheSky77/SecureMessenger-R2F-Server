#include "../include/PacketAnalyzer.h"

#include <QDebug>
#include <regex>
#include <iostream>


std::vector<std::string> PacketAnalyzer::splitPackets(const std::string& packets) {
    std::vector<std::string> packet_list;
    std::regex pattern(R"(\d{2}:\d{2}:\d{2})");

    std::sregex_token_iterator iter(packets.begin(), packets.end(), pattern, {-1, 0});

    std::sregex_token_iterator end;
    std::string current_packet;

    int index = 0;
    for (; iter != end; ++iter) {
        if (!iter->str().empty()) {
            if (!current_packet.empty()) {
                packet_list.push_back(current_packet);
            }
            current_packet = iter->str();
        }
    }
    // add last packet:
   if (!current_packet.empty()) {
       packet_list.push_back(current_packet);
   }
   // workaround:
   std::vector<std::string> combined_result;
   std::string temp_string;
   int size_vec = packet_list.size();
   for (int i = 0; i < size_vec; ++i)
   {
       if (  ( (i % 2) != 0 ) ) {
           temp_string += packet_list[i];
           combined_result.push_back( temp_string );
           temp_string.clear();
       }
       else {
            temp_string = packet_list[i];
       }
   }

   return combined_result;
}

QString PacketAnalyzer::analyze( QString packet )
{
    std::vector<std::string> vec_separeted_packets = splitPackets( packet.toStdString() );

    Packet previous;
    Packet before_previous;
    bool isTwoStepsTcpSync = false;

    std::string analyzed_packets = "  =====  Packets_analysis  ==== =";
    for (const auto& item: vec_separeted_packets) {
        Packet current_packet = classifyPacket( item );
        qDebug() << "current_packet.flags " << QString::fromStdString( current_packet.flags );

        if ( isTwoStepsTcpSync && current_packet.flags == "Flags: ." ) {
            current_packet.packet_classification = "Packet Classification: THIRD packet - third step of  TCP three-way handshake. The client sends an ACK packet to acknowledge the server's SYN-ACK, completing the handshake";
            isTwoStepsTcpSync = false;
        }
        else if ( (!before_previous.isSet) && (current_packet.flags == "S") ) {
            before_previous = current_packet;
            before_previous.isSet = true;
        }
        else if ( !previous.isSet && current_packet.flags == "Flags: S." ) {
            previous = current_packet;
            previous.isSet = true;
            isTwoStepsTcpSync = true;
        }
        else {
            before_previous.isSet = false;
        }

        std::string current_packet_string = fromPacketToString(current_packet);
        analyzed_packets += current_packet_string;
    }

    return QString::fromStdString( analyzed_packets );
}

Packet PacketAnalyzer::classifyPacket( const std::string& input_packet )
{
    Packet packet;

        std::regex time_pattern(R"((\d{2}:\d{2}:\d{2}\.\d{6}))");
        std::smatch match;
        if (std::regex_search(input_packet, match, time_pattern)) {
            packet.timestamp = "Time: " + match.str(1);
        }

        std::regex src_pattern(R"(IP ([^\s]+)\.(\d+) >)");
        if (std::regex_search(input_packet, match, src_pattern)) {
            packet.sourceIP = "Source IP: " + match.str(1);
            packet.source_Port = "Source Port: " + match.str(2);
        }

        std::regex dest_pattern(R"(> ([^\s]+)\.(\d+):)");
        if (std::regex_search(input_packet, match, dest_pattern)) {
            packet.destIP = "Destination IP: " + match.str(1);
            packet.destination_Port = "Destination Port: " + match.str(2);
        }

        std::regex flags_pattern(R"(Flags \[([A-Z.]+)\])");        
        if (std::regex_search(input_packet, match, flags_pattern)) {
            packet.flags = "Flags: " + match.str(1);

            if (match.str(1)=="S.") {
                packet.flag_type = "Flag type - two flags: SYN (Synchronize) + ACK (Acknowledgment)";
                packet.packet_classification = "Packet Classification: Initiates a connection: SECOND packet - second step of  TCP three-way handshake";
            } else if(match.str(1).find('S') != std::string::npos) {
                packet.flag_type = "Flag type: SYN (Synchronize)";
                packet.packet_classification = "Packet Classification: Initiates a connection: FIRST packet - first step of  TCP three-way handshake";
            } else if (match.str(1).find('F') != std::string::npos) {
                packet.flag_type = "Flag type: FIN flag";
                packet.packet_classification = "Packet Classification: Packet is intended to terminate a connection";
            } else if (match.str(1)=="P.") {
                    packet.flag_type = "Flag type - two flags: Push + ACK - both the Push flag and the ACK flag are set";
                    packet.packet_classification = "Packet Classification: meaning the segment is acknowledging receipt of data and also pushing new data to the application on the other side";
            } else if (match.str(1).find('P') != std::string::npos) {
                packet.flag_type = "Flag type: PUSH flag";
                packet.packet_classification = "Packet Classification: ";
            } else if (match.str(1).find('R') != std::string::npos) {
                packet.flag_type = "Flag type: RST (Reset) = Resets a connection";
                packet.packet_classification = "Packet Classification: It indicates that something has gone wrong, and the connection should be aborted";
            } else if (match.str(1).find('A') != std::string::npos) {
                packet.flag_type = "Flag type: ACK flag";
                packet.packet_classification = "Packet Classification: ";
            } else if (match.str(1).find('.') != std::string::npos) {
                packet.flag_type = "Flag type: ACK (Acknowledgment)";
                packet.packet_classification = "Packet Classification: Only acknowledge receipt of data (ACK), but do not contain data";
            } else if (match.str(1).find('U') != std::string::npos) {
                packet.flag_type = "Flag type: URG (Urgent)";
                packet.packet_classification = "Packet Classification: Marks the packet as containing urgent data";
            } else {
                packet.flag_type = "Packet Classification: Other flags";
                packet.packet_classification =  "Packet Classification: no way to classify";
            }
        }

        std::regex seq_ack_pattern(R"(seq (\d+:\d+), ack (\d+))");
        if (std::regex_search(input_packet, match, seq_ack_pattern)) {
            // TODO: add in future, if you will need it
        }

        std::regex payload_pattern(R"(\{.*\})");
        if (std::regex_search(input_packet, match, payload_pattern)) {
            packet.payload = match.str();
        } else {
            packet.payload = "No payload found.";
        }

        std::regex length_patter(R"(length (\d+))");
        if (std::regex_search(input_packet, match, length_patter)) {
              packet.length = std::stoi(match.str(1));
        }

        return packet;
}

std::string PacketAnalyzer::fromPacketToString( Packet packet )
{
    std::string packet_in_string = "\n" + packet.timestamp + "\n";
    packet_in_string += packet.sourceIP + "\n";
    packet_in_string += packet.source_Port + "\n";
    packet_in_string += packet.destIP + "\n";
    packet_in_string += packet.destination_Port + "\n";
    packet_in_string += packet.flags + "\n";
    packet_in_string += packet.flag_type + "\n";
    packet_in_string += packet.packet_classification + "\n";
    packet_in_string += "payload: " + packet.payload + "\n";
    packet_in_string += "length: " + std::to_string( packet.length ) + "\n";
    packet_in_string += "====  Packet_separator  =====";
    qDebug() << "packet_in_string: " << QString::fromStdString( "length: " + std::to_string( packet.length ) + "\n" );
    return packet_in_string;
}
