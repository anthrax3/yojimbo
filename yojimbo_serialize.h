/*
    Yojimbo Network Library.
    
    Copyright © 2016 - 2017, The Network Protocol Company, Inc.
*/

#ifndef YOJIMBO_SERIALIZE_H
#define YOJIMBO_SERIALIZE_H

#include "yojimbo_config.h"
#include "yojimbo_bitpack.h"
#include "yojimbo_stream.h"
#include "yojimbo_address.h"

/** @file */

namespace yojimbo
{
    /**
        Serialize integer value (read/write/measure).

        This is a helper macro to make writing unified serialize functions easier.

        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.

        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.

        @param stream The stream object. May be a read, write or measure stream.
        @param value The integer value to serialize in [min,max].
        @param min The minimum value.
        @param max The maximum value.
     */

    #define serialize_int( stream, value, min, max )                    \
        do                                                              \
        {                                                               \
            yojimbo_assert( min < max );                                \
            int32_t int32_value = 0;                                    \
            if ( Stream::IsWriting )                                    \
            {                                                           \
                yojimbo_assert( int64_t(value) >= int64_t(min) );       \
                yojimbo_assert( int64_t(value) <= int64_t(max) );       \
                int32_value = (int32_t) value;                          \
            }                                                           \
            if ( !stream.SerializeInteger( int32_value, min, max ) )    \
                return false;                                           \
            if ( Stream::IsReading )                                    \
            {                                                           \
                value = int32_value;                                    \
                if ( int64_t(value) < int64_t(min) ||                   \
                     int64_t(value) > int64_t(max) )                    \
                    return false;                                       \
            }                                                           \
        } while (0)

    /**
        Serialize bits to the stream (read/write/measure).

        This is a helper macro to make writing unified serialize functions easier.

        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.

        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.

        @param stream The stream object. May be a read, write or measure stream.
        @param value The unsigned integer value to serialize.
        @param bits The number of bits to serialize in [1,32].
     */

    #define serialize_bits( stream, value, bits )                       \
        do                                                              \
        {                                                               \
            yojimbo_assert( bits > 0 );                                 \
            yojimbo_assert( bits <= 32 );                               \
            uint32_t uint32_value = 0;                                  \
            if ( Stream::IsWriting )                                    \
                uint32_value = (uint32_t) value;                        \
            if ( !stream.SerializeBits( uint32_value, bits ) )          \
                return false;                                           \
            if ( Stream::IsReading )                                    \
                value = uint32_value;                                   \
        } while (0)

    /**
        Serialize a boolean value to the stream (read/write/measure).

        This is a helper macro to make writing unified serialize functions easier.

        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.

        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.

        @param stream The stream object. May be a read, write or measure stream.
        @param value The boolean value to serialize.
     */

    #define serialize_bool( stream, value )                             \
        do                                                              \
        {                                                               \
            uint32_t uint32_bool_value = 0;                             \
            if ( Stream::IsWriting )                                    \
                uint32_bool_value = value ? 1 : 0;                      \
            serialize_bits( stream, uint32_bool_value, 1 );             \
            if ( Stream::IsReading )                                    \
                value = uint32_bool_value ? true : false;               \
        } while (0)

    template <typename Stream> bool serialize_float_internal( Stream & stream, float & value )
    {
        uint32_t int_value;

        if ( Stream::IsWriting )
            memcpy( &int_value, &value, 4 );

        bool result = stream.SerializeBits( int_value, 32 );

        if ( Stream::IsReading )
            memcpy( &value, &int_value, 4 );

        return result;
    }

    /**
        Serialize floating point value (read/write/measure).

        This is a helper macro to make writing unified serialize functions easier.

        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.

        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.

        @param stream The stream object. May be a read, write or measure stream.
        @param value The float value to serialize.
     */

    #define serialize_float( stream, value )                                        \
        do                                                                          \
        {                                                                           \
            if ( !yojimbo::serialize_float_internal( stream, value ) )              \
                return false;                                                       \
        } while (0)

    /**
        Serialize a 32 bit unsigned integer to the stream (read/write/measure).

        This is a helper macro to make unified serialize functions easier.

        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.

        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.

        @param stream The stream object. May be a read, write or measure stream.
        @param value The unsigned 32 bit integer value to serialize.
     */

