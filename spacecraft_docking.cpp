#include <windows.h>
#include <GL/freeglut.h>
#include <GL/glu.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int WINDOW_WIDTH = 1100;
const int WINDOW_HEIGHT = 720;
// SPACECRAFT
float shipX = 300.0f;
float shipY = 350.0f;
float shipAngle = 0.0f;

float velocityX = 0.0f;
float velocityY = 0.0f;
float speed = 0.0f;

float fuel = 100.0f;

// Rear docking port on spacecraft.
const float SHIP_PORT_OFFSET = 55.0f;

float portX = 0.0f;
float portY = 0.0f;
// DOCKING STATION
const float dockingX = 800.0f;
const float dockingY = 350.0f;

float DOCKING_ZONE = 38.0f;

// Final-approach envelope is slightly larger than the actual
// docking zone and is shown as the spacecraft gets close.
float DOCKING_ENVELOPE = 72.0f;

float FINAL_APPROACH_ZONE = 150.0f;
float GUIDANCE_ZONE = 280.0f;

float MAX_DOCKING_SPEED = 2.0f;

// Final-approach relative-motion limits.
float MAX_LATERAL_DOCKING_SPEED = 0.65f;
float COLLISION_LATERAL_SPEED = 1.20f;
float COLLISION_ALIGNMENT = 28.0f;

// Docking-quality speed bands. A controlled contact up to 2 m/s
// is still allowed, but higher contact speed gives a lower score.
float PERFECT_DOCKING_SPEED = 0.25f;
float GOOD_DOCKING_SPEED = 0.75f;

// Low-fuel thrust limits.
const float LOW_FUEL_THRESHOLD = 20.0f;
const float CRITICAL_FUEL_THRESHOLD = 10.0f;

// Docking sequence timing.
const float CONTACT_HOLD_TIME = 0.35f;
const float CLAMP_LOCK_TIME = 0.75f;

// Alignment is based on the actual tail direction and the
// direction from the tail toward the docking port.
float ALIGNMENT_TOLERANCE = 15.0f;
// FUEL THRUST MULTIPLIER
float getThrustMultiplier()
{
    if (fuel <= 0.0f)
        return 0.0f;

    if (fuel <= CRITICAL_FUEL_THRESHOLD)
        return 0.35f;

    if (fuel <= LOW_FUEL_THRESHOLD)
        return 0.65f;

    return 1.0f;
}
// PHYSICS
const float THRUST_ACCELERATION = 0.75f;
const float REVERSE_ACCELERATION = 0.55f;
const float BRAKE_ACCELERATION = 1.40f;

const float MAX_SPEED = 8.0f;
const float SPACE_DRAG = 0.9995f;

const float THRUST_FUEL_RATE = 3.5f;
// GAME STATE
// 0 = playing
// 1 = successful docking
// 2 = failed docking
// 3 = contact detected
// 4 = clamps engaging

int gameState = 0;
// STAGE 9 - MENU / DIFFICULTY / HELP
// screenState: 0 = main menu, 1 = simulator
int screenState = 0;
int selectedDifficulty = 2; // 1 Easy, 2 Medium, 3 Hard
int hoveredDifficulty = 0; // 0 = none, 1 = Easy, 2 = Medium, 3 = Hard
float startingFuel = 100.0f;
int helpOpen = 0;

enum HoverAction
{
    HOVER_NONE = 0, HOVER_EASY, HOVER_MEDIUM, HOVER_HARD,
    HOVER_MENU_QUIT, HOVER_HELP, HOVER_RETRY, HOVER_RESULT_MENU,
    HOVER_RESULT_HELP, HOVER_RESULT_QUIT
};
int hoveredAction = HOVER_NONE;

GLuint spaceBackgroundTexture = 0;
int spaceBackgroundLoaded = 0;

const int EASY = 1;
const int MEDIUM = 2;
const int HARD = 3;

void resetSpacecraft();


char difficultyName[20] = "MEDIUM";

void applyDifficulty(int difficulty)
{
    selectedDifficulty = difficulty;

    if (difficulty == EASY)
    {
        sprintf(difficultyName, "EASY");
        startingFuel = 100.0f;

        DOCKING_ZONE = 48.0f;
        DOCKING_ENVELOPE = 88.0f;
        FINAL_APPROACH_ZONE = 180.0f;
        GUIDANCE_ZONE = 320.0f;

        MAX_DOCKING_SPEED = 2.0f;
        MAX_LATERAL_DOCKING_SPEED = 0.85f;
        COLLISION_LATERAL_SPEED = 1.55f;
        COLLISION_ALIGNMENT = 38.0f;

        PERFECT_DOCKING_SPEED = 0.40f;
        GOOD_DOCKING_SPEED = 0.90f;
        ALIGNMENT_TOLERANCE = 20.0f;
    }
    else if (difficulty == HARD)
    {
        sprintf(difficultyName, "HARD");
        startingFuel = 75.0f;

        DOCKING_ZONE = 30.0f;
        DOCKING_ENVELOPE = 58.0f;
        FINAL_APPROACH_ZONE = 120.0f;
        GUIDANCE_ZONE = 240.0f;

        MAX_DOCKING_SPEED = 1.50f;
        MAX_LATERAL_DOCKING_SPEED = 0.40f;
        COLLISION_LATERAL_SPEED = 0.80f;
        COLLISION_ALIGNMENT = 20.0f;

        PERFECT_DOCKING_SPEED = 0.15f;
        GOOD_DOCKING_SPEED = 0.50f;
        ALIGNMENT_TOLERANCE = 10.0f;
    }
    else
    {
        sprintf(difficultyName, "MEDIUM");
        startingFuel = 90.0f;

        DOCKING_ZONE = 38.0f;
        DOCKING_ENVELOPE = 72.0f;
        FINAL_APPROACH_ZONE = 150.0f;
        GUIDANCE_ZONE = 280.0f;

        MAX_DOCKING_SPEED = 2.0f;
        MAX_LATERAL_DOCKING_SPEED = 0.65f;
        COLLISION_LATERAL_SPEED = 1.20f;
        COLLISION_ALIGNMENT = 28.0f;

        PERFECT_DOCKING_SPEED = 0.25f;
        GOOD_DOCKING_SPEED = 0.75f;
        ALIGNMENT_TOLERANCE = 15.0f;
    }
}

extern int lastTime;

void enterGame(int difficulty)
{
    applyDifficulty(difficulty);
    screenState = 1;
    helpOpen = 0;
    hoveredDifficulty = 0;
    resetSpacecraft();
    lastTime = glutGet(GLUT_ELAPSED_TIME);
}

void returnToMenu()
{
    screenState = 0;
    helpOpen = 0;
    hoveredDifficulty = 0;
    resetSpacecraft();
}
// CONTROLS
int thrustForward = 0;
int thrustBackward = 0;

int rotateLeft = 0;
int rotateRight = 0;

int braking = 0;
// TIME
int lastTime = 0;
// DOCKING DATA
float dockingDistance = 0.0f;
float angleError = 0.0f;
float relativeApproachSpeed = 0.0f;

// Positive = moving toward station.
// Negative is retained internally by the velocity calculation,
// but the HUD displays closing speed as zero when moving away.
float closingSpeedSigned = 0.0f;

// Estimated time before reaching the station at the current
// closing speed.
float timeToDock = 0.0f;

// Distance required to stop at the current closing speed.
float brakingDistance = 0.0f;

// Relative lateral velocity at the rear docking port.
float lateralVelocity = 0.0f;

// Mission timing.
float missionTime = 0.0f;

// Contact / docking sequence.
float dockingSequenceTime = 0.0f;
float contactSpeed = 0.0f;
float contactLateralSpeed = 0.0f;
float contactAngleError = 0.0f;
int dockingQuality = 0; // 0 none, 1 hard, 2 good, 3 perfect; negative = failure reason.
char failureReason[80] = "";

void calculateScore();
// SCORE
int missionScore = 0;
// FUNCTION DECLARATIONS
void resetSpacecraft();

void updateDockingPort();
void updateDistance();
void updateSpeed();
void updateAngleError();
void updateApproachSpeed();
void updateApproachPrediction();
void updateRelativeVelocity();
void updateDockingSequence(float dt);
void setDockingFailure(const char* reason);
void evaluateDockingQuality();
float getThrustMultiplier();

void updatePhysics(float dt);
void limitSpeed();

int isAligned();
int isSafeSpeed();
// DOCKING QUALITY / FAILURE
void evaluateDockingQuality()
{
    dockingQuality = 1;

    if (contactSpeed <= PERFECT_DOCKING_SPEED &&
        contactLateralSpeed <= 0.20f &&
        contactAngleError <= 5.0f)
    {
        dockingQuality = 3;
    }
    else if (contactSpeed <= GOOD_DOCKING_SPEED &&
             contactLateralSpeed <= MAX_LATERAL_DOCKING_SPEED &&
             contactAngleError <= 10.0f)
    {
        dockingQuality = 2;
    }
}

void setDockingFailure(const char* reason)
{
    gameState = 2;
    velocityX = 0.0f;
    velocityY = 0.0f;
    speed = 0.0f;
    dockingSequenceTime = 0.0f;

    int i = 0;
    while (reason[i] != '\0' && i < 78)
    {
        failureReason[i] = reason[i];
        i++;
    }
    failureReason[i] = '\0';

    dockingQuality = -1;
}

void updateDockingSequence(float dt)
{
    dockingSequenceTime += dt;

    if (gameState == 3 &&
        dockingSequenceTime >= CONTACT_HOLD_TIME)
    {
        gameState = 4;
        dockingSequenceTime = 0.0f;
    }
    else if (gameState == 4 &&
             dockingSequenceTime >= CLAMP_LOCK_TIME)
    {
        gameState = 1;
        dockingSequenceTime = 0.0f;
        calculateScore();
    }
}

void calculateScore();

void drawCircle(float cx, float cy, float radius, int segments);

void drawCircle(float cx, float cy, float radius, int segments)
{
    if (segments < 3)
        segments = 3;

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);

    for (int i = 0; i <= segments; ++i)
    {
        float angle =
            2.0f * 3.1415926f *
            (float)i / (float)segments;

        glVertex2f(
            cx + cosf(angle) * radius,
            cy + sinf(angle) * radius
        );
    }

    glEnd();
}

void drawCircleOutline(float cx, float cy, float radius, int segments);
void drawGlowDisc(float cx, float cy, float radius, float r, float g, float b, float alpha);
void drawHexPanel(float cx, float cy, float w, float h);
void drawPanelLines(float x, float y, float w, float h, int cols, int rows);


void drawSceneLightEffects()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Very subtle environmental illumination from the upper-left.
    // These are background light fields, not solid beams.
    glColor4f(1.0f, 0.82f, 0.48f, 0.018f);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.0f, 720.0f);
    glVertex2f(850.0f, 620.0f);
    glVertex2f(720.0f, 420.0f);
    glEnd();

    glColor4f(0.20f, 0.38f, 0.65f, 0.018f);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.0f, 680.0f);
    glVertex2f(720.0f, 390.0f);
    glVertex2f(560.0f, 160.0f);
    glEnd();

    glDisable(GL_BLEND);
}



void drawSun()
{
   
    const float sx = -18.0f;
    const float sy = 700.0f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Broad, very soft corona.
    for (int r = 180; r >= 50; r -= 10)
    {
        float falloff = (float)(r - 50) / 130.0f;
        float alpha = 0.006f + (1.0f - falloff) * 0.010f;
        glColor4f(1.0f, 0.62f + 0.18f * (1.0f - falloff),
                  0.20f, alpha);
        drawCircle(sx, sy, (float)r, 64);
    }

    // Long, faint illumination rays.
    glColor4f(1.0f, 0.78f, 0.38f, 0.018f);
    glBegin(GL_TRIANGLES);
    glVertex2f(sx, sy);
    glVertex2f(430.0f, 720.0f);
    glVertex2f(450.0f, 665.0f);

    glVertex2f(sx, sy);
    glVertex2f(420.0f, 500.0f);
    glVertex2f(390.0f, 475.0f);

    glVertex2f(sx, sy);
    glVertex2f(300.0f, 300.0f);
    glVertex2f(255.0f, 325.0f);
    glEnd();

    // Only a small portion of the solar disk is visible at the corner.
    glColor4f(1.0f, 0.80f, 0.38f, 0.82f);
    drawCircle(sx, sy, 42.0f, 64);

    glColor4f(1.0f, 0.95f, 0.68f, 0.52f);
    drawCircle(sx - 5.0f, sy + 5.0f, 25.0f, 64);

    glDisable(GL_BLEND);
}


void drawDistantStarfield()
{
    // Faint distant layer. It is deliberately sparse so the nearer stars
    // remain readable and the scene does not become visually noisy.
    glPointSize(1.0f);
    glBegin(GL_POINTS);

    for (int i = 0; i < 75; ++i)
    {
        unsigned int n = (unsigned int)(i * 2246822519u + 3266489917u);
        float x = (float)(n % 1800u) - 350.0f;
        n = n * 2246822519u + 3266489917u;
        float y = (float)(n % 1050u) - 160.0f;

        glColor4f(0.38f, 0.48f, 0.62f, 0.32f);
        glVertex2f(x, y);
    }

    glEnd();
}

