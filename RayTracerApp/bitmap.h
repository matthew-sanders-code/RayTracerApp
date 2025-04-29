#ifndef BITMAP_H
#define BITMAP_H

#include <cstdint>
#include <fstream>
#include <vector>
#include <stdexcept>
#include "camera.h"

#include <SFML/Graphics.hpp>

#pragma pack(push, 1)
struct BMPHeader {
    uint16_t bfType = 0x4D42;  // 'BM'
    uint32_t bfSize;
    uint16_t bfReserved1 = 0;
    uint16_t bfReserved2 = 0;
    uint32_t bfOffBits = 54;   // 14 (BMPHeader) + 40 (DIBHeader)
};

struct DIBHeader {
    uint32_t biSize = 40;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes = 1;
    uint16_t biBitCount = 24;  // 24-bit
    uint32_t biCompression = 0;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter = 2835;
    int32_t biYPelsPerMeter = 2835;
    uint32_t biClrUsed = 0;
    uint32_t biClrImportant = 0;
};
#pragma pack(pop)

class Bitmap {
public:
    static void saveBMP(const char* filename, const std::vector<uint8_t>& pixelBuffer, int32_t width, int32_t height) {
        const int bytesPerPixel = 3;
        const int rowSize = ((width * bytesPerPixel + 3) / 4) * 4; // each row must be a multiple of 4 bytes
        const int paddingSize = rowSize - (width * bytesPerPixel);
        const int imageSize = rowSize * height;

        BMPHeader bmpHeader;
        DIBHeader dibHeader;

        dibHeader.biWidth = width;
        dibHeader.biHeight = -height; // negative for top-down
        dibHeader.biSizeImage = imageSize;

        bmpHeader.bfSize = bmpHeader.bfOffBits + dibHeader.biSizeImage;

        std::ofstream file(filename, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to open file for writing.");
        }

        // Write headers
        file.write(reinterpret_cast<const char*>(&bmpHeader), sizeof(bmpHeader));
        file.write(reinterpret_cast<const char*>(&dibHeader), sizeof(dibHeader));

        // Write pixel data row by row with padding
        const uint8_t padding[3] = { 0, 0, 0 }; // max padding is 3 bytes
        for (int y = 0; y < height; ++y) {
            const size_t rowStart = y * width * bytesPerPixel;
            file.write(reinterpret_cast<const char*>(&pixelBuffer[rowStart]), width * bytesPerPixel);
            file.write(reinterpret_cast<const char*>(padding), paddingSize);
        }

        file.close();
    }

    static void DisplayBitmap(int image_width, double aspect_ratio) {
        unsigned int width = static_cast<unsigned int>(image_width);
        unsigned int height = static_cast<unsigned int>(image_width / aspect_ratio);
        sf::RenderWindow window(sf::VideoMode({ width, height }), "RayTracing");
        sf::Texture texture;
        if (!texture.loadFromFile("image.bmp", false))
        {
            std::cout << "Failed to load image" << std::endl;
        }
        sf::Sprite sprite(texture);
        while (window.isOpen())
        {
            while (const std::optional event = window.pollEvent())
            {
                if (event->is<sf::Event::Closed>())
                    window.close();
            }

            window.clear();
            window.draw(sprite);
            window.display();
        }
    }
};

#endif