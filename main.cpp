
/*
g++ -c main.cpp
g++ main.o -o main -lsfml-graphics -lsfml-window -lsfml-system
./main
*/
#include <immintrin.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <SFML/Graphics.hpp>

#define TEST_ON

struct XYset_t {
    const int width;
    const int height;
    float minr, maxr, mini, maxi;
    float scale, scale_coef;
    float dm;
    float dx, dy;
    float ux, uy;
    float mx, my;
    float scrx, scry;
};

enum WindowEvent {
    WINDOW_EVENT_NO   = 0,
    WINDOW_EVENT_EXIT = 1
};

int max(int a, int b)
{
    return (((a) > (b)) ? (a) : (b));
}

int min(int a, int b)
{
    return (((a) < (b)) ? (a) : (b));
}

int pos(int x, int y, XYset_t* XYset)
{
    return ((y) * XYset->width + x) * 4;
}

void render_noavx(sf::Uint8* pixels, XYset_t* XYset)
{
    const int maxcount = 256;
    int pack = 32;
    XYset->scrx = XYset->scale * (XYset->ux - abs(XYset->minr)) + XYset->mx;
    XYset->scry = XYset->scale * (XYset->uy - abs(XYset->mini)) + XYset->my;

    for (int iy = 0; iy < XYset->height; iy++) {
        float cy = XYset->scry + XYset->scale * (XYset->dy * iy);
        for (int ix = 0; ix < XYset->width; ix += pack) {
            for (int i = 0; i < pack; i++) {
                float cx = XYset->scrx + XYset->scale * (XYset->dx * (ix + i));
               
                float zx = 0;
                float zy = 0;
                float steps = 0;
                for (;zx * zx + zy * zy <= 4 && steps < maxcount; steps++) {
                    float temp = zx;
                    zx = (zx * zx) - (zy * zy) + cx;
                    zy = (2 * temp * zy) + cy;
                }

                float I = exp(sqrtf((float)steps / (float)maxcount)) * 255.f;
                unsigned char c = (unsigned char) I;
                if (steps < maxcount) {
                    pixels[pos(ix, iy, XYset) + i * 4 + 0] = (unsigned char) (c % 2) * 255; // copypaste
                    pixels[pos(ix, iy, XYset) + i * 4 + 1] = (unsigned char) (c + 50) % 255;
                    pixels[pos(ix, iy, XYset) + i * 4 + 2] = (unsigned char) (c * 4) % 255;
                    pixels[pos(ix, iy, XYset) + i * 4 + 3] = 255;
                } else {
                    memset(pixels + pos(ix, iy, XYset) + i * 4, 0, 3);
                }
            }
        }
    }
}

