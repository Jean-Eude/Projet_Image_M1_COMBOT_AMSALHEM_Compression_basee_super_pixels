#include "Include/ImageBase.h"
#include <stdio.h>
#include <cmath>
#include <iostream>

int clamp(int value) {
    return std::max(0, std::min(value, 255));
}

struct Point {
    int x, y;
};

double distanceEuclidienne_Pixel(Point p1, Point p2, int R, int G, int B) {
	return sqrt(((p1.x - p2.x) * (p1.x - p2.x)) + ((p1.y - p2.y) * (p1.y - p2.y)));
}

// RGB	->	XYZ

void RGBtoXYZ(ImageBase &In, ImageBase &X, ImageBase &Y, ImageBase &Z, int x, int y) {
   	X[x][y] = clamp(In[x * 3][y * 3] * 0.412453 + In[x * 3][y * 3 + 1] * 0.357580 + In[x * 3][y * 3 + 2] * 0.180423);
    Y[x][y] = clamp(In[x * 3][y * 3] * 0.212671 + In[x * 3][y * 3 + 1] * 0.715160 + In[x * 3][y * 3 + 2] * 0.072169);
    Z[x][y] = clamp(In[x * 3][y * 3] * 0.019334 + In[x * 3][y * 3 + 1] * 0.119193 + In[x * 3][y * 3 + 2] * 0.950227);
}

void XYZtoRGB(ImageBase &X, ImageBase &Y, ImageBase &Z, ImageBase &Out, int x, int y) {
    Out[x * 3][y * 3] = clamp(3.240479 * X[x][y] - 1.537150 * Y[x][y] - 0.498535 * Z[x][y]);
    Out[x * 3][y * 3 + 1] = clamp(-0.969256 * X[x][y] + 1.875992 * Y[x][y] + 0.041556 * Z[x][y]);
    Out[x * 3][y * 3 + 2] = clamp(0.055648 * X[x][y] - 0.204043 * Y[x][y] + 1.057311 * Z[x][y]);
}


// XYZ	->	Lab
double F(double t) {
    return (t > 0.008856) ? pow(t, 1.0 / 3.0) : (7.787 * t + 16.0 / 116.0);
}

void XYZtoLab(ImageBase &X, ImageBase &Y, ImageBase &Z, ImageBase &L, ImageBase &a, ImageBase &b, int x, int y) {
    double Xn = 95.047;
    double Yn = 100.000;
    double Zn = 108.883;

    double f1 = F(X[x][y] / Xn);
    double f2 = F(Y[x][y] / Yn);
    double f3 = F(Z[x][y] / Zn);

    L[x][y] = clamp(116 * f2 - 16);
    a[x][y] = clamp(500 * (f1 - f2)) + 128;
    b[x][y] = clamp(200 * (f2 - f3)) + 128;

	std::cout << int(L[x][y]) << "\t" << int(a[x][y]) << "\t" << int(b[x][y]) << "\t" << std::endl;
}


// Etapes décomposition / recomposition :
/*
1.	RGB	-> 	XYZ
2.	XYZ	->	LAB
3.	...
4.	LAB	-> 	XYZ
5.	XYZ	->	RGB
*/


int main(int argc, char **argv)
{
	char cNomImgLue[250], cNomImgEcrite_X[250], cNomImgEcrite_Y[250], cNomImgEcrite_Z[250], cNomImgEcrite_L[250], cNomImgEcrite_a[250], cNomImgEcrite_b[250], cNomImgEcrite_RGB[250];
  
	if (argc != 9) 
	{
		printf("Usage: ImageIn.pgm ImageOut.pgm \n"); 
		return 1;
	}
	sscanf (argv[1],"%s",cNomImgLue) ;
	sscanf (argv[2],"%s",cNomImgEcrite_X);
	sscanf (argv[3],"%s",cNomImgEcrite_Y);
	sscanf (argv[4],"%s",cNomImgEcrite_Z);
	sscanf (argv[5],"%s",cNomImgEcrite_L);
	sscanf (argv[6],"%s",cNomImgEcrite_a);
	sscanf (argv[7],"%s",cNomImgEcrite_b);
	sscanf (argv[8],"%s",cNomImgEcrite_RGB);

	
	ImageBase imIn;
	imIn.load(cNomImgLue);

	ImageBase X(imIn.getWidth(), imIn.getHeight(), false);
	ImageBase Y(imIn.getWidth(), imIn.getHeight(), false);
	ImageBase Z(imIn.getWidth(), imIn.getHeight(), false);

	ImageBase L(imIn.getWidth(), imIn.getHeight(), false);
	ImageBase a(imIn.getWidth(), imIn.getHeight(), false);
	ImageBase b(imIn.getWidth(), imIn.getHeight(), false);

	ImageBase RGB(imIn.getWidth(), imIn.getHeight(), imIn.getColor());

	// Paramètres de la grille
    int N = 10;

    for(int x = 0; x < imIn.getHeight(); x += 1) {
        for(int y = 0; y < imIn.getWidth(); y += 1) {
			RGBtoXYZ(imIn, X, Y, Z, x, y);
			XYZtoLab(X, Y, Z, L, a, b, x, y);
        }
    }

	L.save(cNomImgEcrite_L);
	a.save(cNomImgEcrite_a);
	b.save(cNomImgEcrite_b);


	return 0;
}
