// Copyright 2009-2012 Toby Schneider (https://launchpad.net/~tes)
//                     Massachusetts Institute of Technology (2007-)
//                     Woods Hole Oceanographic Institution (2007-)
//                     Goby Developers Team (https://launchpad.net/~goby-dev)
// 
//
// This file is part of the Goby Underwater Autonomy Project Libraries
// ("The Goby Libraries").
//
// The Goby Libraries are free software: you can redistribute them and/or modify
// them under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The Goby Libraries are distributed in the hope that they will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Goby.  If not, see <http://www.gnu.org/licenses/>.


#ifndef DCCL20091211H
#define DCCL20091211H

#include <string>
#include <set>
#include <map>
#include <ostream>
#include <stdexcept>
#include <vector>

#include <google/protobuf/descriptor.h>

#include <boost/shared_ptr.hpp>

#include "goby/common/time.h"
#include "goby/common/logger.h"
#include "goby/acomms/acomms_helpers.h"
#include "goby/util/binary.h"
#include "goby/util/dynamic_protobuf_manager.h"

#include "dccl/protobuf/dccl.pb.h"
#include "logger.h"
#include "protobuf_cpp_type_helpers.h"
#include "dccl_exception.h"
#include "dccl_field_codec.h"
#include "dccl_field_codec_fixed.h"
#include "dccl_field_codec_default.h"
#include "dccl_type_helper.h"
#include "dccl_field_codec_manager.h"
 
/// Objects pertaining to acoustic communications (acomms)
namespace dccl
{
    class DCCLFieldCodec;
  
    /// \class Codec dccl/dccl.h dccl/dccl.h
    /// \brief provides an API to the Dynamic CCL Codec.
    /// \ingroup acomms_api
    /// \ingroup dccl_api
    /// \sa acomms_dccl.proto and acomms_modem_message.proto for definition of Google Protocol Buffers messages (namespace dccl::protobuf).
    ///
    /// Simple usage example:
    /// 1. Define a Google Protobuf message with DCCL extensions:
    /// \verbinclude simple.proto
    /// 2. Write a bit of code like this:
    /// \code
    /// dccl::Codec* dccl = dccl::Codec::get();
    /// dccl->validate<Simple>();
    /// Simple message_out;
    /// message_out.set_telegram("Hello!");
    /// std::string bytes;
    /// dccl->encode(&bytes, message);
    /// \\ send bytes across some network
    /// Simple message_in;
    /// dccl->decode(bytes&, message_in);
    /// \endcode
    /// \example acomms/chat/chat.cpp
    /// \example acomms/dccl/dccl_simple/dccl_simple.cpp
    /// simple.proto
    /// \verbinclude simple.proto
    /// dccl_simple.cpp
    /// \example acomms/dccl/two_message/two_message.cpp
    /// two_message.proto
    /// \verbinclude two_message.proto
    /// two_message.cpp
    class Codec
    {
      public:
        static const std::string DEFAULT_CODEC_NAME;
        
        Codec(const std::string& dccl_id_codec = "_default_id_codec");
        virtual ~Codec() { }

        /// \brief Load any codecs present in the given shared library handle
        ///
        /// Codecs must be loaded within the shared library using a C function
        /// (declared extern "C") called "goby_dccl_load" with the signature
        /// void goby_dccl_load(dccl::Codec* codec)
        void load_library(void* dl_handle);

        /// \brief Load any codecs present in the given shared library handle
        ///
        /// Codecs must be loaded within the shared library using a C function
        /// (declared extern "C") called "goby_dccl_load" with the signature
        /// void goby_dccl_load(dccl::Codec* codec)
        void load_library(const std::string& library_path);
        
        /// \brief All messages must be validated (size checks, option extensions checks, etc.) before they can be encoded/decoded. Use this form when the messages used are static (known at compile time).
        ///
        /// \tparam ProtobufMessage Any Google Protobuf Message generated by protoc (i.e. subclass of google::protobuf::Message)
        /// \throw DCCLException if message is invalid. Warnings and errors are written to dccl::dlog.
        template<typename ProtobufMessage>
            void load()
        { load(ProtobufMessage::descriptor()); }

        /// \brief An alterative form for validating messages for message types <i>not</i> known at compile-time ("dynamic").
        ///
        /// \param desc The Google Protobuf "Descriptor" (meta-data) of the message to validate.
        /// \throw DCCLException if message is invalid.
        void load(const google::protobuf::Descriptor* desc);

        void set_crypto_passphrase(const std::string& passphrase);
            
        //@}
            
        /// \name Informational Methods.
        ///
        /// Provides various forms of information about the Codec
        //@{

        /// \brief Writes a human readable summary (including field sizes) of the provided DCCL type to the stream provided.
        ///
        /// \tparam ProtobufMessage Any Google Protobuf Message generated by protoc (i.e. subclass of google::protobuf::Message)
        /// \param os Pointer to a stream to write this information
        template<typename ProtobufMessage>
            void info(std::ostream* os) const
        { info(ProtobufMessage::descriptor(), os); }

        /// \brief An alterative form for getting information for messages for message types <i>not</i> known at compile-time ("dynamic").
        void info(const google::protobuf::Descriptor* desc, std::ostream* os) const;

        /// \brief Writes a human readable summary (including field sizes) of all the loaded (validated) DCCL types
        ///
        /// \param os Pointer to a stream to write this information            
        void info_all(std::ostream* os) const;
            
