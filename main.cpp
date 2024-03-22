
/*
g++ -c main.cpp
g++ main.o -o main -lsfml-graphics -lsfml-window -lsfml-system
./main
*/
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <SFML/Graphics.hpp>

struct XYset_t {
    float minr, maxr, mini, maxi;
    float scale, scale_coef;
    float dm;
    float dx, dy;
    float ux, uy;
    float mx, my;
    float scrx, scry;
};

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define WIDTH  800
#define HEIGHT 600
#define POS(x, y) ((y) * WIDTH + x) * 4

void render_(sf::Uint8* pixels, XYset_t* XYset)
{
    const int maxcount = 256;
    float zx = 0, zy = 0, cx = 0, cy = 0;
    float temp;
    int pack = 32, steps;
    XYset->scrx = XYset->scale * (XYset->ux - abs(XYset->minr)) + XYset->mx;
    XYset->scry = XYset->scale * (XYset->uy - abs(XYset->mini)) + XYset->my;
    for (int iy = 0; iy < HEIGHT; iy++) {
        for (int ix = 0; ix < WIDTH; ix += pack) {
            for (int i = 0; i < pack; i++) {
                cx = XYset->scrx + XYset->scale * (XYset->dx * (ix + i));
                cy = XYset->scry + XYset->scale * (XYset->dy * iy);
               
                zx = 0;
                zy = 0;
                steps = 0;
                for (;zx * zx + zy * zy <= 4 && steps < maxcount; steps++) {
                    temp = zx;
                    zx = (zx * zx) - (zy * zy) + cx;
                    zy = (2 * temp * zy) + cy;
                }

                float I = exp(sqrtf((float)steps / (float)maxcount)) * 255.f;
                unsigned char c = (unsigned char) I;
                if (steps < maxcount) {
                    pixels[POS(ix, iy) + i * 4 + 0] = (unsigned char) MIN(255, exp((float)(c - 255) / 45.f) * 10000.f);
                    pixels[POS(ix, iy) + i * 4 + 1] = (unsigned char) MIN(255, exp((float)(c - 255) / 37.f) * 10000.f);
                    pixels[POS(ix, iy) + i * 4 + 2] = (unsigned char) MIN(255, 255 * pow(cos(c / 255), 2));
                    pixels[POS(ix, iy) + i * 4 + 3] = 255;
                } else {
                    pixels[POS(ix, iy) + i * 4 + 0] = 0;
                    pixels[POS(ix, iy) + i * 4 + 1] = 0;
                    pixels[POS(ix, iy) + i * 4 + 2] = 0;
                    pixels[POS(ix, iy) + i * 4 + 3] = 255;
                }
            }
        }
    }
}

int render(sf::Uint8* pixels, XYset_t* XYset)
{
    clock_t start;
    clock_t end;

    start = clock();
    render_(pixels, XYset);
    end = clock();

    float time = (double)(end - start);
    double seconds = time / CLOCKS_PER_SEC;
    int FPS = 1 / seconds;
    return FPS;
}

int control(sf::RenderWindow* window, sf::Event event, XYset_t* XYset)
{
    sf::Vector2i mouse_pos = sf::Mouse::getPosition(*window);
    switch (event.type) {
        case sf::Event::Closed:
            return 1;
        case sf::Event::MouseButtonReleased:
            switch (event.mouseButton.button) {
                case sf::Mouse::Left:
                    XYset->mx -= (WIDTH  / 2 - (float)mouse_pos.x) * XYset->dm / WIDTH  * XYset->scale;
                    XYset->my -= (HEIGHT / 2 - (float)mouse_pos.y) * XYset->dm / HEIGHT * XYset->scale;
                    XYset->scale /= XYset->scale_coef;
                    break;
                case sf::Mouse::Right:
                    XYset->mx += (WIDTH  / 2 - (float)mouse_pos.x) * XYset->dm / WIDTH  * XYset->scale;
                    XYset->my += (HEIGHT / 2 - (float)mouse_pos.y) * XYset->dm / HEIGHT * XYset->scale;
                    XYset->scale *= XYset->scale_coef;
                    break;
                default:
                    break;
            }
            break;
        case sf::Event::EventType::KeyPressed:
            switch (event.key.code) {
                case sf::Keyboard::Left:
                    XYset->ux -= XYset->dx;
                    break;
                case sf::Keyboard::Right:
                    XYset->ux += XYset->dx;
                    break;
                case sf::Keyboard::Down:
                    XYset->uy += XYset->dy;
                    break;
                case sf::Keyboard::Up:
                    XYset->uy -= XYset->dy;
                    break;
                
                case sf::Keyboard::A:
                    XYset->ux -= XYset->dx;
                    break;
                case sf::Keyboard::D:
                    XYset->ux += XYset->dx;
                    break;
                case sf::Keyboard::S:
                    XYset->uy += XYset->dy;
                    break;
                case sf::Keyboard::W:
                    XYset->uy -= XYset->dy;
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
    sf::Text POS_Text;
    POS_Text.setFont(font);
    POS_Text.setPosition(10, 30);
    POS_Text.setCharacterSize(18);
    POS_Text.setColor(sf::Color(232, 40, 250));

    XYset_t XYset = {-2.f, 2.f, -2.f, 2.f, 1.f, 1.1f, 2.f};
    XYset.dx = (XYset.maxr - XYset.minr) / WIDTH;
    XYset.dy = (XYset.maxi - XYset.mini) / HEIGHT;

    float x = 0.f, y = 0.f, scale = 1.0f;
    int cycle_counter = 0;
    bool image_saved = false;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (control(&window, event, &XYset)) {
                window.close();
                break;
            }
        }

        window.clear();

        float FPS = render(pixels, &XYset);
        texture.update(pixels);
        window.draw(sprite);

        auto fps_string = "FPS: " + std::to_string(FPS);
        FPS_Text.setString(fps_string);
        window.draw(FPS_Text);

        auto pos_string = "POS: " + std::to_string(XYset.scrx) + " " + std::to_string(XYset.scry);
        POS_Text.setString(pos_string);
        window.draw(POS_Text);

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