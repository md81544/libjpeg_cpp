#include "jpeg.h"

#include <iostream>

void display( uint8_t luma )
{
    static std::vector<char> ascii{
        ' ', '.', ',', ':', '-', '=', '+', '*', '#', '%', '@'
        };

    int val = luma / 24;
    std::cout << ascii[val] << ascii[val];
}

int main( int argc, char* argv[] )
{

    if ( argc < 2 )
    {
        std::cout << "No jpeg file specified\n";
        return 1;
    }
    try
    {
        using namespace marengo::jpeg;
        // Constructor expects a filename to load:
        Image imgOriginal( argv[1] );

        // Copy construct a second version so we can
        // shrink non-destructively. Not really necessary
        // here, but just to show it can be done :)
        Image img = imgOriginal;

        // Shrink proportionally to a specific width (in px)
        img.shrink( 60 );

        // Display the image in ASCII, just for fun.
        std::size_t height = img.getHeight();
        std::size_t width  = img.getWidth();
        for ( std::size_t y = 0; y < height; ++y )
        {
            for ( std::size_t x = 0; x < width; ++x )
            {
                uint8_t luma = img.getLuminance( x, y );
                display( luma );
            }
            std::cout << "\n";
        }

        std::cout << "\nImage height: " << img.getHeight();
        std::cout << "\nImage width : " << img.getWidth();
        // Pixel "Size" is 3 bytes for colour images (i.e. R,G, & B)
        // and 1 byte for monochrome.
        std::cout << "\nImage px sz : " << img.getPixelSize();
        std::cout << std::endl;
        return 0;
    }
    catch( const std::exception& e )
    {
        std::cout << "Main() error handler: ";
        std::cout << e.what() << std::endl;
        return 1;
    }
}