void drawStars()
{
    // A wider, layered starfield to make the camera feel less zoomed-in.
    glPointSize(1.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 260; ++i)
    {
        unsigned int n = (unsigned int)(i * 1103515245u + 12345u);
        float x = (float)(n % 1500u) - 200.0f;
        n = n * 1103515245u + 12345u;
        float y = (float)(n % 900u) - 90.0f;

        float brightness = 0.35f + (float)((n >> 8) % 65u) / 100.0f;
        glColor3f(brightness * 0.82f, brightness * 0.90f, brightness);
        glVertex2f(x, y);
    }
    glEnd();

    // A second, dimmer layer suggests depth.
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 55; ++i)
    {
        unsigned int n = (unsigned int)(i * 2654435761u + 777u);
        float x = (float)(n % 1500u) - 200.0f;
        n = n * 2654435761u + 777u;
        float y = (float)(n % 900u) - 90.0f;

        glColor3f(0.70f, 0.78f, 0.92f);
        glVertex2f(x, y);
    }
    glEnd();
}


void drawGlowDisc(float cx, float cy, float radius, float r, float g, float b, float alpha)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 7; i >= 1; --i)
    {
        float rr = radius * (0.45f + 0.55f * (float)i / 7.0f);
        float a = alpha * (1.0f - (float)i / 8.0f) * 0.55f;
        glColor4f(r, g, b, a);
        drawCircle(cx, cy, rr, 48);
    }

    glColor4f(r, g, b, alpha);
    drawCircle(cx, cy, radius * 0.42f, 40);
    glDisable(GL_BLEND);
}

void drawHexPanel(float cx, float cy, float w, float h)
{
    glBegin(GL_POLYGON);
    glVertex2f(cx - w * 0.50f, cy);
    glVertex2f(cx - w * 0.25f, cy + h * 0.50f);
    glVertex2f(cx + w * 0.25f, cy + h * 0.50f);
    glVertex2f(cx + w * 0.50f, cy);
    glVertex2f(cx + w * 0.25f, cy - h * 0.50f);
    glVertex2f(cx - w * 0.25f, cy - h * 0.50f);
    glEnd();
}

