#include <stdio.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <cfloat>
#include "../Include/ImageBase.hpp"



int clamp(int value) {
    return std::max(0, std::min(value, 255));
}

struct Pixel {
    int x, y;
    int L ,a , b , R , G , B;
    
    double F(double t) {return (t > 0.008856) ? pow(t, 1.0 / 3.0) : (7.787 * t + 16.0 / 116.0);}
    void RGBtolab() {
        int X = clamp(R * 0.412453 + G * 0.357580 + B * 0.180423);
        int Y = clamp(R * 0.212671 + G * 0.715160 + B * 0.072169);
        int Z = clamp(R * 0.019334 + G * 0.119193 + B * 0.950227);
        
        double Xn = 95.047;
        double Yn = 100.000;
        double Zn = 108.883;
        
        double f1 = F(X / Xn);
        double f2 = F(Y / Yn);
        double f3 = F(Z / Zn);
        
        L = clamp(116 * f2 - 16);
        a = clamp(500 * (f1 - f2)) + 128;
        b = clamp(200 * (f2 - f3)) + 128;
        
    }
    
    Pixel(){}
    Pixel(int _x , int _y , int _R ,int _G , int _B)
    {   x = _x ; y = _y ; R = _R ; G = _G ; B = _B;
        RGBtolab();
    }
    
  
    
};

struct SuperPixel {
    int x, y;
    int Lmoy, amoy, bmoy, Rmoy, Gmoy, Bmoy;
    std::vector<Pixel> Cluster;
    SuperPixel(){}
    
    
    
    double F(double t) {return (t > 0.008856) ? pow(t, 1.0 / 3.0) : (7.787 * t + 16.0 / 116.0);}
    void RGBtolab() {
        int X = clamp(Rmoy * 0.412453 + Gmoy * 0.357580 + Bmoy * 0.180423);
        int Y = clamp(Rmoy * 0.212671 + Gmoy * 0.715160 + Bmoy * 0.072169);
        int Z = clamp(Rmoy * 0.019334 + Gmoy * 0.119193 + Bmoy * 0.950227);
        
        double Xn = 95.047;
        double Yn = 100.000;
        double Zn = 108.883;
        
        double f1 = F(X / Xn);
        double f2 = F(Y / Yn);
        double f3 = F(Z / Zn);
        
        Lmoy = clamp(116 * f2 - 16);
        amoy = clamp(500 * (f1 - f2)) + 128;
        bmoy = clamp(200 * (f2 - f3)) + 128;
        
    }
    
    
    void calculercluster(){ //calcule la moyenne des pixels et des couleurs RGB et Lab dans le superpixel.
            
        int sizeCluster =  Cluster.size();
        x = 0; y = 0 ; Lmoy = 0; amoy = 0 ; bmoy = 0 ; Rmoy = 0 ; Gmoy = 0 ; Bmoy = 0;
        
        for (int i = 0 ; i < sizeCluster; i++)
        {
            x += Cluster[i].x;
            y += Cluster[i].y;
            
            Lmoy += Cluster[i].L;   Rmoy += Cluster[i].R;
            amoy += Cluster[i].a;   Gmoy += Cluster[i].G;
            bmoy += Cluster[i].b;   Bmoy += Cluster[i].B;            
        }
        
        if ( sizeCluster > 0 ){
            x /= sizeCluster;
            y /= sizeCluster;
            
            Lmoy /= sizeCluster;    Rmoy /= sizeCluster;
            amoy /= sizeCluster;    Gmoy /= sizeCluster;
            bmoy /= sizeCluster;    Bmoy /= sizeCluster;
            
            RGBtolab();
        }
                
    }
    
};

double distanceEuclidienne_spacial(Pixel & p , SuperPixel & sp) {
 	return std::sqrt(((p.x - sp.x) * (p.x - sp.x)) + ((p.y - sp.y) * (p.y - sp.y)));
}

