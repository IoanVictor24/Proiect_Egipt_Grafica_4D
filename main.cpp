#include "C:\Users\Administrator\Documents\libraries\freeglut\include\GL\freeglut.h"
#include <math.h>
#include <vector>


#include "stb_image.h"


#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
// ID-urile texturilor (Redenumite curat)
GLuint texGround, texSky, texLouvre, texAsphalt, texBuilding, texTree;

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

    // Forțăm 4 canale (RGBA) ca să avem unde să punem transparența, chiar dacă imaginea originală nu are
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 4);

    if (data) {
        // --- MAGIA CHROMA KEY (ELIMINĂM FUNDALUL ALB) ---
        // Ne plimbăm prin fiecare pixel al imaginii (din 4 în 4 bytes: R, G, B, Alpha)
        int totalPixels = width * height;
        for (int i = 0; i < totalPixels * 4; i += 4) {
            unsigned char r = data[i];     // Red
            unsigned char g = data[i + 1]; // Green
            unsigned char b = data[i + 2]; // Blue

            // Dacă pixelul este alb (sau un alb-murdar spre gri foarte deschis)
            // Pragul de 240 îl poți modifica. 255 e alb perfect.
            if (r > 240 && g > 240 && b > 240) {
                data[i + 3] = 0; // Setăm Alpha la 0 (invizibil)
            }
            else {
                data[i + 3] = 255; // Orice altceva e opac
            }
        }
        // --------------------------------------------------

        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
    else {
        printf("Eroare la incarcarea texturii: %s\n", path);
    }
    stbi_image_free(data);
    return textureID;
}

float angle = 0.0f;
float angleUpDown = 0.0f;
float skyRotation = 0.0f;

// --- GEOMETRIE AJUTĂTOARE (Modificată cu Acoperiș) ---

// Desenează un bloc parizian cu acoperiș piramidal
void drawBlock(float w, float h, float d, GLuint texture) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glColor3f(1.0f, 1.0f, 1.0f); // Resetăm culoarea pentru fațadă

    // --- CORPUL CLĂDIRII (Cei 4 pereți) ---
    glBegin(GL_QUADS);
    // Fața
    glNormal3f(0.0f, 0.0f, 1.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, 0.0f, d); glTexCoord2f(1.0f, 0.0f); glVertex3f(w, 0.0f, d); glTexCoord2f(1.0f, 1.0f); glVertex3f(w, h, d); glTexCoord2f(0.0f, 1.0f); glVertex3f(-w, h, d);
    // Spate
    glNormal3f(0.0f, 0.0f, -1.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(w, 0.0f, -d); glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, 0.0f, -d); glTexCoord2f(0.0f, 1.0f); glVertex3f(-w, h, -d); glTexCoord2f(1.0f, 1.0f); glVertex3f(w, h, -d);
    // Stânga
    glNormal3f(-1.0f, 0.0f, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, 0.0f, -d); glTexCoord2f(1.0f, 0.0f); glVertex3f(-w, 0.0f, d); glTexCoord2f(1.0f, 1.0f); glVertex3f(-w, h, d); glTexCoord2f(0.0f, 1.0f); glVertex3f(-w, h, -d);
    // Dreapta
    glNormal3f(1.0f, 0.0f, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(w, 0.0f, d); glTexCoord2f(1.0f, 0.0f); glVertex3f(w, 0.0f, -d); glTexCoord2f(1.0f, 1.0f); glVertex3f(w, h, -d); glTexCoord2f(0.0f, 1.0f); glVertex3f(w, h, d);
    glEnd();

    // ==========================================
    // --- ADAUGĂM ACOPERIȘUL PIRAMIDAL ---
    // ==========================================

    // Dezactivăm textura pentru acoperiș ca să-l colorăm solid
    glBindTexture(GL_TEXTURE_2D, 0);

    // Culoare gri-albăstrui clasic pentru acoperișurile din Paris
    glColor3f(0.3f, 0.35f, 0.4f);

    float rh = 3.0f; // Înălțimea acoperișului (cât de ascuțit e)

    // 1. "Capacul" plat (baza acoperișului, ca să nu vedem în interior)
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f); // Normala în sus
    glVertex3f(-w, h, d);
    glVertex3f(w, h, d);
    glVertex3f(w, h, -d);
    glVertex3f(-w, h, -d);
    glEnd();

    // 2. Fețele triunghiulare ale piramidei
    glBegin(GL_TRIANGLES);

    // Vârful piramidei (Apex)
    float apexX = 0.0f;
    float apexY = h + rh;
    float apexZ = 0.0f;

    // Fața (Triunghi) - Calculăm o normală aproximativă
    glNormal3f(0.0f, 0.5f, 1.0f);
    glVertex3f(apexX, apexY, apexZ); glVertex3f(-w, h, d); glVertex3f(w, h, d);

    // Spate
    glNormal3f(0.0f, 0.5f, -1.0f);
    glVertex3f(apexX, apexY, apexZ); glVertex3f(w, h, -d); glVertex3f(-w, h, -d);

    // Stânga
    glNormal3f(-1.0f, 0.5f, 0.0f);
    glVertex3f(apexX, apexY, apexZ); glVertex3f(-w, h, -d); glVertex3f(-w, h, d);

    // Dreapta
    glNormal3f(1.0f, 0.5f, 0.0f);
    glVertex3f(apexX, apexY, apexZ); glVertex3f(w, h, d); glVertex3f(w, h, -d);

    glEnd();

    // Resetăm culoarea la alb pentru restul scenei
    glColor3f(1.0f, 1.0f, 1.0f);
}

