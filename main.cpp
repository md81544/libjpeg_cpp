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
    using namespace marengo::jpeg;

    if ( argc < 2 )
    {
        std::cout << "No jpeg file specified\n";
        return 1;
    }
    try
    {
        Image img( argv[1] );

        // Shrink proportionally to a specific width (in px)
        img.shrink( 80 );

        // Display the image in ASCII
        size_t height = img.getHeight();
        size_t width  = img.getWidth();
        for ( size_t y = 0; y < height; ++y )
        {
            for ( size_t x = 0; x < width; ++x )
            {
                uint8_t luma = img.getLuminance( x, y );
                display( luma );
            }
            std::cout << "\n";
        }

        std::cout << "\nImage height: " << img.getHeight();
        std::cout << "\nImage width : " << img.getWidth();
        std::cout << "\nImage px sz : " << img.getPixelSize();
        std::cout << std::endl;
    }
    catch( const std::exception& e )
    {
        std::cout << "Main() error handler: ";
        std::cout << e.what() << std::endl;
        return 1;
    }
}
