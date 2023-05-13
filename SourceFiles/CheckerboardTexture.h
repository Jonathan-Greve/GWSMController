#pragma once
#include <vector>

enum class Color
{
    Red,
    Green,
    Blue,
    Yellow,
    Cyan,
    Magenta,
    Orange,
    Purple,
    Brown,
    Pink,
    LightGreen,
    DarkGreen,
    LightBlue,
    DarkBlue,
    Gray,
    Black,
    White
};

class CheckerboardTexture
{
public:
    CheckerboardTexture(int width, int height, int tile_size, Color color = Color::Black)
        : m_width(width)
        , m_height(height)
        , m_tile_size(tile_size)
        , m_color(color)
        , m_data(width * height * 4)
    {
        generateTexture();
    }

    const std::vector<uint8_t>& getData() const { return m_data; }

private:
    void generateTexture()
    {
        for (int y = 0; y < m_height; ++y)
        {
            for (int x = 0; x < m_width; ++x)
            {
                bool is_black_tile = ((x / m_tile_size) % 2) ^ ((y / m_tile_size) % 2);
                uint8_t color_value = is_black_tile ? 190 : 255;

                int index = (y * m_width + x) * 4;

                switch (m_color)
                {
                case Color::Red:
                    m_data[index] = color_value; // R
                    m_data[index + 1] = 0; // G
                    m_data[index + 2] = 0; // B
                    break;
                case Color::Green:
                    m_data[index] = 0; // R
                    m_data[index + 1] = color_value; // G
                    m_data[index + 2] = 0; // B
                    break;
                case Color::Blue:
                    m_data[index] = 0; // R
                    m_data[index + 1] = 0; // G
                    m_data[index + 2] = color_value; // B
                    break;
                case Color::Yellow:
                    m_data[index] = color_value; // R
                    m_data[index + 1] = color_value; // G
                    m_data[index + 2] = 0; // B
                    break;
                case Color::Cyan:
                    m_data[index] = 0; // R
                    m_data[index + 1] = color_value; // G
                    m_data[index + 2] = color_value; // B
                    break;
                case Color::Magenta:
                    m_data[index] = color_value; // R
                    m_data[index + 1] = 0; // G
                    m_data[index + 2] = color_value; // B
                    break;
                case Color::Orange:
                    m_data[index] = color_value; // R
                    m_data[index + 1] = color_value / 2; // G
                    m_data[index + 2] = 0; // B
                    break;
                case Color::Purple:
                    m_data[index] = color_value; // R
                    m_data[index + 1] = 0; // G
                    m_data[index + 2] = color_value / 2; // B
                    break;
                case Color::Brown:
                    m_data[index] = color_value / 2; // R
                    m_data[index + 1] = color_value / 4; // G
                    m_data[index + 2] = 0; // B
                    break;
                case Color::Pink:
                    m_data[index] = color_value; // R
                    m_data[index + 1] = color_value / 2; // G
                    m_data[index + 2] = color_value / 2; // B
                    break;
                case Color::LightGreen:
                    m_data[index] = 0; // R
                    m_data[index + 1] = color_value; // G
                    m_data[index + 2] = color_value / 2; // B
                    break;
                case Color::DarkGreen:
                    m_data[index] = 0; // R
                    m_data[index + 1] = color_value / 2; // G
                    m_data[index + 2] = 0; // B
                    break;
                case Color::LightBlue:
                    m_data[index] = 0; // R
                    m_data[index + 1] = color_value / 2; // G
                    m_data[index + 2] = color_value; // B
                    break;
                case Color::DarkBlue:
                    m_data[index] = 0; // R
                    m_data[index + 1] = 0; // G
                    m_data[index + 2] = color_value / 2; // B
                    break;
                case Color::Gray:
                    m_data[index] = color_value / 2; // R
                    m_data[index + 1] = color_value / 2; // G
                    m_data[index + 2] = color_value / 2; // B
                    break;
                case Color::Black:
                case Color::White:
                default:
                    m_data[index] = color_value; // R
                    m_data[index + 1] = color_value; // G
                    m_data[index + 2] = color_value; // B
                    break;
                }

                m_data[index + 3] = 255; // A
            }
        }
    }

    int m_width;
    int m_height;
    int m_tile_size;
    Color m_color;
    std::vector<uint8_t> m_data;
};