void drawCrossedQuadTree(float height, float width) {
    glBindTexture(GL_TEXTURE_2D, texTree);

    // --- REZOLVARE ERORI & MARGINI (Definiția manuală este crucială!) ---
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glDisable(GL_LIGHTING);

    glEnable(GL_ALPHA_TEST);
    // Un prag mai mare (0.8f) este mai sigur pentru margini curate
    glAlphaFunc(GL_GREATER, 0.8f);

    glBegin(GL_QUADS);
    // Panoul 1
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-width, 0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(width, 0.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(width, height, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-width, height, 0.0f);

    // Panoul 2
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, 0.0f, -width);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f, 0.0f, width);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f, height, width);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f, height, -width);
    glEnd();

    glDisable(GL_ALPHA_TEST);
    glEnable(GL_LIGHTING);

    // Revenim la modul de repetare implicit pentru a nu strica asfaltul
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}





// --- GEOMETRIE NOUĂ: FELINAR PARIZIAN ---
void drawStreetLamp(float height) {
    // Dezactivăm texturile pentru forme colorate solid
    glBindTexture(GL_TEXTURE_2D, 0);

    // --- STÂLPUL (Fier forjat negru) ---
    glColor3f(0.1f, 0.1f, 0.1f); // Aproape negru

    // O bază mai groasă
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);
    drawBlock(0.3f, 1.0f, 0.3f, 0);
    glPopMatrix();

    // Corpul stâlpului (subțire și înalt)
    glPushMatrix();
    glTranslatef(0.0f, 1.0f, 0.0f);
    drawBlock(0.1f, height - 2.0f, 0.1f, 0);
    glPopMatrix();

    // --- CAPUL FELINARULUI (Cutia de sticlă) ---
    // Cadrul de fier (baza cutiei)
    glPushMatrix();
    glTranslatef(0.0f, height - 1.0f, 0.0f);
    drawBlock(0.4f, 0.1f, 0.4f, 0);
    glPopMatrix();

    // --- BECUL/LUMINA (Gălbui, Strălucitor) ---
    // Îl facem să strălucească dezactivând lumina globală pentru el
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 0.9f, 0.4f); // Galben cald, puternic

    glPushMatrix();
    glTranslatef(0.0f, height - 0.7f, 0.0f);
    drawBlock(0.2f, 0.5f, 0.2f, 0); // Becul propriu-zis
    glPopMatrix();

    glEnable(GL_LIGHTING); // Reactivăm lumina pentru restul

    // --- ACOPERIȘUL FELINARULUI (O mică piramidă neagră) ---
    glColor3f(0.1f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, height - 0.2f, 0.0f);

    glBegin(GL_TRIANGLES);
    float bw = 0.45f; // Lățime bază acoperiș
    float ah = 0.6f;  // Înălțime acoperiș
    // Cele 4 fețe ale piramidei
    glNormal3f(0.0f, 0.5f, 1.0f); glVertex3f(0.0f, ah, 0.0f); glVertex3f(-bw, 0.0f, bw); glVertex3f(bw, 0.0f, bw);
    glNormal3f(0.0f, 0.5f, -1.0f); glVertex3f(0.0f, ah, 0.0f); glVertex3f(bw, 0.0f, -bw); glVertex3f(-bw, 0.0f, -bw);
    glNormal3f(-1.0f, 0.5f, 0.0f); glVertex3f(0.0f, ah, 0.0f); glVertex3f(-bw, 0.0f, -bw); glVertex3f(-bw, 0.0f, bw);
    glNormal3f(1.0f, 0.5f, 0.0f); glVertex3f(0.0f, ah, 0.0f); glVertex3f(bw, 0.0f, bw); glVertex3f(bw, 0.0f, -bw);
    glEnd();
    glPopMatrix();

    glColor3f(1.0f, 1.0f, 1.0f); // Resetăm culoarea la alb
}

