#include <GLFW/glfw3.h>
#include "linmath.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>
#include <math.h>

using namespace std;

const float DEG2RAD = 3.14159f / 180.0f;

// Function prototypes
void processInput(GLFWwindow* window);
void CheckCircleMerges();

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };
enum ONOFF { ON, OFF };

class Brick
{
public:
    float red, green, blue;
    float x, y, width;
    float speed;
    int directionX; // 1 = right, -1 = left
    BRICKTYPE brick_type;
    ONOFF onoff;
    bool wasHit;

    Brick(BRICKTYPE bt, float xx, float yy, float ww, float rr, float gg, float bb)
    {
        brick_type = bt;
        x = xx;
        y = yy;
        width = ww;
        red = rr;
        green = gg;
        blue = bb;
        onoff = ON;
        wasHit = false;
        speed = 0.01f;
        directionX = 1;
    }

    void drawBrick()
    {
        if (onoff == ON)
        {
            float halfside = width / 2.0f;

            if (wasHit)
                glColor3f(1.0f, 0.0f, 0.0f); // red after hit
            else
                glColor3f(red, green, blue);

            glBegin(GL_POLYGON);
            glVertex2f(x + halfside, y + halfside);
            glVertex2f(x + halfside, y - halfside);
            glVertex2f(x - halfside, y - halfside);
            glVertex2f(x - halfside, y + halfside);
            glEnd();
        }
    }

    void moveBrick()
    {
        if (onoff == OFF)
            return;

        x += speed * directionX;

        if (x + width / 2.0f >= 1.0f || x - width / 2.0f <= -1.0f)
        {
            directionX *= -1;
        }
    }
};

class Circle
{
public:
    float red, green, blue;
    float radius;
    float x;
    float y;
    float speed;
    int direction;
    // 1 = up
    // 2 = right
    // 3 = down
    // 4 = left
    // 5 = up right
    // 6 = up left
    // 7 = down right
    // 8 = down left

    Circle(float xx, float yy, float rad, int dir, float r, float g, float b)
    {
        x = xx;
        y = yy;
        radius = rad;
        direction = dir;
        red = r;
        green = g;
        blue = b;
        speed = 0.01f;
    }

    void CheckCollision(Brick* brk)
    {
        if (brk->onoff == OFF)
            return;

        float half = brk->width / 2.0f;
        float left = brk->x - half;
        float right = brk->x + half;
        float bottom = brk->y - half;
        float top = brk->y + half;

        bool hitX = (x + radius >= left) && (x - radius <= right);
        bool hitY = (y + radius >= bottom) && (y - radius <= top);

        if (hitX && hitY)
        {
            brk->wasHit = true;

            if (brk->brick_type == DESTRUCTABLE)
            {
                brk->onoff = OFF;
            }

            if (direction == 1) direction = 3;
            else if (direction == 3) direction = 1;
            else if (direction == 2) direction = 4;
            else if (direction == 4) direction = 2;
            else if (direction == 5) direction = 8;
            else if (direction == 6) direction = 7;
            else if (direction == 7) direction = 6;
            else if (direction == 8) direction = 5;

            // Small push so the circle does not stay stuck inside the brick
            x += 0.02f;
            y += 0.02f;
        }
    }

    int GetRandomDirection()
    {
        return (rand() % 8) + 1;
    }

    void MoveOneStep()
    {
        // Up
        if (direction == 1 || direction == 5 || direction == 6)
        {
            if (y < 1.0f - radius)
                y += speed;
            else
                direction = GetRandomDirection();
        }

        // Right
        if (direction == 2 || direction == 5 || direction == 7)
        {
            if (x < 1.0f - radius)
                x += speed;
            else
                direction = GetRandomDirection();
        }

        // Down
        if (direction == 3 || direction == 7 || direction == 8)
        {
            if (y > -1.0f + radius)
                y -= speed;
            else
                direction = GetRandomDirection();
        }

        // Left
        if (direction == 4 || direction == 6 || direction == 8)
        {
            if (x > -1.0f + radius)
                x -= speed;
            else
                direction = GetRandomDirection();
        }
    }