    #define serialize_uint32( stream, value )                                       \
        serialize_bits( stream, value, 32 );

    template <typename Stream> bool serialize_uint64_internal( Stream & stream, uint64_t & value )
    {
        uint32_t hi = 0, lo = 0;
        if ( Stream::IsWriting )
        {
            lo = value & 0xFFFFFFFF;
            hi = value >> 32;
        }
        serialize_bits( stream, lo, 32 );
        serialize_bits( stream, hi, 32 );
        if ( Stream::IsReading )
            value = ( uint64_t(hi) << 32 ) | lo;
        return true;
    }

    /**
        Serialize a 64 bit unsigned integer to the stream (read/write/measure).

        This is a helper macro to make unified serialize functions easier.

        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.

        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.

        @param stream The stream object. May be a read, write or measure stream.
        @param value The unsigned 64 bit integer value to serialize.
     */

    #define serialize_uint64( stream, value )                                       \
        do                                                                          \
        {                                                                           \
            if ( !yojimbo::serialize_uint64_internal( stream, value ) )             \
                return false;                                                       \
        } while (0)

    template <typename Stream> bool serialize_double_internal( Stream & stream, double & value )
    {
        union DoubleInt
        {
            double double_value;
            uint64_t int_value;
        };

        DoubleInt tmp = { 0 };
        if ( Stream::IsWriting )
            tmp.double_value = value;

        serialize_uint64( stream, tmp.int_value );

        if ( Stream::IsReading )
            value = tmp.double_value;

        return true;
    }

    /**
        Serialize double precision floating point value to the stream (read/write/measure).

        This is a helper macro to make writing unified serialize functions easier.

        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.

        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.

        @param stream The stream object. May be a read, write or measure stream.
        @param value The double precision floating point value to serialize.
     */

    #define serialize_double( stream, value )                                       \
        do                                                                          \
        {                                                                           \
            if ( !yojimbo::serialize_double_internal( stream, value ) )             \
                return false;                                                       \
        } while (0)

    template <typename Stream> bool serialize_bytes_internal( Stream & stream, uint8_t * data, int bytes )
    {
        return stream.SerializeBytes( data, bytes );
    }

    /**
        Serialize an array of bytes to the stream (read/write/measure).

        This is a helper macro to make unified serialize functions easier.

        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.

        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.

        @param stream The stream object. May be a read, write or measure stream.
        @param data Pointer to the data to be serialized.
        @param bytes The number of bytes to serialize.
     */

    #define serialize_bytes( stream, data, bytes )                                  \
        do                                                                          \
        {                                                                           \
            if ( !yojimbo::serialize_bytes_internal( stream, data, bytes ) )        \
                return false;                                                       \
        } while (0)

    template <typename Stream> bool serialize_string_internal( Stream & stream, char * string, int buffer_size )
    {
        int length = 0;
        if ( Stream::IsWriting )
        {
            length = (int) strlen( string );
            yojimbo_assert( length < buffer_size - 1 );
        }
        serialize_int( stream, length, 0, buffer_size - 1 );
        serialize_bytes( stream, (uint8_t*)string, length );
        if ( Stream::IsReading )
            string[length] = '\0';
        return true;
    }

    /**
        Serialize a string to the stream (read/write/measure).

        This is a helper macro to make writing unified serialize functions easier.

        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.

        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.

        @param stream The stream object. May be a read, write or measure stream.
        @param string The string to serialize write/measure. Pointer to buffer to be filled on read.
        @param buffer_size The size of the string buffer. String with terminating null character must fit into this buffer.
     */

    #define serialize_string( stream, string, buffer_size )                                 \
        do                                                                                  \
        {                                                                                   \
            if ( !yojimbo::serialize_string_internal( stream, string, buffer_size ) )       \
                return false;                                                               \
        } while (0)

    /**
        Serialize an alignment to the stream (read/write/measure).

        This is a helper macro to make writing unified serialize functions easier.

        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.

        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.

        @param stream The stream object. May be a read, write or measure stream.
     */