// --- GEOMETRIE NOUĂ: BĂNCUȚĂ PARIZIANĂ ---
void drawBench() {
    // Dezactivăm texturile
    glBindTexture(GL_TEXTURE_2D, 0);

    // Dimensiuni băncuță
    float bw = 2.5f; // Jumătate din lățimea totală a băncii (5 unități total)
    float bh = 0.6f; // Înălțimea șezutului
    float bd = 0.6f; // Adâncimea șezutului
    float bwh = 0.9f; // Înălțimea spătarului (de la șezut în sus)

    // --- PICIOARELE (Fier negru curbat, simplificat la cuburi) ---
    glColor3f(0.05f, 0.05f, 0.05f); // Negru intens

    // Picior Stânga
    glPushMatrix();
    glTranslatef(-bw + 0.2f, -1.0f, 0.0f); // -1.0f pentru a sta pe pavaj
    drawBlock(0.1f, 0.8f, bd, 0);
    glPopMatrix();

    // Picior Dreapta
    glPushMatrix();
    glTranslatef(bw - 0.2f, -1.0f, 0.0f);
    drawBlock(0.1f, 0.8f, bd, 0);
    glPopMatrix();

    // --- ȘEZUTUL (Lemn maro) ---
    glColor3f(0.4f, 0.25f, 0.1f); // Maro lemn

    glPushMatrix();
    glTranslatef(0.0f, bh - 1.0f, 0.0f);
    drawBlock(bw, 0.1f, bd, 0); // Scândura principală
    glPopMatrix();

    // --- SPĂTARUL (Maro lemn, înclinat ușor) ---
    glPushMatrix();
    glTranslatef(0.0f, bh + bwh - 1.0f, -bd); // Poziționăm în spate și sus
    glRotatef(15.0f, 1.0f, 0.0f, 0.0f); // Înclinăm spătarul cu 15 grade
    drawBlock(bw, 0.3f, 0.05f, 0);
    glPopMatrix();

    glColor3f(1.0f, 1.0f, 1.0f); // Resetăm culoarea
}




