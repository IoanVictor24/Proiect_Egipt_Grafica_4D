#include "C:\Users\Administrator\Documents\libraries\freeglut\include\GL\freeglut.h"
#include <math.h>
#include <stdio.h>
#include "stb_image.h"

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

// --- ID-uri Texturi ---
GLuint texGround, texSky, texLouvre, texAsphalt, texBuilding, texTree;

// --- Variabile Cameră ---
float camX = 0.0f, camY = 3.0f, camZ = 50.0f;
float camYaw = 0.0f, camPitch = 0.0f;
float skyRotation = 0.0f;
bool isShadowPass = false; // Ne spune dacă acum desenăm umbre sau obiecte reale

// ==========================================
// --- FUNCȚII UTILE ---
// ==========================================

GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 4);

    if (data) {
        int totalPixels = width * height;
        for (int i = 0; i < totalPixels * 4; i += 4) {
            unsigned char r = data[i], g = data[i + 1], b = data[i + 2];
            // Eliminăm albul pentru transparență
            if (r > 240 && g > 240 && b > 240) data[i + 3] = 0;
            else data[i + 3] = 255;
        }
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
    else {
        printf("Eroare la incarcarea texturii: %s\n", path);
    }
    stbi_image_free(data);
    return textureID;
}

void buildShadowMatrix(float shadowMat[16], float lightPos[4], float plane[4]) {
    float dot = plane[0] * lightPos[0] + plane[1] * lightPos[1] + plane[2] * lightPos[2] + plane[3] * lightPos[3];
    shadowMat[0] = dot - lightPos[0] * plane[0];
    shadowMat[4] = 0.f - lightPos[0] * plane[1];
    shadowMat[8] = 0.f - lightPos[0] * plane[2];
    shadowMat[12] = 0.f - lightPos[0] * plane[3];
    shadowMat[1] = 0.f - lightPos[1] * plane[0];
    shadowMat[5] = dot - lightPos[1] * plane[1];
    shadowMat[9] = 0.f - lightPos[1] * plane[2];
    shadowMat[13] = 0.f - lightPos[1] * plane[3];
    shadowMat[2] = 0.f - lightPos[2] * plane[0];
    shadowMat[6] = 0.f - lightPos[2] * plane[1];
    shadowMat[10] = dot - lightPos[2] * plane[2];
    shadowMat[14] = 0.f - lightPos[2] * plane[3];
    shadowMat[3] = 0.f - lightPos[3] * plane[0];
    shadowMat[7] = 0.f - lightPos[3] * plane[1];
    shadowMat[11] = 0.f - lightPos[3] * plane[2];
    shadowMat[15] = dot - lightPos[3] * plane[3];
}

// ==========================================
// --- GEOMETRIE DESENARE ---
// ==========================================

// Desenează un cub simplu (pentru bănci, felinare etc)
void drawBlock(float w, float h, float d) {
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f); glVertex3f(-w, 0.0f, d); glVertex3f(w, 0.0f, d); glVertex3f(w, h, d); glVertex3f(-w, h, d);
    glNormal3f(0.0f, 0.0f, -1.0f); glVertex3f(w, 0.0f, -d); glVertex3f(-w, 0.0f, -d); glVertex3f(-w, h, -d); glVertex3f(w, h, -d);
    glNormal3f(-1.0f, 0.0f, 0.0f); glVertex3f(-w, 0.0f, -d); glVertex3f(-w, 0.0f, d); glVertex3f(-w, h, d); glVertex3f(-w, h, -d);
    glNormal3f(1.0f, 0.0f, 0.0f); glVertex3f(w, 0.0f, d); glVertex3f(w, 0.0f, -d); glVertex3f(w, h, -d); glVertex3f(w, h, d);
    glNormal3f(0.0f, 1.0f, 0.0f); glVertex3f(-w, h, d); glVertex3f(w, h, d); glVertex3f(w, h, -d); glVertex3f(-w, h, -d);
    glEnd();
}

