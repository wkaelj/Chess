#include "render_backend.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <stdbool.h>
#include <assert.h>
#include <malloc.h>

#define alloc(type) (malloc(sizeof(type)));

struct RenderWindow
{
    SDL_Window *sdl_window;
    Render *r; // reference to render created for the window
};

struct Render
{
    SDL_Renderer *sdl_render;
    RenderCursorState cursor_state;
    int cursor_x, cursor_y;
    int pixel_scale;
    RenderWindow *window;
};

struct RenderTexture
{
    SDL_Texture *sdl_texture;
};

static bool render_initialized = false;
static unsigned render_count   = 0;
static unsigned window_count   = 0;

// helpers

static SDL_Rect convert_rect(const RenderRect *rect, size_t pixel_scale)
{
    return (SDL_Rect){
        .x = rect->x * pixel_scale,
        .y = rect->y * pixel_scale,
        .w = rect->w * pixel_scale,
        .h = rect->h * pixel_scale,
    };
}

static size_t calculate_pixel_scale(const RenderWindow *w, const Render *r)
{
    // find window pixel scale
    int window_width;
    int render_width;
    SDL_GetWindowSize(w->sdl_window, &window_width, NULL);
    SDL_GetRendererOutputSize(r->sdl_render, &render_width, NULL);
    return render_width / window_width;
}

RenderResult render_init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
        return RENDER_FAILURE;
    if (IMG_Init(IMG_INIT_PNG) != 0)
    {
        SDL_Quit();
        return RENDER_FAILURE;
    }

    render_initialized = true;
    return RENDER_SUCCESS;
}

void render_quit()
{

    if (render_count)
    {
        printf(
            "There are %u renders that have not been distroyed. The render "
            "should not be quit before all renders and windows have been "
            "destroyed.\n",
            render_count);
    }
    if (window_count)
    {
        printf(
            "There are %u windows that have not been distroyed. The render "
            "should not be quit before all renders and windows have been "
            "destroyed.\n",
            window_count);
    }
    render_initialized = false;
    IMG_Quit();
    SDL_Quit();
}

bool render_is_initialized() { return render_initialized; }

RenderWindow *render_create_window(const char *title, int w, int h)
{
    RenderWindow *window = alloc(RenderWindow);
    window->sdl_window   = SDL_CreateWindow(
        title, 0, 0, w, h, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);

    window->sdl_window &&window_count++;
    window->r = NULL;
    return window;
}

