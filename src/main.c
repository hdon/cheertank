#include <SDL.h>
#include <GL/gl.h>
#include <stdio.h>
#include <string.h>
#include "oglconsole.h"

#define FPS 60
#define MAX_15 32768 // 2^15
#define NUMTHINGS 16
#define NUMGAMES 16
#define WIDTH 640
#define HEIGHT 480

/* The injection history buffer represents all data which affects game state but
 * which cannot be inferred from earlier game states. For now, this means only
 * player input. */
struct {
    Sint16 x1, y1;
} injection_history[NUMGAMES];

/* Represents any object of consequence in the game */
struct Thing {
    unsigned char type;
    unsigned char life;
    signed char zen; /* our primary interest :) */
    unsigned char angle; /* 256 angles is enough! */
    float x, y; /* position */
    float vx, vy; /* velocity */
    float ax, ay; /* acceleration */
};
#define PLAYER 1
#define ASTEROID 2
#define CANDY 3
#define HOMER 4

#define ASTEROIDF 0.013
#define HOMERF 2.30
#define CANDYF 2.30

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
    struct GameState *gs;
    struct Thing *things;
    int i;

    gs = &gamestate[0];
    currentgamestate = 0;
    memset(&gamestate, 0, sizeof(gamestate));

    things = gs->things;

    things[0].type = PLAYER;
    things[0].life = 10;
    things[0].zen = 0;
    things[0].x = 100.0;
    things[0].y = 200.0;
    things[0].vx =
    things[0].vy = 0.0;

    for (i=1; i<NUMTHINGS; i++) {
        things[i].type = ASTEROID;
        things[i].life = 10;
        things[i].zen = 0;
        things[i].x = (float)(Uint16)random() / (float)MAX_15 * WIDTH;
        things[i].y = (float)(Uint16)random() / (float)MAX_15 * HEIGHT;
        things[i].vx = (float)(Uint16)random() / (float)MAX_15;
        things[i].vy = (float)(Uint16)random() / (float)MAX_15;
        things[i].ax = 0.0f;
        things[i].ay = 0.0f;
    }

    things[1].type = HOMER;
    things[2].type = HOMER;
    things[3].type = CANDY;
    things[4].type = CANDY;
    gs->numthings = NUMTHINGS;
}
void game_state_step(struct GameState *past, struct GameState *future) {
    struct Thing *pthings, *fthings;
    int i, numliving, numthings;
    float px, py;

    /* Advance timer */
    future->time = past->time + 1;

    /* Grab what we'll be using */
    pthings = past->things;
    fthings = future->things;

    px = pthings[0].x;
    py = pthings[0].y;

    /* For each thing.. */
    numthings = past->numthings;
    numliving = 0;
    for (i=0; i<numthings; i++) {
        struct Thing thing = pthings[i];
        /* Advance thing's position */
        thing.vx += thing.ax;
        thing.vy += thing.ay;
        thing.x += thing.vx;
        if (thing.x < 0) thing.x += WIDTH;
        else if (thing.x >= WIDTH) thing.x -= WIDTH;
        thing.y += thing.vy;
        if (thing.y < 0) thing.y += HEIGHT;
        else if (thing.y >= HEIGHT) thing.y -= HEIGHT;

        /* Collide with player? XXX assumes player is Thing #0 */
        if ((thing.type != PLAYER)
        &&  (fabs(thing.x - px) < 10.0f)
        &&  (fabs(thing.y - py) < 10.0f)) {
            thing.life = 0;
        }

        /* What does a thing do? */
        switch (thing.type) {
            /* XXX IMPORTANT the player is assumed to be Thing #0 XXX */
            case PLAYER:
                px = thing.x;
                py = thing.y;
                break;
            case ASTEROID:
                if (thing.y == py) thing.ax = thing.x>px?-ASTEROIDF:ASTEROIDF;
                else if (thing.x == px) thing.ay = thing.y>py?-ASTEROIDF:ASTEROIDF;
                else {
                    float dx, dy, sum, sx, sy, fx, fy;

                    fx = thing.x>px?-ASTEROIDF:ASTEROIDF;
                    fy = thing.y>py?-ASTEROIDF:ASTEROIDF;
                    dx = fabs(thing.x - px);
                    dy = fabs(thing.y - py);
                    if (dx > (WIDTH /2)) { dx = WIDTH  - dx; fx *= -1; }
                    if (dy > (HEIGHT/2)) { dy = HEIGHT - dy; fy *= -1; }
                    sum = dx + dy;
                    sx = dx / sum;
                    sy = dy / sum;

                    thing.ax = sx * fx;
                    thing.ay = sy * fy;
                }
                break;
            case HOMER:
                if (thing.y == py) thing.vx = thing.x>px?-HOMERF:HOMERF;
                else if (thing.x == px) thing.vy = thing.y>py?-HOMERF:HOMERF;
                else {
                    float dx, dy, sum, sx, sy, fx, fy;

                    fx = thing.x>px?-HOMERF:HOMERF;
                    fy = thing.y>py?-HOMERF:HOMERF;
                    dx = fabs(thing.x - px);
                    dy = fabs(thing.y - py);
                    if (dx > (WIDTH /2)) { dx = WIDTH  - dx; fx *= -1; }
                    if (dy > (HEIGHT/2)) { dy = HEIGHT - dy; fy *= -1; }
                    sum = dx + dy;
                    sx = dx / sum;
                    sy = dy / sum;

                    thing.vx = sx * fx;
                    thing.vy = sy * fy;
                }
                break;
            case CANDY:
                if (thing.y == py) thing.vx = thing.x>px?-CANDYF:CANDYF;
                else if (thing.x == px) thing.vy = thing.y>py?-CANDYF:CANDYF;
                else {
                    float dx, dy, sum, sx, sy, fx, fy;

                    fx = thing.x>px?CANDYF:-CANDYF;
                    fy = thing.y>py?CANDYF:-CANDYF;
                    dx = fabs(thing.x - px);
                    dy = fabs(thing.y - py);
                    if (dx > (WIDTH /2)) { dx = WIDTH  - dx; fx *= -1; }
                    if (dy > (HEIGHT/2)) { dy = HEIGHT - dy; fy *= -1; }
                    sum = dx + dy;
                    sx = dx / sum;
                    sy = dy / sum;

                    thing.vx = sx * fx;
                    thing.vy = sy * fy;
                }
                break;
        }

        /* Avoid zombies ;) */
        if (thing.life > 0)
            fthings[numliving++] = thing;
    }

    /* Set future thing counter */
    future->numthings = numliving;
}
void game_step() {
    struct GameState *past, *future;
    int i;
    past = &gamestate[currentgamestate];
    if (++currentgamestate >= NUMGAMES)
        currentgamestate = 0;
    future = &gamestate[currentgamestate];
    for (i=0; i<1024*4; i++)
    game_state_step(past, future);
}