// Desenează clădirea cu tot cu acoperiș și textură
void drawBuilding(float w, float h, float d, GLuint texture) {
    if (!isShadowPass) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);
        glColor3f(1.0f, 1.0f, 1.0f);
    }

    // Corp clădire
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, 0.0f, d); glTexCoord2f(1.0f, 0.0f); glVertex3f(w, 0.0f, d); glTexCoord2f(1.0f, 1.0f); glVertex3f(w, h, d); glTexCoord2f(0.0f, 1.0f); glVertex3f(-w, h, d);
    glNormal3f(0.0f, 0.0f, -1.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(w, 0.0f, -d); glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, 0.0f, -d); glTexCoord2f(0.0f, 1.0f); glVertex3f(-w, h, -d); glTexCoord2f(1.0f, 1.0f); glVertex3f(w, h, -d);
    glNormal3f(-1.0f, 0.0f, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, 0.0f, -d); glTexCoord2f(1.0f, 0.0f); glVertex3f(-w, 0.0f, d); glTexCoord2f(1.0f, 1.0f); glVertex3f(-w, h, d); glTexCoord2f(0.0f, 1.0f); glVertex3f(-w, h, -d);
    glNormal3f(1.0f, 0.0f, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(w, 0.0f, d); glTexCoord2f(1.0f, 0.0f); glVertex3f(w, 0.0f, -d); glTexCoord2f(1.0f, 1.0f); glVertex3f(w, h, -d); glTexCoord2f(0.0f, 1.0f); glVertex3f(w, h, d);
    glEnd();

    // Acoperiș
    if (!isShadowPass) { glDisable(GL_TEXTURE_2D); glColor3f(0.3f, 0.35f, 0.4f); }
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f); glVertex3f(-w, h, d); glVertex3f(w, h, d); glVertex3f(w, h, -d); glVertex3f(-w, h, -d);
    glEnd();

    glBegin(GL_TRIANGLES);
    float rh = 3.0f;
    glNormal3f(0.0f, 0.5f, 1.0f); glVertex3f(0.0f, h + rh, 0.0f); glVertex3f(-w, h, d); glVertex3f(w, h, d);
    glNormal3f(0.0f, 0.5f, -1.0f); glVertex3f(0.0f, h + rh, 0.0f); glVertex3f(w, h, -d); glVertex3f(-w, h, -d);
    glNormal3f(-1.0f, 0.5f, 0.0f); glVertex3f(0.0f, h + rh, 0.0f); glVertex3f(-w, h, -d); glVertex3f(-w, h, d);
    glNormal3f(1.0f, 0.5f, 0.0f); glVertex3f(0.0f, h + rh, 0.0f); glVertex3f(w, h, d); glVertex3f(w, h, -d);
    glEnd();

    if (!isShadowPass) { glColor3f(1.0f, 1.0f, 1.0f); glEnable(GL_TEXTURE_2D); }
}

void drawCrossedQuadTree(float height, float width) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texTree);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glDisable(GL_LIGHTING);
    glEnable(GL_ALPHA_TEST);

    // Corectăm opacitatea pentru a permite umbrei să se deseneze
    if (isShadowPass) {
        glColor4f(0.05f, 0.05f, 0.05f, 0.5f);
        glAlphaFunc(GL_GREATER, 0.1f); // Lăsăm umbra semi-transparentă să treacă
    }
    else {
        glColor3f(1.0f, 1.0f, 1.0f);
        glAlphaFunc(GL_GREATER, 0.8f); // Tăiem strict marginea albă
    }

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-width, 0.0f, 0.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(width, 0.0f, 0.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f(width, height, 0.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(-width, height, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, 0.0f, -width); glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f, 0.0f, width); glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f, height, width); glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f, height, -width);
    glEnd();

    glDisable(GL_ALPHA_TEST);
    if (!isShadowPass) glEnable(GL_LIGHTING);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void drawStreetLamp(float height) {
    glDisable(GL_TEXTURE_2D);
    if (!isShadowPass) glColor3f(0.1f, 0.1f, 0.1f);

    glPushMatrix(); drawBlock(0.3f, 1.0f, 0.3f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 1.0f, 0.0f); drawBlock(0.1f, height - 2.0f, 0.1f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, height - 1.0f, 0.0f); drawBlock(0.4f, 0.1f, 0.4f); glPopMatrix();

    if (!isShadowPass) {
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 0.9f, 0.4f);
        glPushMatrix(); glTranslatef(0.0f, height - 0.7f, 0.0f); drawBlock(0.2f, 0.5f, 0.2f); glPopMatrix();
        glEnable(GL_LIGHTING);
        glColor3f(0.1f, 0.1f, 0.1f);
    }

    glPushMatrix();
    glTranslatef(0.0f, height - 0.2f, 0.0f);
    glBegin(GL_TRIANGLES);
    float bw = 0.45f, ah = 0.6f;
    glNormal3f(0.0f, 0.5f, 1.0f); glVertex3f(0.0f, ah, 0.0f); glVertex3f(-bw, 0.0f, bw); glVertex3f(bw, 0.0f, bw);
    glNormal3f(0.0f, 0.5f, -1.0f); glVertex3f(0.0f, ah, 0.0f); glVertex3f(bw, 0.0f, -bw); glVertex3f(-bw, 0.0f, -bw);
    glNormal3f(-1.0f, 0.5f, 0.0f); glVertex3f(0.0f, ah, 0.0f); glVertex3f(-bw, 0.0f, -bw); glVertex3f(-bw, 0.0f, bw);
    glNormal3f(1.0f, 0.5f, 0.0f); glVertex3f(0.0f, ah, 0.0f); glVertex3f(bw, 0.0f, bw); glVertex3f(bw, 0.0f, -bw);
    glEnd();
    glPopMatrix();
}

