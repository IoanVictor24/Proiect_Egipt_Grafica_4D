#include "C:\Users\Administrator\Documents\libraries\freeglut\include\GL\freeglut.h"
#include <math.h>
#include <stdio.h>
#include "stb_image.h"

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

// --- ID-uri Texturi ---
GLuint texGround, texSky, texLouvre, texAsphalt, texBuilding, texTree;

// --- Variabile Cameră și Animație ---
float camX = 0.0f, camY = 3.0f, camZ = 50.0f;
float camYaw = 0.0f, camPitch = 0.0f;
float skyRotation = 0.0f;
float carAngle = 0.0f;

// --- Variabile NOI pentru Soare Dinamic ---
float sunAngle = 0.0f;
float sunPos[4] = { 0.0f, 60.0f, 0.0f, 1.0f }; // Soarele e o lumină punctuală pe cer

bool isShadowPass = false;

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

void drawBlock(float w, float h, float d) {
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f); glVertex3f(-w, 0.0f, d); glVertex3f(w, 0.0f, d); glVertex3f(w, h, d); glVertex3f(-w, h, d);
    glNormal3f(0.0f, 0.0f, -1.0f); glVertex3f(w, 0.0f, -d); glVertex3f(-w, 0.0f, -d); glVertex3f(-w, h, -d); glVertex3f(w, h, -d);
    glNormal3f(-1.0f, 0.0f, 0.0f); glVertex3f(-w, 0.0f, -d); glVertex3f(-w, 0.0f, d); glVertex3f(-w, h, d); glVertex3f(-w, h, -d);
    glNormal3f(1.0f, 0.0f, 0.0f); glVertex3f(w, 0.0f, d); glVertex3f(w, 0.0f, -d); glVertex3f(w, h, -d); glVertex3f(w, h, d);
    glNormal3f(0.0f, 1.0f, 0.0f); glVertex3f(-w, h, d); glVertex3f(w, h, d); glVertex3f(w, h, -d); glVertex3f(-w, h, -d);
    glEnd();
}

