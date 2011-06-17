// copyright 2011 t. schneider tes@mit.edu
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

#ifndef MOOSNODE20110419H
#define MOOSNODE20110419H


#include "moos_serializer.h"

#include "goby/core/libcore/node_interface.h"
#include "goby/core/libcore/zeromq_service.h"

namespace goby
{
    namespace moos
    {        
        class MOOSNode : public goby::core::NodeInterface<CMOOSMsg>
        {
          protected:
            MOOSNode(goby::core::ZeroMQService* service);
            
            virtual ~MOOSNode()
            { }
            
            
            // not const because CMOOSMsg requires mutable for many const calls...
            virtual void moos_inbox(CMOOSMsg& msg) = 0;
            
            
            void send(const CMOOSMsg& msg, int socket_id);
            void subscribe(const std::string& full_or_partial_moos_name, int socket_id);
            void unsubscribe(const std::string& full_or_partial_moos_name, int socket_id);
            
            
            
            CMOOSMsg& newest(const std::string& key);
            
          private:
            void inbox(goby::core::MarshallingScheme marshalling_scheme,
                       const std::string& identifier,
                       const void* data,
                       int size,
                       int socket_id);

          private:
            std::map<std::string, boost::shared_ptr<CMOOSMsg> > newest_vars;
            
        };

        inline std::ostream& operator<<(std::ostream& os, const CMOOSMsg& msg)
        {
            os << "[[CMOOSMsg]]" << " Key: " << msg.GetKey()
               << " Type: "
               << (msg.IsDouble() ? "double" : "string")
               << " Value: " << (msg.IsDouble() ? goby::util::as<std::string>(msg.GetDouble()) : msg.GetString())
               << " Time: " << goby::util::unix_double2ptime(msg.GetTime())
               << " Community: " << msg.GetCommunity() 
               << " Source: " << msg.m_sSrc // no getter in CMOOSMsg!!!
               << " Source Aux: " << msg.GetSourceAux();
            return os;
        }
    }
}


#endif