Render *render_create_render(RenderWindow *window)
{
    Render *render     = alloc(Render);
    render->sdl_render = SDL_CreateRenderer(
        window->sdl_window,
        0,
        SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (render->sdl_render == NULL)
        return NULL;

    SDL_GetMouseState(&render->cursor_x, &render->cursor_y);

    render_count++;

    render->pixel_scale = calculate_pixel_scale(window, render);

    window->r      = render;
    render->window = window;

    return render;
}

void render_destroy_render(Render *render)
{
    SDL_DestroyRenderer(render->sdl_render);
    render->window->r = NULL;
    free(render);
    render_count--;
}
void render_destroy_window(RenderWindow *window)
{
    if (window->r)
    {
        printf("Cannot destroy window, as a render relies on it");
        return;
    }

    SDL_DestroyWindow(window->sdl_window);
    free(window);
    window_count--;
}

RenderResult render_get_window_size(const RenderWindow *window, int *w, int *h)
{
    SDL_GetWindowSize(window->sdl_window, w, h);
    return RENDER_SUCCESS;
}

RenderResult render_draw_texture(
    const Render *render,
    const RenderRect *dst_rect,
    const RenderRect *src_rect,
    const RenderTexture *texture,
    float angle)
{
    SDL_Rect dest = convert_rect(dst_rect, render->pixel_scale);
    return SDL_RenderCopyEx(
               render->sdl_render,
               texture->sdl_texture,
               (SDL_Rect *)src_rect,
               &dest,
               angle,
               NULL,
               SDL_FLIP_NONE) == 0
               ? RENDER_SUCCESS
               : RENDER_FAILURE;
}

RenderResult render_set_colour(
    const Render *render, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return SDL_SetRenderDrawColor(render->sdl_render, r, g, b, a) == 0
               ? RENDER_SUCCESS
               : RENDER_FAILURE;
}

RenderResult render_draw_rect(const Render *render, const RenderRect *rect)
{
    SDL_Rect sdl_rect = convert_rect(rect, render->pixel_scale);
    return SDL_RenderFillRect(render->sdl_render, &sdl_rect) == 0
               ? RENDER_SUCCESS
               : RENDER_FAILURE;
}

RenderResult
render_draw_rects(const Render *render, const RenderRect *rects, size_t n)
{
    SDL_Rect sdl_rects[n];
    for (size_t i = 0; i < n; i++)
        sdl_rects[i] = convert_rect(&rects[i], render->pixel_scale);

    return SDL_RenderFillRects(render->sdl_render, sdl_rects, n);
}

RenderResult render_clear(const Render *render)
{
    return SDL_RenderClear(render->sdl_render) == 0 ? RENDER_SUCCESS
                                                    : RENDER_FAILURE;
}

void render_submit(const Render *render)
{
    SDL_RenderPresent(render->sdl_render);
}

RenderEvent render_poll_events(Render *render)
{

    // update the cursor state
    if (render->cursor_state == RENDER_CURSOR_PRESSED)
        render->cursor_state = RENDER_CURSOR_DOWN;

    if (render->cursor_state == RENDER_CURSOR_RELEASED)
        render->cursor_state = RENDER_CURSOR_UP;

    RenderEvent ret = RENDER_EVENT_NONE;

    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_WINDOWEVENT:
            switch (e.window.event)
            {
            case SDL_WINDOWEVENT_RESIZED:
                printf("Window (%d, %d)\n", e.window.data1, e.window.data2);
                render->pixel_scale =
                    calculate_pixel_scale(render->window, render);
                // set return if higher priotety
                if (RENDER_EVENT_WINDOW_RESIZE > ret)
                    ret = RENDER_EVENT_WINDOW_RESIZE;
                break;
            case SDL_WINDOWEVENT_DISPLAY_CHANGED:
                render->pixel_scale =
                    calculate_pixel_scale(render->window, render);
                break;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            render->cursor_state = RENDER_CURSOR_PRESSED;
            break;
        case SDL_MOUSEBUTTONUP:
            render->cursor_state = RENDER_CURSOR_RELEASED;
            break;
        case SDL_MOUSEMOTION:
            render->cursor_x = e.motion.x;
            render->cursor_y = e.motion.y;
            break;
        case SDL_QUIT: ret = RENDER_EVENT_QUIT; break;
        }
    }
    return ret;
}

RenderTexture *
render_create_texture(const Render *render, const char *texture_path)
{
    RenderTexture *t = alloc(RenderTexture);

    t->sdl_texture = SDL_CreateTextureFromSurface(
        render->sdl_render, IMG_Load(texture_path));

    if (t->sdl_texture == NULL)
    {
        free(t);
        return NULL;
    }
    else
        return t;
}

void render_destroy_texture(RenderTexture *texture)
{
    SDL_DestroyTexture(texture->sdl_texture);
    free(texture);
}

RenderResult
render_set_texture_alpha(const RenderTexture *texture, uint8_t alpha)
{
    return SDL_SetTextureAlphaMod(texture->sdl_texture, alpha) == 0
               ? RENDER_SUCCESS
               : RENDER_FAILURE;
}

RenderResult render_get_texture_size(const RenderTexture *t, int *w, int *h)
{
    if (SDL_QueryTexture(t->sdl_texture, NULL, NULL, w, h))
        return RENDER_FAILURE;

    return RENDER_SUCCESS;
}

RenderResult render_get_cursor_pos(const Render *render, int *x, int *y)
{
    if (!(render && (x || y)))
        return RENDER_FAILURE;
    if (x)
        *x = render->cursor_x;
    if (y)
        *y = render->cursor_y;
    return RENDER_SUCCESS;
}

RenderCursorState render_get_cursor_state(const Render *render)
{
    return render->cursor_state;
}