// copyright 2009 t. schneider tes@mit.edu
// 
// this file is part of goby-acomms, a collection of libraries for acoustic underwater networking
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this software.  If not, see <http://www.gnu.org/licenses/>.

#ifndef AcommsConstants20091122H
#define AcommsConstants20091122H

#include <string>
#include <limits>
#include <bitset>

#include <crypto++/filters.h>
#include <crypto++/hex.h>

#include "goby/acomms/protobuf/modem_message.pb.h"
#include "goby/core/core_constants.h"

#include <boost/bind.hpp>

namespace goby
{

    namespace acomms
    {
        const unsigned BITS_IN_BYTE = 8;
        // one hex char is a nibble (4 bits), two nibbles per byte
        const unsigned NIBS_IN_BYTE = 2;

        const int BROADCAST_ID = 0;
        const int QUERY_DESTINATION_ID = -1;
        
        const unsigned char DCCL_CCL_HEADER = 32;
    
        const double NaN = std::numeric_limits<double>::quiet_NaN();
        
        const unsigned DCCL_NUM_HEADER_BYTES = 6;

        const unsigned DCCL_NUM_HEADER_PARTS = 8;

        enum DCCLHeaderPart { head_ccl_id = 0,
                              head_dccl_id = 1,
                              head_time = 2,
                              head_src_id = 3,
                              head_dest_id = 4,
                              head_multimessage_flag = 5,
                              head_broadcast_flag = 6,
                              head_unused = 7
        };
    
        const std::string DCCL_HEADER_NAMES [] = { "_ccl_id",
                                                   "_id",
                                                   "_time",
                                                   "_src_id",
                                                   "_dest_id",
                                                   "_multimessage_flag",
                                                   "_broadcast_flag",
                                                   "_unused",
        };
        inline std::string to_str(DCCLHeaderPart p)
        {
            return DCCL_HEADER_NAMES[p];
        }

        
        enum DCCLHeaderBits { head_ccl_id_size = 8,
                              head_dccl_id_size = 9,
                              head_time_size = 17,
                              head_src_id_size = 5,
                              head_dest_id_size = 5,
                              head_flag_size = 1,
                              head_unused_size = 2
        };


        
            
        inline void hex_decode(const std::string& in, std::string* out)
        {
            CryptoPP::HexDecoder hex(new CryptoPP::StringSink(*out));
            hex.Put((byte*)in.c_str(), in.size());
            hex.MessageEnd();
        }

        inline std::string hex_decode(const std::string& in)
        {
            std::string out;
            hex_decode(in, &out);
            return out;
        }
    
        inline void hex_encode(const std::string& in, std::string* out)
        {
            const bool uppercase = false;
            CryptoPP::HexEncoder hex(new CryptoPP::StringSink(*out), uppercase);
            hex.Put((byte*)in.c_str(), in.size());
            hex.MessageEnd();
        }

        inline std::string hex_encode(const std::string& in)
        {
            std::string out;
            hex_encode(in, &out);
            return out;
        }
        
        // provides stream output operator for Google Protocol Buffers Message 
        inline std::ostream& operator<<(std::ostream& out, const google::protobuf::Message& msg)
        {
            return (out << "[[" << msg.GetDescriptor()->name() <<"]] " << msg.DebugString());
        }        
        
        
    }
}
#endif
