#include "raylib.h"
#include "raymath.h"

#ifndef PLATFORM_WEB

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#else

#define RAND_MAX 2147483647 
int rand(void);

#endif // PLATFORM_WEB

// Colors
// ------------------------------------------------------------
#define GRUVBOX_RED     CLITERAL(Color){0xFB, 0x49, 0x34, 0xFF}
#define GRUVBOX_GREEN   CLITERAL(Color){0xB8, 0xbb, 0x26, 0xFF}
#define GRUVBOX_YELLOW  CLITERAL(Color){0xFA, 0xBD, 0x2F, 0xFF}
#define GRUVBOX_BLUE    CLITERAL(Color){0x83, 0xA5, 0x98, 0xFF}
#define GRUVBOX_PURPLE  CLITERAL(Color){0xD3, 0x86, 0x9B, 0xFF}
#define GRUVBOX_AQUA    CLITERAL(Color){0x8E, 0xC0, 0x7C, 0xFF}
#define GRUVBOX_ORANGE  CLITERAL(Color){0xFE, 0x80, 0x19, 0xFF}
// -------------------------------------------------------------

#define CIRCLES 50 
#define PARTICLES 25
#define MOUSE_PARTICLES 100
#define BG_POINTS 25

#define WIDTH 1500
#define HEIGHT 800

//#define COLLISION


static int width = WIDTH;
static int height = HEIGHT;


// Settings
// --------------------------------------
static bool pop_on_collision = true;
// --------------------------------------

static Color colors[] = {GRUVBOX_RED, GRUVBOX_GREEN, GRUVBOX_YELLOW, GRUVBOX_BLUE, GRUVBOX_PURPLE, GRUVBOX_AQUA, GRUVBOX_ORANGE};
//static Color colors[] = {RED, YELLOW, GREEN, PURPLE, BROWN, PINK, ORANGE, GOLD, BLUE, VIOLET};
static int colors_count = sizeof(colors)/sizeof(*colors);


typedef struct {
    Vector2 pos;
    Vector2 velocity;
    float max_lifetime;
    float lifetime;
    float radius;
    Color color;
} Particle;

static Particle particles[MOUSE_PARTICLES];


typedef enum {
    BORN = 0,
    MOVE,
    POP,
    VANISH,
} CircleState;


typedef struct {
    Vector2 velocity;
    Vector2 pos;
    float radius;
    CircleState state;
    float timer;
    Color color;
    Particle particles[PARTICLES];
} Circle;

static int circle_radius_max = 50;
static int circle_radius_min = 15;

static Circle circles[CIRCLES] = {0};
static bool is_circles_move = true;


void init_mouse_particles(void)
{
    for (int i = 0; i < MOUSE_PARTICLES; ++i) {
        particles[i].lifetime = 0.0f;
    }
}


int get_free_particle_index(void)
{
    for (int i = 0; i < MOUSE_PARTICLES; ++i) {
        if (particles[i].lifetime <= 0.0f) return i;
    }
   return -1; 
}


void rand_mouse_particle(Vector2 pos)
{
    int index = get_free_particle_index();
    if (index < 0) return;
    Particle *particle = &particles[index];
    particle->pos = pos;
    particle->radius = 1 + rand() % 3;
    particle->lifetime = (float)rand() / (float)RAND_MAX;
    particle->max_lifetime = particle->lifetime; 
    particle->velocity.x = -100 + rand() % 200;
    particle->velocity.y = -100 + rand() % 200;
    particle->color = colors[rand() % colors_count];
}


void update_particle_pos(Particle particles[], int particles_size, int index, float dt)
{
    if (index < 0 || index >= particles_size) return;
    Particle *particle = &particles[index];

    float x = particle->pos.x + particle->velocity.x*dt;
    if (x - particle->radius < 0 || x + particle->radius > width) {
        particle->velocity.x *= -1;
    } else {
        particle->pos.x = x;
    }

    float y = particle->pos.y + particle->velocity.y*dt;
    if (y - particle->radius < 0 || y + particle->radius > height) {
        particle->velocity.y *= -1;
    } else {
        particle->pos.y = y;
    }
}


void draw_particles(Particle particles[], int particles_count, float dt)
{
    for (int i = 0; i < particles_count; ++i) {
        Particle *particle = &particles[i];
        if (particle->lifetime <= 0) continue;
        float value = particle->lifetime / particle->max_lifetime;
        DrawCircleV(particle->pos, particle->radius, ColorAlpha(particle->color, value));
        update_particle_pos(particles, particles_count, i, dt);
        particle->lifetime -= dt;
    }
}


