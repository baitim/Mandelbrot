
/*
g++ -c main.cpp
g++ main.o -o main -lsfml-graphics -lsfml-window -lsfml-system
./main
*/
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <SFML/Graphics.hpp>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define WIDTH  800
#define HEIGHT 600
#define DX (float)1/WIDTH
#define DY (float)1/WIDTH
#define POS(x, y) ((y) * WIDTH + x) * 4

void render_(sf::Uint8* pixels, float ux, float uy)
{
    const int max_count = 256;
    const float max_dist = 100.f;
    float x0, y0, X, Y, xx, yy, xy, r;
    int steps;
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x += 4) {
            for (int i = 0; i < 4; i++) {
                X = x0 = ux - (float)(WIDTH / 2) * DX + i * DX + DX * 4 * x / 4 - 1.325f;
                Y = y0 = uy + ((float)y - (float)(HEIGHT / 2))*DY;
                steps = 0;
                for (; steps < max_count; steps++) {
                    xx = X * X;
                    yy = Y * Y;
                    xy = X * Y;
                    r = xx + yy;
                    if (r >= max_dist) break;
                    X = xx - yy + x0;
                    Y = 2 * xy + y0;
                }
                float I = exp(sqrtf((float)steps / (float)max_count)) * 255.f;
                unsigned char c = (unsigned char) I;
                if (steps < max_count) {
                    pixels[POS(x, y) + i * 4 + 0] = (unsigned char) MIN(255, exp((float)(c - 255) / 45.f) * 10000.f);
                    pixels[POS(x, y) + i * 4 + 1] = (unsigned char) MIN(255, exp((float)(c - 255) / 37.f) * 10000.f);
                    pixels[POS(x, y) + i * 4 + 2] = (unsigned char) MIN(255, 255 * pow(cos(c / 255), 2));
                    pixels[POS(x, y) + i * 4 + 3] = 255;
                } else {
                    pixels[POS(x, y) + i * 4 + 0] = 0;
                    pixels[POS(x, y) + i * 4 + 1] = 0;
                    pixels[POS(x, y) + i * 4 + 2] = 0;
                    pixels[POS(x, y) + i * 4 + 3] = 255;
                }
            }
        }
    }
}

int render(sf::Uint8* pixels, float ux, float uy)
{
    clock_t start;
    clock_t end;

    start = clock();
    render_(pixels, ux, uy);
    end = clock();

    float time = (double)(end - start);
    double seconds = time / CLOCKS_PER_SEC;
    int FPS = 1 / seconds;
    return FPS;
}

int control(sf::Event event, float* x, float* y)
{
    switch (event.type) {
        case sf::Event::Closed:
            return 1;
        case sf::Event::EventType::KeyPressed:
            switch (event.key.code) {
                case sf::Keyboard::Left:
                    *x -= DX;
                    break;
                case sf::Keyboard::Right:
                    *x += DX;
                    break;
                case sf::Keyboard::Down:
                    *y += DX;
                    break;
                case sf::Keyboard::Up:
                    *y -= DX;
                    break;
                
                case sf::Keyboard::A:
                    *x -= DX;
                    break;
                case sf::Keyboard::D:
                    *x += DX;
                    break;
                case sf::Keyboard::S:
                    *y += DX;
                    break;
                case sf::Keyboard::W:
                    *y -= DX;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return 0;
}

int main()
{
    srand(time(NULL));
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot");
    sf::Texture texture;
    texture.create(WIDTH, HEIGHT);
    sf::Sprite sprite(texture);
    sf::Uint8* pixels = new sf::Uint8[WIDTH * HEIGHT * 4]; // (RGBA)
    sf::Font font;
    font.loadFromFile("arial.ttf");
    sf::Text FPS_Text;
    FPS_Text.setFont(font);
    FPS_Text.setPosition(10, 10);
    FPS_Text.setCharacterSize(18);
    FPS_Text.setColor(sf::Color(232, 40, 250));

    float x = 0.f, y = 0.f; 
    int cycle_counter = 0;
    bool image_saved = false;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (control(event, &x, &y)) {
                window.close();
                break;
            }
        }

        window.clear();

        float FPS = render(pixels, x, y);
        texture.update(pixels);
        window.draw(sprite);

        auto fps_string = "FPS: " + std::to_string(FPS);
        FPS_Text.setString(fps_string);
        window.draw(FPS_Text);

        window.display();

        if (cycle_counter == 3 && !image_saved) {
            sf::Image image = window.capture();
            image.saveToFile("Mandelbrot.png");
            fprintf(stderr, "Image was saved to file\n");
            image_saved = true;
        }
        cycle_counter = (cycle_counter + 1) % 100;
    }

    return 0;
}