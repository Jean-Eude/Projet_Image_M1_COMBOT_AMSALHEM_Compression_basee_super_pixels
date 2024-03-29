#include "Include/ImageBase.h"
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


// RGB		-->		Lab 	
double F(double t) {return (t > 0.008856) ? pow(t, 1.0 / 3.0) : (7.787 * t + 16.0 / 116.0);}

void RGBtoLab(ImageBase &imIn, ImageBase &imOut, char c) {
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


void Convert2Gradient(ImageBase &imIn, ImageBase &imOut) {
	double gradX, gradY;

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



// Fonction pour perturber les centres des clusters
void perturbClusterCenters(ImageBase &Lab, ImageBase &imIn, ImageBase &gradient, int n, std::vector<Cluster> &clusterCentres) {
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


// Fonctions de distances (Position & Couleurs & les 2 (5D))
double distanceSpatiale(Cluster c, int x, int y) {
    return std::sqrt(((x - c.x) * (x - c.x)) + ((y - c.y) * (y - c.y)));
}

double distanceSpectrale(ImageBase &imOut, Cluster c, int x, int y) {
    return std::sqrt(((imOut[x * 3][y * 3] - c.L) * (imOut[x * 3][y * 3] - c.L)) +
										((imOut[x * 3][y * 3 + 1] - c.a) * (imOut[x * 3][y * 3 + 1] - c.a)) +
										((imOut[x * 3][y * 3 + 2] - c.b) * (imOut[x * 3][y * 3 + 2] - c.b)));
}

double calculDistances(double distanceSpatiale, double distanceSpectrale, int m, int S) {
    return sqrt(((distanceSpectrale/m) * (distanceSpectrale/m)) + (distanceSpatiale/S) * (distanceSpatiale/S));
}


int main(int argc, char **argv)
{
	char cNomImgLue[250], cNomImgEcrite[250];
	int K, m, n, nbIter;

	if (argc != 7) 
	{
		printf("Usage: ImageIn.pgm ImageOut.pgm Seuil \n"); 
		return 1;
	}
	sscanf (argv[1],"%s", cNomImgLue) ;
	sscanf (argv[2],"%s", cNomImgEcrite);
	// Nombre de superpixels
	sscanf (argv[3],"%d", &K);
	// Compacité
	sscanf (argv[4],"%d", &m);
	// vosisnage pertubation
	sscanf (argv[5],"%d", &n);
	// Nombre d'itérations = treshold
	sscanf (argv[6],"%d", &nbIter);
		
	ImageBase imIn;
	imIn.load(cNomImgLue);

	ImageBase imOut(imIn.getWidth(), imIn.getHeight(), imIn.getColor());


	// Nombre de pixels de image
	int N = imIn.getWidth() * imIn.getHeight();
	std::cout << N << std::endl;

	// Taille de chaque superpixels
	double tailleSP = static_cast<double>(N)/static_cast<double>(K);
	// Distance entre chaque superpixel
	int S = sqrt(tailleSP);

	// Liste des centres des clusters
	std::vector<Cluster> clusterCentres;

	// Image de base
	for(int x = 0; x < imIn.getHeight(); x++) {
		for(int y = 0; y < imIn.getWidth(); y++) {
			imOut[x*3][y*3] = imIn[x*3][y*3];
			imOut[x*3][y*3+1] = imIn[x*3][y*3+1];
			imOut[x*3][y*3+2] = imIn[x*3][y*3+2];	
		}
	}

	// Initialisation des clusters

	// 1. Conversion de RGB à Lab
	ImageBase Lab(imIn.getWidth(), imIn.getHeight(), imIn.getColor());	
	//RGBtoLab(imOut, Lab, 'A');

	// 2. Initialisation des cluster
	for(int x = S; x < imOut.getHeight() - S; x+=S) {
		for(int y = S; y < imOut.getWidth() - S; y+=S) {
			Cluster p = {x, y, imOut[x*3][y*3], imOut[x*3][y*3+1], imOut[x*3][y*3+2]};
			clusterCentres.push_back(p);
		}
	}

	// Récupération de la luminance
	ImageBase L(imIn.getWidth(), imIn.getHeight(), false);	
	RGBtoLab(imOut, L, 'L');	

	ImageBase Gradient(imIn.getWidth(), imIn.getHeight(), false);
	Convert2Gradient(L, Gradient);
	perturbClusterCenters(imIn, imOut, Gradient, n, clusterCentres);


	std::vector<std::vector<double>> dis(imIn.getHeight(), std::vector<double>(imIn.getWidth(), std::numeric_limits<double>::max()));
	for(auto& row : dis) {
		std::fill(row.begin(), row.end(), std::numeric_limits<double>::max());
	}

	ImageBase superpixelImage(imIn.getWidth(), imIn.getHeight(), imIn.getColor());
	std::vector<std::vector<int>> clusterIndices(imOut.getHeight(), std::vector<int>(imOut.getWidth(), -1));
	std::vector<std::vector<int>> contourImage(imOut.getHeight(), std::vector<int>(imOut.getWidth(), 0));

	for (int iter = 0; iter < nbIter; iter++) {
		// Réinitialisation des pixels appartenant à chaque cluster
		for (auto& c : clusterCentres) {
			c.pixelIndices.clear(); 
		}
		
		// Attribution de chaque pixel au cluster le plus proche
		for (int x = 0; x < imOut.getHeight(); x++) {
			for (int y = 0; y < imOut.getWidth(); y++) {
				int minClusterIndex = -1;
				double minDistance = std::numeric_limits<double>::max();

				for (int i = 0; i < clusterCentres.size(); i++) {
					Cluster& c = clusterCentres[i];
					double Dc = std::sqrt(((imOut[x * 3][y * 3] - c.L) * (imOut[x * 3][y * 3] - c.L)) +
										((imOut[x * 3][y * 3 + 1] - c.a) * (imOut[x * 3][y * 3 + 1] - c.a)) +
										((imOut[x * 3][y * 3 + 2] - c.b) * (imOut[x * 3][y * 3 + 2] - c.b)));
					double Ds = std::sqrt(((x - c.x) * (x - c.x)) + ((y - c.y) * (y - c.y)));
					double D = sqrt(((Dc/m) * (Dc/m))  + (Ds/S) * (Ds/S));

					if (D < minDistance) {
						minDistance = D;
						minClusterIndex = i;
					}
				}

				Cluster& nearestCluster = clusterCentres[minClusterIndex];
				nearestCluster.addPixelIndex(x, y); 

				superpixelImage[x * 3][y * 3] = nearestCluster.L;
				superpixelImage[x * 3][y * 3 + 1] = nearestCluster.a;
				superpixelImage[x * 3][y * 3 + 2] = nearestCluster.b;
			}
		}

		// Mise à jour progressive des centres de cluster
		for (auto& c : clusterCentres) {
			double sumX = 0, sumY = 0;

			for (const auto& pixel : c.pixelIndices) {
				sumX += pixel.first;
				sumY += pixel.second;
			}

			if (!c.pixelIndices.empty()) {
				// Mise à jour progressive des centres vers le centre de masse des pixels du cluster
				c.x = static_cast<int>(sumX / c.pixelIndices.size());
				c.y = static_cast<int>(sumY / c.pixelIndices.size());
			}
		}

		// Enforce la connectivité 4 entre les cellules appartenant au même superpixel
		for (int x = 1; x < imOut.getHeight() - 1; x++) {
			for (int y = 1; y < imOut.getWidth() - 1; y++) {
				int currentClusterIndex = clusterIndices[x][y];
				int leftClusterIndex = clusterIndices[x - 1][y];
				int rightClusterIndex = clusterIndices[x + 1][y];
				int topClusterIndex = clusterIndices[x][y - 1];
				int bottomClusterIndex = clusterIndices[x][y + 1];

				if (currentClusterIndex != leftClusterIndex ||
					currentClusterIndex != rightClusterIndex ||
					currentClusterIndex != topClusterIndex ||
					currentClusterIndex != bottomClusterIndex) {

					// Réaffecter la cellule disjointe au même superpixel que ses voisins
					clusterIndices[x][y] = currentClusterIndex;
				}
			}
		}
	}

	// Pour avoir les contours
	ImageBase Le(imIn.getWidth(), imIn.getHeight(), false);	
	RGBtoLab(superpixelImage, Le, 'L');
	ImageBase Gradientz(imIn.getWidth(), imIn.getHeight(), false);
	Convert2Gradient(Le, Gradientz);
    for (int x = 0; x < imOut.getHeight(); x++) {
        for (int y = 0; y < imIn.getWidth(); y++) {
			if(Gradientz[x][y] != 0) {
				Gradientz[x][y] = 255;
			}
        }
    }		

	for (int x = 0; x < imOut.getHeight(); x++) {
		for (int y = 0; y < imOut.getWidth(); y++) {
			if (Gradientz[x][y] != 0) {
				superpixelImage[x * 3][y * 3] = 255;
				superpixelImage[x * 3][y * 3 + 1] = 0;
				superpixelImage[x * 3][y * 3 + 2] = 0;
			}
		}
	}

	// Pour avoir les centres des clusters
	for (auto& cluster : clusterCentres) {
		int x = cluster.x;
		int y = cluster.y;
		superpixelImage[x * 3][y * 3] = 255;
		superpixelImage[x * 3][y * 3 + 1] = 255;
		superpixelImage[x * 3][y * 3 + 2] = 255;
	}

	superpixelImage.save(cNomImgEcrite);
}