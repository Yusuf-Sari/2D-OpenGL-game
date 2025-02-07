#include "Angel.h"
#include <cstdlib>
#include <vector>
#include <iostream>

using namespace std;

void timeOut(int id);void updateGame();void drawCoin(vec2 pos);


GLenum glCheckError() {
    const char* msgs[] = { "INVALID_ENUM", "INVALID_VALUE", "INVALID_OPERATION",
                          "STACK_OVERFLOW", "STACK_UNDERFLOW",
                          "INVALID_FRAMEBUFFER_OPERATION" };
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        printf("%s  \n", msgs[errorCode - GL_INVALID_ENUM]);
    }
    return errorCode;
}

//----------------------------------------------------------------------------
int lineVertices = 0;
int sideWalkVertices = 36;
int dir = 0;
bool upKeyPressed = false;
bool downKeyPressed = false;
bool rightKeyPressed = false;
bool leftKeyPressed = false;
int totalPoints = 0;

enum VehicleType { CAR, TRUCK };

GLuint coinVAO, coinVBO, coinCBO;
vec2* line;
vec2* sideWalk = new vec2[sideWalkVertices];
GLuint vao;
GLuint buffer;

vec2 agentVertices[] = { 
    vec2(-0.05, -0.985), 
    vec2(0.05, -0.985), 
    vec2(0,-0.93) 
};

vec3 agentColors[] = {
    vec3(1.0, 0.0, 0.0),
    vec3(1.0, 0.0, 0.0),
    vec3(1.0, 0.0, 0.0)
};

GLuint agentVao;
GLuint agentVbo;
GLuint agentCbo;
GLuint combinedVAO;

struct Vertex {
    vec2 position;
    vec3 color;
};

GLuint rectVAO;
GLuint rectVBO;

void drawRect(vec2 pos, float width, float height, vec3 color);
float computeLane(int lane);
void initRect(GLuint program);
void initCoin(GLuint program);

struct Coin {
    vec2 pos;
    float timeLeft;
};

vector<Coin> coins;

struct Vehicle {
    VehicleType type;
    vec2 pos;
    float width;
    float height;
    float velocity;
    int lane;
    int direction;        // +1 moving right, -1 moving left

    Vehicle(VehicleType t, int laneNum, int dir, float startX, float v, float laneHeight)
        : type(t), lane(laneNum), direction(dir), pos(startX, 0), velocity(v)
    {

        pos.y = computeLane(lane);

        height = laneHeight * 0.8f;

        if (t == TRUCK) {
            width = height * (2.0f + 0.5f * ((float)rand() / RAND_MAX));  // Random extra width
        }
        else {
            width = height;
        }
    }

    // Update the vehicle's position.
    void update(float dt) {
        pos.x += velocity * direction * dt;
    }

    bool isOffScreen(float leftBound, float rightBound) const {
        if (direction == 1)
            return pos.x - width > rightBound;
        else 
            return pos.x + width < leftBound;
    }
};

vector<Vehicle*> vehicles;


