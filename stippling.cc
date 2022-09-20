#include "ScopedTimer.hpp"
#include "Framebuffer.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "Voronoi.hpp"
#include <SDL2/SDL.h>
#include <boost/gil.hpp>
#include <boost/gil/extension/io/png.hpp>
#include <boost/gil/extension/numeric/sampler.hpp>
#include <boost/gil/extension/numeric/resample.hpp>
#include <thread>
#include <iostream>
#include <random>
#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

namespace gil = boost::gil;

const char *title = "Weighted Voronoi Stippling";

std::random_device rdev;
std::mt19937 rng;

int count;
int width, height;
float sigma0 = 0.0f;

gil::gray32f_image_t image;
gil::gray32f_image_t P,Q;

std::vector<glm::vec2> points;
std::vector<glm::vec4> centroids;
VertexBuffer<glm::vec2> pointbuf;

Voronoi voronoi;
Shader stippler;

// Cumulative sum in X axis only.
template <typename Image, typename ImageView>
void cumsumx(Image &dest, ImageView &src) {
    dest.recreate(src.width()+1, src.height());

    auto view = gil::view(dest);
#pragma omp parallel for
    for (int y = 0; y < src.height(); y++) {
        view(0,y) = gil::at_c<0>(src(0,y));
        for (int x = 1; x < src.width(); x++)
            view(x,y) = gil::at_c<0>(src(x,y)) + gil::at_c<0>(view(x-1,y));
        view(src.width(),y) = gil::at_c<0>(view(src.width()-1,y));
    }
}

void init(const std::string &filename) {
    auto imview = gil::flipped_up_down_view(gil::view(image));

    // Invert for density calculations.
#pragma omp parallel for collapse(2) 
    for (int x = 0; x < width; x++)
    for (int y = 0; y < height; y++)
        imview(x,y) = std::max(1e-2f, 1.0f - gil::at_c<0>(imview(x,y)));

    cumsumx(P, imview);
    cumsumx(Q, gil::view(P));

    // Generate random starting points.
    points.resize(count);
    for (int i = 0; i < count; i++) {
        for (;;) {
            points[i] = glm::vec2(std::uniform_int_distribution<>(0, width)(rng),
                                  std::uniform_int_distribution<>(0, height)(rng));
            if (gil::at_c<0>(imview(points[i].x, points[i].y)) >= 2e-2f)
                break;
        }
    }

    voronoi.create(width, height);
    stippler.create("../shaders/stipple");
    centroids.resize(count);
}

float iterate() {
    std::vector<glm::vec4> centroids(count, glm::vec4(0.0f));

    auto collect = [&](int y, int x0, int x1, int id) {
        auto Pv = gil::view(P), Qv = gil::view(Q);

        centroids[id].z += (gil::at_c<0>(Pv(x1,y))-gil::at_c<0>(Pv(x0,y)));
        centroids[id].y += (float(y))*(gil::at_c<0>(Pv(x1,y))-gil::at_c<0>(Pv(x0,y)));
        centroids[id].x += ((float(x1))*gil::at_c<0>(Pv(x1,y))-gil::at_c<0>(Qv(x1,y)))
                         - ((float(x0))*gil::at_c<0>(Pv(x0,y))-gil::at_c<0>(Qv(x0,y)));
        centroids[id].w += x1-x0;
    };

    voronoi.compute(points);

#pragma omp parallel for
    for (int y = 0; y < height; y++) {
        int x0 = 0, x1 = 1;
        int id = voronoi(x0,y);
        while (x1 < width) {
            if (id != voronoi(x1,y)) {
                collect(y, x0, x1, id);
                x0 = x1;
                id = voronoi(x0,y);
            }
            x1++;
        }
        collect(y, x0, x1, id);
    }


#pragma omp parallel for
    for (int i = 0; i < count; i++) {
        float z = std::max(1e-2f, centroids[i].z);
        points[i].x = std::max(std::min(centroids[i].x / z, float(width)), 0.0f);
        points[i].y = std::max(std::min(centroids[i].y / z, float(height)), 0.0f);
    }

    // Calculate the mean area.
    float mu = 0.0f;
    for (int i = 0; i < count; i++)
        mu += centroids[i].w;
    mu /= float(count);

    // Calculate the standard deviation.
    float sigma = 0.0f;
    for (int i = 0; i < count; i++)
        sigma += (centroids[i].w - mu) * (centroids[i].w - mu);
    sigma = std::sqrt(sigma / float(count));

    float diff = std::fabs(sigma - sigma0);
    sigma0 = sigma;
    return diff;
}