double distanceEuclidienne_couleurs(Pixel & p , SuperPixel & sp) {
    int deltaL  = sp.Lmoy - p.L;
    int deltaA  = sp.amoy - p.a;
    int deltaB  = sp.bmoy - p.b;
    return std::sqrt(std::pow(deltaL ,2) +std::pow(deltaA ,2) + std::pow(deltaB ,2));
}
void reconstruction(ImageBase & imOut , std::vector<SuperPixel> & Superpixels,int Gx , int Gy){
    // rconstruction pas bonne 
    
    int segmentHeight = imOut.getHeight() / Gx; 
    int  segmentWidth= imOut.getWidth() / Gy; 
    
    std::cout << Superpixels.size() << std::endl;
    for(int x = 0; x < imOut.getHeight() ; x += segmentHeight) {
        for(int y = 0; y < imOut.getWidth() ; y += segmentWidth) {
            int indexSuperPixelX = x / segmentHeight;
            int indexSuperPixelY = y / segmentWidth;
            
            for(int i = 0; i < segmentHeight; i++) {
                for(int j = 0; j < segmentWidth; j++) {
                    int indexImageX = x + i;
                    int indexImageY = y + j;
                    imOut[indexImageX * 3][indexImageY * 3] = Superpixels[indexSuperPixelX * Gy + indexSuperPixelY].Rmoy;
                    imOut[indexImageX * 3][indexImageY * 3 + 1] = Superpixels[indexSuperPixelX * Gy + indexSuperPixelY ].Gmoy;
                    imOut[indexImageX * 3][indexImageY * 3 + 2] = Superpixels[indexSuperPixelX * Gy + indexSuperPixelY ].Bmoy;
                    
                }
            }
        }
        
    }
    
    
    
    
}



void SLIC(ImageBase & imIn , std::vector<SuperPixel> & Superpixels ,int M ,int Gx , int Gy){
    // algorithme trop long regarder plus en detail. 
    //for (int k = 0 ; k < 5 ; k++){
        std::vector<SuperPixel> new_SuperPixels;
        new_SuperPixels.resize(Gx * Gy);
        
        int segmentHeight = imIn.getHeight() / Gx; 
        int  segmentWidth= imIn.getWidth() / Gy; 
        
        for(int x = 0; x < imIn.getHeight() ; x++) {
            for(int y = 0; y < imIn.getWidth() ; y++) {
                
                Pixel p(x, //indice x dans l'image
                        y , //indice y dans l'image
                        imIn[x * 3][y * 3] ,   //R
                        imIn[x * 3][y * 3 + 1],//G
                        imIn[x * 3][y * 3 + 2] //B
                );
                double D = FLT_MAX;
                SuperPixel bestsp;
                for(int i = 0; i < Gx * Gy ; i++) {
                    SuperPixel sp = Superpixels[i];
                    
                    double dist_spaciale = distanceEuclidienne_spacial (p , sp);
                    double dist_couleur = distanceEuclidienne_couleurs (p , sp);
                    
                    double newD = dist_couleur + dist_spaciale * M;
                    
                    if (newD < D ){ bestsp = sp; D = newD;}
                    
                }
                bestsp.Cluster.push_back(p);
                

                new_SuperPixels[ (x/segmentHeight) * Gy +  (y/segmentWidth)] = bestsp;
            }
        }
        
        for (int i = 0 ; i < Gy * Gx ; i++){
             Superpixels[i] = new_SuperPixels[i];
             std::cout << Superpixels[i].x << " " << Superpixels[i].y << " // " << new_SuperPixels[i].x << " " << new_SuperPixels[i].y  << std::endl;
        }
    //}
//     int c = 0;
//      for (int i = 0 ; i < Gy * Gx ; i++){
//             if (Superpixels[i].Rmoy > 0 or Superpixels[i].Gmoy > 0 or Superpixels[i].Bmoy > 0 ) c ++;
//      }
//      std::cout << c << std::endl;
//     
}



