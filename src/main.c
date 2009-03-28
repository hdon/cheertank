#include <SDL.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK)) {
        return -2;
    }

    if (!SDL_SetVideoMode(640, 480, 0, SDL_OPENGL)) {
        return -1;
    }

    while (1) {
        SDL_Event event;
        if (SDL_PollEvent(&event))
        switch (event.type) {
            case SDL_QUIT:
                goto leave;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    goto leave;
                break;
        }
    }

    leave:
    puts("ok, bye!");
    SDL_Quit();
    return 0;
}