void drawScene() {
    // Setări materiale generale
    GLfloat mat_specular[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat mat_shininess[] = { 8.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    // ==========================================
    // 1. TERENUL PIEȚEI LUVRE (Pavele plate)
    // ==========================================
    glEnable(GL_LIGHTING);
    glBindTexture(GL_TEXTURE_2D, texGround);
    glColor3f(1.0f, 1.0f, 1.0f);

    glNormal3f(0.0f, 1.0f, 0.0f); // Teren plat, lumina cade direct
    glBegin(GL_QUADS);
    float size = 100.0f; // Piață uriașă
    float rep = 30.0f; // Repetăm pavelele
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, -1.0f, -size);
    glTexCoord2f(rep, 0.0f);  glVertex3f(size, -1.0f, -size);
    glTexCoord2f(rep, rep);   glVertex3f(size, -1.0f, size);
    glTexCoord2f(0.0f, rep);  glVertex3f(-size, -1.0f, size);
    glEnd();

    // ==========================================
    // 2. PIRAMIDA LUVRE (Din Sticlă)
    // ==========================================
    glBindTexture(GL_TEXTURE_2D, texLouvre);

    // Facem piramida să strălucească (Specular mare)
    GLfloat louvre_spec[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat louvre_shin[] = { 50.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, louvre_spec);
    glMaterialfv(GL_FRONT, GL_SHININESS, louvre_shin);

    glPushMatrix();
    glTranslatef(0.0f, -1.0f, 0.0f); // Pe centrul pieței

    glBegin(GL_TRIANGLES);
    // Dimensiunile Luvrului (lată și scundă, nu ca cea din Egipt)
    float baza = 15.0f;
    float inaltime = 10.0f;

    // Fața din față
    glNormal3f(0.0f, baza, inaltime); glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, inaltime, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-baza, 0.0f, baza); glTexCoord2f(1.0f, 0.0f); glVertex3f(baza, 0.0f, baza);
    // Fața din dreapta
    glNormal3f(inaltime, baza, 0.0f); glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, inaltime, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(baza, 0.0f, baza); glTexCoord2f(1.0f, 0.0f); glVertex3f(baza, 0.0f, -baza);
    // Fața din spate
    glNormal3f(0.0f, baza, -inaltime); glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, inaltime, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(baza, 0.0f, -baza); glTexCoord2f(1.0f, 0.0f); glVertex3f(-baza, 0.0f, -baza);
    // Fața din stânga
    glNormal3f(-inaltime, baza, 0.0f); glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0f, inaltime, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-baza, 0.0f, -baza); glTexCoord2f(1.0f, 0.0f); glVertex3f(-baza, 0.0f, baza);
    glEnd();
    glPopMatrix();

    // Resetăm materialele
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    // ==========================================
    // P2: CIRCUIT STRADAL (Asfalt)
    // ==========================================
    glBindTexture(GL_TEXTURE_2D, texAsphalt);

    glBegin(GL_QUAD_STRIP); // Inel închis
    float rIn = 30.0f; // Raza interioară
    float rOut = 38.0f; // Raza exterioară
    int seg = 50;

    // MAGIA E AICI: Controlăm de câte ori se repetă poza cu asfaltul
    float repetare_lungime = 3.0f; // De câte ori se repetă pe un segment de curbă
    float repetare_latime = 4.0f;  // De câte ori se repetă pe lățimea drumului (înainte era doar 1.0f)

    for (int i = 0; i <= seg; i++) {
        float u = 2.0f * 3.14159f * float(i) / float(seg);
        glNormal3f(0.0f, 1.0f, 0.0f);

        // Marginea interioară
        glTexCoord2f(i * repetare_lungime, 0.0f);
        glVertex3f(rIn * cos(u), -0.9f, rIn * sin(u));

        // Marginea exterioară
        glTexCoord2f(i * repetare_lungime, repetare_latime); // AICI aplicăm repetarea pe lățime!
        glVertex3f(rOut * cos(u), -0.9f, rOut * sin(u));
    }
    glEnd();

    // ==========================================
    // P2: OBIECTE STATICE (Clădiri și Pomi)
    // ==========================================

    // Raza pe care așezăm obiectele
    float rObj = 45.0f;
    int nrObj = 12; // 6 clădiri și 6 pomi alternând

    for (int i = 0; i < nrObj; i++) {
        float u = 2.0f * 3.14159f * float(i) / float(nrObj);
        float x = rObj * cos(u);
        float z = rObj * sin(u);

        glPushMatrix();
        glTranslatef(x, -1.0f, z);

        if (i % 2 == 0) {
            // Desenăm o CLĂDIRE pariziană
            // x, y, z, latime, inaltime, adancime
            drawBlock(4.0f, 12.0f + (i % 3) * 3.0f, 3.0f, texBuilding);
        }
        else {
            // Desenăm un POM încrucișat
            drawCrossedQuadTree(6.0f, 3.0f);
        }
        glPopMatrix();
    }


    float rObjOut = 45.0f; // Raza pe care așezăm obiectele exterioare
    int nrObjOut = 12;

    for (int i = 0; i < nrObjOut; i++) {
        float u = 2.0f * 3.14159f * float(i) / float(nrObjOut);
        float x = rObjOut * cos(u);
        float z = rObjOut * sin(u);

        glPushMatrix();
        glTranslatef(x, -1.0f, z);

        if (i % 2 == 0) {
            // Desenăm o CLĂDIRE pariziană
            // x, y, z, latime, inaltime, adancime, textură
            drawBlock(4.0f, 12.0f + (i % 3) * 3.0f, 3.0f, texBuilding);
        }
        else {
            // Desenăm un POM încrucișat
            // height, width
            drawCrossedQuadTree(6.0f, 3.0f);
        }
        glPopMatrix();
    }

    // --- INELUL INTERIOR (Felinare și Băncuțe, culori solide) ---
    float rObjIn = 28.0f; // Raza interioară
    int nrObjIn = 20;     // Numar obiecte

    // OPRIM TEXTURILE COMPLET ca să putem folosi culori simple!
    glDisable(GL_TEXTURE_2D);

    for (int i = 0; i < nrObjIn; i++) {
        float u = 2.0f * 3.14159f * float(i) / float(nrObjIn);
        float x = rObjIn * cos(u);
        float z = rObjIn * sin(u);

        // Unghiul de rotație ca obiectul să privească spre centru
        float rotație = -u * 180.0f / 3.14159f - 90.0f;

        glPushMatrix();
        glTranslatef(x, 0.0f, z);
        glRotatef(rotație, 0.0f, 1.0f, 0.0f);

        if (i % 2 == 0) {
            // Desenăm FELINAR
            glColor3f(0.1f, 0.1f, 0.1f); // Fier negru

            glPushMatrix();
            drawBlock(0.1f, 4.0f, 0.1f, 0); // Stâlp

            glTranslatef(0.0f, 4.0f, 0.0f);
            glColor3f(1.0f, 0.9f, 0.4f); // Galben cald (Lumina)

            glBegin(GL_TRIANGLES); // Acoperiș piramidal felinar
            float bw = 0.3f; float ah = 0.5f;
            glNormal3f(0.0f, 0.5f, 1.0f); glVertex3f(0.0f, ah, 0.0f); glVertex3f(-bw, 0.0f, bw); glVertex3f(bw, 0.0f, bw);
            glNormal3f(0.0f, 0.5f, -1.0f); glVertex3f(0.0f, ah, 0.0f); glVertex3f(bw, 0.0f, -bw); glVertex3f(-bw, 0.0f, -bw);
            glNormal3f(-1.0f, 0.5f, 0.0f); glVertex3f(0.0f, ah, 0.0f); glVertex3f(-bw, 0.0f, -bw); glVertex3f(-bw, 0.0f, bw);
            glNormal3f(1.0f, 0.5f, 0.0f); glVertex3f(0.0f, ah, 0.0f); glVertex3f(bw, 0.0f, bw); glVertex3f(bw, 0.0f, -bw);
            glEnd();
            glPopMatrix();
        }
        else {
            // Desenăm BĂNCUȚĂ
            glPushMatrix();
            glTranslatef(0.0f, -1.0f, 0.0f); // Lăsăm banca pe pavaj

            glColor3f(0.4f, 0.25f, 0.1f); // Maro lemn
            drawBlock(2.0f, 0.1f, 0.4f, 0); // Șezut

            glTranslatef(0.0f, 0.8f, -0.4f); // Poziționăm spătarul
            glRotatef(15.0f, 1.0f, 0.0f, 0.0f); // Înclinăm spătarul
            drawBlock(2.0f, 0.15f, 0.05f, 0); // Spătar
            glPopMatrix();
        }
        glPopMatrix();
    }

    // PORNIM TEXTURILE LA LOC pentru restul scenei (cer, etc.)
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f); // Resetăm culoarea globală la alb

    // ==========================================
    // 3. SKYBOX (Cerul Parisului)
    // ==========================================
    glDisable(GL_LIGHTING);
    glBindTexture(GL_TEXTURE_2D, texSky);
    glColor3f(1.0f, 1.0f, 1.0f);

    glPushMatrix();
    glRotatef(skyRotation, 0.0f, 1.0f, 0.0f);

    glBegin(GL_QUADS);
    float d = 90.0f; // Skybox mare
    // Pereții (Fata, Spate, Stanga, Dreapta)
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-d, -20.0f, -d); glTexCoord2f(1.0f, 0.0f); glVertex3f(d, -20.0f, -d); glTexCoord2f(1.0f, 1.0f); glVertex3f(d, d, -d); glTexCoord2f(0.0f, 1.0f); glVertex3f(-d, d, -d);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(d, -20.0f, d); glTexCoord2f(1.0f, 0.0f); glVertex3f(-d, -20.0f, d); glTexCoord2f(1.0f, 1.0f); glVertex3f(-d, d, d); glTexCoord2f(0.0f, 1.0f); glVertex3f(d, d, d);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-d, -20.0f, d); glTexCoord2f(1.0f, 0.0f); glVertex3f(-d, -20.0f, -d); glTexCoord2f(1.0f, 1.0f); glVertex3f(-d, d, -d); glTexCoord2f(0.0f, 1.0f); glVertex3f(-d, d, d);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(d, -20.0f, -d); glTexCoord2f(1.0f, 0.0f); glVertex3f(d, -20.0f, d); glTexCoord2f(1.0f, 1.0f); glVertex3f(d, d, d); glTexCoord2f(0.0f, 1.0f); glVertex3f(d, d, -d);
    // Tavan
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-d, d, -d); glTexCoord2f(1.0f, 0.0f); glVertex3f(d, d, -d); glTexCoord2f(1.0f, 1.0f); glVertex3f(d, d, d); glTexCoord2f(0.0f, 1.0f); glVertex3f(-d, d, d);
    glEnd();
    glPopMatrix();
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    // --- CONFIGURARE LUMINĂ (Modificată pentru contrast) ---
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Am redus ambientul ca umbrele texturilor să rămână închise
    GLfloat light_ambient[] = { 0.15f, 0.15f, 0.15f, 1.0f };
    // Am redus ușor difuzia ca să nu "ardem" texturile
    GLfloat light_diffuse[] = { 0.8f, 0.8f, 0.75f, 1.0f };
    GLfloat light_specular[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat light_position[] = { 30.0f, 50.0f, 30.0f, 0.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glEnable(GL_LIGHT0);

    // --- CEAȚĂ ATMOSFERICĂ (Mult mai rară) ---
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_EXP2);
    GLfloat fogColor[4] = { 0.7f, 0.8f, 0.9f, 1.0f };
    glFogfv(GL_FOG_COLOR, fogColor);

    // REDUCEM densitatea de 5 ori! (de la 0.01 la 0.002)
    glFogf(GL_FOG_DENSITY, 0.002f);
    glHint(GL_FOG_HINT, GL_NICEST);

    // ... aici rămâne codul tău cu loadTexture ...


    // --- ÎNCĂRCARE TEXTURI (Pune căile TALE complete aici!) ---
    texGround = loadTexture("pavaj.jpg");
    texSky = loadTexture("cer_paris.jpg");
    texLouvre = loadTexture("luvru_sticla.jpg");
    texAsphalt = loadTexture("asfalt.jpg");
    texBuilding = loadTexture("cladire_paris.jpg");
    texTree = loadTexture("pom.jpg"); // FOARTE IMPORTANT: PNG cu transparenta!
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Cameră la înălțime medie pentru a vedea circuitul
    gluLookAt(0.0, 10.0, 50.0,
        0.0, 2.0, 0.0,
        0.0, 1.0, 0.0);

    glRotatef(angleUpDown, 1.0f, 0.0f, 0.0f);
    glRotatef(angle, 0.0f, 1.0f, 0.0f);

    drawScene();

    glutSwapBuffers();
}
 

// comment
void idleFunc() {
    skyRotation += 0.01f; // Rotație cinematică foarte lentă
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
    if (key == GLUT_KEY_RIGHT) angle += 5.0f;
    if (key == GLUT_KEY_LEFT) angle -= 5.0f;
    if (key == GLUT_KEY_UP) angleUpDown -= 5.0f;
    if (key == GLUT_KEY_DOWN) angleUpDown += 5.0f;
    if (angleUpDown > 80.0f) angleUpDown = 80.0f;
    if (angleUpDown < -80.0f) angleUpDown = -80.0f;
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("Circuitul Luvru - Task P2");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(specialKeys);
    glutIdleFunc(idleFunc);

    glutMainLoop();
    return 0;
}