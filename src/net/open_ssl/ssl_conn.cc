
#include "base/utils/assert.h"
#include "net/impl/open_ssl/ssl_conn.h"
#include <string.h>

// 与bio之间的适配
static long bio_adapter_ctrl( BIO* b, int cmd, long, void* )
{
    return cmd == BIO_CTRL_FLUSH ? 1 : 0;
}

static int bio_adapter_read( BIO* b, char* out, int outl )
{
    using namespace taro::net;

    SSLAdaptor* inst = ( SSLAdaptor* )BIO_get_data( b );
    if( out == NULL || inst == NULL || outl == 0 )
        return 0;

    auto ret = inst->ssl_read( ( void* )out, ( size_t )outl );
    BIO_clear_retry_flags( b );
    if( ret <= 0 && inst->empty() )
    {
        BIO_set_retry_read( b );
    }
    return ret;
}

static int bio_adapter_write( BIO* b, const char* in, int inl )
{
    USING_NAMESPACE_TARO_NET

    SSLAdaptor* inst = ( SSLAdaptor* )BIO_get_data( b );
    TARO_ASSERT( inst );

    auto ret = inst->ssl_write( ( const void* )in, ( size_t )inl );
    BIO_clear_retry_flags( b );
    return ret;
}

static BIO* BIO_new_tls( taro::net::SSLAdaptor* inst )
{
    BIO_METHOD* method = BIO_meth_new( BIO_get_new_index() | BIO_TYPE_SOURCE_SINK, "taro_bio" );
    BIO_meth_set_ctrl( method, bio_adapter_ctrl );
    BIO_meth_set_read( method, bio_adapter_read );
    BIO_meth_set_write( method, bio_adapter_write );

    BIO* io = BIO_new( method );
    BIO_set_data( io, ( void* )inst );
    BIO_set_init( io, 1 );
    BIO_set_shutdown( io, 0 );
    return io;
}

NAMESPACE_TARO_NET_BEGIN

void SSLAdaptor::net_read( DynPacketSPtr const& packet )
{
    read_packets_.emplace_back( packet );
}

void SSLAdaptor::set_net_write( NetWriteCallback const& cb )
{
    write_callback_ = cb;
}

int32_t SSLAdaptor::ssl_write( const void* data, int32_t bytes )
{
    return write_callback_( ( char* )data, ( uint32_t )bytes );
}

int32_t SSLAdaptor::ssl_read( void* buffer, int32_t bytes )
{
    int32_t totalbytes = 0;

    auto it = read_packets_.begin();
    while( it != read_packets_.end() )
    {
        int32_t readbytes = ( int32_t )( it->packet->size() - it->offset );
        if( bytes >= ( totalbytes + readbytes ) )
        {
            memcpy( ( char* )buffer + totalbytes, ( char* )it->packet->buffer() + it->offset, readbytes );
            it = read_packets_.erase( it );
            totalbytes += ( int32_t )readbytes;
            continue;
        }

        readbytes = bytes - totalbytes;
        if( 0 < readbytes )
        {
            memcpy( ( char* )buffer + totalbytes, ( char* )it->packet->buffer() + it->offset, readbytes );
            it->offset += readbytes;
            totalbytes += readbytes;
        }
        break;
    }
    return totalbytes;
}

bool SSLAdaptor::empty() const
{
    return read_packets_.empty();
}

SSLCtxFactory& SSLCtxFactory::instance()
{
    static SSLCtxFactory inst;
    return inst;
}

SSL_CTX* SSLCtxFactory::find_or_create( SSLContext const& path )
{
    if( path.ca.empty() && path.cert.empty() && path.key.empty() )
    {
        NET_ERROR << "invalid parameter";
        return nullptr;
    }

    std::lock_guard<std::mutex> locker( mutex_ );
    auto map_key = std::make_tuple( path.cert, path.key, path.ca );
    auto it = factory_.find( map_key );
    if( it == factory_.end() )
    {
        auto ctx = SSL_CTX_new( SSLv23_method() );
        if( !load_ssl_files( ctx, ( SSLContext* )&path ) )
        {
            SSL_CTX_free( ctx );
            return nullptr;
        }
        factory_[map_key] = ctx;
        return ctx;
    }
    return it->second;
}