void drawPanelLines(float x, float y, float w, float h, int cols, int rows)
{
    glColor4f(0.65f, 0.78f, 0.92f, 0.35f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (int i=1; i<cols; ++i)
    {
        float xx=x+w*(float)i/(float)cols;
        glVertex2f(xx,y); glVertex2f(xx,y+h);
    }
    for (int j=1; j<rows; ++j)
    {
        float yy=y+h*(float)j/(float)rows;
        glVertex2f(x,yy); glVertex2f(x+w,yy);
    }
    glEnd();
}


void drawCircleOutline(float cx, float cy, float radius, int segments)
{
    glBegin(GL_LINE_LOOP);

    for (int i = 0; i < segments; i++)
    {
        float angle =
            2.0f * 3.1415926f *
            (float)i / (float)segments;

        glVertex2f(
            cx + radius * cosf(angle),
            cy + radius * sinf(angle)
        );
    }

    glEnd();
}
// STARS

// EARTH
void drawEarth()
{
    // Earth is now a layered shaded disc instead of a flat blue circle.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    drawGlowDisc(145, 105, 96, 0.05f, 0.22f, 0.70f, 0.16f);

    glColor3f(0.025f, 0.08f, 0.22f);
    drawCircle(145,105,84,64);

    glColor3f(0.03f,0.20f,0.55f);
    drawCircle(145,105,76,64);

    glColor3f(0.04f,0.38f,0.82f);
    drawCircle(151,112,67,64);

    // Atmospheric highlight.
    glColor4f(0.25f,0.65f,1.0f,0.22f);
    drawCircle(157,120,57,64);

    // Continents.
    glColor3f(0.08f,0.55f,0.27f);
    glBegin(GL_POLYGON);
    glVertex2f(105,140); glVertex2f(119,157); glVertex2f(144,151);
    glVertex2f(163,132); glVertex2f(151,112); glVertex2f(128,118);
    glVertex2f(112,129); glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(166,92); glVertex2f(186,108); glVertex2f(199,94);
    glVertex2f(188,72); glVertex2f(162,67); glVertex2f(151,80); glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(105,80); glVertex2f(126,76); glVertex2f(132,56);
    glVertex2f(114,48); glVertex2f(98,62); glEnd();

    // Night-side mask gives the sphere a lit-side impression.
    glColor4f(0.0f,0.0f,0.04f,0.28f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(145,105);
    for(int i=0;i<=36;i++)
    {
        float a=3.1415926f*(float)i/36.0f-1.15f;
        glVertex2f(145+78*cosf(a),105+78*sinf(a));
    }
    glEnd();

    glColor4f(0.45f,0.78f,1.0f,0.45f);
    glLineWidth(2.0f);
    drawCircleOutline(145,105,84,64);
    glLineWidth(1.0f);
    glDisable(GL_BLEND);
}

// DOCKING PORT
void drawDockingPort()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float glowR = (gameState == 1) ? 0.10f : 0.05f;
    float glowG = (gameState == 1) ? 1.00f : 0.85f;
    drawGlowDisc(dockingX,dockingY,62,glowR,glowG,0.25f,0.12f);

    // Outer docking capture ring.
    glColor4f(0.18f,0.23f,0.30f,0.95f);
    drawCircle(dockingX,dockingY,66,64);
    glColor4f(0.48f,0.55f,0.64f,0.90f);
    glLineWidth(6.0f);
    drawCircleOutline(dockingX,dockingY,61,64);

    // Segmented capture ring.
    glColor4f(0.20f,0.72f,0.96f,0.75f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    for(int i=0;i<12;i++)
    {
        float a1=2.0f*3.1415926f*i/12.0f+0.08f;
        float a2=2.0f*3.1415926f*i/12.0f+0.34f;
        glVertex2f(dockingX+69*cosf(a1),dockingY+69*sinf(a1));
        glVertex2f(dockingX+69*cosf(a2),dockingY+69*sinf(a2));
    }
    glEnd();

    glColor3f(0.08f,0.10f,0.14f);
    drawCircle(dockingX,dockingY,50,64);
    glColor3f(0.20f,0.24f,0.31f);
    drawCircle(dockingX,dockingY,43,64);

    // Inner illuminated docking target.
    glColor3f(0.04f,0.35f,0.18f);
    drawCircle(dockingX,dockingY,27,48);
    glColor3f(0.05f,0.95f,0.28f);
    drawCircleOutline(dockingX,dockingY,27,48);

    glColor3f(0.02f,0.10f,0.06f);
    drawCircle(dockingX,dockingY,12,40);

    glColor3f(0.25f,1.0f,0.38f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2f(dockingX-35,dockingY); glVertex2f(dockingX+35,dockingY);
    glVertex2f(dockingX,dockingY-35); glVertex2f(dockingX,dockingY+35);
    glEnd();
    glLineWidth(1.0f);
    glDisable(GL_BLEND);
}

// SPACE STATION
void drawSpaceStation()
{
    // Large central truss.
    glColor3f(0.16f,0.18f,0.22f);
    glBegin(GL_QUADS);
    glVertex2f(655,334); glVertex2f(945,334); glVertex2f(945,366); glVertex2f(655,366);
    glEnd();

    glColor3f(0.42f,0.45f,0.50f);
    glLineWidth(5.0f);
    glBegin(GL_LINES);
    for(int x=660;x<=940;x+=24)
    {
        glVertex2f((float)x,334); glVertex2f((float)(x+14),366);
    }
    glEnd();
    glLineWidth(1.0f);

    // Central docking module with depth layers.
    glColor3f(0.10f,0.12f,0.16f);
    drawCircle(dockingX,dockingY,128,80);
    glColor3f(0.31f,0.34f,0.40f);
    glLineWidth(11.0f);
    drawCircleOutline(dockingX,dockingY,119,80);
    glLineWidth(1.0f);

    glColor3f(0.12f,0.15f,0.20f);
    drawCircle(dockingX,dockingY,105,80);
    glColor3f(0.35f,0.39f,0.46f);
    glLineWidth(4.0f);
    drawCircleOutline(dockingX,dockingY,102,80);

    // Radial station ribs.
    glColor3f(0.55f,0.59f,0.66f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    for(int i=0;i<16;i++)
    {
        float a=2.0f*3.1415926f*i/16.0f;
        glVertex2f(dockingX+108*cosf(a),dockingY+108*sinf(a));
        glVertex2f(dockingX+124*cosf(a),dockingY+124*sinf(a));
    }
    glEnd();
    glLineWidth(1.0f);

    // Top and bottom solar-array wings.
    glColor3f(0.04f,0.10f,0.22f);
    glBegin(GL_QUADS);
    glVertex2f(735,222); glVertex2f(865,222); glVertex2f(865,252); glVertex2f(735,252);
    glVertex2f(735,448); glVertex2f(865,448); glVertex2f(865,478); glVertex2f(735,478);
    glEnd();
    drawPanelLines(738,225,124,24,10,2);
    drawPanelLines(738,451,124,24,10,2);

    // Side modules.
    glColor3f(0.38f,0.42f,0.48f);
    glBegin(GL_QUADS);
    glVertex2f(675,305); glVertex2f(715,305); glVertex2f(715,395); glVertex2f(675,395);
    glVertex2f(885,305); glVertex2f(925,305); glVertex2f(925,395); glVertex2f(885,395);
    glEnd();

    glColor3f(0.14f,0.20f,0.27f);
    drawPanelLines(681,312,28,76,2,6);
    drawPanelLines(891,312,28,76,2,6);

    // Material/light contrast: warm top-facing metal and darker lower
    // structural surfaces make the station read as a solid object.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(1.0f, 0.84f, 0.55f, 0.08f);
    glBegin(GL_QUADS);
    glVertex2f(735, 448);
    glVertex2f(865, 448);
    glVertex2f(865, 478);
    glVertex2f(735, 478);
    glEnd();

    glColor4f(0.0f, 0.02f, 0.06f, 0.14f);
    glBegin(GL_QUADS);
    glVertex2f(735, 222);
    glVertex2f(865, 222);
    glVertex2f(865, 252);
    glVertex2f(735, 252);
    glEnd();

    glDisable(GL_BLEND);

    // Small blue status lights around the structure.
    glColor3f(0.20f,0.70f,1.0f);
    for(int i=0;i<8;i++)
    {
        float a=2.0f*3.1415926f*i/8.0f+0.2f;
        drawCircle(dockingX+112*cosf(a),dockingY+112*sinf(a),2.5f,12);
    }

    // Antenna mast.
    glColor3f(0.55f,0.58f,0.64f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2f(800,220); glVertex2f(800,190);
    glVertex2f(790,205); glVertex2f(810,205);
    glEnd();
    glLineWidth(1.0f);
    glColor3f(0.2f,0.8f,1.0f);
    drawCircle(800,187,4,16);

    drawDockingPort();
}

// SPACECRAFT DOCKING PORT POSITION
void updateDockingPort()
{
    float radians =
        shipAngle *
        3.1415926f /
        180.0f;

    portX =
        shipX -
        cosf(radians) *
        SHIP_PORT_OFFSET;

    portY =
        shipY -
        sinf(radians) *
        SHIP_PORT_OFFSET;
}
// DISTANCE
void updateDistance()
{
    updateDockingPort();

    float dx = dockingX - portX;
    float dy = dockingY - portY;

    dockingDistance =
        sqrtf(dx * dx + dy * dy);
}
// SPEED
void updateSpeed()
{
    speed =
        sqrtf(
            velocityX * velocityX +
            velocityY * velocityY
        );
}
// ANGLE ERROR
void updateAngleError()
{
    float dx = dockingX - portX;
    float dy = dockingY - portY;

    float distance =
        sqrtf(dx * dx + dy * dy);

    if (distance < 0.001f)
    {
        angleError = 0.0f;
        return;
    }

    dx /= distance;
    dy /= distance;

    float radians =
        shipAngle *
        3.1415926f /
        180.0f;

    // Direction from spacecraft body toward its rear/tail.
    float tailX = -cosf(radians);
    float tailY = -sinf(radians);

    float dot =
        tailX * dx +
        tailY * dy;

    if (dot > 1.0f)
        dot = 1.0f;

    if (dot < -1.0f)
        dot = -1.0f;

    float cross =
        tailX * dy -
        tailY * dx;

    float error =
        atan2f(cross, dot) *
        180.0f /
        3.1415926f;

    if (error < 0.0f)
        error = -error;

    angleError = error;
}
// APPROACH SPEED
void updateApproachSpeed()
{
    float dx = dockingX - portX;
    float dy = dockingY - portY;

    float distance =
        sqrtf(dx * dx + dy * dy);

    if (distance < 0.001f)
    {
        relativeApproachSpeed = 0.0f;
        closingSpeedSigned = 0.0f;
        return;
    }

    dx /= distance;
    dy /= distance;

    closingSpeedSigned =
        velocityX * dx +
        velocityY * dy;

    if (closingSpeedSigned > 0.0f)
        relativeApproachSpeed = closingSpeedSigned;
    else
        relativeApproachSpeed = 0.0f;
}
// RELATIVE LATERAL VELOCITY
// Closing speed is measured along the docking line. Lateral speed
// is measured perpendicular to that line. During final approach,
// both should be small for a stable docking contact.
void updateRelativeVelocity()
{
    float dx = dockingX - portX;
    float dy = dockingY - portY;

    float distance = sqrtf(dx * dx + dy * dy);

    if (distance < 0.001f)
    {
        lateralVelocity = 0.0f;
        return;
    }

    dx /= distance;
    dy /= distance;

    float perpendicularX = -dy;
    float perpendicularY = dx;

    lateralVelocity =
        velocityX * perpendicularX +
        velocityY * perpendicularY;

    if (lateralVelocity < 0.0f)
        lateralVelocity = -lateralVelocity;
}
// APPROACH PREDICTION
void updateApproachPrediction()
{
    updateSpeed();
    updateApproachSpeed();
    updateRelativeVelocity();

    // Stopping distance using v^2 / (2a).
    if (relativeApproachSpeed > 0.0f)
    {
        brakingDistance =
            (relativeApproachSpeed *
             relativeApproachSpeed) /
            (2.0f * BRAKE_ACCELERATION);
    }
    else
    {
        brakingDistance = 0.0f;
    }

    if (relativeApproachSpeed > 0.01f)
    {
        timeToDock =
            dockingDistance /
            relativeApproachSpeed;
    }
    else
    {
        timeToDock = 0.0f;
    }
}
// ALIGNMENT
int isAligned()
{
    updateAngleError();

    if (angleError <= ALIGNMENT_TOLERANCE)
        return 1;

    return 0;
}
// SAFE SPEED
int isSafeSpeed()
{
    updateApproachSpeed();

    if (relativeApproachSpeed <= MAX_DOCKING_SPEED)
        return 1;

    return 0;
}
// SPEED LIMIT
void limitSpeed()
{
    updateSpeed();

    if (speed > MAX_SPEED)
    {
        float scale =
            MAX_SPEED / speed;

        velocityX *= scale;
        velocityY *= scale;

        speed = MAX_SPEED;
    }
}
// PHYSICS
void updatePhysics(float dt)
{
    // DOCKING SEQUENCE

    if (gameState == 3 || gameState == 4)
    {
        updateDockingSequence(dt);
        return;
    }

    if (gameState != 0)
        return;

    missionTime += dt;

    // ROTATION

    float rotationSpeed = 80.0f;

    if (rotateLeft)
        shipAngle += rotationSpeed * dt;

    if (rotateRight)
        shipAngle -= rotationSpeed * dt;

    if (shipAngle >= 360.0f)
        shipAngle -= 360.0f;

    if (shipAngle < 0.0f)
        shipAngle += 360.0f;

    // FORWARD THRUST

    if (thrustForward && fuel > 0.0f)
    {
        float radians =
            shipAngle *
            3.1415926f /
            180.0f;

        float thrustMultiplier = getThrustMultiplier();

        velocityX +=
            cosf(radians) *
            THRUST_ACCELERATION *
            thrustMultiplier *
            dt;

        velocityY +=
            sinf(radians) *
            THRUST_ACCELERATION *
            thrustMultiplier *
            dt;

        fuel -= THRUST_FUEL_RATE * dt;
    }

    // REVERSE THRUST

    if (thrustBackward && fuel > 0.0f)
    {
        float radians =
            shipAngle *
            3.1415926f /
            180.0f;

        float thrustMultiplier = getThrustMultiplier();

        velocityX -=
            cosf(radians) *
            REVERSE_ACCELERATION *
            thrustMultiplier *
            dt;

        velocityY -=
            sinf(radians) *
            REVERSE_ACCELERATION *
            thrustMultiplier *
            dt;

        fuel -= THRUST_FUEL_RATE * 0.7f * dt;
    }

    // MANUAL BRAKING

    if (braking)
    {
        updateSpeed();

        if (speed > 0.001f)
        {
            float brakeAmount = BRAKE_ACCELERATION * dt;
            float newSpeed = speed - brakeAmount;

            if (newSpeed < 0.0f)
                newSpeed = 0.0f;

            if (newSpeed <= 0.0f)
            {
                velocityX = 0.0f;
                velocityY = 0.0f;
            }
            else
            {
                float scale = newSpeed / speed;
                velocityX *= scale;
                velocityY *= scale;
            }

            fuel -= 0.5f * dt;
        }
    }

    // VERY SMALL SPACE DRAG

    velocityX *= powf(SPACE_DRAG, dt * 60.0f);
    velocityY *= powf(SPACE_DRAG, dt * 60.0f);

    // POSITION

    shipX += velocityX * dt * 60.0f;
    shipY += velocityY * dt * 60.0f;

    // BOUNDARIES

    if (shipX < 80.0f)
    {
        shipX = 80.0f;
        velocityX = 0.0f;
    }

    if (shipX > 980.0f)
    {
        shipX = 980.0f;
        velocityX = 0.0f;
    }

    if (shipY < 150.0f)
    {
        shipY = 150.0f;
        velocityY = 0.0f;
    }

    if (shipY > 550.0f)
    {
        shipY = 550.0f;
        velocityY = 0.0f;
    }

    if (fuel < 0.0f)
        fuel = 0.0f;

    // DATA

    limitSpeed();
    updateDistance();
    updateSpeed();
    updateAngleError();
    updateApproachSpeed();
    updateRelativeVelocity();
    updateApproachPrediction();
    // DOCKING / COLLISION

    if (dockingDistance <= DOCKING_ZONE)
    {
        updateRelativeVelocity();
        updateApproachSpeed();
        updateAngleError();

        // Entering the physical docking zone above the safe approach
        // speed is treated as an impact.
        if (relativeApproachSpeed > MAX_DOCKING_SPEED)
        {
            setDockingFailure("APPROACH SPEED TOO HIGH");
        }
        else if (lateralVelocity > COLLISION_LATERAL_SPEED)
        {
            setDockingFailure("LATERAL SPEED TOO HIGH");
        }
        else if (angleError > COLLISION_ALIGNMENT)
        {
            setDockingFailure("DOCKING MISALIGNMENT");
        }
        else if (isAligned() &&
                 relativeApproachSpeed >= 0.0f &&
                 lateralVelocity <= MAX_LATERAL_DOCKING_SPEED)
        {
            contactSpeed = relativeApproachSpeed;
            contactLateralSpeed = lateralVelocity;
            contactAngleError = angleError;

            evaluateDockingQuality();

            velocityX = 0.0f;
            velocityY = 0.0f;
            speed = 0.0f;

            // Snap the rear port to the station for the contact sequence.
            float radians =
                shipAngle *
                3.1415926f /
                180.0f;

            shipX =
                dockingX +
                cosf(radians) *
                SHIP_PORT_OFFSET;

            shipY =
                dockingY +
                sinf(radians) *
                SHIP_PORT_OFFSET;

            updateDockingPort();
            updateDistance();

            dockingSequenceTime = 0.0f;
            gameState = 3;
        }
    }

    // If propulsion is completely exhausted and the ship has stopped
    // away from the station, the mission is over.
    if (fuel <= 0.0f &&
        speed < 0.02f &&
        dockingDistance > DOCKING_ZONE)
    {
        setDockingFailure("FUEL EXHAUSTED");
    }
}
// SCORE
void calculateScore()
{
    missionScore = 0;

    // Fuel conservation.
    missionScore += (int)(fuel * 0.35f);

    // Alignment quality.
    if (contactAngleError <= 5.0f)
        missionScore += 25;
    else if (contactAngleError <= 10.0f)
        missionScore += 18;
    else
        missionScore += 10;

    // Contact-speed quality.
    if (contactSpeed <= PERFECT_DOCKING_SPEED)
        missionScore += 30;
    else if (contactSpeed <= GOOD_DOCKING_SPEED)
        missionScore += 22;
    else
        missionScore += 12;

    // Lateral-motion quality.
    if (contactLateralSpeed <= 0.20f)
        missionScore += 15;
    else if (contactLateralSpeed <= MAX_LATERAL_DOCKING_SPEED)
        missionScore += 8;

    // Controlled-mission time bonus.
    if (missionTime <= 60.0f)
        missionScore += 5;

    if (missionScore > 100)
        missionScore = 100;
}
// DOCKING CORRIDOR
void drawDockingCorridor()
{
    if (gameState != 0 || dockingDistance > GUIDANCE_ZONE)
        return;

    float dx=dockingX-portX;
    float dy=dockingY-portY;
    float distance=sqrtf(dx*dx+dy*dy);
    if(distance<1.0f) return;
    dx/=distance; dy/=distance;
    float px=-dy, py=dx;

    float halfWidth=(dockingDistance<=FINAL_APPROACH_ZONE) ?
        16.0f+14.0f*(dockingDistance/FINAL_APPROACH_ZONE) : 30.0f;

    // Subtle translucent corridor, no solid yellow beam.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    if(isAligned()) glColor4f(0.10f,0.95f,0.35f,0.055f);
    else glColor4f(1.0f,0.65f,0.10f,0.045f);

    glBegin(GL_QUADS);
    glVertex2f(portX+px*halfWidth,portY+py*halfWidth);
    glVertex2f(dockingX+px*halfWidth,dockingY+py*halfWidth);
    glVertex2f(dockingX-px*halfWidth,dockingY-py*halfWidth);
    glVertex2f(portX-px*halfWidth,portY-py*halfWidth);
    glEnd();

    // Segmented guidance markers create a cleaner HUD-like tunnel.
    for(float t=0.10f;t<0.98f;t+=0.12f)
    {
        float cx=portX+dx*distance*t;
        float cy=portY+dy*distance*t;
        float size=4.0f+(1.0f-t)*3.0f;

        if(isAligned()) glColor4f(0.25f,1.0f,0.50f,0.75f);
        else glColor4f(1.0f,0.72f,0.18f,0.60f);

        glBegin(GL_LINES);
        glVertex2f(cx-dx*size-px*size,cy-dy*size-py*size);
        glVertex2f(cx+dx*size,cy+dy*size);
        glVertex2f(cx-dx*size+px*size,cy-dy*size+py*size);
        glVertex2f(cx+dx*size,cy+dy*size);
        glEnd();
    }

    // A narrow center guide only in the final approach, segmented.
    if(dockingDistance<=FINAL_APPROACH_ZONE)
    {
        if(isAligned()) glColor4f(0.25f,1.0f,0.45f,0.55f);
        else glColor4f(1.0f,0.70f,0.15f,0.45f);
        glLineWidth(1.5f);
        for(float t=0.0f;t<1.0f;t+=0.08f)
        {
            float t2=t+0.045f;
            if(t2>1.0f)t2=1.0f;
            glBegin(GL_LINES);
            glVertex2f(portX+dx*distance*t,portY+dy*distance*t);
            glVertex2f(portX+dx*distance*t2,portY+dy*distance*t2);
            glEnd();
        }
        glLineWidth(1.0f);
    }
    glDisable(GL_BLEND);
}

// FINAL APPROACH / DOCKING ENVELOPE
void drawDockingEnvelope()
{
    if (gameState != 0)
        return;

    if (dockingDistance > 220.0f)
        return;

    glEnable(GL_BLEND);

    if (dockingDistance <= DOCKING_ZONE)
    {
        glColor4f(
            0.1f,
            1.0f,
            0.2f,
            0.12f
        );
    }
    else if (dockingDistance <= FINAL_APPROACH_ZONE)
    {
        glColor4f(
            0.1f,
            0.8f,
            1.0f,
            0.08f
        );
    }
    else
    {
        glColor4f(
            1.0f,
            0.7f,
            0.1f,
            0.05f
        );
    }

    drawCircle(
        dockingX,
        dockingY,
        DOCKING_ENVELOPE,
        60
    );

    if (isAligned())
    {
        glColor3f(
            0.1f,
            1.0f,
            0.25f
        );
    }
    else
    {
        glColor3f(
            1.0f,
            0.65f,
            0.1f
        );
    }

    glLineWidth(2.0f);

    drawCircleOutline(
        dockingX,
        dockingY,
        DOCKING_ENVELOPE,
        60
    );

    glLineWidth(1.0f);
}
// VELOCITY VECTOR
void drawVelocityVector()
{
    updateSpeed();

    if (speed < 0.05f)
        return;

    float vectorScale = 25.0f;

    float vx =
        velocityX *
        vectorScale;

    float vy =
        velocityY *
        vectorScale;

    float vectorLength =
        sqrtf(vx * vx + vy * vy);

    if (vectorLength > 110.0f)
    {
        float scale =
            110.0f / vectorLength;

        vx *= scale;
        vy *= scale;
    }

    float endX = shipX + vx;
    float endY = shipY + vy;

    // Green = safe velocity, yellow = caution, red = too fast.
    if (speed <= MAX_DOCKING_SPEED)
    {
        glColor3f(
            0.1f,
            1.0f,
            0.3f
        );
    }
    else if (speed <= 4.0f)
    {
        glColor3f(
            1.0f,
            0.75f,
            0.1f
        );
    }
    else
    {
        glColor3f(
            1.0f,
            0.25f,
            0.1f
        );
    }

    glLineWidth(3.0f);

    glBegin(GL_LINES);

    glVertex2f(shipX, shipY);
    glVertex2f(endX, endY);

    glEnd();

    float angle =
        atan2f(vy, vx);

    float headLength = 12.0f;

    float leftAngle =
        angle + 2.6f;

    float rightAngle =
        angle - 2.6f;

    glBegin(GL_LINES);

    glVertex2f(endX, endY);

    glVertex2f(
        endX + cosf(leftAngle) * headLength,
        endY + sinf(leftAngle) * headLength
    );

    glVertex2f(endX, endY);

    glVertex2f(
        endX + cosf(rightAngle) * headLength,
        endY + sinf(rightAngle) * headLength
    );

    glEnd();

    glLineWidth(1.0f);
}
// SPEED GAUGE
void DrawText(float x,
    float y,
    const char* text
);

void drawSpeedGauge()
{
    float x = 30.0f;
    float y = 305.0f;

    float width = 230.0f;
    float height = 18.0f;

    glColor3f(
        0.12f,
        0.12f,
        0.16f
    );

    glBegin(GL_QUADS);

    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);

    glEnd();

    float fill =
        speed / MAX_SPEED;

    if (fill > 1.0f)
        fill = 1.0f;

    if (speed <= MAX_DOCKING_SPEED)
    {
        glColor3f(
            0.1f,
            0.9f,
            0.2f
        );
    }
    else if (speed <= 4.0f)
    {
        glColor3f(
            1.0f,
            0.75f,
            0.1f
        );
    }
    else
    {
        glColor3f(
            1.0f,
            0.25f,
            0.1f
        );
    }

    glBegin(GL_QUADS);

    glVertex2f(x, y);

    glVertex2f(
        x + width * fill,
        y
    );

    glVertex2f(
        x + width * fill,
        y + height
    );

    glVertex2f(
        x,
        y + height
    );

    glEnd();

    // 2 m/s safe marker.
    float safePosition =
        width *
        (MAX_DOCKING_SPEED / MAX_SPEED);

    glColor3f(
        1.0f,
        1.0f,
        1.0f
    );

    glLineWidth(2.0f);

    glBegin(GL_LINES);

    glVertex2f(
        x + safePosition,
        y - 4
    );

    glVertex2f(
        x + safePosition,
        y + height + 4
    );

    glEnd();

    glLineWidth(1.0f);

    char text[80];

    sprintf(
        text,
        "VELOCITY  %.2f m/s    SAFE <= 2.00",
        speed
    );

    glColor3f(
        0.8f,
        0.9f,
        1.0f
    );

    DrawText(
        x,
        y - 18,
        text
    );
}
// SPACECRAFT
void drawSpacecraftShadow()
{
    glPushMatrix();
    glTranslatef(shipX + 8.0f, shipY - 8.0f, 0.0f);
    glScalef(1.55f, 0.34f, 1.0f);
    glColor4f(0.0f,0.0f,0.0f,0.18f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawCircle(0,0,45,48);
    glDisable(GL_BLEND);
    glPopMatrix();
}

void drawSpacecraft()
{
    glPushMatrix();
    glTranslatef(shipX,shipY,0.0f);
    glRotatef(shipAngle,0.0f,0.0f,1.0f);

    // STAGE 11.5 - spacecraft depth / layered 3D-style foundation.
    // A dark offset silhouette acts as the lower hull, giving the ship
    // visible thickness without changing the existing docking geometry.
    glColor3f(0.08f,0.10f,0.13f);
    glBegin(GL_POLYGON);
    glVertex2f(-58,-5); glVertex2f(-20,35); glVertex2f(42,34);
    glVertex2f(82,3); glVertex2f(42,-35); glVertex2f(-20,-36);
    glEnd();

    // Lower hull bevel.
    glColor3f(0.18f,0.21f,0.26f);
    glBegin(GL_POLYGON);
    glVertex2f(-48,-8); glVertex2f(-12,-31); glVertex2f(39,-30);
    glVertex2f(70,-5); glVertex2f(39,-25); glVertex2f(-10,-26);
    glEnd();

    // Upper dorsal ridge creates a raised center section.
    glColor3f(0.52f,0.57f,0.64f);
    glBegin(GL_POLYGON);
    glVertex2f(-28,18); glVertex2f(-12,35); glVertex2f(35,33);
    glVertex2f(48,18); glVertex2f(34,24); glVertex2f(-8,25);
    glEnd();

    // Engine glow behind the ship.
    if(thrustForward)
    {
        drawGlowDisc(-61,18,14,1.0f,0.35f,0.05f,0.18f);
        drawGlowDisc(-61,-18,14,1.0f,0.35f,0.05f,0.18f);

        glColor4f(1.0f,0.25f,0.03f,0.22f);
        glBegin(GL_TRIANGLES);
        glVertex2f(-55,24); glVertex2f(-105,38); glVertex2f(-105,18);
        glVertex2f(-55,-24); glVertex2f(-105,-38); glVertex2f(-105,-18);
        glEnd();
        glColor3f(1.0f,0.72f,0.12f);
        glBegin(GL_TRIANGLES);
        glVertex2f(-56,21); glVertex2f(-88,29); glVertex2f(-88,18);
        glVertex2f(-56,-21); glVertex2f(-88,-29); glVertex2f(-88,-18);
        glEnd();
        glColor3f(1.0f,0.95f,0.55f);
        drawCircle(-57,18,4,16); drawCircle(-57,-18,4,16);
    }

    if(thrustBackward)
    {
        drawGlowDisc(72,0,12,0.12f,0.45f,1.0f,0.16f);
        glColor4f(0.15f,0.55f,1.0f,0.22f);
        glBegin(GL_TRIANGLES);
        glVertex2f(62,10); glVertex2f(98,20); glVertex2f(98,2);
        glVertex2f(62,-10); glVertex2f(98,-20); glVertex2f(98,-2);
        glEnd();
    }

    if(braking && speed>0.05f)
    {
        glColor4f(1.0f,0.75f,0.10f,0.22f);
        glBegin(GL_TRIANGLES);
        glVertex2f(-39,28); glVertex2f(-63,45); glVertex2f(-50,23);
        glVertex2f(-39,-28); glVertex2f(-63,-45); glVertex2f(-50,-23);
        glEnd();
    }

    // Rear engine pods, separated from the docking port.
    glColor3f(0.10f,0.12f,0.15f);
    glBegin(GL_POLYGON);
    glVertex2f(-62,11); glVertex2f(-50,28); glVertex2f(-34,27); glVertex2f(-30,18); glVertex2f(-45,8); glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(-62,-11); glVertex2f(-50,-28); glVertex2f(-34,-27); glVertex2f(-30,-18); glVertex2f(-45,-8); glEnd();

    glColor3f(0.32f,0.36f,0.42f);
    glBegin(GL_LINES);
    glVertex2f(-51,14); glVertex2f(-37,23);
    glVertex2f(-51,-14); glVertex2f(-37,-23);
    glEnd();

    // Engine nozzle depth rings.
    glColor3f(0.05f,0.07f,0.09f);
    drawCircle(-57,18,8,24);
    drawCircle(-57,-18,8,24);

    glColor3f(0.32f,0.38f,0.46f);
    glLineWidth(2.0f);
    drawCircleOutline(-57,18,8,24);
    drawCircleOutline(-57,-18,8,24);
    glLineWidth(1.0f);

    // Small metallic inner nozzles.
    glColor3f(0.12f,0.16f,0.20f);
    drawCircle(-57,18,4.5f,20);
    drawCircle(-57,-18,4.5f,20);

    // Main hull: dark base + lighter upper facet for a 2.5D look.
    glColor3f(0.28f,0.32f,0.38f);
    glBegin(GL_POLYGON);
    glVertex2f(-55,0); glVertex2f(-18,32); glVertex2f(40,31); glVertex2f(78,0);
    glVertex2f(40,-31); glVertex2f(-18,-32); glEnd();

    glColor3f(0.72f,0.76f,0.82f);
    glBegin(GL_POLYGON);
    glVertex2f(-47,2); glVertex2f(-15,28); glVertex2f(38,27); glVertex2f(66,2);
    glVertex2f(38,8); glVertex2f(-10,9); glEnd();

    glColor3f(0.48f,0.53f,0.60f);
    glBegin(GL_POLYGON);
    glVertex2f(-47,-2); glVertex2f(-10,-9); glVertex2f(38,-8); glVertex2f(66,-2);
    glVertex2f(38,-27); glVertex2f(-15,-28); glEnd();

    // Nose cap.
    glColor3f(0.42f,0.47f,0.54f);
    glBegin(GL_TRIANGLES);
    glVertex2f(78,0); glVertex2f(40,27); glVertex2f(40,-27); glEnd();
    glColor3f(0.70f,0.75f,0.82f);
    glBegin(GL_LINES);
    glVertex2f(52,0); glVertex2f(73,0);
    glEnd();

    // Cockpit canopy with highlight.
    glColor3f(0.025f,0.12f,0.23f);
    glBegin(GL_POLYGON);
    glVertex2f(-4,0); glVertex2f(14,21); glVertex2f(39,19); glVertex2f(51,0);
    glVertex2f(39,-19); glVertex2f(14,-21); glEnd();
    glColor3f(0.08f,0.34f,0.56f);
    glBegin(GL_POLYGON);
    glVertex2f(3,0); glVertex2f(16,15); glVertex2f(34,14); glVertex2f(43,0);
    glVertex2f(34,-14); glVertex2f(16,-15); glEnd();
    glColor4f(0.45f,0.80f,1.0f,0.55f);
    glLineWidth(2.0f);
    glBegin(GL_LINES); glVertex2f(10,12); glVertex2f(31,12); glEnd();
    glLineWidth(1.0f);

    // Raised cockpit rim / bevel.
    glColor3f(0.58f,0.65f,0.74f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-4,0); glVertex2f(14,21); glVertex2f(39,19);
    glVertex2f(51,0); glVertex2f(39,-19); glVertex2f(14,-21);
    glEnd();
    glLineWidth(1.0f);

    // Small side armor panels emphasize the hull's separate surfaces.
    glColor3f(0.34f,0.39f,0.46f);
    glBegin(GL_QUADS);
    glVertex2f(43,9); glVertex2f(62,4); glVertex2f(62,-4); glVertex2f(43,-9);
    glEnd();

    // Structural spine and panel seams.
    glColor3f(0.24f,0.29f,0.35f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(-28,0); glVertex2f(38,0);
    glVertex2f(-20,22); glVertex2f(-20,-22);
    glVertex2f(8,27); glVertex2f(8,-27);
    glEnd();
    glLineWidth(1.0f);

    // Thin specular strips make the hull read as metallic rather than flat.
    glColor4f(0.85f,0.92f,1.0f,0.38f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(43,12); glVertex2f(58,5);
    glVertex2f(43,-12); glVertex2f(58,-5);
    glVertex2f(-7,27); glVertex2f(27,27);
    glEnd();
    glLineWidth(1.0f);

    // Wings / radiators.
    glColor3f(0.38f,0.43f,0.50f);
    glBegin(GL_POLYGON);
    glVertex2f(-8,27); glVertex2f(-40,58); glVertex2f(24,27); glVertex2f(13,22); glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(-8,-27); glVertex2f(-40,-58); glVertex2f(24,-27); glVertex2f(13,-22); glEnd();
    glColor3f(0.15f,0.22f,0.32f);
    glBegin(GL_LINES);
    glVertex2f(-14,30); glVertex2f(-36,52);
    glVertex2f(-14,-30); glVertex2f(-36,-52);
    glEnd();

    // Center rear docking collar — deliberately distinct from engines.
    glColor3f(0.20f,0.24f,0.29f);
    glBegin(GL_QUADS);
    glVertex2f(-64,-9); glVertex2f(-51,-9); glVertex2f(-51,9); glVertex2f(-64,9); glEnd();
    glColor3f(0.62f,0.67f,0.74f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-64,-9); glVertex2f(-51,-9); glVertex2f(-51,9); glVertex2f(-64,9); glEnd();
    glLineWidth(1.0f);

    // Recessed collar lip gives the docking port a real mechanical depth.
    glColor3f(0.10f,0.13f,0.17f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-67,-11); glVertex2f(-50,-11);
    glVertex2f(-50,11); glVertex2f(-67,11);
    glEnd();
    glLineWidth(1.0f);

    glColor3f(0.10f,0.95f,0.30f);
    drawCircle(-58,0,7,24);
    glColor3f(0.02f,0.12f,0.06f);
    drawCircle(-58,0,3,18);
    glColor3f(0.55f,1.0f,0.65f);
    drawCircleOutline(-58,0,10,24);

    // FINAL VISUAL LIGHT PASS
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Sun-facing upper hull plane.
    glColor4f(1.0f, 0.84f, 0.55f, 0.13f);
    glBegin(GL_POLYGON);
    glVertex2f(-43, 4);
    glVertex2f(-15, 27);
    glVertex2f(34, 26);
    glVertex2f(61, 5);
    glVertex2f(38, 10);
    glVertex2f(-8, 11);
    glEnd();

    // Shadow plane on the underside.
    glColor4f(0.01f, 0.035f, 0.075f, 0.20f);
    glBegin(GL_POLYGON);
    glVertex2f(-43, -4);
    glVertex2f(-12, -27);
    glVertex2f(38, -26);
    glVertex2f(61, -5);
    glVertex2f(35, -10);
    glVertex2f(-9, -11);
    glEnd();

    // Narrow metallic rim highlights.
    glColor4f(0.88f, 0.94f, 1.0f, 0.28f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(-15, 28);
    glVertex2f(34, 27);
    glVertex2f(40, 24);
    glVertex2f(62, 5);
    glEnd();
    glLineWidth(1.0f);

    glDisable(GL_BLEND);

    // Engine labels by visual separators, not text.
    glColor3f(0.75f,0.80f,0.88f);
    glBegin(GL_LINES);
    glVertex2f(-52,10); glVertex2f(-44,16);
    glVertex2f(-52,-10); glVertex2f(-44,-16);
    glEnd();

    glPopMatrix();
}

// TEXT
void DrawText(
    float x,
    float y,
    const char* text
)
{
    glRasterPos2f(x, y);

    for (int i = 0; text[i] != '\0'; i++)
    {
        glutBitmapCharacter(
            GLUT_BITMAP_8_BY_13,
            text[i]
        );
    }
}
// HUD
void drawHUD()
{
    updateDistance();
    updateSpeed();
    updateAngleError();
    updateApproachSpeed();
    updateApproachPrediction();

    char text[120];

    // TITLE

    glColor3f(
        0.30f,
        1.0f,
        0.40f
    );

    DrawText(
        30,
        675,
        "SPACECRAFT DOCKING SIMULATOR - STAGE 11"
    );

    // STATUS
    if (gameState == 1)
    {
        glColor3f(
            0.1f,
            1.0f,
            0.2f
        );

        DrawText(
            30,
            640,
            "STATUS: DOCKING SUCCESSFUL!"
        );
    }
    else if (gameState == 2)
    {
        glColor3f(
            1.0f,
            0.2f,
            0.2f
        );

        DrawText(
            30,
            640,
            "STATUS: DOCKING FAILED!"
        );
    }
    else
    {
        if (dockingDistance > 300.0f)
        {
            glColor3f(
                0.1f,
                1.0f,
                0.3f
            );

            DrawText(
                30,
                640,
                "STATUS: APPROACHING STATION"
            );
        }
        else if (dockingDistance > FINAL_APPROACH_ZONE)
        {
            glColor3f(
                0.2f,
                0.8f,
                1.0f
            );

            DrawText(
                30,
                640,
                "STATUS: ALIGNMENT PHASE"
            );
        }
        else
        {
            glColor3f(
                1.0f,
                0.7f,
                0.1f
            );

            DrawText(
                30,
                640,
                "STATUS: FINAL APPROACH"
            );
        }
    }

    // DIFFICULTY

    sprintf(text, "DIFFICULTY: %s", difficultyName);
    glColor3f(1.0f, 0.85f, 0.25f);
    DrawText(780, 675, text);

    // FUEL

    sprintf(
        text,
        "FUEL: %.1f%%",
        fuel
    );

    glColor3f(
        0.7f,
        0.8f,
        1.0f
    );

    DrawText(
        30,
        610,
        text
    );

    // DISTANCE

    sprintf(
        text,
        "DOCKING DISTANCE: %.1f m",
        dockingDistance
    );

    DrawText(
        30,
        585,
        text
    );

    // ALIGNMENT

    sprintf(
        text,
        "ALIGNMENT ERROR: %.1f deg",
        angleError
    );

    if (angleError <= ALIGNMENT_TOLERANCE)
    {
        glColor3f(
            0.1f,
            1.0f,
            0.2f
        );
    }
    else
    {
        glColor3f(
            1.0f,
            0.7f,
            0.1f
        );
    }

    DrawText(
        30,
        560,
        text
    );

    // APPROACH SPEED

    sprintf(
        text,
        "APPROACH SPEED: %.2f m/s",
        relativeApproachSpeed
    );

    if (relativeApproachSpeed <= MAX_DOCKING_SPEED)
    {
        glColor3f(
            0.1f,
            1.0f,
            0.2f
        );
    }
    else
    {
        glColor3f(
            1.0f,
            0.2f,
            0.1f
        );
    }

    DrawText(
        30,
        535,
        text
    );

    // TOTAL VELOCITY

    sprintf(
        text,
        "TOTAL VELOCITY: %.2f m/s",
        speed
    );

    glColor3f(0.7f, 0.8f, 1.0f);

    DrawText(30, 510, text);

    sprintf(
        text,
        "LATERAL VELOCITY: %.2f m/s",
        lateralVelocity
    );

    if (lateralVelocity <= MAX_LATERAL_DOCKING_SPEED)
        glColor3f(0.1f, 1.0f, 0.2f);
    else
        glColor3f(1.0f, 0.5f, 0.1f);

    DrawText(30, 485, text);

    sprintf(text, "MISSION TIME: %.1f s", missionTime);
    glColor3f(0.7f, 0.8f, 1.0f);
    DrawText(30, 460, text);

    if (fuel <= CRITICAL_FUEL_THRESHOLD)
    {
        glColor3f(1.0f, 0.2f, 0.1f);
        DrawText(30, 435, "FUEL CRITICAL - THRUST REDUCED");
    }
    else if (fuel <= LOW_FUEL_THRESHOLD)
    {
        glColor3f(1.0f, 0.7f, 0.1f);
        DrawText(30, 435, "LOW FUEL - THRUST LIMITED");
    }

    // ALIGNMENT STATUS

    if (isAligned())
    {
        glColor3f(
            0.1f,
            1.0f,
            0.2f
        );

        DrawText(
            30,
            405,
            "TAIL ALIGNMENT: LOCKED"
        );
    }
    else
    {
        glColor3f(
            1.0f,
            0.6f,
            0.1f
        );

        DrawText(
            30,
            405,
            "TAIL ALIGNMENT: ROTATE"
        );
    }

    // BRAKING STATUS
    if (braking)
    {
        glColor3f(
            1.0f,
            0.8f,
            0.1f
        );

        DrawText(
            30,
            380,
            "BRAKING THRUST: ACTIVE"
        );
    }
    else if (dockingDistance < 200.0f &&
             brakingDistance > dockingDistance &&
             relativeApproachSpeed > 0.5f)
    {
        glColor3f(
            1.0f,
            0.2f,
            0.1f
        );

        DrawText(
            30,
            380,
            "WARNING: BRAKE NOW!"
        );
    }
    else
    {
        glColor3f(
            0.3f,
            0.9f,
            0.4f
        );

        DrawText(
            30,
            380,
            "BRAKING: READY"
        );
    }

 
    // SPEED GAUGE

    drawSpeedGauge();


    // CONTROLS

    glColor3f(
        0.7f,
        0.8f,
        1.0f
    );

    DrawText(
        30,
        45,
        "CONTROLS"
    );

    DrawText(
        30,
        25,
        "UP/DOWN: THRUST   LEFT/RIGHT: ROTATE   SPACE: BRAKE   R: RESET   M: MENU   H: HELP"
    );

    // GUIDANCE

    if (gameState == 0)
    {
        glColor3f(
            0.4f,
            0.9f,
            0.5f
        );

        DrawText(
            700,
            145,
            "DOCKING GUIDANCE"
        );

        if (dockingDistance > GUIDANCE_ZONE)
        {
            glColor3f(
                0.7f,
                0.8f,
                1.0f
            );

            DrawText(
                700,
                122,
                "GUIDANCE: APPROACH STATION"
            );
        }
        else if (!isAligned())
        {
            glColor3f(
                1.0f,
                0.7f,
                0.1f
            );

            DrawText(
                700,
                122,
                "TAIL: ROTATE INTO CORRIDOR"
            );
        }
        else
        {
            glColor3f(
                0.1f,
                1.0f,
                0.2f
            );

            DrawText(
                700,
                122,
                "TAIL: ALIGNED"
            );
        }

        if (dockingDistance <= FINAL_APPROACH_ZONE)
        {
            glColor3f(
                0.2f,
                0.9f,
                1.0f
            );

            DrawText(
                700,
                99,
                "FINAL APPROACH ENVELOPE ACTIVE"
            );
        }

        if (relativeApproachSpeed > MAX_DOCKING_SPEED)
        {
            glColor3f(
                1.0f,
                0.2f,
                0.1f
            );

            DrawText(
                700,
                76,
                "APPROACH TOO FAST - BRAKE"
            );
        }
        else if (relativeApproachSpeed > 1.2f &&
                 dockingDistance < 120.0f)
        {
            glColor3f(
                1.0f,
                0.7f,
                0.1f
            );

            DrawText(
                700,
                76,
                "REDUCE SPEED FOR FINAL DOCK"
            );
        }
        else
        {
            glColor3f(
                0.1f,
                1.0f,
                0.2f
            );

            DrawText(
                700,
                76,
                "APPROACH SPEED SAFE"
            );
        }

        if (lateralVelocity > MAX_LATERAL_DOCKING_SPEED &&
            dockingDistance < FINAL_APPROACH_ZONE)
        {
            glColor3f(1.0f, 0.5f, 0.1f);
            DrawText(700, 53, "REDUCE LATERAL MOTION");
        }
        else if (dockingDistance < FINAL_APPROACH_ZONE)
        {
            glColor3f(0.1f, 1.0f, 0.2f);
            DrawText(700, 53, "LATERAL VELOCITY SAFE");
        }

        glColor3f(0.7f, 0.8f, 1.0f);

        DrawText(
            700,
            30,
            "GREEN CORRIDOR = IDEAL PATH"
        );
    }

    // DOCKING SEQUENCE

    if (gameState == 3)
    {
        glColor3f(0.2f, 0.9f, 1.0f);
        DrawText(700, 145, "CONTACT DETECTED");
        DrawText(700, 120, "DOCKING CONTACT STABLE");
        DrawText(700, 95, "CLAMPS PREPARING TO ENGAGE");
    }
    else if (gameState == 4)
    {
        glColor3f(1.0f, 0.8f, 0.1f);
        DrawText(700, 145, "DOCKING CLAMPS ENGAGING");
        DrawText(700, 120, "VELOCITY DAMPED");
        DrawText(700, 95, "DOCKING LOCK IN PROGRESS");
    }

    // SUCCESS

    if (gameState == 1)
    {
        glColor3f(
            0.1f,
            1.0f,
            0.2f
        );

        DrawText(
            700,
            145,
            "DOCKING COMPLETE!"
        );

        sprintf(
            text,
            "MISSION SCORE: %d / 100",
            missionScore
        );

        DrawText(
            700,
            120,
            text
        );

        if (dockingQuality == 3)
        {
            glColor3f(0.1f, 1.0f, 0.2f);
            DrawText(700, 95, "DOCKING QUALITY: PERFECT");
        }
        else if (dockingQuality == 2)
        {
            glColor3f(0.2f, 0.8f, 1.0f);
            DrawText(700, 95, "DOCKING QUALITY: GOOD");
        }
        else
        {
            glColor3f(1.0f, 0.7f, 0.1f);
            DrawText(700, 95, "DOCKING QUALITY: HARD CONTACT");
        }

        sprintf(text, "CONTACT: %.2f m/s  LATERAL: %.2f m/s", contactSpeed, contactLateralSpeed);
        glColor3f(0.7f, 0.8f, 1.0f);
        DrawText(700, 70, text);

        DrawText(
            700,
            45,
            "REAR DOCKING PORT CONNECTED"
        );

        DrawText(
            700,
            20,
            "PRESS R TO RETRY   |   M TO RETURN TO MENU"
        );
    }

    // FAILURE

    if (gameState == 2)
    {
        glColor3f(
            1.0f,
            0.2f,
            0.2f
        );

        DrawText(
            700,
            145,
            "DOCKING FAILED!"
        );

        sprintf(text, "REASON: %s", failureReason);
        DrawText(700, 120, text);

        DrawText(700, 95, "PRESS H OR CLICK ? FOR DOCKING REQUIREMENTS");
        DrawText(700, 70, "PRESS R TO RETRY   |   M TO RETURN TO MENU");

    }
}
// STAGE 11 - STARTING MENU UI
void drawSpaceBackground();
int loadSpaceBackgroundBMP(const char* filename);

// RESIZE / CENTERING
int viewportX = 0;
int viewportY = 0;
int viewportW = WINDOW_WIDTH;
int viewportH = WINDOW_HEIGHT;
float viewportScale = 1.0f;

void reshape(int w, int h)
{
    if (w < 1) w = 1;
    if (h < 1) h = 1;

    float sx = (float)w / (float)WINDOW_WIDTH;
    float sy = (float)h / (float)WINDOW_HEIGHT;
    viewportScale = (sx < sy) ? sx : sy;

    viewportW = (int)(WINDOW_WIDTH * viewportScale);
    viewportH = (int)(WINDOW_HEIGHT * viewportScale);
    viewportX = (w - viewportW) / 2;
    viewportY = (h - viewportH) / 2;

    glViewport(viewportX, viewportY, viewportW, viewportH);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, (GLdouble)WINDOW_WIDTH,
               0.0, (GLdouble)WINDOW_HEIGHT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void getGameMouseCoordinates(int x, int y, float &mx, float &my)
{
    int actualW = glutGet(GLUT_WINDOW_WIDTH);
    int actualH = glutGet(GLUT_WINDOW_HEIGHT);
    if (actualW < 1) actualW = WINDOW_WIDTH;
    if (actualH < 1) actualH = WINDOW_HEIGHT;

    float localX = (float)x - (float)viewportX;
    float localY = (float)(actualH - y) - (float)viewportY;

    mx = localX / viewportScale;
    my = localY / viewportScale;
}

void drawCenteredText(float centerX, float y, const char* text)
{
    int width = glutBitmapLength(GLUT_BITMAP_8_BY_13,
                                 (const unsigned char*)text);
    DrawText(centerX - (float)width * 0.5f, y, text);
}

void drawCenteredStrokeText(float centerX, float y, const char* text,
                            float scale)
{
    int width = glutStrokeLength(GLUT_STROKE_ROMAN,
                                 (const unsigned char*)text);
    glPushMatrix();
    glTranslatef(centerX - (float)width * scale * 0.5f, y, 0.0f);
    glScalef(scale, scale, 1.0f);
    for (int i = 0; text[i] != '\0'; ++i)
        glutStrokeCharacter(GLUT_STROKE_ROMAN, text[i]);
    glPopMatrix();
}

void drawSciFiFrame(float x, float y, float w, float h)
{
    // Main dark glass panel.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.015f, 0.035f, 0.075f, 0.84f);

    glBegin(GL_POLYGON);
    glVertex2f(x + 24, y);
    glVertex2f(x + w - 24, y);
    glVertex2f(x + w, y + 24);
    glVertex2f(x + w, y + h - 24);
    glVertex2f(x + w - 24, y + h);
    glVertex2f(x + 24, y + h);
    glVertex2f(x, y + h - 24);
    glVertex2f(x, y + 24);
    glEnd();

    // Outer cyan frame.
    glColor4f(0.18f, 0.70f, 1.0f, 0.78f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x + 24, y);
    glVertex2f(x + w - 24, y);
    glVertex2f(x + w, y + 24);
    glVertex2f(x + w, y + h - 24);
    glVertex2f(x + w - 24, y + h);
    glVertex2f(x + 24, y + h);
    glVertex2f(x, y + h - 24);
    glVertex2f(x, y + 24);
    glEnd();

    // Inner frame.
    glColor4f(0.22f, 0.52f, 0.76f, 0.38f);
    glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x + 32, y + 8);
    glVertex2f(x + w - 32, y + 8);
    glVertex2f(x + w - 8, y + 32);
    glVertex2f(x + w - 8, y + h - 32);
    glVertex2f(x + w - 32, y + h - 8);
    glVertex2f(x + 32, y + h - 8);
    glVertex2f(x + 8, y + h - 32);
    glVertex2f(x + 8, y + 32);
    glEnd();

    // Technical corner brackets.
    glColor4f(0.28f, 0.76f, 1.0f, 0.72f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2f(x + 42, y + h - 34); glVertex2f(x + 110, y + h - 34);
    glVertex2f(x + 42, y + h - 34); glVertex2f(x + 42, y + h - 78);

    glVertex2f(x + w - 42, y + h - 34); glVertex2f(x + w - 110, y + h - 34);
    glVertex2f(x + w - 42, y + h - 34); glVertex2f(x + w - 42, y + h - 78);

    glVertex2f(x + 42, y + 34); glVertex2f(x + 110, y + 34);
    glVertex2f(x + 42, y + 34); glVertex2f(x + 42, y + 78);

    glVertex2f(x + w - 42, y + 34); glVertex2f(x + w - 110, y + 34);
    glVertex2f(x + w - 42, y + 34); glVertex2f(x + w - 42, y + 78);
    glEnd();
    glLineWidth(1.0f);
    glDisable(GL_BLEND);
}

void drawTopTitlePlate()
{
    const float cx = 550.0f;
    const float cy = 632.0f;
    const float w = 430.0f;
    const float h = 82.0f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.025f, 0.085f, 0.15f, 0.94f);
    glBegin(GL_POLYGON);
    glVertex2f(cx - w*0.50f + 24, cy - h*0.50f);
    glVertex2f(cx + w*0.50f - 24, cy - h*0.50f);
    glVertex2f(cx + w*0.50f, cy - h*0.50f + 24);
    glVertex2f(cx + w*0.50f, cy + h*0.50f - 24);
    glVertex2f(cx + w*0.50f - 24, cy + h*0.50f);
    glVertex2f(cx - w*0.50f + 24, cy + h*0.50f);
    glVertex2f(cx - w*0.50f, cy + h*0.50f - 24);
    glVertex2f(cx - w*0.50f, cy - h*0.50f + 24);
    glEnd();

    glColor4f(0.25f, 0.80f, 1.0f, 0.88f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx - w*0.50f + 24, cy - h*0.50f);
    glVertex2f(cx + w*0.50f - 24, cy - h*0.50f);
    glVertex2f(cx + w*0.50f, cy - h*0.50f + 24);
    glVertex2f(cx + w*0.50f, cy + h*0.50f - 24);
    glVertex2f(cx + w*0.50f - 24, cy + h*0.50f);
    glVertex2f(cx - w*0.50f + 24, cy + h*0.50f);
    glVertex2f(cx - w*0.50f, cy + h*0.50f - 24);
    glVertex2f(cx - w*0.50f, cy - h*0.50f + 24);
    glEnd();
    glLineWidth(1.0f);
    glDisable(GL_BLEND);

    glColor3f(0.38f, 0.92f, 1.0f);
    drawCenteredStrokeText(cx, 640.0f, "SPACECRAFT DOCKING", 0.17f);
    glColor3f(0.48f, 0.88f, 1.0f);
    drawCenteredStrokeText(cx, 616.0f, "SIMULATOR", 0.095f);
}

void drawDifficultyIcon(int difficulty, float cx, float cy, int hovered)
{
    float glow = hovered ? 0.28f : 0.12f;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.12f, 0.70f, 1.0f, glow);
    drawGlowDisc(cx, cy, 48.0f, 0.08f, 0.55f, 1.0f, glow);

    glColor4f(0.30f, 0.82f, 1.0f, hovered ? 0.92f : 0.62f);
    glLineWidth(1.5f);

    if (difficulty == EASY)
    {
        // Small capsule/lander.
        glBegin(GL_LINE_LOOP);
        glVertex2f(cx - 27, cy - 15); glVertex2f(cx - 18, cy + 18);
        glVertex2f(cx + 18, cy + 18); glVertex2f(cx + 27, cy - 15);
        glVertex2f(cx + 17, cy - 24); glVertex2f(cx - 17, cy - 24);
        glEnd();
        glBegin(GL_LINES);
        glVertex2f(cx - 16, cy + 18); glVertex2f(cx - 26, cy + 28);
        glVertex2f(cx + 16, cy + 18); glVertex2f(cx + 26, cy + 28);
        glEnd();
        drawCircleOutline(cx, cy + 2, 5, 16);
    }
    else if (difficulty == MEDIUM)
    {
        // Robotic docking arm.
        glBegin(GL_LINES);
        glVertex2f(cx - 28, cy + 12); glVertex2f(cx - 5, cy + 2);
        glVertex2f(cx - 5, cy + 2); glVertex2f(cx + 12, cy + 18);
        glVertex2f(cx + 12, cy + 18); glVertex2f(cx + 28, cy + 5);
        glVertex2f(cx - 5, cy + 2); glVertex2f(cx + 4, cy - 18);
        glEnd();
        drawCircleOutline(cx - 28, cy + 12, 6, 16);
        drawCircleOutline(cx - 5, cy + 2, 6, 16);
        drawCircleOutline(cx + 12, cy + 18, 6, 16);
        drawCircleOutline(cx + 28, cy + 5, 6, 16);
    }
    else
    {
        // Engine/nozzle silhouette.
        glBegin(GL_LINE_LOOP);
        glVertex2f(cx - 30, cy - 18); glVertex2f(cx - 18, cy + 20);
        glVertex2f(cx + 14, cy + 25); glVertex2f(cx + 31, cy + 5);
        glVertex2f(cx + 19, cy - 25); glVertex2f(cx - 12, cy - 28);
        glEnd();
        glBegin(GL_LINES);
        glVertex2f(cx - 5, cy - 20); glVertex2f(cx + 8, cy + 19);
        glVertex2f(cx + 8, cy + 19); glVertex2f(cx + 23, cy + 9);
        glEnd();
        drawCircleOutline(cx + 30, cy + 5, 7, 20);
    }

    glLineWidth(1.0f);
    glDisable(GL_BLEND);
}

void drawMenuButton(float x, float y, float w, float h,
                    const char* label, int difficulty, int action)
{
    int hovered = (hoveredAction == action);

    // Hover glow behind the card.
    if (hovered)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.08f, 0.45f, 1.0f, 0.18f);
        drawHexPanel(x + w*0.5f, y + h*0.5f, w + 16.0f, h + 14.0f);
        glDisable(GL_BLEND);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Normal/hover glass surface.
    if (hovered)
        glColor4f(0.035f, 0.22f, 0.40f, 0.96f);
    else
        glColor4f(0.018f, 0.065f, 0.115f, 0.92f);

    glBegin(GL_POLYGON);
    glVertex2f(x + 24, y);
    glVertex2f(x + w - 24, y);
    glVertex2f(x + w, y + 24);
    glVertex2f(x + w, y + h - 24);
    glVertex2f(x + w - 24, y + h);
    glVertex2f(x + 24, y + h);
    glVertex2f(x, y + h - 24);
    glVertex2f(x, y + 24);
    glEnd();

    // Outer edge.
    glColor4f(0.20f, 0.70f, 1.0f, hovered ? 1.0f : 0.72f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x + 24, y);
    glVertex2f(x + w - 24, y);
    glVertex2f(x + w, y + 24);
    glVertex2f(x + w, y + h - 24);
    glVertex2f(x + w - 24, y + h);
    glVertex2f(x + 24, y + h);
    glVertex2f(x, y + h - 24);
    glVertex2f(x, y + 24);
    glEnd();

    // Inner technical line.
    glColor4f(0.25f, 0.62f, 0.84f, hovered ? 0.65f : 0.30f);
    glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x + 12, y + 12);
    glVertex2f(x + w - 12, y + 12);
    glVertex2f(x + w - 12, y + h - 12);
    glVertex2f(x + 12, y + h - 12);
    glEnd();

    glDisable(GL_BLEND);

    glColor3f(0.88f, 0.96f, 1.0f);
    drawCenteredText(x + w*0.50f, y + h*0.50f - 5.0f, label);

    drawDifficultyIcon(difficulty, x + w - 105.0f, y + h*0.50f, hovered);
}

void drawQuitButton(float x, float y, float w, float h)
{
    int hovered = (hoveredAction == HOVER_MENU_QUIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (hovered)
        glColor4f(0.48f, 0.055f, 0.075f, 0.98f);
    else
        glColor4f(0.18f, 0.025f, 0.045f, 0.94f);

    glBegin(GL_POLYGON);
    glVertex2f(x + 14, y);
    glVertex2f(x + w - 14, y);
    glVertex2f(x + w, y + 14);
    glVertex2f(x + w, y + h - 14);
    glVertex2f(x + w - 14, y + h);
    glVertex2f(x + 14, y + h);
    glVertex2f(x, y + h - 14);
    glVertex2f(x, y + 14);
    glEnd();

    glColor4f(1.0f, hovered ? 0.45f : 0.18f, hovered ? 0.45f : 0.18f, 0.95f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x + 14, y);
    glVertex2f(x + w - 14, y);
    glVertex2f(x + w, y + 14);
    glVertex2f(x + w, y + h - 14);
    glVertex2f(x + w - 14, y + h);
    glVertex2f(x + 14, y + h);
    glVertex2f(x, y + h - 14);
    glVertex2f(x, y + 14);
    glEnd();

    glDisable(GL_BLEND);

    glColor3f(1.0f, 0.88f, 0.88f);
    drawCenteredText(x + w*0.50f, y + h*0.50f - 5.0f, "QUIT GAME");
}

void drawMenu()
{
    drawSpaceBackground();

    // Main futuristic console.
    drawSciFiFrame(118, 42, 864, 628);

    // Top title plate.
    drawTopTitlePlate();

    // Small horizontal HUD accents beside the title.
    glColor3f(0.18f, 0.68f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(150, 632); glVertex2f(335, 632);
    glVertex2f(765, 632); glVertex2f(950, 632);
    glEnd();
    glLineWidth(1.0f);

    glColor3f(0.55f, 0.86f, 1.0f);
    drawCenteredText(550, 585, "MISSION SELECT: CHOOSE YOUR CHALLENGE");

    drawMenuButton(205, 450, 690, 68, "[ 1 ]   EASY", EASY, HOVER_EASY);
    drawMenuButton(205, 355, 690, 68, "[ 2 ]   MEDIUM", MEDIUM, HOVER_MEDIUM);
    drawMenuButton(205, 260, 690, 68, "[ 3 ]   HARD", HARD, HOVER_HARD);

    // Mission profile panel.
    const float px = 205.0f;
    const float py = 125.0f;
    const float pw = 690.0f;
    const float ph = 105.0f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.018f, 0.075f, 0.12f, 0.93f);
    glBegin(GL_POLYGON);
    glVertex2f(px + 18, py);
    glVertex2f(px + pw - 18, py);
    glVertex2f(px + pw, py + 18);
    glVertex2f(px + pw, py + ph - 18);
    glVertex2f(px + pw - 18, py + ph);
    glVertex2f(px + 18, py + ph);
    glVertex2f(px, py + ph - 18);
    glVertex2f(px, py + 18);
    glEnd();

    glColor4f(0.25f, 0.70f, 1.0f, 0.82f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(px + 18, py);
    glVertex2f(px + pw - 18, py);
    glVertex2f(px + pw, py + 18);
    glVertex2f(px + pw, py + ph - 18);
    glVertex2f(px + pw - 18, py + ph);
    glVertex2f(px + 18, py + ph);
    glVertex2f(px, py + ph - 18);
    glVertex2f(px, py + 18);
    glEnd();
    glDisable(GL_BLEND);

    glColor3f(0.40f, 1.0f, 0.55f);
    drawCenteredText(550, 200, "MISSION PROFILE");

    glColor3f(0.70f, 0.98f, 0.78f);
    drawCenteredText(550, 175, "EASY     -     Learn the controls and docking basics");
    glColor3f(0.78f, 0.88f, 0.98f);
    drawCenteredText(550, 152, "MEDIUM   -     Standard mission conditions");
    drawCenteredText(550, 129, "HARD     -     Precise docking with tighter limits");

    drawQuitButton(400, 58, 300, 52);

    glColor3f(0.46f, 0.72f, 0.94f);
    drawCenteredText(550, 28,
                     "CLICK A BUTTON   |   1 / 2 / 3 SELECT DIFFICULTY   |   Q QUIT");
}

void mouseMove(int x, int y)
{
    float mx, my;
    getGameMouseCoordinates(x, y, mx, my);

    hoveredAction = HOVER_NONE;

    if (screenState == 0)
    {
        if (mx >= 205 && mx <= 895 && my >= 450 && my <= 518)
            hoveredAction = HOVER_EASY;
        else if (mx >= 205 && mx <= 895 && my >= 355 && my <= 423)
            hoveredAction = HOVER_MEDIUM;
        else if (mx >= 205 && mx <= 895 && my >= 260 && my <= 328)
            hoveredAction = HOVER_HARD;
        else if (mx >= 400 && mx <= 700 && my >= 58 && my <= 110)
            hoveredAction = HOVER_MENU_QUIT;
    }
    else if (helpOpen)
    {
        if (mx >= 1005 && mx <= 1075 && my >= 610 && my <= 695)
            hoveredAction = HOVER_HELP;
    }
    else if (gameState == 1 || gameState == 2)
    {
        if (mx >= 125 && mx <= 320 && my >= 105 && my <= 167)
            hoveredAction = HOVER_RETRY;
        else if (mx >= 345 && mx <= 540 && my >= 105 && my <= 167)
            hoveredAction = HOVER_RESULT_MENU;
        else if (mx >= 565 && mx <= 760 && my >= 105 && my <= 167)
            hoveredAction = HOVER_RESULT_HELP;
        else if (mx >= 785 && mx <= 975 && my >= 105 && my <= 167)
            hoveredAction = HOVER_RESULT_QUIT;
        else if (mx >= 1005 && mx <= 1075 && my >= 610 && my <= 695)
            hoveredAction = HOVER_RESULT_HELP;
    }
    else
    {
        if (mx >= 1005 && mx <= 1075 && my >= 610 && my <= 695)
            hoveredAction = HOVER_HELP;
    }

    glutPostRedisplay();
}

void mouseClick(int button, int state, int x, int y)
{
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN)
        return;

    float mx, my;
    getGameMouseCoordinates(x, y, mx, my);

    if (screenState == 0)
    {
        if (mx >= 205 && mx <= 895 && my >= 450 && my <= 518)
            enterGame(EASY);
        else if (mx >= 205 && mx <= 895 && my >= 355 && my <= 423)
            enterGame(MEDIUM);
        else if (mx >= 205 && mx <= 895 && my >= 260 && my <= 328)
            enterGame(HARD);
        else if (mx >= 400 && mx <= 700 && my >= 58 && my <= 110)
            exit(0);
    }
    else if (helpOpen)
    {
        if (mx >= 1005 && mx <= 1075 && my >= 610 && my <= 695)
            helpOpen = 0;
    }
    else if (gameState == 1 || gameState == 2)
    {
        if (mx >= 125 && mx <= 320 && my >= 105 && my <= 167)
            resetSpacecraft();
        else if (mx >= 345 && mx <= 540 && my >= 105 && my <= 167)
            returnToMenu();
        else if (mx >= 565 && mx <= 760 && my >= 105 && my <= 167)
        {
            helpOpen = 1;
            braking = 0;
            thrustForward = 0;
            thrustBackward = 0;
            rotateLeft = 0;
            rotateRight = 0;
        }
        else if (mx >= 785 && mx <= 975 && my >= 105 && my <= 167)
            exit(0);
    }
    else
    {
        if (mx >= 1005 && mx <= 1075 && my >= 610 && my <= 695)
        {
            helpOpen = 1;
            braking = 0;
            thrustForward = 0;
            thrustBackward = 0;
            rotateLeft = 0;
            rotateRight = 0;
        }
    }

    glutPostRedisplay();
}

void drawPanel(float x, float y, float w, float h)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.018f, 0.055f, 0.095f, 0.94f);
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x+w, y);
    glVertex2f(x+w, y+h); glVertex2f(x, y+h);
    glEnd();

    glColor4f(0.20f, 0.65f, 0.95f, 0.78f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y); glVertex2f(x+w, y);
    glVertex2f(x+w, y+h); glVertex2f(x, y+h);
    glEnd();
    glLineWidth(1.0f);
    glDisable(GL_BLEND);
}

void drawHelpButton()
{
    float x=1035.0f,y=650.0f,r=24.0f;
    int hovered=(hoveredAction==HOVER_HELP || hoveredAction==HOVER_RESULT_HELP);
    glColor4f(hovered?0.08f:0.025f,hovered?0.38f:0.16f,hovered?0.62f:0.28f,0.98f); drawCircle(x,y,r+2,40);
    glColor3f(0.55f,0.85f,1.0f); drawCircle(x,y,r,40);
    glColor3f(0.02f,0.07f,0.14f); drawCircle(x,y,r-4,40);
    glColor3f(1,1,1); drawCenteredText(x,y-5,"?");
}

void drawHelpOverlay()
{
    glColor4f(0,0,0.02f,0.80f); glBegin(GL_QUADS);
    glVertex2f(0,0); glVertex2f(WINDOW_WIDTH,0); glVertex2f(WINDOW_WIDTH,WINDOW_HEIGHT); glVertex2f(0,WINDOW_HEIGHT); glEnd();
    drawPanel(200,70,700,580);
    glColor3f(0.30f,1.0f,0.40f); drawCenteredText(550,605,"DOCKING HELP");
    glColor3f(0.72f,0.84f,1.0f); DrawText(250,560,"CURRENT DIFFICULTY:");
    glColor3f(1.0f,0.85f,0.25f); DrawText(470,560,difficultyName);
    char helpText[100];
    glColor3f(0.90f,0.95f,1.0f); DrawText(250,515,"APPROACH VELOCITY");
    glColor3f(0.20f,1.0f,0.35f); DrawText(280,485,"SAFE:"); sprintf(helpText,"<= %.2f m/s",MAX_DOCKING_SPEED); DrawText(470,485,helpText);
    glColor3f(0.90f,0.95f,1.0f); DrawText(250,445,"LATERAL VELOCITY");
    glColor3f(0.20f,1.0f,0.35f); DrawText(280,415,"SAFE:"); sprintf(helpText,"<= %.2f m/s",MAX_LATERAL_DOCKING_SPEED); DrawText(470,415,helpText);
    glColor3f(0.90f,0.95f,1.0f); DrawText(250,375,"TAIL ALIGNMENT");
    glColor3f(0.20f,1.0f,0.35f); DrawText(280,345,"DOCK:"); sprintf(helpText,"<= %.1f degrees",ALIGNMENT_TOLERANCE); DrawText(470,345,helpText);
    glColor3f(0.90f,0.95f,1.0f); DrawText(250,300,"CONTROLS");
    DrawText(280,265,"UP / DOWN  : THRUST"); DrawText(570,265,"LEFT / RIGHT : ROTATE");
    DrawText(280,235,"SPACE      : BRAKE"); DrawText(570,235,"R : RESET");
    DrawText(280,205,"M : MENU"); DrawText(570,205,"Q : QUIT");
    glColor3f(0.45f,0.72f,0.92f); drawCenteredText(550,125,"H / ? OR CLICK THE ? BUTTON TO CLOSE");
}

void drawResultButton(float x,float y,float w,float h,const char* label,int action)
{
    int hovered=(hoveredAction==action);

    if(hovered)
        glColor4f(0.035f,0.28f,0.52f,0.98f);
    else
        glColor4f(0.015f,0.055f,0.11f,0.96f);

    glBegin(GL_QUADS);
    glVertex2f(x,y); glVertex2f(x+w,y);
    glVertex2f(x+w,y+h); glVertex2f(x,y+h);
    glEnd();

    if(hovered) glColor3f(0.30f,0.86f,1.0f);
    else glColor3f(0.18f,0.56f,0.90f);

    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x,y); glVertex2f(x+w,y);
    glVertex2f(x+w,y+h); glVertex2f(x,y+h);
    glEnd();
    glLineWidth(1.0f);

    glColor3f(0.92f,0.97f,1.0f);
    drawCenteredText(x+w*0.5f,y+h*0.5f-5.0f,label);
}

void drawResultOverlay()
{
    if(gameState!=1 && gameState!=2) return;

    glColor4f(0.0f,0.005f,0.025f,0.88f);
    glBegin(GL_QUADS);
    glVertex2f(0,0); glVertex2f(WINDOW_WIDTH,0);
    glVertex2f(WINDOW_WIDTH,WINDOW_HEIGHT); glVertex2f(0,WINDOW_HEIGHT);
    glEnd();

    drawPanel(95,55,910,610);
    char text[120];

    if(gameState==1)
    {
        glColor3f(0.18f,1.0f,0.30f);
        drawCenteredText(550,615,"DOCKING COMPLETE!");
        glColor3f(0.55f,0.95f,0.62f);
        drawCenteredText(550,585,"MISSION SUCCESS  •  REAR PORT SECURED");
    }
    else
    {
        glColor3f(1.0f,0.25f,0.22f);
        drawCenteredText(550,615,"DOCKING FAILED");
        glColor3f(1.0f,0.70f,0.28f);
        sprintf(text,"REASON: %s",failureReason);
        drawCenteredText(550,585,text);
    }

    // Header information row.
    glColor3f(0.78f,0.88f,1.0f);
    sprintf(text,"DIFFICULTY: %s",difficultyName);
    DrawText(155,535,text);
    sprintf(text,"MISSION SCORE: %d / 100",missionScore);
    DrawText(700,535,text);

    // Two aligned stat cards.
    drawPanel(145,350,365,145);
    drawPanel(590,350,365,145);

    glColor3f(0.42f,0.75f,1.0f);
    DrawText(175,465,"APPROACH / CONTACT");
    DrawText(620,465,"DOCKING CONDITION");

    if(gameState==1)
    {
        sprintf(text,"CONTACT SPEED     %.2f m/s",contactSpeed); DrawText(175,430,text);
        sprintf(text,"LATERAL SPEED     %.2f m/s",contactLateralSpeed); DrawText(175,402,text);
        sprintf(text,"ALIGNMENT         %.1f deg",contactAngleError); DrawText(175,374,text);

        if(dockingQuality==3) glColor3f(0.20f,1.0f,0.30f);
        else if(dockingQuality==2) glColor3f(0.30f,0.82f,1.0f);
        else glColor3f(1.0f,0.72f,0.15f);

        if(dockingQuality==3) DrawText(620,430,"QUALITY          PERFECT");
        else if(dockingQuality==2) DrawText(620,430,"QUALITY          GOOD");
        else DrawText(620,430,"QUALITY          HARD CONTACT");

        glColor3f(0.78f,0.88f,1.0f);
        sprintf(text,"FUEL REMAINING    %.1f%%",fuel); DrawText(620,402,text);
        DrawText(620,374,"REAR PORT         CONNECTED");
    }
    else
    {
        glColor3f(1.0f,0.72f,0.30f);
        sprintf(text,"APPROACH SPEED    %.2f m/s",relativeApproachSpeed); DrawText(175,430,text);
        sprintf(text,"LATERAL SPEED     %.2f m/s",lateralVelocity); DrawText(175,402,text);
        sprintf(text,"ALIGNMENT ERROR   %.1f deg",angleError); DrawText(175,374,text);

        glColor3f(0.78f,0.88f,1.0f);
        sprintf(text,"FUEL REMAINING    %.1f%%",fuel); DrawText(620,430,text);
        DrawText(620,402,"MISSION           TERMINATED");
        DrawText(620,374,"SELECT RETRY TO TRY AGAIN");
    }

    glColor3f(0.50f,0.72f,0.92f);
    drawCenteredText(550,300,"SELECT AN ACTION");
    glColor3f(0.42f,0.65f,0.84f);
    drawCenteredText(550,275,"Mouse hover highlights the action under the cursor");

    // Four equal buttons, all with visible keyboard shortcuts.
    drawResultButton(125,105,195,62,"RETRY (R)",HOVER_RETRY);
    drawResultButton(345,105,195,62,"MENU (M)",HOVER_RESULT_MENU);
    drawResultButton(565,105,195,62,"HELP (H)",HOVER_RESULT_HELP);
    drawResultButton(785,105,190,62,"QUIT (Q)",HOVER_RESULT_QUIT);

    glColor3f(0.48f,0.70f,0.88f);
    drawCenteredText(550,75,"R  RETRY     M  MENU     H  HELP     Q  QUIT");
}

void drawGameplayVignette()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.0f,0.0f,0.015f,0.10f);
    glBegin(GL_QUADS);
    glVertex2f(0,0); glVertex2f(WINDOW_WIDTH,0);
    glVertex2f(WINDOW_WIDTH,WINDOW_HEIGHT); glVertex2f(0,WINDOW_HEIGHT);
    glEnd();

    glDisable(GL_BLEND);
}

