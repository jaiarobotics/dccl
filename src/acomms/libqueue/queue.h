// copyright 2008, 2009 t. schneider tes@mit.edu 
//
// this file is part of the Queue Library (libqueue),
// the goby-acomms message queue manager. goby-acomms is a collection of 
// libraries for acoustic underwater networking
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

#ifndef Queue20080605H
#define Queue20080605H

#include <iostream>
#include <vector>
#include <deque>
#include <sstream>
#include <bitset>
#include <list>
#include <string>
#include <map>

#include <boost/algorithm/string.hpp>

#include "goby/util/time.h"
#include "goby/util/string.h"

#include "goby/acomms/protobuf/queue.pb.h"

typedef std::list<goby::acomms::protobuf::ModemDataTransmission>::iterator messages_it;
typedef std::multimap<unsigned, messages_it>::iterator waiting_for_ack_it;

namespace goby
{
    namespace acomms
    {
        class Queue
        {
          public:


            Queue(const protobuf::QueueConfig cfg = protobuf::QueueConfig(),
                  std::ostream* log = 0,
                  int modem_id = 0);

            bool push_message(const protobuf::ModemDataTransmission& data_msg);
            protobuf::ModemDataTransmission give_data(const protobuf::ModemDataRequest& request_msg);
            bool pop_message(unsigned frame);
            bool pop_message_ack(unsigned frame, protobuf::ModemDataTransmission* data_msg);
            void stream_for_pop(const protobuf::ModemDataTransmission& data_msg);


            std::vector<protobuf::ModemDataTransmission> expire();
          
            bool priority_values(double& priority,
                                 boost::posix_time::ptime& last_send_time,
                                 const protobuf::ModemDataRequest& request_msg,
                                 const protobuf::ModemDataTransmission& data_msg);
        
            void clear_ack_queue()
            { waiting_for_ack_.clear(); }

            void flush();
        
            size_t size() const 
            { return messages_.size(); }
    
            bool on_demand() const
            { return on_demand_; }

            boost::posix_time::ptime last_send_time() const
            { return last_send_time_; }

            boost::posix_time::ptime newest_msg_time() const
            {
                return size()
                    ? boost::posix_time::from_iso_string(messages_.back().base().iso_time())
                    : boost::posix_time::ptime();
            }
        
            void set_on_demand(bool b)
            { on_demand_ = b; }

            void set_on_demand(const std::string& s)
            { set_on_demand(util::string2bool(s)); }

            const protobuf::QueueConfig cfg() const
            { return cfg_; }
        
            std::string summary() const;
            
          private:
            waiting_for_ack_it find_ack_value(messages_it it_to_find);
            messages_it next_message_it();    
    
          private:
            const protobuf::QueueConfig cfg_;
        
            bool on_demand_;
    
            boost::posix_time::ptime last_send_time_;    
            
            std::ostream* log_;
    
            std::list<protobuf::ModemDataTransmission> messages_;

            // map frame number onto messages list iterator
            // can have multiples in the same frame now
            std::multimap<unsigned, messages_it> waiting_for_ack_;
    
        };
        std::ostream & operator<< (std::ostream & os, const Queue & oq);
    }

}
#endif