void drawBench() {
    glDisable(GL_TEXTURE_2D);
    float bw = 2.5f, bh = 0.6f, bd = 0.6f, bwh = 0.9f;

    if (!isShadowPass) glColor3f(0.05f, 0.05f, 0.05f);

    glPushMatrix(); glTranslatef(-bw + 0.2f, 0.0f, 0.0f); drawBlock(0.1f, 0.8f, bd); glPopMatrix();
    glPushMatrix(); glTranslatef(bw - 0.2f, 0.0f, 0.0f); drawBlock(0.1f, 0.8f, bd); glPopMatrix();

    if (!isShadowPass) glColor3f(0.4f, 0.25f, 0.1f);
    glPushMatrix(); glTranslatef(0.0f, bh, 0.0f); drawBlock(bw, 0.1f, bd); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, bh + bwh, -bd); glRotatef(15.0f, 1.0f, 0.0f, 0.0f); drawBlock(bw, 0.3f, 0.05f); glPopMatrix();
}

// ==========================================
// --- RANDARE SCENĂ ---
// ==========================================

void drawEnvironment() {
    glEnable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);

    // --- SOARELE (Globul galben fizic) ---
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 0.9f, 0.6f); // Galben-Soare
    glTranslatef(30.0f, 50.0f, 30.0f); // Poziția lui în cer
    glutSolidSphere(3.0, 30, 30);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glPopMatrix();

    // Pavajul
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texGround);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    float size = 150.0f; float rep = 45.0f;
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, -1.0f, -size);
    glTexCoord2f(rep, 0.0f);  glVertex3f(size, -1.0f, -size);
    glTexCoord2f(rep, rep);   glVertex3f(size, -1.0f, size);
    glTexCoord2f(0.0f, rep);  glVertex3f(-size, -1.0f, size);
    glEnd();

    // Circuitul stradal
    glBindTexture(GL_TEXTURE_2D, texAsphalt);
    glBegin(GL_QUAD_STRIP);
    float rIn = 30.0f, rOut = 40.0f;
    for (int i = 0; i <= 50; i++) {
        float u = 2.0f * 3.14159f * float(i) / 50.0f;
        glNormal3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(i * 0.4f, 0.0f); glVertex3f(rIn * cos(u), -0.9f, rIn * sin(u));
        glTexCoord2f(i * 0.4f, 1.0f); glVertex3f(rOut * cos(u), -0.9f, rOut * sin(u));
    }
    glEnd();

    // Skybox
    glDisable(GL_LIGHTING); glDisable(GL_FOG);
    glBindTexture(GL_TEXTURE_2D, texSky);
    glPushMatrix();
    glRotatef(skyRotation, 0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    float d = 150.0f;
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-d, -20.0f, -d); glTexCoord2f(1.0f, 0.0f); glVertex3f(d, -20.0f, -d); glTexCoord2f(1.0f, 1.0f); glVertex3f(d, d, -d); glTexCoord2f(0.0f, 1.0f); glVertex3f(-d, d, -d);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(d, -20.0f, d); glTexCoord2f(1.0f, 0.0f); glVertex3f(-d, -20.0f, d); glTexCoord2f(1.0f, 1.0f); glVertex3f(-d, d, d); glTexCoord2f(0.0f, 1.0f); glVertex3f(d, d, d);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-d, -20.0f, d); glTexCoord2f(1.0f, 0.0f); glVertex3f(-d, -20.0f, -d); glTexCoord2f(1.0f, 1.0f); glVertex3f(-d, d, -d); glTexCoord2f(0.0f, 1.0f); glVertex3f(-d, d, d);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(d, -20.0f, -d); glTexCoord2f(1.0f, 0.0f); glVertex3f(d, -20.0f, d); glTexCoord2f(1.0f, 1.0f); glVertex3f(d, d, d); glTexCoord2f(0.0f, 1.0f); glVertex3f(d, d, -d);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-d, d, -d); glTexCoord2f(1.0f, 0.0f); glVertex3f(d, d, -d); glTexCoord2f(1.0f, 1.0f); glVertex3f(d, d, d); glTexCoord2f(0.0f, 1.0f); glVertex3f(-d, d, d);
    glEnd();
    glPopMatrix();
    glEnable(GL_LIGHTING); glEnable(GL_FOG);
}