void rand_circle(int index) 
{
    circles[index].radius = circle_radius_min + (rand() % (circle_radius_max - circle_radius_min));
    circles[index].pos.x = circles[index].radius + (rand() % (width - (int)circles[index].radius * 2));
    if (circles[index].pos.x + circles[index].radius >= width) 
        circles[index].pos.x = width - circles[index].radius;
    circles[index].pos.y = circles[index].radius + (rand() % (height - (int)circles[index].radius * 2));
    if (circles[index].pos.y + circles[index].radius >= height) 
        circles[index].pos.y = height - circles[index].radius;
    circles[index].velocity.x = -150 + rand() % 300;
    circles[index].velocity.y = -150 + rand() % 300;
    circles[index].timer = 0.0f;
    circles[index].color = colors[rand() % colors_count];
}

void init_circles(void)
{
    for (int i = 0; i < CIRCLES; ++i) {
        rand_circle(i); 
        circles[i].state = MOVE;
    }
}

void init_circle_particles(int index)
{
    Circle *circle = &circles[index]; 

    for (int i = 0; i < PARTICLES; ++i) {
        Particle *particle = &circle->particles[i];
        particle->pos = circle->pos;
        particle->radius = 5 + rand() % 10;
        particle->lifetime = (float)rand() / (float)RAND_MAX;
        particle->max_lifetime = particle->lifetime; 
        particle->velocity.x = -500 + rand() % 1000;
        particle->velocity.y = -500 + rand() % 1000;
        particle->color = circle->color; //colors[rand() % colors_count];
    }
}

void print_circle(int index) 
{
    (void) index;
    return; 
}

/*
void print_circle(int index)
{
    Circle circle = circles[index];
    printf("circles[%d] = {\n", index);
    printf("\t.pos      = { .x = %.2f, .y = %.2f},\n", circle.pos.x, circle.pos.y);
    printf("\t.velocity = { .x = %.2f, .y = %.2f },\n", circle.velocity.x, circle.velocity.y);
    printf("\t.radius   = %.2f\n", circle.radius);
    printf("}\n");
}
*/

bool update_circle_collision(int index, float x, float y)
{
    Vector2 pos = {.x = x, .y = y};
    float radius = circles[index].radius;
    for (int i = 0; i < CIRCLES; ++i) {
        if (i == index) continue;
        Circle *circle = &circles[i];
        if (circle->state != MOVE) continue;
        if (CheckCollisionCircles(pos, radius, circle->pos, circle->radius)) {
            if (pop_on_collision) {
                circle->state = POP;
                circle->timer = 0.0f;
                init_circle_particles(i);

                circles[index].state = POP;
                circles[index].timer = 0.0f;
                init_circle_particles(index);
                return true;
            } 
#ifdef COLLISION  
            else {
                circle->velocity = Vector2Negate(circle->velocity);  
                circles[index].velocity = Vector2Negate(circles[index].velocity);  
                return true;
            }
#endif
        }
    }
    return false;
}

void update_circle_pos(int index, float dt) 
{
    if (!is_circles_move) return;

    Circle *circle = &circles[index];
    float x = circle->pos.x + circle->velocity.x*dt;
    float y = circle->pos.y + circle->velocity.y*dt;
    
    if (update_circle_collision(index, x, y)) {
        return;
    }
       
    if (x - circle->radius < 0 || x + circle->radius > width) {
        if (pop_on_collision) {
            circle->state = POP;
            init_circle_particles(index);
            circle->timer = 0.0f;
        } else {
            circle->velocity.x *= -1;
        }
    } else {
        circle->pos.x = x;
    }

    if (y - circle->radius < 0 || y + circle->radius > height) {
        if (pop_on_collision) {
            circle->state = POP;
            init_circle_particles(index);
            circle->timer = 0.0f;
        } else {
            circle->velocity.y *= -1;
        }
    } else {
        circle->pos.y = y;
    }

}


void draw_circle_move(int index, float dt) 
{
    Circle *circle = &circles[index];
    DrawCircleGradient(circle->pos.x, circle->pos.y, circle->radius, circle->color, ColorAlpha(circle->color, 0.5));

    if (CheckCollisionPointCircle(GetMousePosition(), circle->pos, circle->radius)) {
        circle->radius += dt*50;
        if (circle->radius > circle_radius_max + 10) {
            circle->state = POP;
            init_circle_particles(index);
            circle->timer = 0.0f;
        } 
    }
    
    update_circle_pos(index, dt); 
}


void draw_circle_pop(int index, float dt) {
    Circle *circle = &circles[index];
    circle->timer += dt;
    //float radius = circle->radius * circle->timer / 1.0f; 
    //DrawRing(circle->pos, radius, circle->radius, 0, 360, 360, ColorAlpha(circle->color, 0.5f));
    draw_particles(circle->particles, PARTICLES, dt); 

    //update_circle_pos(index, dt); 
    
    if (circle->timer > 1.0f) {
        rand_circle(index);
        circle->state = VANISH;
    }
}