        /// \brief Gives the DCCL id (defined by the custom message option extension "(goby.msg).dccl.id" in the .proto file). This ID is used on the wire to unique identify incoming message types.
        ///
        /// \tparam ProtobufMessage Any Google Protobuf Message generated by protoc (i.e. subclass of google::protobuf::Message)
        template <typename ProtobufMessage>
            unsigned id() const
        { return id(ProtobufMessage::descriptor()); }            

        /// \brief Get the DCCL ID of an unknown encoded DCCL message.
        /// 
        /// You can use this method along with id() to handle multiple types of known (static) incoming DCCL messages. For example:
        /// \code
        /// unsigned dccl_id = codec->id_from_encoded(bytes);    
        /// if(dccl_id == codec->id<MyProtobufType1>())
        /// {
        ///     MyProtobufType1 msg_out1;
        ///     codec->decode(bytes, &msg_out1);
        /// }
        /// else if(dccl_id == codec->id<MyProtobufType2>())
        /// {
        ///     MyProtobufType2 msg_out2;
        ///     codec->decode(bytes, &msg_out2);
        /// }
        /// \endcode
        /// \param bytes encoded message to get the DCCL ID of
        /// \return DCCL ID
        /// \sa test/acomms/dccl8/test.cpp and test/acomms/dccl8/test.proto
        unsigned id(const std::string& bytes);

        /// \brief Provides the DCCL ID given a DCCL type.
        unsigned id(const google::protobuf::Descriptor* desc) const {
            return desc->options().GetExtension(dccl::msg).id();
        }            

        //@}
            
        /// \brief Provides the encoded size (in bytes) of msg. This is useful if you need to know the size of a message before encoding it (encoding it is generally much more expensive than calling this method)
        ///
        /// \param msg Google Protobuf message with DCCL extensions for which the encoded size is requested
        /// \return Encoded (using DCCL) size in bytes
        unsigned size(const google::protobuf::Message& msg);
            
        //@}
       
        /// \name Codec functions.
        ///
        /// This is where the real work happens.
        //@{

        /// \brief Encodes a DCCL message
        ///
        /// \param bytes Pointer to byte string to store encoded msg
        /// \param msg Message to encode (must already have been validated)
        /// \throw DCCLException if message cannot be encoded.
        void encode(std::string* bytes, const google::protobuf::Message& msg);
            
        /// \brief Decode a DCCL message when the type is known at compile time.
        ///
        /// \param bytes encoded message to decode (must already have been validated)
        /// \param msg Pointer to any Google Protobuf Message generated by protoc (i.e. subclass of google::protobuf::Message). The decoded message will be written here.
        /// \throw DCCLException if message cannot be decoded.
        void decode(const std::string& bytes, google::protobuf::Message* msg);

        /// \brief Decode a DCCL message when the type is known at compile time.
        ///
        /// \param bytes encoded message to decode (must already have been validated) which will have the used bytes stripped from the front of the message
        /// \param msg Pointer to any Google Protobuf Message generated by protoc (i.e. subclass of google::protobuf::Message). The decoded message will be written here.
        /// \throw DCCLException if message cannot be decoded.
        void decode(std::string* bytes, google::protobuf::Message* msg);

        /// \brief An alterative form for decoding messages for message types <i>not</i> known at compile-time ("dynamic").
        ///
        /// \tparam GoogleProtobufMessagePointer anything that acts like a pointer (has operator*) to a google::protobuf::Message (smart pointers like boost::shared_ptr included)
        /// \param bytes the byte string returned by encode
        /// \throw DCCLException if message cannot be decoded
        /// \return pointer to decoded message (a google::protobuf::Message). You are responsible for deleting the memory used by this pointer. This message can be examined using the Google Reflection/Descriptor API.
        template<typename GoogleProtobufMessagePointer>
            GoogleProtobufMessagePointer decode(const std::string& bytes)
        {
            // ownership of this object goes to the caller of decode()
            unsigned this_id = id(bytes);   

            if(!id2desc_.count(this_id))
                throw(DCCLException("Message id " + goby::util::as<std::string>(this_id) + " has not been validated. Call validate() before decoding this type."));
                    
            GoogleProtobufMessagePointer msg =
                goby::util::DynamicProtobufManager::new_protobuf_message<GoogleProtobufMessagePointer>(id2desc_.find(this_id)->second);
            decode(bytes, &(*msg));
            return msg;
        }

      private:
        Codec(const Codec&);
        Codec& operator= (const Codec&);

        void encrypt(std::string* s, const std::string& nonce);
        void decrypt(std::string* s, const std::string& nonce);

        void set_default_codecs();

        boost::shared_ptr<DCCLFieldCodecBase> id_codec() const
        {
            return DCCLFieldCodecManager::find(google::protobuf::FieldDescriptor::TYPE_INT32,
                                               id_codec_);
        }
        
        
      private:
        // SHA256 hash of the crypto passphrase
        std::string crypto_key_;
        
        // maps `dccl.id`s onto Message Descriptors
        std::map<int32, const google::protobuf::Descriptor*> id2desc_;
        std::string id_codec_;

    };

    inline std::ostream& operator<<(std::ostream& os, const Codec& codec)
    {
        codec.info_all(&os);
        return os;
    }
}

#endif
