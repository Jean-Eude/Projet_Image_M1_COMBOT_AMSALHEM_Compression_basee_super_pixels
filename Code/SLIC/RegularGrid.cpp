#include "Include/ImageBase.h"
#include <stdio.h>
#include <cmath>


struct Point {
    int x, y;
};


double distanceEuclidienne_Pixel(Point p1, Point p2) {
	return sqrt(((p1.x - p2.x) * (p1.x - p2.x)) + ((p1.y - p2.y) * (p1.y - p2.y)));
}



int clamp(int value) {
    return std::max(0, std::min(value, 255));
}


int main(int argc, char **argv)
{
	char cNomImgLue[250], cNomImgEcrite[250];
  
	if (argc != 3) 
	{
		printf("Usage: ImageIn.pgm ImageOut.pgm \n"); 
		return 1;
	}
	sscanf (argv[1],"%s",cNomImgLue) ;
	sscanf (argv[2],"%s",cNomImgEcrite);
	
	
	ImageBase imIn;
	imIn.load(cNomImgLue);

	ImageBase imOut(imIn.getWidth(), imIn.getHeight(), imIn.getColor());

	// Paramètres de la grille
    int Gx = 2;
    int Gy = 2;

    int segmentWidth = imIn.getHeight() / Gx; 
    int segmentHeight = imIn.getWidth() / Gy; 

    for(int x = 0; x < imIn.getHeight() + 10; x += segmentWidth) {
        for(int y = 0; y < imIn.getWidth() + 10; y += segmentHeight) {
            int segmentIndexWidth = x / segmentWidth;
            int segmentIndexHeight = y / segmentHeight;

            for(int i = 0; i < segmentWidth; i++) {
                for(int j = 0; j < segmentHeight; j++) {
                    int newX = x + i;
                    int newY = y + j;
                    
                    if(newX < imIn.getHeight() && newY < imIn.getWidth()) {
                        imOut[newX * 3][newY * 3] = imIn[x * 3][y * 3];
                        imOut[newX * 3][newY * 3 + 1] = imIn[x * 3][y * 3 + 1];
                        imOut[newX * 3][newY * 3 + 2] = imIn[x * 3][y * 3 + 2];
                    }
                }
            }

            Point p {imIn.getHeight()/2 , imIn.getWidth()/2};
            imOut[p.x * 3][p.y * 3] = 255;
            imOut[p.x * 3][p.y * 3 + 1] = 0;
            imOut[p.x * 3][p.y * 3 + 2] = 0;

            imOut[(p.x-1) * 3][p.y * 3] = 255;
            imOut[(p.x-1) * 3][p.y * 3 + 1] = 0;
            imOut[(p.x-1) * 3][p.y * 3 + 2] = 0;

            imOut[p.x * 3][(p.y-1) * 3] = 255;
            imOut[p.x * 3][(p.y-1) * 3 + 1] = 0;
            imOut[p.x * 3][(p.y-1) * 3 + 2] = 0;

            imOut[(p.x-1) * 3][(p.y-1) * 3] = 255;
            imOut[(p.x-1) * 3][(p.y-1) * 3 + 1] = 0;
            imOut[(p.x-1) * 3][(p.y-1) * 3 + 2] = 0;    
        }
    }

		
	imOut.save(cNomImgEcrite);

	return 0;
}
