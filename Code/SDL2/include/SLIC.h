#pragma once

#include <Window.h>
#include <GUI.h>
#include <ImageBase.h>

#include <stdio.h>
#include <iostream>
#include <cmath>
#include <algorithm>



struct Cluster {
    int x, y;
    int L, a, b;
    std::vector<std::pair<int, int>> pixelIndices; 

    void addPixelIndex(int x, int y) {
        pixelIndices.push_back({x, y});
    }
};

// Fonction pour clamp une valeur
int clamp(int value) {
    return std::max(0, std::min(value, 255));
}

#pragma omp single
void OriginalImage(ImageBase &imIn, ImageBase &imOut, int x, int y, uint32_t *buffer, int size, int width) {
    #pragma omp parallel for collapse(2) schedule(static)
    for (int px = x; px < x + size; px++) {
        for (int py = y; py < y + size; py++) {
            int imgX_N = px - x;
            int imgY_N = py - y;

            if (imgX_N >= 0 && imgX_N < size && imgY_N >= 0 && imgY_N < size) {
                uint32_t pixel = buffer[py * width + px];
                uint8_t r_s = (pixel >> 16) & 0xFF;
                uint8_t g_s = (pixel >> 8) & 0xFF;
                uint8_t b_s = pixel & 0xFF;

                imOut[imgY_N * 3][imgX_N * 3] = r_s;
                imOut[imgY_N * 3][imgX_N * 3 + 1] = g_s;
                imOut[imgY_N * 3][imgX_N * 3 + 2] = b_s;
            }
        }
    }
}



// RGB		-->		Lab 	
double F(double t) {return (t > 0.008856) ? pow(t, 1.0 / 3.0) : (7.787 * t + 16.0 / 116.0);}

#pragma omp single
void RGBtoLab(ImageBase &imIn, ImageBase &imOut, char c) {
    #pragma omp parallel for collapse(2) schedule(static)
    for (int x = 0; x < imIn.getHeight(); x++) {
        for (int y = 0; y < imIn.getWidth(); y++) {       

			double X = clamp(imIn[x*3][y*3] * 0.412453 + imIn[x*3][y*3+1] * 0.357580 + imIn[x*3][y*3+2] * 0.180423);
			double Y = clamp(imIn[x*3][y*3] * 0.212671 + imIn[x*3][y*3+1] * 0.715160 + imIn[x*3][y*3+2] * 0.072169);
			double Z = clamp(imIn[x*3][y*3] * 0.019334 + imIn[x*3][y*3+1] * 0.119193 + imIn[x*3][y*3+2] * 0.950227);
				
			double Xn = 95.047;
			double Yn = 100.000;
			double Zn = 108.883;
				
			double f1 = F(X / Xn);
			double f2 = F(Y / Yn);
			double f3 = F(Z / Zn);

			if(c == 'L') {
				imOut[x][y] = static_cast<int>(clamp(116 * f2 - 16));
			} else if(c == 'a') {
				imOut[x][y] = static_cast<int>(clamp(500 * (f1 - f2)) + 128);

			} else if(c == 'b') {
				imOut[x][y] = static_cast<int>(clamp(200 * (f2 - f3)) + 128);
			} else if(c == 'A') {
				imOut[x*3][y*3] = static_cast<int>(clamp(116 * f2 - 16));
				imOut[x*3][y*3+1] = static_cast<int>(clamp(500 * (f1 - f2)) + 128);
				imOut[x*3][y*3+1] = static_cast<int>(clamp(200 * (f2 - f3)) + 128);
			}           
        }
    }
}


#pragma omp single
void Convert2Gradient(ImageBase &imIn, ImageBase &imOut) {
	double gradX, gradY;
    #pragma omp parallel for collapse(2) schedule(static)
    for (int x = 0; x < imOut.getHeight(); x++)
    {
        for (int y = 0; y < imIn.getWidth(); y++)
        {
            gradX = (y < imIn.getWidth() - 1) ? imIn[x][y + 1] - imIn[x][y] : imIn[x][y] - imIn[x][y - 1];
            gradY = (x < imIn.getHeight() - 1) ? imIn[x + 1][y] - imIn[x][y] : imIn[x][y] - imIn[x - 1][y];

            imOut[x][y] = std::clamp(sqrt(gradX * gradX + gradY * gradY), 0., 255.);
        }
    }	
}


#pragma omp single
// Fonction pour perturber les centres des clusters
void perturbClusterCenters(ImageBase &Lab, ImageBase &imIn, ImageBase &gradient, int n, std::vector<Cluster> &clusterCentres) {
    #pragma omp parallel for collapse(3) schedule(static)
    for (int i = 0; i < clusterCentres.size(); ++i) {
        int x = clusterCentres[i].x;
        int y = clusterCentres[i].y;

        double minGradient = gradient[x][y];
        int newX = x, newY = y;
        
        for (int dx = -n/2; dx <= n/2; ++dx) {
            for (int dy = -n/2; dy <= n/2; ++dy) {
                int nx = x + dx;
                int ny = y + dy;

                if (nx >= 0 && nx < gradient.getHeight() && ny >= 0 && ny < gradient.getWidth()) {
                    if (gradient[nx][ny] < minGradient) {
                        minGradient = gradient[nx][ny];
                        newX = nx;
                        newY = ny;
                    }
                }
            }
        }

        clusterCentres[i].x = newX;
        clusterCentres[i].y = newY;
		clusterCentres[i].L = Lab[newX*3][newY*3];
		clusterCentres[i].a = Lab[newX*3][newY*3+1];
		clusterCentres[i].b = Lab[newX*3][newY*3+2];
    }
}

