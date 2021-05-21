#ifndef __HGO_PROTOCOL__
#define __HGO_PROTOCOL__
#include <string>
#include <iostream>


#define __HGO_PROTOCOL__SWITCH_TYPE(name)\
    case Message::TYPE::name :\
        return #name;

namespace HGO::P2P
{
    
    struct Message
    {
        using HGO_BYTE = unsigned char;
        enum class TYPE : unsigned int
        {
            NONE,
            PEER_PORT,
            PEER_LIST,
            MESSAGE
        };
        //-----------

        union
        {
            struct {
                HGO_BYTE isRequest      : 1;
                HGO_BYTE isResponse     : 1;
                HGO_BYTE isMasterNode   : 1;
                HGO_BYTE isOfficialNode : 1;
                HGO_BYTE isForward      : 1;
                HGO_BYTE isTest         : 1;
                HGO_BYTE reserved : 2;                
            } flags;
            HGO_BYTE full_header;
        } header;

        unsigned int msg_size = 0;
        TYPE msg_type; //unsigned int
        std::string str;

        inline const std::string data() const
        {
            std::string dt;
            // 1 Byte Header + 4 bytes for size + 4 bytes for type + size of str
            dt.reserve(9 + str.size());

            dt.push_back(header.full_header);
            dt.append(reinterpret_cast<const char *>(&msg_size), 4);
            dt.append(reinterpret_cast<const char *>(&msg_type), 4);
            dt.append(str);
            return dt;
        }

        inline static Message fromByteArray(const HGO_BYTE * bytes)
        {
            Message msg;
            msg.header.full_header = *bytes;
            bytes++; //advance on next char
            msg.msg_size = *reinterpret_cast<const unsigned int*>(bytes);

            bytes += sizeof(unsigned int); //advance 4 bytes
            msg.msg_type = *reinterpret_cast<const Message::TYPE*>(bytes);

            bytes += sizeof(unsigned int);
            msg.str = std::string(reinterpret_cast<const char *>(bytes), msg.msg_size);
            
            return msg;
        }
    };

    constexpr char const * MSG_TYPE_STR(const Message::TYPE & t )
    {
        switch(t)
        {
            __HGO_PROTOCOL__SWITCH_TYPE(NONE)
            __HGO_PROTOCOL__SWITCH_TYPE(MESSAGE)
            __HGO_PROTOCOL__SWITCH_TYPE(PEER_PORT)
            __HGO_PROTOCOL__SWITCH_TYPE(PEER_LIST)
            default:
                return "Unknown";
        }
    }

    inline std::ostream & operator<<(std::ostream & o, const Message & msg)
    {
        o<<"--MSG-- \n"
        <<"\t@@ Header @@ \n"
        <<"\t - isMasterNode : "<<(bool)msg.header.flags.isMasterNode<<"\n"
        <<"\t - isTest : "<<(bool)msg.header.flags.isTest<<"\n"
        <<"\t - isOfficialNode : "<<(bool)msg.header.flags.isOfficialNode<<"\n"
        <<"\t - isForward : "<<(bool)msg.header.flags.isForward<<"\n"
        <<"\t - isRequest : "<<(bool)msg.header.flags.isRequest<<"\n"
        <<"\t - isResponse : "<<(bool)msg.header.flags.isResponse<<"\n"
        <<"\t - reserved : "<<(int)msg.header.flags.reserved<<"\n";

        o<<"\t## BODY ## \n"
        <<"\t - size : "<<msg.msg_size<<"\n"
        <<"\t - msg_type : "<<MSG_TYPE_STR(msg.msg_type)<<"\n";

        o<<"\tData : \n"
        <<"\t|-- "<<msg.str;
        return o.flush();
    }
    

}


#endif