void draw(float pointsize) {
    auto imview = gil::flipped_up_down_view(gil::view(image));
    for (int i = 0; i < points.size(); i++)
        if (gil::at_c<0>(imview(points[i].x, points[i].y)) <= 2e-2f)
            points[i].x = points[i].y = -1;

    pointbuf.compile(points);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0,0,width,height);
    glClearColor(1,1,1,1);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glPointSize(3);
    stippler.use();
    stippler.uniform("pointsize", pointsize);
    stippler.uniform("ortho", glm::ortho(0.0f, (float)width, 0.0f, (float)height, -1.0f, 1024.0f));
    pointbuf.draw(GL_POINTS);
}

void writeimg(const std::string &filename) {
    std::vector<gil::rgba8_pixel_t> pixels(width*height);

    // Read in the stippled image from the window.
    glFinish();
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    // Write out to a png file.
    gil::write_view(filename,
        gil::flipped_up_down_view(gil::interleaved_view(width, height, pixels.data(), sizeof(pixels[0])*width)),
        gil::png_tag{});
}

int main(int argc, char **argv)
try {
    ScopedTimer timer("");

    if (argc < 3 || argc > 7) {
        std::cerr << "Usage: " << argv[0] << " SRC DST THRESH POINTSIZE COUNT SCALE" << std::endl;
        return EXIT_FAILURE;
    }
    const float thresh = (argc > 3)? std::stof(argv[3]) : 1e-4f;
    const float pointsize = (argc > 4)? std::stof(argv[4]) : 11.0f;
    count = (argc > 5)? std::stoi(argv[5]) : 1280;
    int scale = (argc > 6)? std::stoi(argv[6]) : 3;

    gil::gray32f_image_t src;
    gil::read_and_convert_image(argv[1], src, gil::png_tag());
    image.recreate(src.width()*scale, src.height()*scale);
    gil::resize_view(gil::const_view(src), gil::view(image), gil::bilinear_sampler{});
    width = image.width();
    height = image.height();
    std::cout << width << "x" << height << std::endl;

    SDL_Window* window = nullptr;
    SDL_GLContext context = nullptr;
    SDL_Texture* texture = nullptr;

    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO)) {
        std::cerr << "Error: SDL failed to initialize." << std::endl;
        return EXIT_FAILURE;
    }
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetSwapInterval(1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width*scale, height*scale, SDL_WINDOW_OPENGL|SDL_WINDOW_HIDDEN);
    if (!window) {
        std::cerr << "Error: Failed to create SDL2 window." << std::endl;
        goto Cleanup;
    }
    context = SDL_GL_CreateContext(window);
    if (!context) {
        std::cerr << "Error: Failed to create SDL2 context." << std::endl;
        goto Cleanup;
    }
    SDL_GL_MakeCurrent(window, context);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Error: Failed to initialize GLEW." << std::endl;
        return false;
    }

    init(argv[1]);
    SDL_ShowWindow(window);

    for (;;) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT)
                goto Done;
            else if (ev.type == SDL_KEYDOWN
                  && ev.key.keysym.sym == SDLK_RETURN)
                goto Done;
        }

        float dA = iterate();
        if (dA < thresh)
            goto Done;
        std::cout << dA << std::endl;

        SDL_GL_SwapWindow(window);
    }
Done:
    draw(pointsize);
    writeimg(argv[2]);

Cleanup:
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
} catch (const std::exception &err) {
    std::cerr << err.what() << std::endl;
    return EXIT_FAILURE;
}