    #define serialize_align( stream )                                                       \
        do                                                                                  \
        {                                                                                   \
            if ( !stream.SerializeAlign() )                                                 \
                return false;                                                               \
        } while (0)

    /**
        Serialize a safety check to the stream (read/write/measure).

        This is a helper macro to make writing unified serialize functions easier.

        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.

        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.

        @param stream The stream object. May be a read, write or measure stream.
     */

    #define serialize_check( stream )                                                       \
        do                                                                                  \
        {                                                                                   \
            if ( !stream.SerializeCheck() )                                                 \
                return false;                                                               \
        } while (0)

    /**
        Serialize an object to the stream (read/write/measure).

        This is a helper macro to make writing unified serialize functions easier.

        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.

        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.

        @param stream The stream object. May be a read, write or measure stream.
        @param object The object to serialize. Must have a serialize method on it.
     */

    #define serialize_object( stream, object )                                              \
        do                                                                                  \
        {                                                                                   \
            if ( !object.Serialize( stream ) )                                              \
                return false;                                                               \
        }                                                                                   \
        while(0)

    template <typename Stream> bool serialize_address_internal( Stream & stream, Address & address )
    {
        char buffer[MaxAddressLength];

        if ( Stream::IsWriting )
        {
            yojimbo_assert( address.IsValid() );
            address.ToString( buffer, sizeof( buffer ) );
        }

        serialize_string( stream, buffer, sizeof( buffer ) );

        if ( Stream::IsReading )
        {
            address = Address( buffer );
            if ( !address.IsValid() )
                return false;
        }

        return true;
    }

    /**
        Serialize an address to the stream (read/write/measure).

        This is a helper macro to make writing unified serialize functions easier.

        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.

        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.

        @param stream The stream object. May be a read, write or measure stream.
        @param value The address to serialize. Must be a valid address.
     */

    #define serialize_address( stream, value )                                              \
        do                                                                                  \
        {                                                                                   \
            if ( !yojimbo::serialize_address_internal( stream, value ) )                    \
                return false;                                                               \
        } while (0)

    template <typename Stream, typename T> bool serialize_int_relative_internal( Stream & stream, T previous, T & current )
    {
        uint32_t difference = 0;

        if ( Stream::IsWriting )
        {
            yojimbo_assert( previous < current );
            difference = current - previous;
        }

        bool oneBit = false;
        if ( Stream::IsWriting )
            oneBit = difference == 1;
        serialize_bool( stream, oneBit );
        if ( oneBit )
        {
            if ( Stream::IsReading )
                current = previous + 1;
            return true;
        }

        bool twoBits = false;
        if ( Stream::IsWriting )
            twoBits = difference <= 6;
        serialize_bool( stream, twoBits );
        if ( twoBits )
        {
            serialize_int( stream, difference, 2, 6 );
            if ( Stream::IsReading )
                current = previous + difference;
            return true;
        }

        bool fourBits = false;
        if ( Stream::IsWriting )
            fourBits = difference <= 23;
        serialize_bool( stream, fourBits );
        if ( fourBits )
        {
            serialize_int( stream, difference, 7, 23 );
            if ( Stream::IsReading )
                current = previous + difference;
            return true;
        }

        bool eightBits = false;
        if ( Stream::IsWriting )
            eightBits = difference <= 280;
        serialize_bool( stream, eightBits );
        if ( eightBits )
        {
            serialize_int( stream, difference, 24, 280 );
            if ( Stream::IsReading )
                current = previous + difference;
            return true;
        }

        bool twelveBits = false;
        if ( Stream::IsWriting )
            twelveBits = difference <= 4377;
        serialize_bool( stream, twelveBits );
        if ( twelveBits )
        {
            serialize_int( stream, difference, 281, 4377 );
            if ( Stream::IsReading )
                current = previous + difference;
            return true;
        }

        bool sixteenBits = false;
        if ( Stream::IsWriting ) 
            sixteenBits = difference <= 69914;
        serialize_bool( stream, sixteenBits );
        if ( sixteenBits )
        {
            serialize_int( stream, difference, 4378, 69914 );
            if ( Stream::IsReading )
                current = previous + difference;
            return true;
        }

        uint32_t value = current;
        serialize_uint32( stream, value );
        if ( Stream::IsReading )
            current = value;

        return true;
    }

