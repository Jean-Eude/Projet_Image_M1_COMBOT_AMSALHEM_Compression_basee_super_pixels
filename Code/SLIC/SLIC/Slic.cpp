#include <stdio.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cfloat>

#include "../Include/ImageBase.hpp"

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



int main(int argc, char **argv) {
    char cNomImgLue[250], cNomImgEcrite[250];
    int S, m , N;
    if (argc != 6) {
        printf("Usage: ImageIn.ppm ImageOut.ppm S m N\n");
        return 1;
    }
    sscanf(argv[1], "%s", cNomImgLue);
    sscanf(argv[2], "%s", cNomImgEcrite);
    sscanf(argv[3], "%d", &S);
    sscanf(argv[4], "%d", &m);
    sscanf(argv[5], "%d", &N);

    ImageBase imIn;
    imIn.load(cNomImgLue);

    int width = imIn.getWidth();
    int height = imIn.getHeight();
    
    ImageBase imOut(imIn.getWidth(), imIn.getHeight(), imIn.getColor());

    std::vector<Pixel> image;
    
    image.resize(width * height);
    
    std::vector<SuperPixel> clustercentres;
    
    int hsizec = 0;
    int wsizec = 0;
    for (int x = 0; x < height; x+=S) {
        hsizec++;
        wsizec = 0 ;
        for (int y = 0; y < width; y+=S) {
            wsizec++;
            SuperPixel sp(height * width);
            for (int i = 0; i<S ; i++){
                for (int j = 0; j<S ; j++){
                    Pixel p((x+i) , (y+j) , imIn[(x+i)*3][ (y+j)*3] , imIn[(x+i)*3][ (y+j)*3+1]  , imIn[(x+i)*3][ (y+j)*3+2]);
                    RGBtoLab(p);
                    image[(x+i) * width + (y+j)] = p;
                    sp.indicespixels[(x+i) * width + (y+j)] = (x + i) * width + y + j;
                }
            }
            calculMoyenne(sp , image);
            clustercentres.push_back(sp);
            RGBtoLab(sp);
        }
    }
    
    
    
    for (int x = 0; x < hsizec; x++) {
        for (int y = 0; y < wsizec; y++) {
            
            int i = x*wsizec+y;
            
             for (int k = std::max(x-1 , 0); k <= std::min(x+1 ,hsizec) ; k++) {
                for (int l = std::max(y-1 , 0); l <= std::min(y+1 ,wsizec) ; l++) {
                    
                    clustercentres[i].indice_adj.push_back( k * wsizec + l);
                 
                }
             }
        }
    }
    
    
for ( int i = 0 ;i < N ; i ++){
    for (SuperPixel & c : clustercentres){
        
        for (int x = 0; x <height ; x++){
            for (int y = 0 ; y <width; y++){
                if (c.indicespixels[x * width + y] != -1){
                    double minDist = FLT_MAX;
                    SuperPixel* minsp;

                    for (int i  : c.indice_adj){

                        double dist = calculDistances(image[x * width + y] , clustercentres[i] , S , m);
                        //std::cout<< dist <<" " << i <<std::endl;

                        if (dist < minDist){
                            
                            minDist = dist;
                            minsp = &clustercentres[i];

                            
                        }
                    }
                    
                    if (minsp != &c){
                        c.indicespixels[x * width + y] = -1;
                        minsp->indicespixels[x * width + y] = x * width + y;
                        //std::cout<< &c <<" " << minsp <<std::endl;
                    }
                }
                  
            }
        }
        calculMoyenne(c , image);
        RGBtoLab(c);
       
    }
}
    
    
    for (SuperPixel & c : clustercentres){
        for(int i : c.indicespixels){
            if(i != -1){
                
              imOut[ (int)image[i].x * 3][(int)image[i].y * 3] =  (int)c.R;
              imOut[ (int)image[i].x * 3][(int)image[i].y * 3 + 1 ] = (int) c.G;
              imOut[ (int)image[i].x * 3][(int)image[i].y * 3 + 2] =  (int)c.B;
                
            }
        }
    }
    

    double EQM_r, EQM_g, EQM_b, EQM;
    double PSNR;

    for (int i = 0; i < imIn.getHeight(); ++i) {
        for (int j = 0; j < imIn.getWidth(); ++j) {
            EQM_r += (imIn[i][j] - imOut[i][j]) * (imIn[i][j] - imOut[i][j]);
            EQM_g += (imIn[i][j + 1] - imOut[i][j + 1]) * (imIn[i][j + 1] - imOut[i][j + 1]);
            EQM_b += (imIn[i][j + 2] - imOut[i][j + 2]) * (imIn[i][j + 2] - imOut[i][j + 2]);
        }
    }

    EQM_r /= (imIn.getHeight() * imIn.getWidth());
    EQM_g /= (imIn.getHeight() * imIn.getWidth());
    EQM_b /= (imIn.getHeight() * imIn.getWidth());

    EQM = (EQM_r + EQM_g + EQM_b) / 3.0;

    PSNR = 10 * std::log10((255 * 255) / EQM);

    std::cout << PSNR << std::endl;


    imOut.save(cNomImgEcrite);

    return 0;
}
