#pragma once 

#include <string>
#include <iostream>
#include <vector>
#include <utility>
#include <cmath>

struct Pixel{
    double x, y;
    double R,G,B;
    double L,a,b;
    Pixel(){}
    Pixel(int _x ,int _y ,int _R ,int _G,int _B)
    {
        x = _x;
        y = _y;
        R = _R;
        G = _G;
        B = _B;
    }
    
};


struct SuperPixel {
    double x, y;
    double R,G,B;
    double L,a,b;
    
    std::vector<int> indicespixels;
    std::vector<int> indice_adj;
    
    SuperPixel(){}
    SuperPixel(long tailleimage)
    {
        indicespixels.resize(tailleimage);
        for (int i = 0 ; i< indicespixels.size() ; i++ ){
            indicespixels[i] = -1;
        }
    }
    SuperPixel(int _x,int _y )
    {
        x = _x;
        y = _y;
    }
    
};

void calculMoyenne(SuperPixel & sp , std::vector<Pixel> & image)
{
    double X , Y;
    double R , G , B;
    double size;
    
    for (int i : sp.indicespixels){
        if (i != -1) {  
            size ++;
            X += image[i].x; Y += image[i].y;
            R += image[i].R; G += image[i].G; B += image[i].B;
        }
    }
    X = X / size; Y = Y / size;
    R = R / size; G = G / size; B = B / size;
    
    sp.R = R; sp.G = G; sp.B = B;  
    sp.x = X; sp.y = Y;


}


// Fonction pour clamp un valeur
int clamp(int value) {
    return std::max(0, std::min(value, 255));
}

// Fonctions de distances (Position & Couleurs & les 2 (5D))
double distanceSpaciale(Pixel & p1, SuperPixel & p2) {
    return std::sqrt(((p1.x - p2.x) * (p1.x - p2.x)) + ((p1.y - p2.y) * (p1.y - p2.y)));
}

double distanceSpectral(Pixel p, SuperPixel sp) {
    int deltaL = p.L - sp.L;
    int deltaA = p.a - sp.a;
    int deltaB = p.b - sp.b;
    return std::sqrt(deltaL * deltaL + deltaA * deltaA + deltaB * deltaB);
}

double calculDistances(Pixel & pixel, SuperPixel & centre, int S, int m) {
    double terme1 = ((distanceSpectral(pixel, centre) /(double) m) * (distanceSpectral(pixel , centre) / (double) m));
    double terme2 = ((distanceSpaciale(pixel, centre) / (double) S) * (distanceSpaciale(pixel, centre) / (double)S));

    return sqrt(terme1 + terme2);
}

// RGB  -->  Lab
double F(double t) {return (t > 0.008856) ? pow(t, 1.0 / 3.0) : (7.787 * t + 16.0 / 116.0);}

void RGBtoLab(Pixel p) {
    double X = clamp(p.R * 0.412453 + p.G * 0.357580 + p.B * 0.180423);
    double Y = clamp(p.R * 0.212671 + p.G * 0.715160 + p.B * 0.072169);
    double Z = clamp(p.R * 0.019334 + p.G * 0.119193 + p.B * 0.950227);
        
    double Xn = 95.047;
    double Yn = 100.000;
    double Zn = 108.883;
        
    double f1 = F(X / Xn);
    double f2 = F(Y / Yn);
    double f3 = F(Z / Zn);

    p.L = static_cast<int>(clamp(116 * f2 - 16));
    p.a = static_cast<int>(clamp(500 * (f1 - f2)) + 128);
    p.b = static_cast<int>(clamp(200 * (f2 - f3)) + 128);
}
void RGBtoLab(SuperPixel p) {
    double X = clamp(p.R * 0.412453 + p.G * 0.357580 + p.B * 0.180423);
    double Y = clamp(p.R * 0.212671 + p.G * 0.715160 + p.B * 0.072169);
    double Z = clamp(p.R * 0.019334 + p.G * 0.119193 + p.B * 0.950227);
        
    double Xn = 95.047;
    double Yn = 100.000;
    double Zn = 108.883;
        
    double f1 = F(X / Xn);
    double f2 = F(Y / Yn);
    double f3 = F(Z / Zn);

    p.L = static_cast<int>(clamp(116 * f2 - 16));
    p.a = static_cast<int>(clamp(500 * (f1 - f2)) + 128);
    p.b = static_cast<int>(clamp(200 * (f2 - f3)) + 128);
}
