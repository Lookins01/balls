#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "raylib.h"
#include "raymath.h"


#define CIRCLES 50
#define PARTICLES 50

#define WIDTH 800
#define HEIGHT 600
#define CIRCLE_RADIUS_MAX 25
#define CIRCLE_RADIUS_MIN 15

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
} Circle;

static Circle circles[CIRCLES] = {0};
static bool is_circles_move = true;


typedef struct {
    Vector2 pos;
    Vector2 velocity;
    float lifetime;
    float radius;
} Particle;

static Particle particles[PARTICLES];


static int width;
static int height;


void init_particles()
{
    for (int i = 0; i < PARTICLES; ++i) {
        particles[i].lifetime = 0.0f;
    }
}


int get_free_particle_index()
{
    for (int i = 0; i < PARTICLES; ++i) {
        if (particles[i].lifetime <= 0.0f) return i;
    }
   return -1; 
}


void rand_particle(Vector2 pos)
{
    int index = get_free_particle_index();
    if (index < 0) return;
    Particle *particle = &particles[index];
    particle->pos = pos;
    particle->radius = 1 + rand() % 3;
    particle->lifetime = (float)rand() / (float)RAND_MAX;
    particle->velocity.x = -100 + rand() % 200;
    particle->velocity.y = -100 + rand() % 200;

}


void update_particle_pos(int index, float dt)
{
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


void draw_particles(float dt)
{
    for (int i = 0; i < PARTICLES; ++i) {
        Particle *particle = &particles[i];
        if (particle->lifetime <= 0) continue;
        DrawCircleV(particle->pos, particle->radius, RED);
        update_particle_pos(i, dt);
        particle->lifetime -= dt;
    }
}


void rand_circle(int index) 
{
    circles[index].radius = CIRCLE_RADIUS_MIN + (rand() % (CIRCLE_RADIUS_MAX - CIRCLE_RADIUS_MIN));
    circles[index].pos.x = circles[index].radius + (rand() % (width - (int)circles[index].radius * 2));
    if (circles[index].pos.x + circles[index].radius >= width) 
        circles[index].pos.x = width - circles[index].radius;
    circles[index].pos.y = circles[index].radius + (rand() % (height - (int)circles[index].radius * 2));
    if (circles[index].pos.y + circles[index].radius >= height) 
        circles[index].pos.y = height - circles[index].radius;
    circles[index].velocity.x = 100 + rand() % 50;
    circles[index].velocity.y = 100 + rand() % 50;
    circles[index].timer = 0.0f;
}

void init_circles(void)
{
    for (int i = 0; i < CIRCLES; ++i) {
        rand_circle(i); 
        circles[i].state = MOVE;
    }
}


void print_circle(int index)
{
    Circle circle = circles[index];
    printf("circles[%d] = {\n", index);
    printf("\t.pos      = { .x = %.2f, .y = %.2f},\n", circle.pos.x, circle.pos.y);
    printf("\t.velocity = { .x = %.2f, .y = %.2f },\n", circle.velocity.x, circle.velocity.y);
    printf("\t.radius   = %.2f\n", circle.radius);
    printf("}\n");
}


void update_circle_pos(int index, float dt) 
{
    if (!is_circles_move) return;

    Circle *circle = &circles[index];
    float x = circle->pos.x + circle->velocity.x*dt;
    if (x - circle->radius < 0 || x + circle->radius > width) {
        circle->velocity.x *= -1;
    } else {
        circle->pos.x = x;
    }

    float y = circle->pos.y + circle->velocity.y*dt;
    if (y - circle->radius < 0 || y + circle->radius > height) {
        circle->velocity.y *= -1;
    } else {
        circle->pos.y = y;
    }

}


void draw_circle_move(int index, float dt) 
{
    Circle *circle = &circles[index];
    DrawCircleGradient(circle->pos.x, circle->pos.y, circle->radius, GREEN, YELLOW);
    
    update_circle_pos(index, dt); 

    bool hoverover = CheckCollisionPointCircle(GetMousePosition(), circle->pos, circle->radius);

    if (hoverover) {
        circle->radius += dt*10;
        if (circle->radius > CIRCLE_RADIUS_MAX + 10) {
            circle->state = POP;
            circle->timer = 0.0f;
        } 
    }
}


void draw_circle_pop(int index, float dt) {
    Circle *circle = &circles[index];
    circle->timer += dt;
    float radius = circle->radius * circle->timer / 0.5f; 
    DrawRing(circle->pos, circle->radius, radius, 0, 360, 360, YELLOW);
    
    update_circle_pos(index, dt); 
    
    if (circle->timer > 0.5f) {
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
    DrawCircleGradient(circle->pos.x, circle->pos.y, radius, GREEN, YELLOW);
    //DrawCircleV(circle->pos, radius, YELLOW);
    
    update_circle_pos(index, dt); 
    
    if (circle->timer > 0.75f) {
        circle->state = MOVE;
    }
}


int main()
{
    SetTraceLogLevel(LOG_WARNING);
    //InitWindow(WIDTH, HEIGHT, "Balls");
    InitWindow(0, 0, "Balls");
    SetTargetFPS(60);
    width = GetScreenWidth();
    height = GetScreenHeight();

    init_circles();
    init_particles();
    while (!WindowShouldClose()) {
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
                
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (CheckCollisionPointCircle(mouse, circles[i].pos, circles[i].radius))
                    print_circle(i); 
                } 
            }
            
            Vector2 mouse_delta = GetMouseDelta();
            if (mouse_delta.x != 0 || mouse_delta.y != 0) rand_particle(mouse);
            draw_particles(dt);
           
            if (IsKeyPressed(KEY_SPACE)) {
                is_circles_move = !is_circles_move;
            }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