void init(void) {
    // combined vertex array for lines and sidewalks.
    vec2* combinedVertices = new vec2[lineVertices + sideWalkVertices];
    memcpy(combinedVertices, line, lineVertices * sizeof(vec2));
    memcpy(combinedVertices + lineVertices, sideWalk, sideWalkVertices * sizeof(vec2));

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, (lineVertices + sideWalkVertices) * sizeof(vec2), combinedVertices, GL_STATIC_DRAW);

    GLuint program = InitShader("vshader21.glsl", "fshader21.glsl");
    glUseProgram(program);

    glGenVertexArrays(1, &combinedVAO);
    glBindVertexArray(combinedVAO);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    GLuint loc = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), BUFFER_OFFSET(0));

    GLuint colorLoc = glGetAttribLocation(program, "vColor");
    if (colorLoc != -1) {
        glDisableVertexAttribArray(colorLoc);
        glVertexAttrib3f(colorLoc, 0.0f, 0.0f, 0.0f);
    }
    glBindVertexArray(0);


    glGenVertexArrays(1, &agentVao);
    glBindVertexArray(agentVao);

    glGenBuffers(1, &agentVbo);
    glBindBuffer(GL_ARRAY_BUFFER, agentVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(agentVertices), agentVertices, GL_STATIC_DRAW);

    GLuint agentPosLoc = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(agentPosLoc);
    glVertexAttribPointer(agentPosLoc, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    glGenBuffers(1, &agentCbo);
    glBindBuffer(GL_ARRAY_BUFFER, agentCbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(agentColors), agentColors, GL_STATIC_DRAW);

    GLuint agentColorLoc = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(agentColorLoc);
    glVertexAttribPointer(agentColorLoc, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    glBindVertexArray(0);

    initRect(program);
    initCoin(program);

    glCheckError();

    glClearColor(1.0, 1.0, 1.0, 1.0);

}

void initRect(GLuint program) {
    glGenVertexArrays(1, &rectVAO);
    glBindVertexArray(rectVAO);

    glGenBuffers(1, &rectVBO);
    glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);

    GLuint posLoc = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(0));

    GLuint colorLoc = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(colorLoc);
    glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec2)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

//----------------------------------------------------------------------------
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(combinedVAO);
    glDrawArrays(GL_LINES, 0, lineVertices);
    glDrawArrays(GL_TRIANGLES, lineVertices, sideWalkVertices);
    glBindVertexArray(0);

    glBindVertexArray(agentVao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    for (size_t i = 0; i < vehicles.size(); i++) {
        vec3 color = (vehicles[i]->type == TRUCK) ? vec3(0, 0.4, 0.1) : vec3(0.6, 0.3, 0.5);
        drawRect(vehicles[i]->pos, vehicles[i]->width, vehicles[i]->height, color);
    }

    for (size_t i = 0; i < coins.size(); i++) {
        drawCoin(coins[i].pos);
    }

    glutSwapBuffers();
}


void spawnCoin() {
    Coin coin;
    coin.pos = vec2(((rand() % 200) - 100) / 100.0f, ((rand() % 200) - 100) / 100.0f);
    coin.timeLeft = 5.0f;  // coin lasts 5 seconds
    coins.push_back(coin);
}

void updateCoins(float dt) {
    for (auto x = coins.begin(); x != coins.end(); ) {
        x->timeLeft -= dt;
        if (x->timeLeft <= 0) 
            x = coins.erase(x);
        else 
            ++x;
    }
}

