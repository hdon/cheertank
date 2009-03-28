#include <SDL.h>
#include <GL/gl.h>
#include <stdio.h>
#include <string.h>
#include "oglconsole.h"

#define NUMTHINGS 1
#define NUMGAMES 16

/* The Player Input Buffer (PIB) holds the history of the player's input! */
int current_pib = 0;
struct {
    Sint16 x1, y1, x2, y2;
} PIB[NUMGAMES];

/* Represents any object of consequence in the game */
struct Thing {
    unsigned char type;
    unsigned char life;
    signed char zen; /* our primary interest :) */
    unsigned char angle; /* 256 angles is enough! */
    float x, y; /* position */
    float vx, vy; /* velocity */
};
#define PLAYER 1
#define ASTEROID 2

/* Represents all objects of consequence at a current point in time */
struct GameState {
    int time;
    int numthings;
    struct Thing things[NUMTHINGS];
};

/* We need a whole heap of game states! */
struct GameState gamestate[NUMGAMES];
int currentgamestate = 0;

void new_game() {
    struct GameState *gs = &gamestate[0];
    currentgamestate = 0;
    memset(&gamestate, 0, sizeof(gamestate));
    gs->things[0].type = PLAYER;
    gs->things[0].life = 10;
    gs->things[0].x =
    gs->things[0].y =
    gs->things[0].vx =
    gs->things[0].vy = 0.0;
    gs->numthings = 1;
}
void game_state_step(struct GameState *past, struct GameState *future) {
    struct Thing *pthings, *fthings;
    int i, j, numthings;

    /* Advance timer */
    future->time = past->time + 1;

    /* Grab what we'll be using */
    pthings = past->things;
    fthings = future->things;

    /* For each thing.. */
    j = 0;
    numthings = past->numthings;
    for (i=0; i<numthings; i++) {
        struct Thing thing = pthings[i];
        /* Advance thing's position */
        thing.x += thing.vx;
        thing.y += thing.vy;
        /* What does a thing do? */
        switch (thing.type) {
            case PLAYER:
                break;
        }
        /* Does the thing survive into the future? */
        if (thing.life > 0) {
            fthings[j++] = thing;
        }
    }

    /* Set future thing counter */
    future->numthings = j;
}
void game_step() {
    struct GameState *past, *future;
    past = &gamestate[currentgamestate];
    if (++currentgamestate >= NUMGAMES)
        currentgamestate = 0;
    future = &gamestate[currentgamestate];
    game_state_step(past, future);
}

int main(int argc, char **argv) {
    SDL_Joystick *joy = NULL;
    int i, n;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK)) {
        return -2;
    }

    if (!SDL_SetVideoMode(640, 480, 32, SDL_DOUBLEBUF|SDL_OPENGL)) {
        return -1;
    }

    /* Initialize joystick */
    /* Disable joystick events, we will poll the joystick */
    SDL_JoystickEventState(SDL_IGNORE);
    /* Enumerate joysticks */
    n = SDL_NumJoysticks();
    for (i=0; i<n; i++) {
        SDL_Joystick *joystick;
        const char *name;
        name = SDL_JoystickName(i);
        printf("Joystick %d: %s\n", i, name ? name : "Unknown Joystick");
        joystick = SDL_JoystickOpen(i);
        if (i == 0) joy = joystick;
        if (joystick == NULL) {
            fprintf(stderr, "SDL_JoystickOpen(%d) failed: %s\n", i,
                    SDL_GetError());
        } else {
            printf("       axes: %d\n", SDL_JoystickNumAxes(joystick));
            printf("      balls: %d\n", SDL_JoystickNumBalls(joystick));
            printf("       hats: %d\n", SDL_JoystickNumHats(joystick));
            printf("    buttons: %d\n", SDL_JoystickNumButtons(joystick));
            if (i>0) SDL_JoystickClose(joystick);
        }
    }

    /* Initialize OGLCONSOLE */
    OGLCONSOLE_Create();

    /* Initialize game */
    new_game();

    /* Main loop! */
    while (1) {
        float x, y;
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

        /* Game logic! */
        /* Get joystick input */
        SDL_JoystickUpdate();
        x = 0.15 * (float) (SDL_JoystickGetAxis(joy, 0) / 0x1000);
        y = 0.15 * (float) (SDL_JoystickGetAxis(joy, 1) / 0x1000);
        /* Save player input history into player input buffer */
        PIB[currentgamestate].x1 = x;
        PIB[currentgamestate].y1 = y;
        /* Update player control state TODO move this someplace else */
        gamestate[currentgamestate].things[0].vx = x;
        gamestate[currentgamestate].things[0].vy = y;
        game_step();

        /* Graphics! */
        glClear(GL_COLOR_BUFFER_BIT);

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

        /* Draw the player */
        glColor3ub(0, 255, 0);
        /* Player's "head" leads their position */
        x = x*3 + gamestate[currentgamestate].things[0].x;
        y = y*3 + gamestate[currentgamestate].things[0].y;
        glVertex2d(x +5, y +5);
        glVertex2d(x -5, y +5);
        glVertex2d(x -5, y -5);
        glVertex2d(x +5, y -5);

        /* Player's "body" shows their position */
        x = gamestate[currentgamestate].things[0].x;
        y = gamestate[currentgamestate].things[0].y;
        glVertex2d(x +5, y +5);
        glVertex2d(x -5, y +5);
        glVertex2d(x -5, y -5);
        glVertex2d(x +5, y -5);

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