void render_avx(sf::Uint8* pixels, XYset_t* XYset)
{
    const int maxcount = 256, pack = 4;
    __m256d m256_maxcount = _mm256_set_pd(maxcount, maxcount, maxcount, maxcount);
    __m256d m256_255      = _mm256_set_pd(255.f, 255.f, 255.f, 255.f);
    __m256d m256_2        = _mm256_set_pd(2.f, 2.f, 2.f, 2.f);
    __m256d m256_4        = _mm256_set_pd(4.f, 4.f, 4.f, 4.f);

    XYset->scrx = XYset->scale * (XYset->ux - abs(XYset->minr)) + XYset->mx;
    XYset->scry = XYset->scale * (XYset->uy - abs(XYset->mini)) + XYset->my;

    float dx = XYset->scale * XYset->dx;
    for (int iy = 0; iy < XYset->height; iy++) {
        float y0 = XYset->scry + XYset->scale * XYset->dy * iy;
        for (int ix = 0; ix < XYset->width; ix += pack) {
            float x0 = XYset->scrx + XYset->scale * XYset->dx * ix;
            
            __m256d cx = _mm256_set_pd(x0 + 0 * dx, x0 + 1 * dx, x0 + 2 * dx, x0 + 3 * dx);
            __m256d cy = _mm256_set_pd(y0, y0, y0, y0);
            
            __m256d zx = _mm256_set_pd(0.f, 0.f, 0.f, 0.f);
            __m256d zy = _mm256_set_pd(0.f, 0.f, 0.f, 0.f);
            __m256i steps = _mm256_setzero_si256();

            for (int n = 0; n < maxcount; n++) {
                __m256d temp = zx;
                zx = _mm256_add_pd(_mm256_sub_pd(_mm256_mul_pd(zx, zx), _mm256_mul_pd(zy, zy)), cx);
                zy = _mm256_add_pd(_mm256_mul_pd(_mm256_mul_pd(m256_2, temp), zy), cy);
                __m256d cmp = _mm256_cmp_pd(_mm256_add_pd(_mm256_mul_pd(zx, zx), _mm256_mul_pd(zy, zy)), m256_4, _CMP_LT_OQ);
                
                int mask = _mm256_movemask_pd(cmp);
                if (!mask) break;
                
                steps = _mm256_sub_epi64(steps, _mm256_castpd_si256(cmp));
            }

            uint64_t* point_steps = (uint64_t*) &steps;
            for (int i = 0; i < pack; i++) {
                double point_I = exp(sqrtf((double)point_steps[i] / (double)maxcount)) * 255.f;
                unsigned char c = (unsigned char) point_I;
                if (point_steps[i] < maxcount) {
                    pixels[pos(ix, iy, XYset) + (pack - i - 1) * 4 + 0] = (unsigned char) (c % 2) * 255;
                    pixels[pos(ix, iy, XYset) + (pack - i - 1) * 4 + 1] = (unsigned char) (c + 50) % 255;
                    pixels[pos(ix, iy, XYset) + (pack - i - 1) * 4 + 2] = (unsigned char) (c * 4) % 255;
                } else {
                    memset(pixels + pos(ix, iy, XYset) + (pack - i - 1) * 4, 0, 3);
                }
            }
        }
    }
}

uint64_t render(sf::Uint8* pixels, XYset_t* XYset)
{
    uint64_t start = __rdtsc();
    render_avx(pixels, XYset);
    uint64_t end = __rdtsc();

    uint64_t timer_ticks = end - start;
    return timer_ticks;
}

