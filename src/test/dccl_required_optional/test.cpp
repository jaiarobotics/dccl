// Copyright 2009-2012 Toby Schneider (https://launchpad.net/~tes)
//                     Massachusetts Institute of Technology (2007-)
//                     Woods Hole Oceanographic Institution (2007-)
//                     Goby Developers Team (https://launchpad.net/~goby-dev)
// 
//
// This file is part of the Goby Underwater Autonomy Project Binaries
// ("The Goby Binaries").
//
// The Goby Binaries are free software: you can redistribute them and/or modify
// them under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The Goby Binaries are distributed in the hope that they will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Goby.  If not, see <http://www.gnu.org/licenses/>.


// tests required versus optional encoding of fields using a presence bit

#include "dccl/dccl.h"
#include "test.pb.h"

using dccl::operator<<;

int main(int argc, char* argv[])
{
    dccl::dlog.connect(dccl::logger::ALL, &std::cerr);
    
    dccl::Codec codec;
    
    codec.validate<BytesMsg>();
    codec.info<BytesMsg>(&dccl::dlog);

    BytesMsg msg_in;

    msg_in.set_req_bytes(goby::util::hex_decode("88abcd1122338754"));
    msg_in.set_opt_bytes(goby::util::hex_decode("102030adef2cb79d"));

    std::string encoded;
    codec.encode(&encoded, msg_in);
    
    BytesMsg msg_out;
    codec.decode(encoded, &msg_out);

    assert(msg_in.SerializeAsString() == msg_out.SerializeAsString());
    
    
    std::cout << "all tests passed" << std::endl;
}