    /**
        Serialize an integer value relative to another (read/write/measure).

        This is a helper macro to make writing unified serialize functions easier.

        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.

        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.

        @param stream The stream object. May be a read, write or measure stream.
        @param previous The previous integer value.
        @param current The current integer value.
     */

    #define serialize_int_relative( stream, previous, current )                             \
        do                                                                                  \
        {                                                                                   \
            if ( !yojimbo::serialize_int_relative_internal( stream, previous, current ) )   \
                return false;                                                               \
        } while (0)

    template <typename Stream> bool serialize_ack_relative_internal( Stream & stream, uint16_t sequence, uint16_t & ack )
    {
        int ack_delta = 0;
        bool ack_in_range = false;

        if ( Stream::IsWriting )
        {
            if ( ack < sequence )
                ack_delta = sequence - ack;
            else
                ack_delta = (int)sequence + 65536 - ack;

            yojimbo_assert( ack_delta > 0 );
            yojimbo_assert( uint16_t( sequence - ack_delta ) == ack );
            
            ack_in_range = ack_delta <= 64;
        }

        serialize_bool( stream, ack_in_range );

        if ( ack_in_range )
        {
            serialize_int( stream, ack_delta, 1, 64 );
            if ( Stream::IsReading )
                ack = sequence - ack_delta;
        }
        else
        {
            serialize_bits( stream, ack, 16 );
        }

        return true;
    }

    /**
        Serialize an ack relative to the current sequence number (read/write/measure).

        This is a helper macro to make writing unified serialize functions easier.

        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.

        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.

        @param stream The stream object. May be a read, write or measure stream.
        @param sequence The current sequence number.
        @param ack The ack sequence number, which is typically near the current sequence number.
     */

    #define serialize_ack_relative( stream, sequence, ack  )                                        \
        do                                                                                          \
        {                                                                                           \
            if ( !yojimbo::serialize_ack_relative_internal( stream, sequence, ack ) )               \
                return false;                                                                       \
        } while (0)

    template <typename Stream> bool serialize_sequence_relative_internal( Stream & stream, uint16_t sequence1, uint16_t & sequence2 )
    {
        if ( Stream::IsWriting )
        {
            uint32_t a = sequence1;
            uint32_t b = sequence2 + ( ( sequence1 > sequence2 ) ? 65536 : 0 );
            serialize_int_relative( stream, a, b );
        }
        else
        {
            uint32_t a = sequence1;
            uint32_t b = 0;
            serialize_int_relative( stream, a, b );
            if ( b >= 65536 )
                b -= 65536;
            sequence2 = uint16_t( b );
        }

        return true;
    }

    /**
        Serialize a sequence number relative to another (read/write/measure).

        This is a helper macro to make writing unified serialize functions easier.

        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.

        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.

        @param stream The stream object. May be a read, write or measure stream.
        @param sequence1 The first sequence number to serialize relative to.
        @param sequence2 The second sequence number to be encoded relative to the first.
     */

    #define serialize_sequence_relative( stream, sequence1, sequence2 )                             \
        do                                                                                          \
        {                                                                                           \
            if ( !yojimbo::serialize_sequence_relative_internal( stream, sequence1, sequence2 ) )   \
                return false;                                                                       \
        } while (0)

    // read macros corresponding to each serialize_*. useful when you want separate read and write functions for some reason.

    #define read_bits( stream, value, bits )                                                \
    do                                                                                      \
    {                                                                                       \
        yojimbo_assert( bits > 0 );                                                         \
        yojimbo_assert( bits <= 32 );                                                       \
        uint32_t uint32_value= 0;                                                           \
        if ( !stream.SerializeBits( uint32_value, bits ) )                                  \
            return false;                                                                   \
        value = uint32_value;                                                               \
    } while (0)