void draw_circle_vanish(int index, float dt) {
    Circle *circle = &circles[index];
    circle->timer += dt;
    if (circle->timer > 1.0f) {
        rand_circle(index);
        circle->state = BORN;
    }
}


void draw_circle_born(int index, float dt)
{
    Circle *circle = &circles[index];
    circle->timer += dt;
    float value = circle->timer / 0.75f; 
    float radius = circle->radius * value; 
    DrawCircleGradient(circle->pos.x, circle->pos.y, radius, circle->color, ColorAlpha(circle->color, 0.5f));
    
    update_circle_pos(index, dt); 
    
    if (circle->timer > 0.75f) {
        circle->state = MOVE;
    }
}

/*
void procees_slider(int cx, int cy)
{
    static bool down = false;
    Vector2 center = {.x = cx, .y = cy};
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointCircle(GetMousePosition(), center, 6)) {
            down = true;
        }
    }
    if (down) {
    }
}
*/


void draw_menu(void) 
{
    Vector2 mouse = GetMousePosition();
    int x = 10, y = 10;
    int text_size = 20;
/*    
    // Draw radius slidebar controls   
    Rectangle max_radius_slider = {x, y, 200, 10};
    int MAX_RADIUS_SLIDEBAR_RADIUS = 6;
    int cx = x + Lerp(0.0f, 100.0f, (float)circle_radius_max/100);
    int cy = y + 10/2;
    DrawRectangleRec(max_radius_slider, RED);
    DrawCircle(cx, cy, MAX_RADIUS_SLIDEBAR_RADIUS, BLACK); 
    procees_slider(cx, cy);
*/
    Rectangle pop_on_collision_rec = {x, y, 20, 20};
    Color color = BLACK; //pop_on_collision ? RED : BLACK;
    int pop_on_collision_text_x = pop_on_collision_rec.x + 10 + pop_on_collision_rec.width;
    int pop_on_collision_text_y = pop_on_collision_rec.y + pop_on_collision_rec.height/2 - text_size/2;
    DrawRectangleRec(pop_on_collision_rec, color);
    DrawText("Pop on collision", pop_on_collision_text_x, pop_on_collision_text_y, text_size, RED);
    if (pop_on_collision) {
        DrawRectangle(pop_on_collision_rec.x+5, pop_on_collision_rec.y+5, pop_on_collision_rec.width-10, pop_on_collision_rec.height-10, GREEN);
    } else {
        DrawRectangle(pop_on_collision_rec.x+5, pop_on_collision_rec.y+5, pop_on_collision_rec.width-10, pop_on_collision_rec.height-10, RED);
    }

    if (CheckCollisionPointRec(mouse, pop_on_collision_rec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        pop_on_collision = !pop_on_collision;
    }

    
}




void game_frame(void)
{
    BeginDrawing();
    float dt = GetFrameTime();
    height = GetScreenHeight();
    width = GetScreenWidth();

    Vector2 mouse = GetMousePosition();
    
    ClearBackground(RAYWHITE);
    for (int i = 0; i < CIRCLES; ++i) {
        switch (circles[i].state) {
            case POP: draw_circle_pop(i, dt); break;
            case VANISH: draw_circle_vanish(i, dt); break;
            case BORN: draw_circle_born(i, dt); break;
            case MOVE: draw_circle_move(i, dt); break; 
            default: break;
        }
        
        //if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        //    if (CheckCollisionPointCircle(mouse, circles[i].pos, circles[i].radius))
        //    print_circle(i); 
        //} 
    }
    
    Vector2 mouse_delta = GetMouseDelta();
    if (mouse_delta.x != 0 || mouse_delta.y != 0) rand_mouse_particle(mouse); 
    draw_particles(particles, MOUSE_PARTICLES, dt);
   
    if (IsKeyPressed(KEY_SPACE)) {
        is_circles_move = !is_circles_move;
    }
    draw_menu();
    EndDrawing();
}


void raylib_js_set_entry(void (*entry)(void));


int main(void)
{
    #ifndef PLATFORM_WEB
        srand(time(NULL));
        SetTraceLogLevel(LOG_WARNING);
        SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    #endif

    //InitWindow(WIDTH, HEIGHT, "Balls");
    InitWindow(0, 0, "Balls");
    SetTargetFPS(60);
    width = GetScreenWidth();
    height = GetScreenHeight();

    init_circles();
    init_mouse_particles();

#ifdef PLATFORM_WEB
    raylib_js_set_entry(game_frame);
#else 
    while (!WindowShouldClose()) {
            game_frame();
    }
    CloseWindow();
#endif

    return 0;
}