void drawSpaceBackground()
{
    if(spaceBackgroundLoaded)
    {
        glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D,spaceBackgroundTexture); glColor3f(1,1,1);
        glBegin(GL_QUADS);
        glTexCoord2f(0,0); glVertex2f(0,0); glTexCoord2f(1,0); glVertex2f(WINDOW_WIDTH,0);
        glTexCoord2f(1,1); glVertex2f(WINDOW_WIDTH,WINDOW_HEIGHT); glTexCoord2f(0,1); glVertex2f(0,WINDOW_HEIGHT);
        glEnd(); glBindTexture(GL_TEXTURE_2D,0); glDisable(GL_TEXTURE_2D);
    }
    else
    {
        glColor3f(0,0,0.02f); glBegin(GL_QUADS); glVertex2f(0,0); glVertex2f(WINDOW_WIDTH,0); glVertex2f(WINDOW_WIDTH,WINDOW_HEIGHT); glVertex2f(0,WINDOW_HEIGHT); glEnd();
        drawDistantStarfield(); drawStars();
    }
}

int loadSpaceBackgroundBMP(const char* filename)
{
    FILE* file=fopen(filename,"rb"); if(!file)return 0;
    unsigned char header[54];
    if(fread(header,1,54,file)!=54||header[0]!='B'||header[1]!='M'){fclose(file);return 0;}
    int dataOffset=*(int*)&header[10], width=*(int*)&header[18], height=*(int*)&header[22];
    short planes=*(short*)&header[26], bits=*(short*)&header[28]; int compression=*(int*)&header[30];
    if(width<=0||height<=0||planes!=1||bits!=24||compression!=0){fclose(file);return 0;}
    int rowSize=(width*3+3)&(~3); size_t dataSize=(size_t)rowSize*(size_t)height;
    unsigned char* data=(unsigned char*)malloc(dataSize); if(!data){fclose(file);return 0;}
    fseek(file,dataOffset,SEEK_SET); if(fread(data,1,dataSize,file)!=dataSize){free(data);fclose(file);return 0;} fclose(file);
    size_t rgbSize=(size_t)width*(size_t)height*3; unsigned char* rgb=(unsigned char*)malloc(rgbSize); if(!rgb){free(data);return 0;}
    for(int y=0;y<height;y++) for(int x=0;x<width;x++)
    { unsigned char* s=data+(size_t)(height-1-y)*rowSize+(size_t)x*3; unsigned char* d=rgb+((size_t)y*(size_t)width+(size_t)x)*3; d[0]=s[2];d[1]=s[1];d[2]=s[0]; }
    free(data);
    glGenTextures(1,&spaceBackgroundTexture); glBindTexture(GL_TEXTURE_2D,spaceBackgroundTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP); glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP); glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,rgb); glBindTexture(GL_TEXTURE_2D,0); free(rgb);
    spaceBackgroundLoaded=1; return 1;
}

