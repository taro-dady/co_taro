
#include "base/utils/base64.h"
#include "base/utils/assert.h"

NAMESPACE_TARO_BEGIN

static std::string alphabet_map( "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" );

static uint32_t base64_encode( const uint8_t *text, uint32_t text_len, uint8_t *encode )
{
    uint32_t i, j;
    for( i = 0, j = 0; i + 3 <= text_len; i += 3 )
    {
        encode[j++] = alphabet_map[text[i] >> 2];                                             
        encode[j++] = alphabet_map[( ( text[i] << 4 ) & 0x30 ) | ( text[i + 1] >> 4 )];       
        encode[j++] = alphabet_map[( ( text[i + 1] << 2 ) & 0x3c ) | ( text[i + 2] >> 6 )];  
        encode[j++] = alphabet_map[text[i + 2] & 0x3f];                                     
    }

    if( i < text_len )
    {
        uint32_t tail = text_len - i;
        if( tail == 1 )
        {
            encode[j++] = alphabet_map[text[i] >> 2];
            encode[j++] = alphabet_map[( text[i] << 4 ) & 0x30];
            encode[j++] = '=';
        }
        else //tail==2
        {
            encode[j++] = alphabet_map[text[i] >> 2];
            encode[j++] = alphabet_map[( ( text[i] << 4 ) & 0x30 ) | ( text[i + 1] >> 4 )];
            encode[j++] = alphabet_map[( text[i + 1] << 2 ) & 0x3c];
            encode[j++] = '=';
        }
    }
    return j;
}

static uint32_t base64_decode( const uint8_t *code, uint32_t code_len, uint8_t *plain )
{
    TARO_ASSERT( ( code_len & 0x03 ) == 0 );

    static uint8_t reverse_map[] =
    {
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 62, 255, 255, 255, 63,
        52,  53, 54, 55, 56, 57, 58, 59, 60, 61, 255, 255, 255, 255, 255, 255,
        255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 255,
        255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 255, 255, 255, 255, 255
    };

    uint32_t i, j = 0;
    uint8_t quad[4];
    for( i = 0; i < code_len; i += 4 )
    {
        for( uint32_t k = 0; k < 4; k++ )
        {
            quad[k] = reverse_map[code[i + k]];
        }

        TARO_ASSERT( quad[0] < 64 && quad[1] < 64 );

        plain[j++] = ( quad[0] << 2 ) | ( quad[1] >> 4 ); 

        if( quad[2] >= 64 )
            break;
        else if( quad[3] >= 64 )
        {
            plain[j++] = ( quad[1] << 4 ) | ( quad[2] >> 2 ); 
            break;
        }
        else
        {
            plain[j++] = ( quad[1] << 4 ) | ( quad[2] >> 2 );
            plain[j++] = ( quad[2] << 6 ) | quad[3];
        }
    }
    return j;
}

std::string Base64::encode( std::string const& in )
{
    if ( in.empty() )
    {
        return "";
    }

    size_t buff_bytes = in.length() * 5 / 3;  // normal is 4/3
    char* buff = ( char* )malloc( buff_bytes );
    memset( buff, 0, buff_bytes );
    auto bytes = base64_encode( ( const uint8_t* )in.c_str(), ( uint32_t )in.length(), ( uint8_t* )buff );
    std::string code( buff, bytes );
    free( buff );
    return code;
}

std::string Base64::decode( std::string const& in )
{
    if ( in.empty() )
    {
        return "";
    }

    size_t buff_bytes = in.length(); // decoded length less than in string
    char* buff = ( char* )malloc( buff_bytes );
    memset( buff, 0, buff_bytes );
    auto bytes = base64_decode( ( const uint8_t* )in.c_str(), ( uint32_t )in.length(), ( uint8_t* )buff );
    std::string code( buff, bytes );
    free( buff );
    return code;
}

NAMESPACE_TARO_END
