
#include "base/error_no.h"
#include "base/utils/assert.h"
#include "log/sink_rotate_file.h"
#include "base/system/file_sys.h"
#include "base/utils/arg_expander.h"
#include <list>
#include <vector>

NAMESPACE_TARO_LOG_BEGIN

struct RotateFileSink::Impl
{
    /**
    * @brief 构造函数
    */
    Impl()
        : file_( nullptr )
        , max_file_num_( 3 )
        , written_bytes_( 0 )
        , max_bytes_( 1024 * 1024 * 2 )
    {

    }

    /**
    * @brief 写数据
    */
    void write( const char* message )
    {
        if ( nullptr == file_ )
        {
            initiliaze();
        }
        
        if ( written_bytes_ >= max_bytes_ )
        {
            close_file();
            open_file();
        }

        auto msg_len = strlen( message );
        ( void )fwrite( message, strlen( message ), 1, file_ );
        ( void )fflush( file_ );
        written_bytes_ += msg_len;
    }

    /**
    * @brief 初始化
    */
    void initiliaze()
    {
        std::list<FileSysNode> log_files;
        if( FileSystem::check_dir( directory_.c_str() ) == TARO_OK )
        {
            auto find_result = FileSystem::get_nodes( directory_.c_str() );
            std::list<FileSysNode> log_files;
            for( auto const& one : find_result )
            {
                if( one.type != eNodeTypeFile || one.name.find( "taro_log_" ) == std::string::npos )
                {
                    continue;
                }
                log_files.emplace_back( one );
            }
        }
        else
        {
            if( FileSystem::create_dir( directory_.c_str() ) != TARO_OK )
            {
                TARO_ASSERT( 0, "create directory", directory_, "failed" );
            }
        }
        
        if ( log_files.empty() )
        {
            open_file();
            return;
        }

        log_files.sort( [&]( FileSysNode const& l, FileSysNode const& r )
        {
            return l.mtime < r.mtime;
        } );

        int32_t index = 0;
        for ( auto it = log_files.rbegin(); it != log_files.rend(); ++it )
        {
            if ( index++ < max_file_num_ )
            {
                written_files_.push_front( it->name );
            }
            else
            {
                std::string path = directory_ + "/" + ( *it ).name;
                ( void )::remove( path.c_str() );
            }
        }

        if ( log_files.back().size < max_bytes_ )
        {
            std::string path = directory_ + "/" + log_files.back().name;
            file_ = fopen( path.c_str(), "a+" );
            TARO_ASSERT( nullptr != file_ );
            written_bytes_ = log_files.back().size;
        }
        else
        {
            close_file();
            open_file();
        }
    }

    /**
    * @brief 打开新文件
    */
    void open_file()
    {
        std::string file_name = SystemTime().to_ms_str("taro_log_%04d_%02d_%02d_%02d%02d%02d_%03lld.txt");
        std::string path = directory_ + "/" + file_name;
        file_ = fopen( path.c_str(), "a+" );
        TARO_ASSERT( file_ != nullptr );
        written_files_.push_back( file_name );
        written_bytes_ = 0;
    }

    /**
    * @brief 关闭当前文件
    */
    void close_file()
    {
        if ( nullptr != file_ )
        {
            fclose( file_ );
        }
        file_ = nullptr;

        if ( ( int32_t )written_files_.size() < max_file_num_ )
        {
            return;
        }

        auto filename = written_files_.front();
        written_files_.pop_front();
        std::string path = directory_ + "/" + filename;
        ::remove( filename.c_str() );
    }

    FILE* file_;
    int32_t max_file_num_;     // 最大文件数量，超过该数量则删除最早的文件
    uint64_t written_bytes_;
    uint64_t max_bytes_;       // 单个文件的最大字节
    std::string directory_;    // 日志输出的文件夹
    std::list<std::string> written_files_;
};

RotateFileSink::RotateFileSink( const char* name )
    : Sink( name )
    , impl_( new Impl )
{

}

RotateFileSink::~RotateFileSink()
{
    delete impl_;
}

int32_t RotateFileSink::set_param( const char* out_dir, uint32_t max_file_num, uint32_t file_size )
{
    if ( !STRING_CHECK( out_dir ) )
    {
        return TARO_ERR_INVALID_ARG;
    }
    
    impl_->directory_    = out_dir;
    impl_->max_file_num_ = max_file_num;
    impl_->max_bytes_    = file_size;
    return TARO_OK;
}

const char* RotateFileSink::type() const
{
    return "RotateFileSink";
}

void RotateFileSink::write( const char* log_msg, ELogLevel const& )
{
    TARO_ASSERT( STRING_CHECK( log_msg ) );
    impl_->write( log_msg );
}

NAMESPACE_TARO_LOG_END
