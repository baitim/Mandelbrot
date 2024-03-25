
/*
g++ -c main.cpp
g++ main.o -o main -lsfml-graphics -lsfml-window -lsfml-system
./main
*/
#include <immintrin.h>
#include <stdio.h>
#include <string.h>
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
    const int maxcount = 256, pack = 4;
    float dx, x0, y0;
    __m256d m256_maxcount = _mm256_set_pd(maxcount, maxcount, maxcount, maxcount);
    __m256d m256_255      = _mm256_set_pd(255.f, 255.f, 255.f, 255.f);
    __m256d m256_2        = _mm256_set_pd(2.f, 2.f, 2.f, 2.f);
    __m256d m256_4        = _mm256_set_pd(4.f, 4.f, 4.f, 4.f);
    __m256d cx, cy, zx, zy, temp, cmp, I;
    __m256i steps;
    XYset->scrx = XYset->scale * (XYset->ux - abs(XYset->minr)) + XYset->mx;
    XYset->scry = XYset->scale * (XYset->uy - abs(XYset->mini)) + XYset->my;
    dx = XYset->scale * XYset->dx;
    for (int iy = 0; iy < HEIGHT; iy++) {
        y0 = XYset->scry + XYset->scale * XYset->dy * iy;
        for (int ix = 0; ix < WIDTH; ix += pack) {
            x0 = XYset->scrx + XYset->scale * XYset->dx * ix;
            cx = _mm256_set_pd(x0, x0 + dx, x0 + 2 * dx, x0 + 3 * dx);
            cy = _mm256_set_pd(y0, y0, y0, y0);
            zx = zy = _mm256_set_pd(0.f, 0.f, 0.f, 0.f);
            steps = _mm256_setzero_si256();
            for (int n = 0; n < maxcount; n++) {
                temp = zx;
                zx = _mm256_add_pd(_mm256_sub_pd(_mm256_mul_pd(zx, zx), _mm256_mul_pd(zy, zy)), cx);
                zy = _mm256_add_pd(_mm256_mul_pd(_mm256_mul_pd(m256_2, temp), zy), cy);
                cmp = _mm256_cmp_pd(_mm256_add_pd(_mm256_mul_pd(zx, zx), _mm256_mul_pd(zy, zy)), m256_4, _CMP_LT_OQ);
                int mask = _mm256_movemask_pd(cmp);
                if (!mask) break;
                steps = _mm256_sub_epi64(steps, _mm256_castpd_si256(cmp));
            }

            uint64_t* point_steps = (uint64_t*) &steps;
            for (int i = 0; i < pack; i++) {
                double point_I = exp(sqrtf((double)point_steps[i] / (double)maxcount)) * 255.f;
                unsigned char c = (unsigned char) point_I;
                if (point_steps[i] < maxcount) {
                    pixels[POS(ix, iy) + (pack - i - 1) * 4 + 0] = (unsigned char) MIN(255, exp((float)(c - 255) / 45.f) * 10000.f);
                    pixels[POS(ix, iy) + (pack - i - 1) * 4 + 1] = (unsigned char) MIN(255, cos(exp((float)(c - 255) / 37.f) * 10000.f / 2.f) * 255.f);
                    pixels[POS(ix, iy) + (pack - i - 1) * 4 + 2] = (unsigned char) MIN(255, 255 * pow(cos(c / 255 / 2), 2));
                } else {
                    memset(pixels + POS(ix, iy) + (pack - i - 1) * 4, 0, 3);
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
                    XYset->ux -= XYset->dx * 5;
                    break;
                case sf::Keyboard::Right:
                    XYset->ux += XYset->dx * 5;
                    break;
                case sf::Keyboard::Down:
                    XYset->uy += XYset->dy * 5;
                    break;
                case sf::Keyboard::Up:
                    XYset->uy -= XYset->dy * 5;
                    break;
                
                case sf::Keyboard::A:
                    XYset->ux -= XYset->dx * 5;
                    break;
                case sf::Keyboard::D:
                    XYset->ux += XYset->dx * 5;
                    break;
                case sf::Keyboard::S:
                    XYset->uy += XYset->dy * 5;
                    break;
                case sf::Keyboard::W:
                    XYset->uy -= XYset->dy * 5;
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
    memset(pixels, 255, sizeof(sf::Uint8) * WIDTH * HEIGHT * 4);
    sf::Font font;
    font.loadFromFile("arial.ttf");
    sf::Text FPS_Text;
    FPS_Text.setFont(font);
    FPS_Text.setPosition(10, 10);
    FPS_Text.setCharacterSize(18);
    FPS_Text.setColor(sf::Color(120, 255, 255));
    sf::Text POS_Text;
    POS_Text.setFont(font);
    POS_Text.setPosition(10, 30);
    POS_Text.setCharacterSize(18);
    POS_Text.setColor(sf::Color(120, 255, 255));

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