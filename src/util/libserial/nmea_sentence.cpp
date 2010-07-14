// copyright 2009 t. schneider tes@mit.edu
//           2010 c. murphy    cmurphy@whoi.edu
// 
// this file is part of serial, a library for handling serial
// communications.
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

#include "nmea_sentence.h"

#include <cstdio>

serial::NMEASentence::NMEASentence(std::string s, strategy cs_strat = VALIDATE)
  : std::vector<std::string>() {
    bool found_csum = false;
    unsigned int cs;
    // Silently drop leading/trailing whitespace if present.
    boost::trim(s);
    // Basic error checks ($, empty)
    if (s.empty())
      throw std::runtime_error("NMEASentence: no message provided.");
    if (s[0] != '$')
      throw std::runtime_error("NMEASentence: no $: '" + s + "'.");
    // Check if the checksum exists and is correctly placed, and strip it.
    // If it's not correctly placed, we'll interpret it as part of message.
    // NMEA spec doesn't seem to say that * is forbidden elsewhere? (should be)
    if (s.size() > 3 && s.at(s.size()-3) == '*') {
      std::string hex_csum = s.substr(s.size()-2);
      found_csum = tes_util::hex_string2number(hex_csum, cs);
      s = s.substr(0, s.size()-3);
    }
    // If we require a checksum and haven't found one, fail.
    if (cs_strat == REQUIRE and !found_csum)
      throw std::runtime_error("NMEASentence: no checksum: '" + s + "'.");
    // If we found a bad checksum and we care, fail.
    if (found_csum && (cs_strat == REQUIRE || cs_strat == VALIDATE)) {
      unsigned char calc_cs = NMEASentence::checksum(s);
      if (calc_cs != cs)
        throw std::runtime_error("NMEASentence: bad checksum: '" + s + "'.");
    }
    // Split string into parts.
    boost::split(*(std::vector<std::string>*)this, s, boost::is_any_of(","));
    // Validate talker size.
    if (this->front().size() != 6)
      throw std::runtime_error("NMEASentence: bad talker length '" + s + "'.");
}

unsigned char serial::NMEASentence::checksum(const std::string& s) {
    unsigned char csum = 0;

    if(s.empty())
      throw std::runtime_error("NMEASentence::checksum: no message provided.");
    std::string::size_type star = s.find_first_of("*\r\n");
    std::string::size_type dollar = s.find('$');
    
    if(dollar == std::string::npos)
      throw std::runtime_error("NMEASentence::checksum: no $ found.");

    if(star == std::string::npos) star = s.length();
    
    for(std::string::size_type i = dollar+1; i < star; ++i) csum ^= s[i];
    return csum;
}

std::string serial::NMEASentence::message_no_cs() const {
    std::string message = "";

    for(const_iterator it = begin(), n = end(); it < n; ++it)
      message += *it + ",";

    // kill last ","
    message.resize(message.size()-1);
    return message;
}

std::string serial::NMEASentence::message() const {
    std::string bare = message_no_cs();
    std::stringstream message;
    unsigned char csum = NMEASentence::checksum(bare);
    message << bare << "*";
    message << std::uppercase << std::hex << unsigned(csum);
    return message.str();
}
