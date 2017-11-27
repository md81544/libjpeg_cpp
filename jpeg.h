#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// forward declarations of jpeglib structs
struct jpeg_decompress_struct;
struct jpeg_compress_struct;
struct jpeg_error_mgr;

namespace marengo
{
namespace jpeg
{

class Image
{
public:
    // Currently can only construct with an existing file.
    // Will throw if file cannot be loaded, or is in the wrong format,
    // or some other error is encountered.
    explicit Image( const std::string& fileName );

    // For the time being, disallow any copy construction or assignment
    Image(const Image& other)            = delete;
    Image(Image&& other)                 = delete;
    Image& operator=(const Image& other) = delete;
    Image& operator=(Image&& other)      = delete;

    ~Image();

    // Will throw if file cannot be saved. If no
    // filename is supplied, writes to fileName supplied in load()
    // (if that was called, otherwise throws)
    // Quality's usable values are 0-100
    void save( const std::string& fileName, int quality = 95 ) const;

    // Mainly for testing, writes an uncompressed PPM file
    void savePpm( const std::string& fileName ) const;

    size_t getHeight()    const { return m_height; }
    size_t getWidth()     const { return m_width;  }
    size_t getPixelSize() const { return m_pixelSize; }

    // Will return a vector of pixel components. The vector's
    // size will be 1 for monochrome or 3 for RGB.
    // Elements for the latter will be in order R, G, B.
    std::vector<uint8_t> getPixel( size_t x, size_t y ) const;

    // Returns a fast approximation of perceived brightness for RGB images.
    // For monochrome, will just return the pixel's value directly.
    uint8_t getLuminance( size_t x, size_t y ) const;

    // Get average of a box of pixels, returns a vector<uint8_t> of
    // size 1 for monochrome or three for RGB.
    // Note x & y specify the top left pixel of the box.
    // If the box runs off the end of the row or column then it is
    // shifted left/up to fit which means averages may be a little
    // odd near the right edge / bottom :)
    std::vector<uint8_t> getAverage( size_t x, size_t y, size_t boxSize ) const;

    // Shrink (resize smaller, retaining proportion). Does nothing
    // if the specified new width is larger than the existing width.
    // Simply averages pixels' values.
    void shrink( size_t newWidth );

private:
    std::unique_ptr<::jpeg_decompress_struct>  m_decompressInfo;
    std::unique_ptr<::jpeg_compress_struct>    m_compressInfo;
    std::unique_ptr<::jpeg_error_mgr>          m_errorMgr;
    std::vector<std::vector<uint8_t>>          m_bitmapData;
    size_t                                     m_width;
    size_t                                     m_height;
    size_t                                     m_pixelSize;
};

} // namespace jpeg
} // namespace marengo