void drawBuilding(float w, float h, float d, GLuint texture) {
    if (!isShadowPass) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    else {
        glDisable(GL_TEXTURE_2D);
        glColor4f(0.0f, 0.0f, 0.0f, 1.0f); // UMBRĂ COMPLET NEAGRĂ
    }

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, 0.0f, d); glTexCoord2f(1.0f, 0.0f); glVertex3f(w, 0.0f, d); glTexCoord2f(1.0f, 1.0f); glVertex3f(w, h, d); glTexCoord2f(0.0f, 1.0f); glVertex3f(-w, h, d);
    glNormal3f(0.0f, 0.0f, -1.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(w, 0.0f, -d); glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, 0.0f, -d); glTexCoord2f(0.0f, 1.0f); glVertex3f(-w, h, -d); glTexCoord2f(1.0f, 1.0f); glVertex3f(w, h, -d);
    glNormal3f(-1.0f, 0.0f, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, 0.0f, -d); glTexCoord2f(1.0f, 0.0f); glVertex3f(-w, 0.0f, d); glTexCoord2f(1.0f, 1.0f); glVertex3f(-w, h, d); glTexCoord2f(0.0f, 1.0f); glVertex3f(-w, h, -d);
    glNormal3f(1.0f, 0.0f, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(w, 0.0f, d); glTexCoord2f(1.0f, 0.0f); glVertex3f(w, 0.0f, -d); glTexCoord2f(1.0f, 1.0f); glVertex3f(w, h, -d); glTexCoord2f(0.0f, 1.0f); glVertex3f(w, h, d);
    glEnd();

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

    if (isShadowPass) {
        glColor4f(0.0f, 0.0f, 0.0f, 1.0f); // UMBRĂ COMPLET NEAGRĂ
        glAlphaFunc(GL_GREATER, 0.5f);
    }
    else {
        glColor3f(1.0f, 1.0f, 1.0f);
        glAlphaFunc(GL_GREATER, 0.8f);
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
    else glColor4f(0.0f, 0.0f, 0.0f, 1.0f); // UMBRĂ COMPLET NEAGRĂ

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
    else {
        glPushMatrix(); glTranslatef(0.0f, height - 0.7f, 0.0f); drawBlock(0.2f, 0.5f, 0.2f); glPopMatrix();
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
    else glColor4f(0.0f, 0.0f, 0.0f, 1.0f); // UMBRĂ COMPLET NEAGRĂ

    glPushMatrix(); glTranslatef(-bw + 0.2f, 0.0f, 0.0f); drawBlock(0.1f, 0.8f, bd); glPopMatrix();
    glPushMatrix(); glTranslatef(bw - 0.2f, 0.0f, 0.0f); drawBlock(0.1f, 0.8f, bd); glPopMatrix();

    if (!isShadowPass) glColor3f(0.4f, 0.25f, 0.1f);
    glPushMatrix(); glTranslatef(0.0f, bh, 0.0f); drawBlock(bw, 0.1f, bd); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, bh + bwh, -bd); glRotatef(15.0f, 1.0f, 0.0f, 0.0f); drawBlock(bw, 0.3f, 0.05f); glPopMatrix();
}

void drawCar() {
    glDisable(GL_TEXTURE_2D);

    if (isShadowPass) {
        glColor4f(0.0f, 0.0f, 0.0f, 1.0f); // UMBRĂ COMPLET NEAGRĂ MAȘINĂ
        glPushMatrix(); glTranslatef(0.0f, 0.4f, 0.0f); drawBlock(1.5f, 0.6f, 0.8f); glPopMatrix();
        glPushMatrix(); glTranslatef(0.0f, 1.0f, 0.0f); drawBlock(0.8f, 0.5f, 0.7f); glPopMatrix();

        float wR = 0.4f, wW = 0.2f;
        glPushMatrix(); glTranslatef(1.0f, 0.4f, 0.8f); glutSolidCylinder(wR, wW, 20, 1); glPopMatrix();
        glPushMatrix(); glTranslatef(1.0f, 0.4f, -1.0f); glutSolidCylinder(wR, wW, 20, 1); glPopMatrix();
        glPushMatrix(); glTranslatef(-1.0f, 0.4f, 0.8f); glutSolidCylinder(wR, wW, 20, 1); glPopMatrix();
        glPushMatrix(); glTranslatef(-1.0f, 0.4f, -1.0f); glutSolidCylinder(wR, wW, 20, 1); glPopMatrix();
        return;
    }

    glColor3f(0.8f, 0.1f, 0.1f);
    glPushMatrix(); glTranslatef(0.0f, 0.4f, 0.0f); drawBlock(1.5f, 0.6f, 0.8f); glPopMatrix();

    glColor3f(0.2f, 0.2f, 0.2f);
    glPushMatrix(); glTranslatef(0.0f, 1.0f, 0.0f); drawBlock(0.8f, 0.5f, 0.7f); glPopMatrix();

    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 0.8f);
    glPushMatrix(); glTranslatef(1.5f, 0.7f, 0.5f); drawBlock(0.05f, 0.2f, 0.2f); glPopMatrix();
    glPushMatrix(); glTranslatef(1.5f, 0.7f, -0.5f); drawBlock(0.05f, 0.2f, 0.2f); glPopMatrix();
    glEnable(GL_LIGHTING);

    glColor3f(0.05f, 0.05f, 0.05f);
    float wheelRadius = 0.4f, wheelWidth = 0.2f;
    glPushMatrix(); glTranslatef(1.0f, 0.4f, 0.8f); glutSolidCylinder(wheelRadius, wheelWidth, 20, 1); glPopMatrix();
    glPushMatrix(); glTranslatef(1.0f, 0.4f, -1.0f); glutSolidCylinder(wheelRadius, wheelWidth, 20, 1); glPopMatrix();
    glPushMatrix(); glTranslatef(-1.0f, 0.4f, 0.8f); glutSolidCylinder(wheelRadius, wheelWidth, 20, 1); glPopMatrix();
    glPushMatrix(); glTranslatef(-1.0f, 0.4f, -1.0f); glutSolidCylinder(wheelRadius, wheelWidth, 20, 1); glPopMatrix();

    glColor3f(1.0f, 1.0f, 1.0f);
}

// ==========================================
// --- RANDARE SCENĂ ---
// ==========================================

void drawEnvironment() {
    glEnable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);

    // --- SOARELE DINAMIC ---
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 0.9f, 0.0f);
    glTranslatef(sunPos[0], sunPos[1], sunPos[2]);
    glutSolidSphere(5.0, 30, 30);
    glColor3f(1.0f, 1.0f, 1.0f);
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
    glDisable(GL_LIGHTING);
    glBindTexture(GL_TEXTURE_2D, texSky);
    glPushMatrix();
    glRotatef(skyRotation, 0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    float d = 200.0f;
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-d, -20.0f, -d); glTexCoord2f(1.0f, 0.0f); glVertex3f(d, -20.0f, -d); glTexCoord2f(1.0f, 1.0f); glVertex3f(d, d, -d); glTexCoord2f(0.0f, 1.0f); glVertex3f(-d, d, -d);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(d, -20.0f, d); glTexCoord2f(1.0f, 0.0f); glVertex3f(-d, -20.0f, d); glTexCoord2f(1.0f, 1.0f); glVertex3f(-d, d, d); glTexCoord2f(0.0f, 1.0f); glVertex3f(d, d, d);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-d, -20.0f, d); glTexCoord2f(1.0f, 0.0f); glVertex3f(-d, -20.0f, -d); glTexCoord2f(1.0f, 1.0f); glVertex3f(-d, d, -d); glTexCoord2f(0.0f, 1.0f); glVertex3f(-d, d, d);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(d, -20.0f, -d); glTexCoord2f(1.0f, 0.0f); glVertex3f(d, -20.0f, d); glTexCoord2f(1.0f, 1.0f); glVertex3f(d, d, d); glTexCoord2f(0.0f, 1.0f); glVertex3f(d, d, -d);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-d, d, -d); glTexCoord2f(1.0f, 0.0f); glVertex3f(d, d, -d); glTexCoord2f(1.0f, 1.0f); glVertex3f(d, d, d); glTexCoord2f(0.0f, 1.0f); glVertex3f(-d, d, d);
    glEnd();
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawObjects() {
    if (!isShadowPass) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texLouvre);
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    else {
        glDisable(GL_TEXTURE_2D);
        glColor4f(0.0f, 0.0f, 0.0f, 1.0f); // UMBRĂ COMPLET NEAGRĂ
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

    for (int i = 0; i < 12; i++) {
        float u = 2.0f * 3.14159f * float(i) / 12.0f;
        glPushMatrix();
        glTranslatef(45.0f * cos(u), -1.0f, 45.0f * sin(u));
        if (i % 2 == 0) drawBuilding(4.0f, 12.0f + (i % 3) * 3.0f, 3.0f, texBuilding);
        else drawCrossedQuadTree(6.0f, 3.0f);
        glPopMatrix();
    }

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

    glPushMatrix();
    float rCar = 35.0f;
    float radCar = carAngle * 3.14159f / 180.0f;
    glTranslatef(rCar * cos(radCar), -1.0f, rCar * sin(radCar));
    glRotatef(-carAngle - 90.0f, 0.0f, 1.0f, 0.0f);
    drawCar();
    glPopMatrix();

    glEnable(GL_TEXTURE_2D);
}

