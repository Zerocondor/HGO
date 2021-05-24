#ifndef __HGO_PROTOCOL__
#define __HGO_PROTOCOL__
#include <string>
#include <iostream>
#include "../exceptions.h"

#define __HGO_PROTOCOL__SWITCH_TYPE(name)\
    case Message::TYPE::name :\
        return #name;

namespace HGO::NETWORK
{
    
    struct Message
    {
        using HGO_BYTE = unsigned char;
        enum class TYPE : unsigned int
        {
            NONE,
            PEER_INFORMATIONS,
            PEER_LIST,
            MESSAGE,
            NEW_TRANSACTION,
            NEW_BLOCK
        };
        //-----------
        Message() {
            header.config.full_header = 0;
        }
        struct
        {
            unsigned short magic_number = 0x07E0;
            union {
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
            } config;
        } header;

        unsigned int msg_size = 0;
        TYPE msg_type; //unsigned int
        std::string str;

        inline const std::string data() const
        {
            
            std::string dt;
            // 3 Byte Header + 4 bytes for size + 4 bytes for type + size of str
            dt.reserve(11 + str.size());

            dt.append(reinterpret_cast<const char *>(&header.magic_number), 2);
            dt.push_back(header.config.full_header);
            dt.append(reinterpret_cast<const char *>(&msg_size), 4);
            dt.append(reinterpret_cast<const char *>(&msg_type), 4);
            dt.append(str);
            return dt;
        }

        inline static Message fromByteArray(const HGO_BYTE * bytes)
        {
            Message msg;
            unsigned short magic_number = *reinterpret_cast<const unsigned short*>(bytes);
            if(magic_number != 0x07E0){
                throw HGO::EXCEPTION::P2PError("Unable to parse this message wrong magic number");
            }
            bytes += sizeof(unsigned short);
            msg.header.config.full_header = *bytes;
            bytes++; //advance on next char
            msg.msg_size = *reinterpret_cast<const unsigned int*>(bytes);

            bytes += sizeof(unsigned int); //advance 4 bytes
            msg.msg_type = *reinterpret_cast<const Message::TYPE*>(bytes);

            bytes += sizeof(unsigned int);
            msg.str = std::string(reinterpret_cast<const char *>(bytes), msg.msg_size);
            
            return msg;
        }
        inline static Message fromByteArray(const char * bytes)
        {          
            return fromByteArray(reinterpret_cast<const unsigned char *>(bytes));
        }
        inline static Message fromByteArray(const std::string & str)
        {          
            return fromByteArray(reinterpret_cast<const char *>(str.c_str()));
        }
    };

    constexpr char const * MSG_TYPE_STR(const Message::TYPE & t )
    {
        switch(t)
        {
            __HGO_PROTOCOL__SWITCH_TYPE(NONE)
            __HGO_PROTOCOL__SWITCH_TYPE(MESSAGE)
            __HGO_PROTOCOL__SWITCH_TYPE(PEER_INFORMATIONS)
            __HGO_PROTOCOL__SWITCH_TYPE(PEER_LIST)
            __HGO_PROTOCOL__SWITCH_TYPE(NEW_TRANSACTION)
            __HGO_PROTOCOL__SWITCH_TYPE(NEW_BLOCK)
            default:
                return "Unknown";
        }
    }

    inline std::ostream & operator<<(std::ostream & o, const Message & msg)
    {
        o<<"--MSG-- \n"
        <<"\t@@ Header @@ \n"
        <<"\t - isMasterNode : "<<(bool)msg.header.config.flags.isMasterNode<<"\n"
        <<"\t - isTest : "<<(bool)msg.header.config.flags.isTest<<"\n"
        <<"\t - isOfficialNode : "<<(bool)msg.header.config.flags.isOfficialNode<<"\n"
        <<"\t - isForward : "<<(bool)msg.header.config.flags.isForward<<"\n"
        <<"\t - isRequest : "<<(bool)msg.header.config.flags.isRequest<<"\n"
        <<"\t - isResponse : "<<(bool)msg.header.config.flags.isResponse<<"\n"
        <<"\t - reserved : "<<(int)msg.header.config.flags.reserved<<"\n";

        o<<"\t## BODY ## \n"
        <<"\t - size : "<<msg.msg_size<<"\n"
        <<"\t - msg_type : "<<MSG_TYPE_STR(msg.msg_type)<<"\n";

        o<<"\tData : \n"
        <<"\t|-- "<<msg.str;
        return o.flush();
    }
    

}


#endif