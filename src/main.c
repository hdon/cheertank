#include <SDL.h>
#include <GL/gl.h>
#include <stdio.h>
#include "oglconsole.h"

int main(int argc, char **argv) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK)) {
        return -2;
    }

    if (!SDL_SetVideoMode(640, 480, 32, SDL_DOUBLEBUF|SDL_OPENGL)) {
        return -1;
    }

    OGLCONSOLE_Create();

    while (1) {
        int i;
        SDL_Event event;
        if (SDL_PollEvent(&event))
        if (!OGLCONSOLE_SDLEvent(&event))
        switch (event.type) {
            case SDL_QUIT:
                goto leave;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    goto leave;
                break;
        }

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glOrtho(0, 640, 480, 0, 1, -1);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
        for (i=0; i<10; i++) {
            int c = random();
            glColor3ub(c&255, c>>8&255, c>>16&255);
            glVertex2d(10*i -10, 10*i +10);
            glVertex2d(10*i -10, 10*i -10);
            glVertex2d(10*i +10, 10*i -10);
            glVertex2d(10*i +10, 10*i +10);
        }
        glEnd();

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        OGLCONSOLE_Draw();
        SDL_GL_SwapBuffers();
    }

    leave:
    puts("ok, bye!");
    OGLCONSOLE_Quit();
    SDL_Quit();
    return 0;
}

