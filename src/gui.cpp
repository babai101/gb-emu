#include "gui.hpp"

namespace GUI
{
    // The window we'll be rendering to
    SDL_Window *gWindow = NULL;
    SDL_Window *ppuViewer = NULL;
    // SDL_Window *spriteViewer = NULL;

    // The window renderer
    SDL_Renderer *gRenderer = NULL;
    SDL_Renderer *ppuViewereRenderer = NULL;
    // SDL_Renderer *spriteViewereRenderer = NULL;

    // Current displayed texture
    SDL_Texture *gTexture = NULL;
    SDL_Texture *ppuViewerTexture = NULL;
    // SDL_Texture *spriteViewerTexture = NULL;
    u32 off_screen_buffer[23040];
    u32 bgTiles_screen_buffer[65536];
    // u32 spriteTiles_screen_buffer[65536];

    unsigned int randInt()
    {
        return rand() % 255;
    }

    unsigned int randColor()
    {
        return (0x0 << 24) | randInt() | (randInt() << 8) | (randInt() << 16);
    }

    bool init(char *rom_name)
    {
        // Initialization flag
        bool success = true;
        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            // Set texture filtering to linear
            if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"))
            {
                printf("Warning: Linear texture filtering not enabled!");
            }

            // Create window
            gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
            if (gWindow == NULL)
            {
                printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
                success = false;
            }
            else
            {
                // Make window resizable
                SDL_SetWindowResizable(gWindow, SDL_TRUE);

                // Create renderer for window
                gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
                if (gRenderer == NULL)
                {
                    printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
                    success = false;
                }

                gTexture = SDL_CreateTexture(gRenderer,
                                             SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                             SCREEN_WIDTH, SCREEN_HEIGHT);
                if (gTexture == NULL)
                {
                    printf("Textures could not be created! SDL Error: %s\n", SDL_GetError());
                    success = false;
                }

                if (success)
                {
                    SDL_SetWindowSize(gWindow, 640, 576);
                }

                // Create debug windows
                ppuViewer = SDL_CreateWindow("PPU viewer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 256, 256, SDL_WINDOW_SHOWN);
                if (ppuViewer == NULL)
                {
                    printf("PPU Viewer Window could not be created! SDL Error: %s\n", SDL_GetError());
                    success = false;
                }
                else
                {
                    SDL_SetWindowResizable(ppuViewer, SDL_TRUE);
                    ppuViewereRenderer = SDL_CreateRenderer(ppuViewer, -1, SDL_RENDERER_ACCELERATED);
                    if (ppuViewereRenderer == NULL)
                    {
                        printf("PPU Viewer Renderer could not be created! SDL Error: %s\n", SDL_GetError());
                        success = false;
                    }

                    ppuViewerTexture = SDL_CreateTexture(ppuViewereRenderer,
                                                         SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                                         256, 256);
                    if (ppuViewerTexture == NULL)
                    {
                        printf("PPU Viewer Textures could not be created! SDL Error: %s\n", SDL_GetError());
                        success = false;
                    }
                    if (success)
                    {
                        SDL_SetWindowSize(ppuViewer, 512, 512);
                    }
                }
                // spriteViewer = SDL_CreateWindow("Sprite viewer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 256, 256, SDL_WINDOW_SHOWN);
                // if (ppuViewer == NULL)
                // {
                //     printf("Sprite Viewer Window could not be created! SDL Error: %s\n", SDL_GetError());
                //     success = false;
                // }
                // else
                // {
                //     SDL_SetWindowResizable(spriteViewer, SDL_TRUE);
                //     spriteViewereRenderer = SDL_CreateRenderer(spriteViewer, -1, SDL_RENDERER_ACCELERATED);
                //     if (spriteViewereRenderer == NULL)
                //     {
                //         printf("Sprite Viewer Renderer could not be created! SDL Error: %s\n", SDL_GetError());
                //         success = false;
                //     }

                //     spriteViewerTexture = SDL_CreateTexture(spriteViewereRenderer,
                //                                          SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                //                                          256, 256);
                //     if (spriteViewerTexture == NULL)
                //     {
                //         printf("Sprite Viewer Textures could not be created! SDL Error: %s\n", SDL_GetError());
                //         success = false;
                //     }
                //     if (success)
                //     {
                //         SDL_SetWindowSize(spriteViewer, 512, 512);
                //     }
                // }
            }
            CPU::reset();
            PPU::reset();
            if (!bootloader::load(rom_name))
            {
                std::cout << "Failed to load ROM...Quitting" << std::endl;
                success = false;
            }
            return success;
        }
    }
    // void run() { CPU::run(); }

    void close()
    {
        // Free loaded image
        SDL_DestroyTexture(gTexture);
        gTexture = NULL;

        SDL_DestroyTexture(ppuViewerTexture);
        ppuViewerTexture = NULL;
        // Destroy window
        SDL_DestroyRenderer(gRenderer);
        SDL_DestroyRenderer(ppuViewereRenderer);
        SDL_DestroyWindow(gWindow);
        SDL_DestroyWindow(ppuViewer);
        gWindow = NULL;
        ppuViewer = NULL;
        gRenderer = NULL;
        ppuViewereRenderer = NULL;

        // Quit SDL subsystems
        SDL_Quit();
    }

    void update_texture(u32 *pixels)
    {
        SDL_UpdateTexture(gTexture, NULL, pixels, SCREEN_WIDTH * sizeof(unsigned int));
    }

    void update_ppuViewerTexture(u32 *pixels)
    {
        SDL_UpdateTexture(ppuViewerTexture, NULL, pixels, 256 * sizeof(unsigned int));
    }

    // void update_spriteViewerTexture(u32 *pixels)
    // {
    //     SDL_UpdateTexture(spriteViewerTexture, NULL, pixels, 256 * sizeof(unsigned int));
    // }

    void render()
    {
        // Clear screen
        SDL_RenderClear(gRenderer);
        SDL_RenderClear(ppuViewereRenderer);

        // Render texture to screen
        SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
        SDL_RenderCopy(ppuViewereRenderer, ppuViewerTexture, NULL, NULL);
        // SDL_RenderCopy(spriteViewereRenderer, spriteViewerTexture, NULL, NULL);

        // Update screen
        SDL_RenderPresent(gRenderer);
        SDL_RenderPresent(ppuViewereRenderer);
        // SDL_RenderPresent(spriteViewereRenderer);
    }

    int run()
    {
        // Main loop flag
        bool quit = false;

        // Event handler
        SDL_Event e;

        const int FPS = 60;
        const int DELAY = 1000.0f / FPS;
        unsigned int frameTime, frameStart;

        // While application is running
        while (!quit)
        {
            frameStart = SDL_GetTicks();
            // Handle events on queue
            while (SDL_PollEvent(&e) != 0)
            {
                // User requests quit
                if (e.type == SDL_QUIT)
                {
                    quit = true;
                }
            }
            // std::cout << "Running CPU" << std::endl;
            CPU::run();
            printf("Cycles ran this frame: %d\n", PPU::main_cycles);
            PPU::main_cycles = 0;
            // run nes frame here
            render();

            // Wait to mantain framerate:
            frameTime = SDL_GetTicks() - frameStart;
            if (frameTime < DELAY)
                SDL_Delay((int)(DELAY - frameTime));
            update_texture(off_screen_buffer);
            PPU::renderBGTiles();
            update_ppuViewerTexture(bgTiles_screen_buffer);
            // PPU::render_sprite_tiles();
            // update_spriteViewerTexture(spriteTiles_screen_buffer);
        }

        // Free resources and close SDL
        close();

        return 0;
    }
} // namespace GUI
