#include <stdio.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "../Include/ImageBase.hpp"

struct Point {
    int x, y;
};

// Fonction pour clamp un valeur
int clamp(int value) {
    return std::max(0, std::min(value, 255));
}

// Fonctions de distances (Position & Couleurs & les 2 (5D))
double distanceSpaciale(Point p1, Point p2) {
    return std::sqrt(((p1.x - p2.x) * (p1.x - p2.x)) + ((p1.y - p2.y) * (p1.y - p2.y)));
}

double distanceSpectral(ImageBase& L, ImageBase& a, ImageBase& b, Point p1, Point p2) {
    int deltaL = L[p2.x][p2.y] - L[p1.x][p1.y];
    int deltaA = a[p2.x][p2.y] - a[p1.x][p1.y];
    int deltaB = b[p2.x][p2.y] - b[p1.x][p1.y];
    return std::sqrt(deltaL * deltaL + deltaA * deltaA + deltaB * deltaB);
}

double calculDistances(ImageBase& L, ImageBase& a, ImageBase& b, Point& centre, Point& pixel, int S, int m) {
    double terme1 = ((distanceSpectral(L, a, b, centre, pixel) / m) * (distanceSpectral(L, a, b, centre, pixel) / m));
    double terme2 = ((distanceSpaciale(centre, pixel) / S) * (distanceSpaciale(centre, pixel) / S));

    return sqrt(terme1 + terme2);
}

// RGB  -->  Lab
double F(double t) {return (t > 0.008856) ? pow(t, 1.0 / 3.0) : (7.787 * t + 16.0 / 116.0);}

void RGBtoLab(int R, int G, int B, ImageBase &L, ImageBase &a, ImageBase &b, int x, int y) {
    double X = clamp(R * 0.412453 + G * 0.357580 + B * 0.180423);
    double Y = clamp(R * 0.212671 + G * 0.715160 + B * 0.072169);
    double Z = clamp(R * 0.019334 + G * 0.119193 + B * 0.950227);
        
    double Xn = 95.047;
    double Yn = 100.000;
    double Zn = 108.883;
        
    double f1 = F(X / Xn);
    double f2 = F(Y / Yn);
    double f3 = F(Z / Zn);

    L[x][y] = static_cast<int>(clamp(116 * f2 - 16));
    a[x][y] = static_cast<int>(clamp(500 * (f1 - f2)) + 128);
    b[x][y] = static_cast<int>(clamp(200 * (f2 - f3)) + 128);
}



int main(int argc, char **argv) {
    char cNomImgLue[250], cNomImgEcrite[250];
    int S, m;
    if (argc != 5) {
        printf("Usage: ImageIn.ppm ImageOut.ppm S \n");
        return 1;
    }
    sscanf(argv[1], "%s", cNomImgLue);
    sscanf(argv[2], "%s", cNomImgEcrite);
    sscanf(argv[3], "%d", &S);
    sscanf(argv[4], "%d", &m);

    ImageBase imIn;
    imIn.load(cNomImgLue);

    int width = imIn.getWidth();
    int height = imIn.getHeight();

    std::vector<Point> clustercentres;
    std::vector<Point> clustercentresNouveau;


    for (int x = S; x < height; x += S) {
        for (int y = S; y < width; y += S) {
            clustercentres.push_back({x, y});
        }
    }

    ImageBase imOut(imIn.getWidth(), imIn.getHeight(), imIn.getColor());
    ImageBase L(imIn.getWidth(), imIn.getHeight(), false);
    ImageBase a(imIn.getWidth(), imIn.getHeight(), false);
    ImageBase b(imIn.getWidth(), imIn.getHeight(), false);


    for (int x = 0; x < height; x++) {
        for (int y = 0; y < width; y++) {
            RGBtoLab(imIn[x*3][y*3], imIn[x*3][y*3+1], imIn[x*3][y*3+2], L, a, b, x, y);
        }
    }

    for (Point& centre : clustercentres) {
        for (int x = std::max(0, centre.x - 2 * S); x < std::min(height, centre.x + 2 * S); x++) {
            for (int y = std::max(0, centre.y - 2 * S); y < std::min(width, centre.y + 2 * S); y++) {
                Point pixel = {x, y};
                double minDist = std::numeric_limits<double>::max();
                for (Point& clustercentre : clustercentres) {
                        // Nouveaux centres
                        clustercentresNouveau.push_back(clustercentre);
                    double dist = calculDistances(L, a, b, clustercentre, pixel, S, m);
                    if (dist < minDist) {
                        minDist = dist;
                        imOut[pixel.x * 3][pixel.y * 3] = imIn[clustercentre.x * 3][clustercentre.y * 3];
                        imOut[pixel.x * 3][pixel.y * 3 + 1] = imIn[clustercentre.x * 3][clustercentre.y * 3 + 1];
                        imOut[pixel.x * 3][pixel.y * 3 + 2] = imIn[clustercentre.x * 3][clustercentre.y * 3 + 2];
                    }
                }
            }
        }
    }

    /*
    for (Point& centre : clustercentresNouveau) {
        imOut[centre.x * 3][centre.y * 3] = 255; 
        imOut[centre.x * 3][centre.y * 3 + 1] = 0; 
        imOut[centre.x * 3][centre.y * 3 + 2] = 0; 
    }*/


    imOut.save(cNomImgEcrite);

    return 0;
}