void drawObjects() {
    // Piramida Luvru
    if (!isShadowPass) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texLouvre);
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    else {
        glDisable(GL_TEXTURE_2D);
    }

    glPushMatrix();
    glTranslatef(0.0f, -1.0f, 0.0f);
    glBegin(GL_TRIANGLES);
    float baza = 15.0f, inaltime = 10.0f;
    glNormal3f(0.0f, baza, inaltime); glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, inaltime, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-baza, 0.0f, baza); glTexCoord2f(1.0f, 0.0f); glVertex3f(baza, 0.0f, baza);
    glNormal3f(inaltime, baza, 0.0f); glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, inaltime, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(baza, 0.0f, baza); glTexCoord2f(1.0f, 0.0f); glVertex3f(baza, 0.0f, -baza);
    glNormal3f(0.0f, baza, -inaltime); glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, inaltime, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(baza, 0.0f, -baza); glTexCoord2f(1.0f, 0.0f); glVertex3f(-baza, 0.0f, -baza);
    glNormal3f(-inaltime, baza, 0.0f); glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, inaltime, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-baza, 0.0f, -baza); glTexCoord2f(1.0f, 0.0f); glVertex3f(-baza, 0.0f, baza);
    glEnd();
    glPopMatrix();

    // Clădiri și Copaci (Puse pe sol la y = -1.0f)
    for (int i = 0; i < 12; i++) {
        float u = 2.0f * 3.14159f * float(i) / 12.0f;
        glPushMatrix();
        glTranslatef(45.0f * cos(u), -1.0f, 45.0f * sin(u));
        if (i % 2 == 0) drawBuilding(4.0f, 12.0f + (i % 3) * 3.0f, 3.0f, texBuilding);
        else drawCrossedQuadTree(6.0f, 3.0f);
        glPopMatrix();
    }

    // Felinare și Bănci (De asemenea puse pe sol la y = -1.0f)
    glDisable(GL_TEXTURE_2D);
    for (int i = 0; i < 20; i++) {
        float u = 2.0f * 3.14159f * float(i) / 20.0f;
        glPushMatrix();
        glTranslatef(28.0f * cos(u), -1.0f, 28.0f * sin(u));
        glRotatef(-u * 180.0f / 3.14159f - 90.0f, 0.0f, 1.0f, 0.0f);

        if (i % 2 == 0) drawStreetLamp(4.5f);
        else if (i % 4 == 1) drawBench();
        glPopMatrix();
    }
}