    void DrawCircle()
    {
        glColor3f(red, green, blue);

        glBegin(GL_POLYGON);
        for (int i = 0; i < 360; i++)
        {
            float degInRad = i * DEG2RAD;
            glVertex2f((cos(degInRad) * radius) + x,
                (sin(degInRad) * radius) + y);
        }
        glEnd();
    }
};

// Global vectors
vector<Circle> world;
vector<Brick> bricks;
bool bkeypressed = false;
bool spacePressed = false;

int main(void)
{
    srand((unsigned int)time(NULL));

    if (!glfwInit())
    {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(600, 600, "8-2 Assignment", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glClearColor(0.1f, 0.1f, 0.3f, 1.0f);

    bricks.push_back(Brick(REFLECTIVE, 0.5f, -0.33f, 0.2f, 1.0f, 1.0f, 0.0f));
    bricks.push_back(Brick(DESTRUCTABLE, -0.5f, 0.33f, 0.2f, 0.0f, 1.0f, 0.0f));
    bricks.push_back(Brick(DESTRUCTABLE, -0.5f, -0.33f, 0.2f, 0.0f, 1.0f, 1.0f));
    bricks.push_back(Brick(REFLECTIVE, 0.0f, 0.0f, 0.2f, 1.0f, 0.5f, 0.5f));

    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT);

        processInput(window);

        // Move circles and check brick collisions
        for (int i = 0; i < world.size(); i++)
        {
            world[i].MoveOneStep();

            for (int j = 0; j < bricks.size(); j++)
            {
                world[i].CheckCollision(&bricks[j]);
            }
        }

        // Merge circles if they touch
        CheckCircleMerges();

        // Draw circles
        for (int i = 0; i < world.size(); i++)
        {
            world[i].DrawCircle();
        }

        // Move and draw bricks
        for (int i = 0; i < bricks.size(); i++)
        {
            bricks[i].moveBrick();
            bricks[i].drawBrick();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    exit(EXIT_SUCCESS);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    // Space creates one circle per press
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spacePressed)
    {
        float r = (float)rand() / RAND_MAX;
        float g = (float)rand() / RAND_MAX;
        float b = (float)rand() / RAND_MAX;

        Circle newCircle(0.0f, 0.0f, 0.05f, 2, r, g, b);
        world.push_back(newCircle);

        spacePressed = true;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        spacePressed = false;
    }

    // B creates one brick per press
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !bkeypressed)
    {
        float x = ((float)rand() / RAND_MAX) * 1.6f - 0.8f;
        float y = ((float)rand() / RAND_MAX) * 1.6f - 0.8f;

        float r = (float)rand() / RAND_MAX;
        float g = (float)rand() / RAND_MAX;
        float b = (float)rand() / RAND_MAX;

        BRICKTYPE type;

        if (rand() % 2 == 0)
            type = REFLECTIVE;
        else
            type = DESTRUCTABLE;

        bricks.push_back(Brick(type, x, y, 0.2f, r, g, b));

        bkeypressed = true;
    }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
    {
        bkeypressed = false;
    }
}

void CheckCircleMerges()
{
    for (int i = 0; i < world.size(); i++)
    {
        for (int j = i + 1; j < world.size(); j++)
        {
            float dx = world[i].x - world[j].x;
            float dy = world[i].y - world[j].y;
            float distance = sqrt(dx * dx + dy * dy);

            if (distance <= world[i].radius + world[j].radius)
            {
                float newX = (world[i].x + world[j].x) / 2.0f;
                float newY = (world[i].y + world[j].y) / 2.0f;
                float newRadius = (world[i].radius + world[j].radius) * 0.75f;

                float newR = (world[i].red + world[j].red) / 2.0f;
                float newG = (world[i].green + world[j].green) / 2.0f;
                float newB = (world[i].blue + world[j].blue) / 2.0f;

                int newDirection = world[i].direction;

                Circle merged(newX, newY, newRadius, newDirection, newR, newG, newB);

                world.erase(world.begin() + j);
                world.erase(world.begin() + i);
                world.push_back(merged);

                return;
            }
        }
    }
}