WindowEvent control(sf::RenderWindow* window, sf::Event event, XYset_t* XYset)
{
    sf::Vector2i mouse_pos = sf::Mouse::getPosition(*window);
    int real_dx = XYset->dx * 5;
    int real_dy = XYset->dy * 5;
    switch (event.type) {
        case sf::Event::Closed:
            return WINDOW_EVENT_EXIT;
        case sf::Event::MouseButtonReleased:
            switch (event.mouseButton.button) {
                case sf::Mouse::Left:
                    XYset->mx -= (XYset->width  / 2 - (float)mouse_pos.x) * XYset->dm / XYset->width  * XYset->scale;
                    XYset->my -= (XYset->height / 2 - (float)mouse_pos.y) * XYset->dm / XYset->height * XYset->scale;
                    XYset->scale /= XYset->scale_coef;
                    break;
                case sf::Mouse::Right:
                    XYset->mx += (XYset->width  / 2 - (float)mouse_pos.x) * XYset->dm / XYset->width  * XYset->scale;
                    XYset->my += (XYset->height / 2 - (float)mouse_pos.y) * XYset->dm / XYset->height * XYset->scale;
                    XYset->scale *= XYset->scale_coef;
                    break;
                default:
                    break;
            }
            break;
        case sf::Event::EventType::KeyPressed:
            switch (event.key.code) {
                case sf::Keyboard::Left:
                    XYset->ux -= real_dx;
                    break;
                case sf::Keyboard::Right:
                    XYset->ux += real_dx;
                    break;
                case sf::Keyboard::Down:
                    XYset->uy += real_dy;
                    break;
                case sf::Keyboard::Up:
                    XYset->uy -= real_dy;
                    break;
                
                case sf::Keyboard::A:
                    XYset->ux -= real_dx;
                    break;
                case sf::Keyboard::D:
                    XYset->ux += real_dx;
                    break;
                case sf::Keyboard::S:
                    XYset->uy += real_dy;
                    break;
                case sf::Keyboard::W:
                    XYset->uy -= real_dy;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return WINDOW_EVENT_NO;
}

void test_render(sf::Uint8* pixels, XYset_t* XYset, FILE* output_file)
{
    const int count_iters = 3000;
    const int step_iter = count_iters / 100;
    uint64_t sum_time = 0;
    uint64_t values[count_iters];
    for (int i = 0; i < count_iters; i++) {
        uint64_t start = __rdtsc();
        render_avx(pixels, XYset);
        uint64_t end = __rdtsc();
        values[i] = end - start;
        if (i % step_iter == 0) fprintf(stderr, "%d%%\n", i / step_iter);
    }

    for (int i = 0; i < count_iters; i++)
        sum_time += values[i];

    uint64_t average_time = sum_time / count_iters;

    uint64_t deviation_sum2 = 0;
    for (int i = 0; i < count_iters; i++)
        deviation_sum2 += (values[i] - average_time) * (values[i] - average_time);

    uint64_t deviation = sqrt(deviation_sum2 / count_iters);

    fprintf(output_file, "average_time = %ld\n", average_time / 10000);
    fprintf(output_file, "deviation = %ld\n", deviation / 10000);
}

void all_free(char* clocks_string, sf::Uint8* pixels)
{
    free(clocks_string);
    free(pixels);
}

int main()
{
    srand(time(NULL));
    const int width = 800, height = 600;
#ifndef TEST_ON
    // WINDOW
    sf::RenderWindow window(sf::VideoMode(width, height), "Mandelbrot");
    sf::Texture texture;
    texture.create(width, height);
    sf::Sprite sprite(texture);
    
    // TEXT
    sf::Font font;
    font.loadFromFile("arial.ttf");
    sf::Text clock_text;
    clock_text.setFont(font);
    clock_text.setPosition(10, 10);
    clock_text.setCharacterSize(18);
    clock_text.setColor(sf::Color(120, 255, 255));
#endif

    // PIXELS
    sf::Uint8* pixels = (sf::Uint8*) calloc(sizeof(sf::Uint8), width * height * 4); // (RGBA)
    memset(pixels, 255, sizeof(sf::Uint8) * width * height * 4); // set 255 at all (need for A = 255)

    // XY setting
    XYset_t XYset = {.width = 800, .height = 600, .minr = -2.f, .maxr = 2.f, 
                     .mini = -2.f, .maxi = 2.f, .scale = 1.f, .scale_coef = 1.1f, .dm = 2.f};
    XYset.dx = (XYset.maxr - XYset.minr) / width;
    XYset.dy = (XYset.maxi - XYset.mini) / height;

    FILE* output_file = fopen("test.txt", "w");
    test_render(pixels, &XYset, output_file);
    fclose(output_file);

#ifndef TEST_ON
    // Main cycle
    float x = 0.f, y = 0.f, scale = 1.0f;
    int len_clocks_string = 100;
    char* clocks_string = (char*) calloc(len_clocks_string, sizeof(char));
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (control(&window, event, &XYset) == WINDOW_EVENT_EXIT) {
                window.close();
                all_free(clocks_string, pixels);
                return 0;
            }
        }

        window.clear();

        uint64_t timer_ticks = render(pixels, &XYset);
        texture.update(pixels);
        window.draw(sprite);

        snprintf(clocks_string, len_clocks_string, "CPU TICKS: %.1e\n", (double)timer_ticks);
        clock_text.setString(clocks_string);
        window.draw(clock_text);

        window.display();
    }

    all_free(clocks_string, pixels);
#endif
    return 0;
}