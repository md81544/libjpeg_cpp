#include "jpeg.h"

#include <jpeglib.h>

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace marengo
{
namespace jpeg
{

Image::Image( const std::string& fileName )
{
    m_decompressInfo = std::make_unique<::jpeg_decompress_struct>();
    m_compressInfo = std::make_unique<::jpeg_compress_struct>();
    m_errorMgr = std::make_unique<::jpeg_error_mgr>();

    FILE* infile = fopen(fileName.c_str(), "rb");
    if ( infile == NULL )
    {
        throw std::runtime_error( "Could not open " + fileName );
    }

    m_decompressInfo->err = ::jpeg_std_error( m_errorMgr.get() );
    m_errorMgr->error_exit = [](::j_common_ptr cinfo)
        {
            char jpegLastErrorMsg[JMSG_LENGTH_MAX];
            // Call the function pointer to get the error message
            ( *( cinfo->err->format_message ) )
                ( cinfo, jpegLastErrorMsg );
            throw std::runtime_error( jpegLastErrorMsg );
        };
    ::jpeg_create_decompress( m_decompressInfo.get() );

    // Read the file:
    ::jpeg_stdio_src( m_decompressInfo.get(), infile);
    int rc = ::jpeg_read_header( m_decompressInfo.get(), TRUE );
    if (rc != 1)
    {
        throw std::runtime_error(
            "File does not seem to be a normal JPEG"
            );
    }
    ::jpeg_start_decompress( m_decompressInfo.get() );

    m_width     = m_decompressInfo->output_width;
    m_height    = m_decompressInfo->output_height;
    m_pixelSize = m_decompressInfo->output_components;

    size_t row_stride = m_width * m_pixelSize;

    m_bitmapData.clear();
    m_bitmapData.reserve( m_height );

    while ( m_decompressInfo->output_scanline < m_height )
    {
        std::vector<uint8_t> vec(row_stride);
        uint8_t* p = vec.data();
        ::jpeg_read_scanlines( m_decompressInfo.get(), &p, 1 );
        m_bitmapData.push_back( vec );
    }
    ::jpeg_finish_decompress( m_decompressInfo.get() );
}

Image::~Image()
{
    ::jpeg_destroy_decompress( m_decompressInfo.get() );
    ::jpeg_destroy_compress( m_compressInfo.get() );
}

void Image::save( const std::string& fileName, int quality ) const
{
    if ( quality < 0 )
    {
        quality = 0;
    }
    if ( quality > 100 )
    {
        quality = 100;
    }
    FILE* outfile = fopen( fileName.c_str(), "wb" );
    if ( outfile == NULL )
    {
        throw std::runtime_error(
            "Could not open " + fileName + " for writing"
            );
    }
    ::jpeg_create_compress( m_compressInfo.get() );
    ::jpeg_stdio_dest( m_compressInfo.get(), outfile);
    m_compressInfo->image_width = m_width;
    m_compressInfo->image_height = m_height;
    m_compressInfo->input_components = m_pixelSize;
    m_compressInfo->in_color_space = m_decompressInfo->out_color_space;
    m_compressInfo->err = ::jpeg_std_error( m_errorMgr.get() );
    ::jpeg_set_defaults( m_compressInfo.get() );
    ::jpeg_set_quality( m_compressInfo.get(), quality, TRUE );
    ::jpeg_start_compress( m_compressInfo.get(), TRUE);
    for ( auto const& vecLine : m_bitmapData )
    {
        ::JSAMPROW rowPtr[1];
        // Casting const-ness away because the jpeglib
        // call expects a non-const pointer...
        rowPtr[0] = const_cast<::JSAMPROW>( vecLine.data() );
        ::jpeg_write_scanlines(
            m_compressInfo.get(),
            rowPtr,
            1
            );
    }
    ::jpeg_finish_compress( m_compressInfo.get() );
    fclose( outfile );
}

void Image::savePpm( const std::string& fileName ) const
{
    std::ofstream ofs( fileName, std::ios::out | std::ios::binary );
    if ( ! ofs )
    {
        throw std::runtime_error(
            "Could not open " + fileName + " for saving"
            );
    }
    // Write the header
    ofs << "P6 " << m_width << " " << m_height << " 255\n";
    for ( auto& v : m_bitmapData )
    {
        ofs.write( reinterpret_cast<const char *>(v.data()), v.size() );
    }
    ofs.close();
}

std::vector<uint8_t> Image::getPixel( size_t x, size_t y ) const
{
    if ( y >= m_bitmapData.size() )
    {
        throw std::out_of_range( "Y value too large" );
    }
    if ( x >= m_bitmapData[0].size() / m_pixelSize )
    {
        throw std::out_of_range( "X value too large" );
    }
    std::vector<uint8_t> vec;
    for ( size_t n = 0; n < m_pixelSize; ++n )
    {
        vec.push_back( m_bitmapData[ y ][ x * m_pixelSize + n ] );
    }
    return vec;
}

uint8_t Image::getLuminance( size_t x, size_t y ) const
{
    auto vec = getPixel( x, y );
    if ( vec.size() == 1 )
    {   // monochrome image
        return vec[0];
    }
    if ( vec.size() == 3 )
    {
        // fast approximation of luminance:
        return static_cast<uint8_t>(
            ( vec[0] * 2 + vec[1] * 3 + vec[2] ) / 6
            );
    }
    return 0;
}

std::vector<uint8_t>
Image::getAverage( size_t x, size_t y, size_t boxSize ) const
{
    if ( boxSize > m_width )
    {
        throw std::out_of_range( "Box size is greater than image width" );
    }
    if ( boxSize > m_height )
    {
        throw std::out_of_range( "Box size is greater than image height" );
    }
    if ( x + boxSize  >= m_width )
    {
        x = m_width - boxSize;
    }
    if ( y + boxSize >= m_height )
    {
        y = m_height - boxSize;
    }
    // running totals
    size_t r{ 0 }; // we just use this one for mono images
    size_t g{ 0 };
    size_t b{ 0 };
    for ( size_t row = y; row < y + boxSize; ++row )
    {
        for ( size_t col = x; col < x + boxSize; ++col )
        {
            auto vec = getPixel( col, row );
            r += vec[0];
            if ( vec.size() == 3 )
            {
                g += vec[1];
                b += vec[2];
            }
        }
    }
    std::vector<uint8_t> retVec;
    r /= ( boxSize * boxSize );
    retVec.push_back( r );
    if ( m_pixelSize == 3 )
    {
        g /= ( boxSize * boxSize );
        retVec.push_back( g );
        b /= ( boxSize * boxSize );
        retVec.push_back( b );
    }
    return retVec;
}

void Image::shrink( size_t newWidth )
{
    if ( newWidth >= m_width )
    {
        return;
    }

    // We process the original bitmap line by line rather than
    // calling getAverage() on every (new) pixel to ensure we make the
    // most of data already in existing cache lines & attempt to
    // allow branch prediction to work optimally. This has resulted
    // in a three-times speedup when shrinking a 21Mpx file.

    float scaleFactor = static_cast<float>(newWidth) / m_width;
    size_t newHeight = scaleFactor * m_height;
    std::vector<std::vector<uint8_t>> vecNewBitmap;
    vecNewBitmap.reserve( newHeight );

    // Yes, I probably could do a rolling average
    std::vector<size_t> runningTotals( newWidth * m_pixelSize );
    std::vector<size_t> runningCounts( newWidth * m_pixelSize );
    size_t oldRow = 0;
    for ( size_t row = 0; row < m_height; ++row )
    {
        for ( size_t col = 0; col < m_width * m_pixelSize; ++col )
        {
            size_t idx = scaleFactor * col;
            runningTotals[ idx ] += m_bitmapData[row][col];
            ++runningCounts[ idx ];
        }
        if ( static_cast<size_t>( scaleFactor * row ) > oldRow )
        {
            oldRow = scaleFactor * row;
            std::vector<uint8_t> newLine;
            newLine.reserve( newWidth * m_pixelSize );
            for ( size_t i = 0; i < newWidth * m_pixelSize; ++i )
            {
                newLine.push_back( runningTotals[i] / runningCounts[i] );
                runningTotals[i] = 0;
                runningCounts[i] = 0;
            }
            vecNewBitmap.push_back( newLine );
        }
    }
    m_bitmapData = vecNewBitmap;
    m_height = m_bitmapData.size();
    m_width = m_bitmapData[0].size() / m_pixelSize;
}

} // namespace jpeg
} // namespace marengo