int main(int argc, char **argv) {
    SDL_Joystick *joy = NULL;
    int i, n;
    Uint32 next_tick = 0;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK)) {
        return -2;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    if (!SDL_SetVideoMode(640, 480, 32, SDL_OPENGL)) {
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
        int i, numthings;
        struct Thing *things;
        SDL_Event event;
        Uint32 current_tick;

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
        x = 0.0003 * (float) SDL_JoystickGetAxis(joy, 0);
        y = 0.0003 * (float) SDL_JoystickGetAxis(joy, 1);
        /* Save player input history into injection history buffer */
        injection_history[currentgamestate].x1 = x;
        injection_history[currentgamestate].y1 = y;
        /* Update player control state TODO move this someplace else */
        gamestate[currentgamestate].things[0].vx = x;
        gamestate[currentgamestate].things[0].vy = y;
        game_step();

        /* Graphics! */
        glClear(GL_COLOR_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glOrtho(0, WIDTH, HEIGHT, 0, 1, -1);
        //glTranslated(320, 240, 0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_TEXTURE_2D);

        numthings = gamestate[currentgamestate].numthings;
        things = gamestate[currentgamestate].things;
        glColor3ub(255,0,255);
        for (i=1; i<numthings; i++) {
            int x, y;
            const int c = 12, d = 9;
            x = things[i].x;
            y = things[i].y;
            switch (things[i].type) {
                case CANDY:
                    glColor3ub(255, 255, 90);
                    break;
                case HOMER:
                    glColor3ub(255, 0, 0);
                    break;
                case ASTEROID:
                    glColor3ub(100, 120, 140);
                    break;
            }
            glBegin(GL_TRIANGLE_FAN);
            glVertex2d(x, y);
            glVertex2d(x  , y+c);
            glVertex2d(x+d, y+d);
            glVertex2d(x+c, y  );
            glVertex2d(x+d, y-d);
            glVertex2d(x  , y-c);
            glVertex2d(x-d, y-d);
            glVertex2d(x-c, y  );
            glVertex2d(x-d, y+d);
            glVertex2d(x  , y+c);
            glEnd();
        }

        /* Draw the player */
        glBegin(GL_QUADS);
        glColor3ub(0, 255, 0);
        /* Player's "head" leads their position */
        x = x + gamestate[currentgamestate].things[0].x;
        y = y + gamestate[currentgamestate].things[0].y;
        glVertex2d(x +5, y +5);
        glVertex2d(x -5, y +5);
        glVertex2d(x -5, y -5);
        glVertex2d(x +5, y -5);

        /* Player's "body" shows their real position */
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

        /* Wait! */
        current_tick = SDL_GetTicks();
        /* Did we hitch? */
        if (current_tick >= next_tick) {
            static Uint32 last_hitch_warning = 0;
            static int hitch_total = 0;

            /* First time through? Not a hitch! */
            if (next_tick != 0) {
                hitch_total++;
                if (current_tick >= last_hitch_warning + 10000) {
                    OGLCONSOLE_Print("WARNING: hitch! (total %d)\n",
                            hitch_total);
                    last_hitch_warning = current_tick;
                }
            }

            /* Recalculate our next tick */
            next_tick = current_tick + 1000/FPS;

        } else {
            /* Wait a little while! */
            if (next_tick - current_tick > 10)
                SDL_Delay(next_tick - current_tick);
            next_tick += 1000/FPS;
        }
    }

    leave:
    puts("ok, bye!");
    OGLCONSOLE_Quit();
    SDL_Quit();
    return 0;
}