    #define read_int( stream, value, min, max )                                             \
        do                                                                                  \
        {                                                                                   \
            yojimbo_assert( min < max );                                                    \
            int32_t int32_value = 0;                                                        \
            if ( !stream.SerializeInteger( int32_value, min, max ) )                        \
                return false;                                                               \
            value = int32_value;                                                            \
            if ( value < min || value > max )                                               \
                return false;                                                               \
        } while (0)

    #define read_bool( stream, value ) read_bits( stream, value, 1 )

    #define read_float                  serialize_float
    #define read_uint32                 serialize_uint32
    #define read_uint64                 serialize_uint64
    #define read_double                 serialize_double
    #define read_bytes                  serialize_bytes
    #define read_string                 serialize_string
    #define read_align                  serialize_align
    #define read_check                  serialize_check
    #define read_object                 serialize_object
    #define read_address                serialize_address
    #define read_int_relative           serialize_int_relative
    #define read_ack_relative           serialize_ack_relative
    #define read_sequence_relative      serialize_sequence_relative

    // write macros corresponding to each serialize_*. useful when you want separate read and write functions for some reason.

    #define write_bits( stream, value, bits )                                               \
        do                                                                                  \
        {                                                                                   \
            yojimbo_assert( bits > 0 );                                                     \
            yojimbo_assert( bits <= 32 );                                                   \
            uint32_t uint32_value = (uint32_t) value;                                       \
            if ( !stream.SerializeBits( uint32_value, bits ) )                              \
                return false;                                                               \
        } while (0)

    #define write_int( stream, value, min, max )                                            \
        do                                                                                  \
        {                                                                                   \
            yojimbo_assert( min < max );                                                    \
            yojimbo_assert( value >= min );                                                 \
            yojimbo_assert( value <= max );                                                 \
            int32_t int32_value = (int32_t) value;                                          \
            if ( !stream.SerializeInteger( int32_value, min, max ) )                        \
                return false;                                                               \
        } while (0)

    #define write_float                 serialize_float
    #define write_uint32                serialize_uint32
    #define write_uint64                serialize_uint64
    #define write_double                serialize_double
    #define write_bytes                 serialize_bytes
    #define write_string                serialize_string
    #define write_align                 serialize_align
    #define write_check                 serialize_check
    #define write_object                serialize_object
    #define write_address               serialize_address
    #define write_int_relative          serialize_int_relative
    #define write_ack_relative          serialize_ack_relative
    #define write_sequence_relative     serialize_sequence_relative

    /**
        Interface for an object that knows how to read, write and measure how many bits it would take up in a bit stream.

        IMPORTANT: Instead of overriding the serialize virtual methods method directly, use the YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS macro in your derived class to override and redirect them to your templated serialize method.

        This way you can implement read and write for your messages in a single method and the C++ compiler takes care of generating specialized read, write and measure implementations for you.

        See tests/shared.h for some examples of this.

        @see ReadStream
        @see WriteStream
        @see MeasureStream
     */
    
    class Serializable
    {  
    public:

        virtual ~Serializable() {}

        /**
            Virtual serialize function (read).

            Reads the object in from a bitstream.

            @param stream The stream to read from.
         */

        virtual bool SerializeInternal( class ReadStream & stream ) = 0;

        /**
            Virtual serialize function (write).

            Writes the object to a bitstream.

            @param stream The stream to write to.
         */

        virtual bool SerializeInternal( class WriteStream & stream ) = 0;

        /**
            Virtual serialize function (measure).

            Quickly measures how many bits the object would take if it were written to a bit stream.

            @param stream The read stream.
         */

        virtual bool SerializeInternal( class MeasureStream & stream ) = 0;
    };

    /**
        Helper macro to define virtual serialize functions for read, write and measure that call into the templated serialize function.

        This helps avoid writing boilerplate code, which is nice when you have lots of hand coded message types.

        See tests/shared.h for examples of usage.
     */

    #define YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS()                                                               \
        bool SerializeInternal( class yojimbo::ReadStream & stream ) { return Serialize( stream ); };           \
        bool SerializeInternal( class yojimbo::WriteStream & stream ) { return Serialize( stream ); };          \
        bool SerializeInternal( class yojimbo::MeasureStream & stream ) { return Serialize( stream ); };         
}

#endif // #ifndef YOJIMBO_SERIALIZE_H
