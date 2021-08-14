#include <SDL2/SDL.h>

namespace Gfx
{

/*
 * Draw a pixel to a locked 24-bit RGB texture.
 * The function does not check the validity of the arguments.
 *
 * Arguments:
 *      pixelData: A pointer to the pixel data of the destination texture.
 *      texturePitch: Number of bytes per one texture scanline.
 *      xPos: The x coordinate of the pixel.
 *      yPos: The y coordinate of the pixel.
 *      color: The color to set the pixel to. The alpha channel is ignored.
 */
inline void drawPoint(uint8_t* pixelData, int texturePitch, int xPos, int yPos, const SDL_Color& color)
{
    const int offset{xPos * 3 + yPos * texturePitch};
    pixelData[offset + 0] = color.r;
    pixelData[offset + 1] = color.g;
    pixelData[offset + 2] = color.b;
}

} // End of namespace