// ==========================================
// --- FUNCȚIA DE DISPLAY PRINCIPALĂ ---
// ==========================================
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    float radYaw = camYaw * 3.14159f / 180.0f;
    float radPitch = camPitch * 3.14159f / 180.0f;
    float lookX = camX + sin(radYaw) * cos(radPitch);
    float lookY = camY + sin(radPitch);
    float lookZ = camZ - cos(radYaw) * cos(radPitch);

    gluLookAt(camX, camY, camZ, lookX, lookY, lookZ, 0.0, 1.0, 0.0);

    glLightfv(GL_LIGHT0, GL_POSITION, sunPos);

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
    // DEZACTIVĂM BLEND CA SĂ FIE 100% NEGRU OBTURANT
    glDisable(GL_BLEND);

    glDepthMask(GL_FALSE);

    // MODIFICARE CRITICĂ: Planul matematic mutat la Y = -1.0f 
    // ca să prindă perfect și mașina care e pe asfalt
    float groundPlane[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    float shadowMatrix[16];

    // Umbra dinamică de la Soare 
    buildShadowMatrix(shadowMatrix, sunPos, groundPlane);
    glPushMatrix();
    // Ridicăm vizual umbra cu 0.02 ca să nu dea glitch-uri pe asfalt
    glTranslatef(0.0f, 0.02f, 0.0f);
    glMultMatrixf(shadowMatrix);
    drawObjects();
    glPopMatrix();

    // Umbre Locale (Felinare)
    float lamp1Pos[] = { 28.0f, 25.0f, 0.0f, 1.0f };
    float lamp2Pos[] = { -28.0f, 25.0f, 0.0f, 1.0f };

    buildShadowMatrix(shadowMatrix, lamp1Pos, groundPlane);
    glPushMatrix(); glTranslatef(0.0f, 0.02f, 0.0f); glMultMatrixf(shadowMatrix); drawObjects(); glPopMatrix();

    buildShadowMatrix(shadowMatrix, lamp2Pos, groundPlane);
    glPushMatrix(); glTranslatef(0.0f, 0.02f, 0.0f); glMultMatrixf(shadowMatrix); drawObjects(); glPopMatrix();

    // Resetăm totul la final
    glDepthMask(GL_TRUE);
    isShadowPass = false;
    glEnable(GL_LIGHTING);

    glutSwapBuffers();
}

// ==========================================
// --- INITIALIZARE & CONTROL ---
// ==========================================
void init() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    GLfloat light_ambient[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light_specular[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glEnable(GL_LIGHT0);

    glDisable(GL_FOG);

    texGround = loadTexture("pavaj.jpg");
    texSky = loadTexture("cer_paris.jpg");
    texLouvre = loadTexture("luvru_sticla.jpg");
    texAsphalt = loadTexture("asfalt.jpg");
    texBuilding = loadTexture("cladire_paris.jpg");
    texTree = loadTexture("pom.jpg");
}

void timerFunc(int value) {
    skyRotation += 0.05f;
    if (skyRotation > 360.0f) skyRotation -= 360.0f;

    carAngle -= 0.5f;
    if (carAngle < 0.0f) carAngle += 360.0f;

    sunAngle += 0.005f;
    if (sunAngle > 2 * 3.14159f) sunAngle -= 2 * 3.14159f;

    float rSun = 100.0f;
    sunPos[0] = rSun * cos(sunAngle);
    sunPos[1] = 60.0f;
    sunPos[2] = rSun * sin(sunAngle);

    glutPostRedisplay();
    glutTimerFunc(16, timerFunc, 0);
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
    glutCreateWindow("Circuitul Luvru - Proiect Final Complet");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(specialKeys);
    glutKeyboardFunc(keyboard);

    glutTimerFunc(0, timerFunc, 0);

    glutMainLoop();
    return 0;
}