void initCoin(GLuint program) {

    glGenVertexArrays(1, &coinVAO);
    glBindVertexArray(coinVAO);

    glGenBuffers(1, &coinVBO);
    glBindBuffer(GL_ARRAY_BUFFER, coinVBO);
    glBufferData(GL_ARRAY_BUFFER, 38 * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);

    GLuint posLoc = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(0));

    GLuint colorLoc = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(colorLoc);
    glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec2)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void drawRect(vec2 pos, float width, float height, vec3 color) {
    Vertex vertices[6];

    vertices[0].position = pos;
    vertices[0].color = color;

    vertices[1].position = vec2(pos.x + width, pos.y);
    vertices[1].color = color;

    vertices[2].position = vec2(pos.x + width, pos.y + height);
    vertices[2].color = color;

    vertices[3].position = pos;
    vertices[3].color = color;

    vertices[4].position = vec2(pos.x + width, pos.y + height);
    vertices[4].color = color;

    vertices[5].position = vec2(pos.x, pos.y + height);
    vertices[5].color = color;

    glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glBindVertexArray(rectVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void drawCoin(vec2 pos) {

    Vertex vertices[38];

    vec3 yellow = vec3(1.0, 1.0, 0.0);

    vertices[0].position = pos;
    vertices[0].color = yellow;

    for (int i = 1; i <= 37; i++) {
        float angle = (2.0f * M_PI * i) / 36;
        float x = pos.x + cos(angle) * 0.03f;
        float y = pos.y + sin(angle) * 0.03f;

        vertices[i].position = vec2(x, y);
        vertices[i].color = yellow;
    }


    glBindBuffer(GL_ARRAY_BUFFER, coinVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glBindVertexArray(coinVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 38);
    glBindVertexArray(0);
}

void lines() {
    const float lineSpacing = 0.083f;
    const float dashLength = 0.06f;
    const float gap = 0.03f;
    const float start = -1.0f;
    const float end = 1.0f;

    for (float y = start; y <= end; y += lineSpacing) {
        float x = -1.0f;
        while (x < 1.0f) {
            lineVertices += 2; // two vertices per dash
            x += dashLength + gap;
        }
    }


    line = new vec2[lineVertices];
    int index = 0;

    for (float y = start; y <= end; y += lineSpacing) {
        float x = -1.0f;
        while (x < 1.0f) {
            float xEnd = x + dashLength;
            if (xEnd > 1.0f) xEnd = 1.0f;

            // two points for each dash
            line[index++] = vec2(x, y);
            line[index++] = vec2(xEnd, y);

            x += dashLength + gap;
        }
    }
}

void sideWalks() {
    int index = 0;
    float height = 0.083f;
    float x1 = -1.0f;
    float x2 = 1.0f;
    float y1 = -1.0f;

    int road[] = {5,4,5,5,4,5};

    for (int i = 0; i < 6; i++) {

        sideWalk[index++] = vec2(x1, y1);
        sideWalk[index++] = vec2(x2, y1);
        sideWalk[index++] = vec2(x2, y1 + height);
        sideWalk[index++] = vec2(x1, y1);
        sideWalk[index++] = vec2(x2, y1 + height);
        sideWalk[index++] = vec2(x1, y1 + height);

        y1 += road[i] * height;
    }
}



float computeLane(int lane) {
    if (lane == 0)
        lane = 1;
    if (lane == 5)
        lane = 6;
    if (lane == 9)
        lane = 10;
    if (lane == 14)
        lane = 15;
    if (lane == 19)
        lane = 20;
    if (lane == 23)
        lane = 22;

    float laneSpacing = 0.083f;
    return -1.0f + lane * laneSpacing + 0.007;
}
//----------------------------------------------------------------------------

bool timer = true;
bool save = false;

void keyboard(unsigned char c, int x, int y) {
    switch (c) {
    case 'q':
        exit(0);
    case 'p':
        if (timer)
            timer = false;
        else {
            timer = true;
            glutTimerFunc(20, timeOut, 0);
        }
        break;
    case 's':
        save = true;
        glutTimerFunc(20, timeOut, 0);

        cout << "*** Save state ***" << "\n";
        cout << "Total Points:  " << totalPoints << "\n";
        cout << "Agent Vertices: " << agentVertices[0] << agentVertices[1] << agentVertices[2] << "\n";
        cout << "Vehicle Count: " << vehicles.size() << "\n";

        for (size_t i = 0; i < vehicles.size(); i++) {
            cout << "Vehicle " << i << " Location: " << vehicles[i]->pos << "\n";
        }
        
        cout <<  "\n\n";

        break;
    }
    glutPostRedisplay();
}



void specialKeys(int key, int x, int y) {
    updateGame();
    switch (key) {
    case GLUT_KEY_UP:

        if (dir == 1) exit(0);

        if (!upKeyPressed && dir == 0) {
            totalPoints++;
            cout << "Total Points:  " << totalPoints << "\n\n";
            upKeyPressed = true;
            for (int i = 0; i < 3; i++) {
                agentVertices[i].y += 0.083f;
            }

            if (agentVertices[0].y > 0.917) {
                agentVertices[0].y = 0.985f;
                agentVertices[1].y = 0.985f;
                agentVertices[2].y = 0.930f;
                dir = 1;
            }

            glBindBuffer(GL_ARRAY_BUFFER, agentVbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(agentVertices), agentVertices, GL_STATIC_DRAW);
        }


        break;

    case GLUT_KEY_DOWN:

        if (dir == 0) exit(0);
        
        if (!downKeyPressed && dir == 1) {
            totalPoints++;
            cout << "Total Points:  " << totalPoints << "\n\n";
            downKeyPressed = true;
            for (int i = 0; i < 3; i++) {
                agentVertices[i].y -= 0.083f;
            }

            if (agentVertices[0].y < -0.917) {
                agentVertices[0].y = -0.985f;
                agentVertices[1].y = -0.985f;
                agentVertices[2].y = -0.930f;
                dir = 0;
            }

            glBindBuffer(GL_ARRAY_BUFFER, agentVbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(agentVertices), agentVertices, GL_STATIC_DRAW);
        }

        break;

    case GLUT_KEY_RIGHT:

        if (!rightKeyPressed) {
            rightKeyPressed = true;

            bool stop = false;

            for (int i = 0; i < 3; i++) {
                if (agentVertices[i].x > 0.917)
                    stop = true;
            }

            for (int i = 0; i < 3; i++) {
                if (!stop)
                    agentVertices[i].x += 0.083f;
            }

            glBindBuffer(GL_ARRAY_BUFFER, agentVbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(agentVertices), agentVertices, GL_STATIC_DRAW);
        }

        break;
        
    case GLUT_KEY_LEFT:

        if (!leftKeyPressed) {
            leftKeyPressed = true;

            bool stop = false;

            for (int i = 0; i < 3; i++) {
                if (agentVertices[i].x < -0.917)
                    stop = true;

            }

            for (int i = 0; i < 3; i++) {
                if (!stop)
                    agentVertices[i].x -= 0.083f;
            }

            glBindBuffer(GL_ARRAY_BUFFER, agentVbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(agentVertices), agentVertices, GL_STATIC_DRAW);
        }

        break;

    }
    glutPostRedisplay();
}

void specialKeysUp(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP:
        upKeyPressed = false;
        break;
    case GLUT_KEY_DOWN:
        downKeyPressed = false;
        break;
      case GLUT_KEY_RIGHT:
        rightKeyPressed = false;
        break;
    case GLUT_KEY_LEFT:
        leftKeyPressed = false;
        break;
    }
}

const float LEFT_BOUND = -1.2f;
const float RIGHT_BOUND = 1.2f;
float lastTime = 0.0f;

void timeOut(int id) {
    if (!timer)
        return;

    updateGame();

    float x = 0.02f;


    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float dt = currentTime - lastTime;
    lastTime = currentTime;

    updateCoins(dt);
    if (rand() % 100 < 0.5)
        spawnCoin();

    for (size_t i = 0; i < vehicles.size();) {
        vehicles[i]->update(x);
        if (vehicles[i]->isOffScreen(LEFT_BOUND, RIGHT_BOUND)) {
            delete vehicles[i];
            vehicles.erase(vehicles.begin() + i);
        }
        else {
            i++;
        }
    }

    if (rand() % 100 < 25) {
        int lane = rand() % 24;
        VehicleType type = (rand() % 2 == 0) ? CAR : TRUCK;
        int direction = 1;
        if (lane == 0) direction = 1;
        else if (lane == 1) direction = 1;
        else if (lane == 2) direction = -1;
        else if (lane == 3) direction = -1;
        else if (lane == 4) direction = 1;
        else if (lane == 5) direction = -1;
        else if (lane == 6) direction = -1;
        else if (lane == 7) direction = 1;
        else if (lane == 8) direction = -1;
        else if (lane == 9) direction = 1;
        else if (lane == 10) direction = 1;
        else if (lane == 11) direction = -1;
        else if (lane == 12) direction = 1;
        else if (lane == 13) direction = 1;
        else if (lane == 14) direction = -1;
        else if (lane == 15) direction = -1;
        else if (lane == 16) direction = 1;
        else if (lane == 17) direction = -1;
        else if (lane == 18) direction = 1;
        else if (lane == 19) direction = -1;
        else if (lane == 20) direction = -1;
        else if (lane == 21) direction = -1;
        else if (lane == 22) direction = 1;
        else if (lane == 23) direction = 1;


        float startX = (direction == 1) ? LEFT_BOUND - 0.1f : RIGHT_BOUND + 0.1f;
        float velocity = 0.8f + ((float)rand() / RAND_MAX);
        float laneSpacing = 0.083f;
        Vehicle* v = new Vehicle(type, lane, direction, startX, velocity, laneSpacing);
        vehicles.push_back(v);
    }

    glutPostRedisplay();
    if (!save)
        glutTimerFunc(20, timeOut, 0);
}

void collisionDetection(vec2 vertice1, vec2 vertice2, vec2 vertice3, Vehicle* vehicle) {


    float agentWidth = vertice2.x - vertice1.x;
    float agentHeight = vertice3.y - vertice1.y;
    if (dir == 0) {
        agentHeight = vertice3.y - vertice1.y;

        bool collisionDetected = vertice1.x < vehicle->pos.x + vehicle->width &&
            vertice1.x + agentWidth > vehicle->pos.x &&
            vertice1.y < vehicle->pos.y + vehicle->height &&
            vertice1.y + agentHeight > vehicle->pos.y;

        if (collisionDetected) exit(0);
    }    
    else {
        agentHeight = vertice1.y - vertice3.y;

        bool collisionDetected = vertice1.x < vehicle->pos.x + vehicle->width &&
            vertice1.x + agentWidth > vehicle->pos.x &&
            vertice3.y < vehicle->pos.y + vehicle->height &&
            vertice3.y + agentHeight > vehicle->pos.y;

        if (collisionDetected) exit(0);
    }
}
bool collisionDetectionCoin(vec2 vertice1, vec2 vertice2, vec2 vertice3, Coin &coin) {

    float agentWidth = vertice2.x - vertice1.x;
    float agentHeight = vertice3.y - vertice1.y;
    if (dir == 0) {
        agentHeight = vertice3.y - vertice1.y;


        return (vertice1.x < coin.pos.x + 0.04f &&
            vertice1.x + agentWidth > coin.pos.x - 0.03f &&
            vertice1.y < coin.pos.y + 0.04f &&
            vertice1.y + agentHeight > coin.pos.y - 0.03f);
    }
    else {
        agentHeight = vertice1.y - vertice3.y;


        return (vertice1.x < coin.pos.x + 0.04f &&
            vertice1.x + agentWidth > coin.pos.x - 0.03f &&
            vertice3.y < coin.pos.y + 0.04f &&
            vertice3.y + agentHeight > coin.pos.y - 0.03f);
    }

}

void checkCollisions() {
    for (size_t i = 0; i < vehicles.size(); i++) {
        collisionDetection(agentVertices[0], agentVertices[1], agentVertices[2], vehicles[i]);
    }    
    for (auto it = coins.begin(); it != coins.end(); ) {
        if (collisionDetectionCoin(agentVertices[0], agentVertices[1], agentVertices[2], *it)) {
            totalPoints += 5;
            cout << "Coin collected.\n";
            cout << "Total Points:" << totalPoints << "\n";
            it = coins.erase(it);
        }
        else {
            ++it;
        }
    }
}

void updateGame() {
    checkCollisions();
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
int main(int argc, char** argv) {
    glutInit(&argc, argv);

#ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGBA);
#else
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB );
#endif
    srand((unsigned int)time(NULL));

    glutInitWindowSize(500, 600);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("2D Game");

    glewExperimental = GL_TRUE;
    glewInit();

    printf("%s\n", glGetString(GL_VERSION));

    lines();
    sideWalks();
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(NULL);
    glutSpecialFunc(specialKeys);
    glutSpecialUpFunc(specialKeysUp);
    glutTimerFunc(20, timeOut, 0);

    glutMainLoop();

    return 0;
}
