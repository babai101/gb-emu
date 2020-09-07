#include "gui.hpp"

namespace GUI
{
    //The window we'll be rendering to
    SDL_Window *gWindow = NULL;

    //The window renderer
    SDL_Renderer *gRenderer = NULL;

    //Current displayed texture
    SDL_Texture *gTexture = NULL;
    u32 off_screen_buffer [23040];

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
        //Initialization flag
        bool success = true;
        //Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            //Set texture filtering to linear
            if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
            {
                printf("Warning: Linear texture filtering not enabled!");
            }

            //Create window
            gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
            if (gWindow == NULL)
            {
                printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
                success = false;
            }
            else
            {
                //Make window resizable
                SDL_SetWindowResizable(gWindow, SDL_TRUE);                           

                //Create renderer for window
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

                // if (success)
                // {
                //     int i = SDL_RenderSetScale(gRenderer, 3, 3);
                //     if (i != 0)
                //     {
                //         printf("Textures could not be scaled: %s\n", SDL_GetError());
                //         success = false;
                //     }
                // }
                // if (success)
                // {
                //     int i = SDL_RenderSetLogicalSize(gRenderer, 480, 432);
                //     if (i != 0)
                //     {
                //         printf("Textures could not be scaled: %s\n", SDL_GetError());
                //         success = false;
                //     }
                // }
            }
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
    // void run() { CPU::run(); }

    void close()
    {
        //Free loaded image
        SDL_DestroyTexture(gTexture);
        gTexture = NULL;

        //Destroy window
        SDL_DestroyRenderer(gRenderer);
        SDL_DestroyWindow(gWindow);
        gWindow = NULL;
        gRenderer = NULL;

        //Quit SDL subsystems
        SDL_Quit();
    }

    void update_texture(u32 *pixels)
    {
        SDL_UpdateTexture(gTexture, NULL, pixels, SCREEN_WIDTH * sizeof(unsigned int));
    }

    void render()
    {
        //Clear screen
        SDL_RenderClear(gRenderer);

        //Render texture to screen
        SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);

        //Update screen
        SDL_RenderPresent(gRenderer);
    }

    int run()
    {
        //Main loop flag
        bool quit = false;

        //Event handler
        SDL_Event e;

        const int FPS = 60;
        const int DELAY = 1000.0f / FPS;
        unsigned int frameTime, frameStart;

        //While application is running
        while (!quit)
        {
            frameStart = SDL_GetTicks();
            //Handle events on queue
            while (SDL_PollEvent(&e) != 0)
            {
                //User requests quit
                if (e.type == SDL_QUIT)
                {
                    quit = true;
                }
            }
            // std::cout << "Running CPU" << std::endl;
            CPU::run();
            //run nes frame here
            render();

            // Wait to mantain framerate:
            frameTime = SDL_GetTicks() - frameStart;
            if (frameTime < DELAY)
                SDL_Delay((int)(DELAY - frameTime));
            // unsigned int a[SCREEN_HEIGHT * SCREEN_WIDTH];
            // for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++)
            // {
            //     a[i] = randColor();
            // }
            // update_texture(a);
            update_texture(off_screen_buffer);
        }

        //Free resources and close SDL
        close();

        return 0;
    }
} // namespace GUI