// DISPLAY
float worldZoom = 0.76f;
float worldPanX = 18.0f;
float worldPanY = 0.0f;

void beginWorldView()
{
    glPushMatrix();

    glTranslatef(WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.5f, 0.0f);
    glScalef(worldZoom, worldZoom, 1.0f);
    glTranslatef(-WINDOW_WIDTH * 0.5f + worldPanX,
                 -WINDOW_HEIGHT * 0.5f + worldPanY, 0.0f);
}

void endWorldView()
{
    glPopMatrix();
}


void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(0.0f, 0.0f, 0.02f);

    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(WINDOW_WIDTH, 0);
    glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
    glVertex2f(0, WINDOW_HEIGHT);
    glEnd();

    if (screenState == 0)
    {
        drawMenu();
    }
    else
    {
        drawSpaceBackground();
        drawGameplayVignette();
        drawSun();
        drawSceneLightEffects();
        beginWorldView();
        drawEarth();

        drawDockingCorridor();
        drawDockingEnvelope();
        drawSpaceStation();
        drawVelocityVector();
        drawSpacecraftShadow();
    drawSpacecraft();
                endWorldView();

drawHUD();

        drawHelpButton();

        if ((gameState == 1 || gameState == 2) && !helpOpen)
            drawResultOverlay();

        if (helpOpen)
            drawHelpOverlay();
    }

    glutSwapBuffers();
}
// TIMER
void timer(int value)
{
    int currentTime =
        glutGet(GLUT_ELAPSED_TIME);

    float dt =
        (currentTime - lastTime) /
        1000.0f;

    lastTime = currentTime;

    if (dt > 0.05f)
        dt = 0.05f;

    if (dt < 0.0f)
        dt = 0.0f;

    if (screenState == 1 && !helpOpen)
        updatePhysics(dt);

    glutPostRedisplay();

    glutTimerFunc(
        16,
        timer,
        0
    );
}
// KEYBOARD DOWN
void keyboardDown(
    unsigned char key,
    int x,
    int y
)
{
    if (screenState == 0)
    {
        if (key == '1')
            enterGame(EASY);
        else if (key == '2')
            enterGame(MEDIUM);
        else if (key == '3')
            enterGame(HARD);
        else if (key == 'h' || key == 'H' || key == '?')
        {
            // Help is intentionally only available inside the game.
        }
        else if (key == 'q' || key == 'Q')
        {
            exit(0);
        }

        glutPostRedisplay();
        return;
    }

    if (key == ' ')
    {
        braking = 1;
    }
    else if (key == 'r' || key == 'R')
    {
        resetSpacecraft();
    }
    else if (key == 'h' || key == 'H' || key == '?')
    {
        helpOpen = !helpOpen;
        braking = 0;
        thrustForward = 0;
        thrustBackward = 0;
        rotateLeft = 0;
        rotateRight = 0;
    }
    else if (key == 'm' || key == 'M')
    {
        returnToMenu();
    }
    else if (key == 'q' || key == 'Q')
    {
        exit(0);
    }
    else if (key == 27)
    {
        if (helpOpen)
            helpOpen = 0;
        else
            returnToMenu();
    }

    glutPostRedisplay();
}
// KEYBOARD UP
void keyboardUp(
    unsigned char key,
    int x,
    int y
)
{
    if (key == ' ')
        braking = 0;

    glutPostRedisplay();
}
// SPECIAL KEY DOWN
void specialDown(
    int key,
    int x,
    int y
)
{
    if (screenState != 1 || helpOpen || gameState != 0)
        return;

    if (key == GLUT_KEY_UP)
    {
        thrustForward = 1;
    }
    else if (key == GLUT_KEY_DOWN)
    {
        thrustBackward = 1;
    }
    else if (key == GLUT_KEY_LEFT)
    {
        rotateLeft = 1;
    }
    else if (key == GLUT_KEY_RIGHT)
    {
        rotateRight = 1;
    }

    glutPostRedisplay();
}
// SPECIAL KEY UP
void specialUp(
    int key,
    int x,
    int y
)
{
    if (key == GLUT_KEY_UP)
    {
        thrustForward = 0;
    }
    else if (key == GLUT_KEY_DOWN)
    {
        thrustBackward = 0;
    }
    else if (key == GLUT_KEY_LEFT)
    {
        rotateLeft = 0;
    }
    else if (key == GLUT_KEY_RIGHT)
    {
        rotateRight = 0;
    }

    glutPostRedisplay();
}
// RESET
void resetSpacecraft()
{
    shipX = 300.0f;
    shipY = 350.0f;

    shipAngle = 0.0f;

    velocityX = 0.0f;
    velocityY = 0.0f;

    speed = 0.0f;

    fuel = startingFuel;

    dockingDistance = 0.0f;
    angleError = 0.0f;
    relativeApproachSpeed = 0.0f;

    closingSpeedSigned = 0.0f;
    timeToDock = 0.0f;
    brakingDistance = 0.0f;

    missionScore = 0;

    lateralVelocity = 0.0f;
    missionTime = 0.0f;
    dockingSequenceTime = 0.0f;
    contactSpeed = 0.0f;
    contactLateralSpeed = 0.0f;
    contactAngleError = 0.0f;
    dockingQuality = 0;
    failureReason[0] = '\0';

    gameState = 0;
    hoveredAction = HOVER_NONE;

    thrustForward = 0;
    thrustBackward = 0;

    rotateLeft = 0;
    rotateRight = 0;

    braking = 0;

    updateDockingPort();
    updateDistance();
    updateSpeed();
    updateAngleError();
    updateApproachSpeed();
    updateApproachPrediction();

    glutPostRedisplay();
}
// INITIALIZATION
void init()
{
    glClearColor(
        0.0f,
        0.0f,
        0.02f,
        1.0f
    );

    // Establish the fixed game canvas and center it in the actual window.
    reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

    glEnable(GL_BLEND);

    glBlendFunc(
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA
    );

    updateDockingPort();
    updateDistance();
    updateRelativeVelocity();

    loadSpaceBackgroundBMP("stage11_space_background.bmp");

    lastTime =
        glutGet(GLUT_ELAPSED_TIME);
}
// MAIN
int main(
    int argc,
    char* argv[]
)
{
    glutInit(
        &argc,
        argv
    );

    glutInitDisplayMode(
        GLUT_DOUBLE |
        GLUT_RGB
    );

    glutInitWindowPosition(
        80,
        40
    );

    glutInitWindowSize(
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );

    glutCreateWindow(
        "Spacecraft Docking Simulator - Stage 11"
    );

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);

    glutSpecialFunc(specialDown);
    glutSpecialUpFunc(specialUp);
    glutMouseFunc(mouseClick);
    glutPassiveMotionFunc(mouseMove);
    glutMotionFunc(mouseMove);

    applyDifficulty(MEDIUM);

    glutTimerFunc(
        16,
        timer,
        0
    );

    glutMainLoop();

    return 0;
}