bool SSLCtxFactory::load_ssl_files( SSL_CTX* ctx, SSLContext* path )
{
    if( !path->ca.empty() )
    {
        if( 0 == SSL_CTX_load_verify_locations( ctx, path->ca.c_str(), NULL ) )
        {
            NET_ERROR << "load verify locations failed, file:" << path->ca;
            return false;
        }
    }

    if( !path->cert.empty() )
    {
        if( 0 == SSL_CTX_use_certificate_file( ctx, path->cert.c_str(), SSL_FILETYPE_PEM ) ) // or SSL_FILETYPE_ASN1
        {
            NET_ERROR << "load use certificate file failed, file:" << path->cert;
            return false;
        }
    }

    if( !path->key.empty() )
    {
        if( 0 == SSL_CTX_use_PrivateKey_file( ctx, path->key.c_str(), SSL_FILETYPE_PEM ) ) // or SSL_FILETYPE_ASN1
        {
            NET_ERROR << "load use private key file failed, file:" << path->key;
            return false;
        }

        if( 0 == SSL_CTX_check_private_key( ctx ) )
        {
            NET_ERROR << "private key file invalid, file:" << path->key;
            return false;
        }
    }
    return true;
}

SSLConn::SSLConn( SSLContext const& path )
    : ssl_ctx_( SSLCtxFactory::instance().find_or_create( path ) )
    , ssl_( SSL_new( ssl_ctx_ ) )
    , handshaked_( false )
{
    TARO_ASSERT( ssl_ctx_ && ssl_ );

    BIO* b = BIO_new_tls( &bio_ );
    SSL_set_bio( ssl_, b, b );
    SSL_set_connect_state( ssl_ );
}

SSLConn::SSLConn( SSL_CTX* ctx )
    : ssl_ctx_( ctx )
    , ssl_( SSL_new( ctx ) )
    , handshaked_( false )
{
    TARO_ASSERT( ssl_ctx_ && ssl_ );

    BIO* b = BIO_new_tls( &bio_ );
    SSL_set_bio( ssl_, b, b );
    SSL_set_accept_state( ssl_ );
}

SSLConn::~SSLConn()
{
    if( nullptr != ssl_ )
    {
        SSL_free( ssl_ );
        ssl_ = nullptr;
    }
}

void SSLConn::set_write_cb( NetWriteCallback const& func )
{
    bio_.set_net_write( func );
}

bool SSLConn::state() const
{
    return handshaked_.load();
}

int32_t SSLConn::handshake()
{
    int32_t hs = SSL_do_handshake( ssl_ );
    if( 1 == hs )
    {
        handshaked_.store( true );
        return TARO_OK;
    }

    int32_t err = SSL_get_error( ssl_, hs );
    if( err != SSL_ERROR_WANT_WRITE && err != SSL_ERROR_WANT_READ )
    {
        NET_ERROR << "handshake failed.";
        return TARO_ERR_FAILED;
    }
    return TARO_ERR_STATE;
}

void SSLConn::handle_read_event( DynPacketSPtr const& packet )
{
    bio_.net_read( packet );
}

int32_t SSLConn::read( char* buffer, int32_t bytes )
{
    int32_t r = SSL_read( ssl_, buffer, bytes );
    if( r <= 0 )
    {
        int32_t err = SSL_get_error( ssl_, r );
        if( err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE )
        {
            return TARO_ERR_DISCONNECT;
        }
        return 0;
    }
    return r;
}

bool SSLConn::write( char* buffer, int32_t bytes )
{
    int32_t r = SSL_write( ssl_, buffer, bytes );
    if( r <= 0 )
    {
        int32_t err = SSL_get_error( ssl_, r );
        if( err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE )
        {
            return false;
        }
    }
    return true;
}

NAMESPACE_TARO_NET_END