//renvoie l'image divisé en une grille regulière chaque cellule contient la moyenne de la couleur de la grille.
void regulargrid (ImageBase & imIn , ImageBase & imOut , int Gx , int Gy , std::vector<SuperPixel> & Superpixels){
    
    int segmentWidth = imIn.getHeight() / Gx; 
    int segmentHeight = imIn.getWidth() / Gy; 
    
    
    for(int x = 0; x < imIn.getHeight() ; x += segmentHeight) {
        for(int y = 0; y < imIn.getWidth() ; y += segmentWidth) {
            
         int indexSuperPixelX = x / segmentHeight;
         int indexSuperPixelY = y / segmentWidth;
            
            SuperPixel sp;
            
            for(int i = 0; i < segmentHeight; i++) {
                for(int j = 0; j < segmentWidth; j++) {
                    
                    int indexImageX = x + i;
                    int indexImageY = y + j;
                    
                    Pixel p(indexImageX , //indice x dans l'image
                            indexImageY , //indice y dans l'image
                            imIn[indexImageX * 3][indexImageY * 3] ,   //R
                            imIn[indexImageX * 3][indexImageY * 3 + 1],//G
                            imIn[indexImageX * 3][indexImageY * 3 + 2] //B
                           );
                
                    
                    sp.Cluster.push_back(p); //ajoute le pixel dans le superPixel.
                    
                }
            }
            sp.calculercluster();
            Superpixels[indexSuperPixelX * Gy + indexSuperPixelY] = sp;

            //std::cout << Superpixels[indexSuperPixelX * Gy + indexSuperPixelY].x << " " << Superpixels[indexSuperPixelX * Gy + indexSuperPixelY].y << std::endl;

            
            for(int i = 0; i < segmentHeight; i++) {
                for(int j = 0; j < segmentWidth; j++) {
                    int indexImageX = x + i;
                    int indexImageY = y + j;
                    imOut[indexImageX * 3][indexImageY * 3] = Superpixels[indexSuperPixelX * Gy + indexSuperPixelY].Rmoy;
                    imOut[indexImageX * 3][indexImageY * 3 + 1] = Superpixels[indexSuperPixelX * Gy + indexSuperPixelY ].Gmoy;
                    imOut[indexImageX * 3][indexImageY * 3 + 2] = Superpixels[indexSuperPixelX * Gy + indexSuperPixelY ].Bmoy;
                    
                }
            }
            

        }
    }
}



int main(int argc, char **argv)
{
	//char cNomImgLue[250], cNomImgEcrite_X[250], cNomImgEcrite_Y[250], cNomImgEcrite_Z[250], cNomImgEcrite_L[250], cNomImgEcrite_a[250], cNomImgEcrite_b[250], cNomImgEcrite_RGB[250];
   char cNomImgLue[250] , cNomImgEcrite[250] ;
   int Gx , Gy ; 
	if (argc != 5) 
	{
		printf("Usage: ImageIn.ppm ImageOut.ppm Gx Gy \n"); 
		return 1;
	}
     sscanf (argv[1],"%s",cNomImgLue);
     sscanf (argv[2],"%s",cNomImgEcrite);
     sscanf (argv[3],"%d",&Gx);
     sscanf (argv[4],"%d",&Gy);
    
     
     
     ImageBase imIn;
     imIn.load(cNomImgLue);
     ImageBase imOut(imIn.getWidth(), imIn.getHeight(), imIn.getColor());
     std::vector<SuperPixel> compressedImage;
     compressedImage.resize(Gx * Gy);
     regulargrid (imIn , imOut , Gx , Gy , compressedImage);
     SLIC(imIn , compressedImage , 40 , Gx , Gy);
     reconstruction (imOut , compressedImage , Gx ,Gy ); 
     imOut.save(cNomImgEcrite);
     
    

    


	return 0;
}