// ==========================================
// --- FUNCȚIA DE DISPLAY PRINCIPALĂ ---
// ==========================================
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Calcul cameră free-look
    float radYaw = camYaw * 3.14159f / 180.0f;
    float radPitch = camPitch * 3.14159f / 180.0f;
    float lookX = camX + sin(radYaw) * cos(radPitch);
    float lookY = camY + sin(radPitch);
    float lookZ = camZ - cos(radYaw) * cos(radPitch);

    gluLookAt(camX, camY, camZ, lookX, lookY, lookZ, 0.0, 1.0, 0.0);

    // Setăm poziția luminii DUPĂ ce am mișcat camera, pentru a fi fixă în lume
    GLfloat light_position[] = { 30.0f, 50.0f, 30.0f, 0.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    GLfloat mat_specular[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat mat_shininess[] = { 8.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    // 1. Randăm mediul și obiectele normal
    isShadowPass = false;
    drawEnvironment();
    drawObjects();

    // 2. Randăm Umbrele
    isShadowPass = true;

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Setăm culoarea umbrei (cu forța, peste tot)
    glColor4f(0.05f, 0.05f, 0.05f, 0.5f);

    // Planul podelei: 1.0f la final înseamnă matematic y = -1.0f
    float groundPlane[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    float shadowMatrix[16];

    // Umbra de la Soare
    buildShadowMatrix(shadowMatrix, light_position, groundPlane);
    glPushMatrix();
    glTranslatef(0.0f, 0.02f, 0.0f); // Ridicăm umbra extrem de puțin ca să nu se bată cu asfaltul
    glMultMatrixf(shadowMatrix);
    drawObjects();
    glPopMatrix();

    // Umbre Locale (Felinare)
    float lamp1Pos[] = { 28.0f, 4.0f, 0.0f, 1.0f };
    float lamp2Pos[] = { -28.0f, 4.0f, 0.0f, 1.0f };

    buildShadowMatrix(shadowMatrix, lamp1Pos, groundPlane);
    glPushMatrix(); glTranslatef(0.0f, 0.02f, 0.0f); glMultMatrixf(shadowMatrix); drawObjects(); glPopMatrix();

    buildShadowMatrix(shadowMatrix, lamp2Pos, groundPlane);
    glPushMatrix(); glTranslatef(0.0f, 0.02f, 0.0f); glMultMatrixf(shadowMatrix); drawObjects(); glPopMatrix();

    // Resetăm totul
    isShadowPass = false;
    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);

    glutSwapBuffers();
}

// ==========================================
// --- INITIALIZARE & CONTROL ---
// ==========================================
void init() {
    glClearColor(0.5f, 0.7f, 0.9f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    GLfloat light_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat light_diffuse[] = { 0.8f, 0.8f, 0.75f, 1.0f };
    GLfloat light_specular[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glEnable(GL_LIGHT0);

    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_EXP2);
    GLfloat fogColor[4] = { 0.7f, 0.8f, 0.9f, 1.0f };
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_DENSITY, 0.002f);
    glHint(GL_FOG_HINT, GL_NICEST);

    texGround = loadTexture("pavaj.jpg");
    texSky = loadTexture("cer_paris.jpg");
    texLouvre = loadTexture("luvru_sticla.jpg");
    texAsphalt = loadTexture("asfalt.jpg");
    texBuilding = loadTexture("cladire_paris.jpg");
    texTree = loadTexture("pom.jpg");
}

void idleFunc() {
    skyRotation += 0.005f;
    if (skyRotation > 360.0f) skyRotation -= 360.0f;
    glutPostRedisplay();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    float aspect = (float)w / (float)h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.0f, aspect, 0.1f, 300.0f);
    glMatrixMode(GL_MODELVIEW);
}

void specialKeys(int key, int x, int y) {
    if (key == GLUT_KEY_RIGHT) camYaw -= 3.0f;
    if (key == GLUT_KEY_LEFT) camYaw += 3.0f;
    if (key == GLUT_KEY_UP) camPitch += 3.0f;
    if (key == GLUT_KEY_DOWN) camPitch -= 3.0f;
    if (camPitch > 80.0f) camPitch = 80.0f;
    if (camPitch < -80.0f) camPitch = -80.0f;
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    float speed = 1.0f;
    float radYaw = camYaw * 3.14159f / 180.0f;

    if (key == 'w' || key == 'W') { camX += sin(radYaw) * speed; camZ -= cos(radYaw) * speed; }
    if (key == 's' || key == 'S') { camX -= sin(radYaw) * speed; camZ += cos(radYaw) * speed; }
    if (key == 'a' || key == 'A') { camX -= cos(radYaw) * speed; camZ -= sin(radYaw) * speed; }
    if (key == 'd' || key == 'D') { camX += cos(radYaw) * speed; camZ += sin(radYaw) * speed; }
    if (key == 'q' || key == 'Q') { camY -= speed; }
    if (key == 'e' || key == 'E') { camY += speed; }

    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("Circuitul Luvru - P3 Finalizat");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(specialKeys);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idleFunc);

    glutMainLoop();
    return 0;
}