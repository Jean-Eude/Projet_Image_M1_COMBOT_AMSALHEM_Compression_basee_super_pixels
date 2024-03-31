#include "Include/ImageBase.h"
#include <stdio.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <set>
#include <tuple>
#include <vector>


struct Palette {
	int r, g, b;
};

struct Cluster {
    int x, y;
    int L, a, b;
    std::vector<std::pair<int, int>> pixelIndices; 

    void addPixelIndex(int x, int y) {
        pixelIndices.push_back({x, y});
    }
};

//Fonction pour clamp une valeur
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


int moyenne(int A ,int B , int C ,int D ,int E ,int F , int G , int H , int I){
    return (A + B + C + D + E + F + G + H + I)/9 ;
} 
void moyenneur(ImageBase & imIn , ImageBase & imMoy)
{
     
    for(int x = 0 ; x < imIn.getHeight(); x++){
        for(int y = 0 ; y < imIn.getWidth()  ; y++){
                             
            int _x = std::max(x - 1 , 0);
            int _y = std::max(y - 1 , 0);
            
            int x_ = std::min(x + 1 , imIn.getHeight() - 1);
            int y_ = std::min(y + 1 , imIn.getWidth()- 1);
            
            
            int pix_moy = moyenne(imIn[_x][_y],   imIn[_x][y],   imIn[_x][y_],
                                  imIn[x][_y]  ,   imIn[x][y]  ,   imIn[x][y_], 
                                  imIn[x_][_y],   imIn[x_][y],   imIn[x_][y_]
                                );                

            imMoy[x][y] = pix_moy;
        }
    }
}

int gradient(int A ,int B , int C ,int D ,int E ,int F , int c){
    return A + B + c * C + c * D + E + F ;
}

void Convert2Gradient(ImageBase &imIn, ImageBase &imOut) {
	double gradX, gradY;
    ImageBase moy(imIn.getWidth(), imIn.getHeight(), false);	
    moyenneur(imIn , moy);
    
    for (int x = 0; x < imOut.getHeight() ; x++)
    {
        for (int y = 0; y < imIn.getWidth(); y++)
        {
            
            int _x = std::max(x - 1 , 0);
            int _y = std::max(y - 1 , 0);
            
            int x_ = std::min(x + 1 , imIn.getHeight()- 1);
            int y_ = std::min(y + 1 , imIn.getWidth()- 1);
            
            int grad_h = gradient(moy[_x][_y],  - moy[_x][y_] , 
                                  moy[x][_y] ,  - moy[x][y_] ,
                                  moy[x_][_y],  - moy[x_][y_] , 1); 
            
            int grad_v = gradient(moy[_x][_y],  - moy[x_][_y],
                                  moy[_x][y] ,  - moy[x_][y],
                                  moy[_x][y_],  - moy[x_][y_], 1);                
            
            imOut[x][y] = clamp((int)sqrt(grad_h * grad_h + grad_v * grad_v));
        
           
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
double psnr(ImageBase & imIn , ImageBase & imOut) {
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
    return PSNR;
}

void Contour_image_et_centre(ImageBase & imIn ,ImageBase & imOut , std::vector<Cluster> & clusterCentres ) 
{
    
    //Pour avoir les contours
    ImageBase superpixelImageLab(imIn.getWidth(), imIn.getHeight(), true);	
    RGBtoLab(imIn, superpixelImageLab, 'A');
    
    for (int x = 0; x < imOut.getHeight(); x++) {
        for (int y = 1; y < imOut.getWidth(); y++) {
            if(superpixelImageLab[x * 3][y * 3]  != superpixelImageLab[x * 3][(y-1) * 3] or
                superpixelImageLab[x * 3][y * 3 + 1]  != superpixelImageLab[x * 3][(y-1) * 3 + 1] or 
                superpixelImageLab[x * 3][y * 3 + 2]  != superpixelImageLab[x * 3][(y-1) * 3 + 2] 
            ) {
                imOut[x * 3][y * 3] = 255;
                imOut[x * 3][y * 3 + 1] = 0;
                imOut[x * 3][y * 3 + 2] = 0;
            }
            else{
                imOut[x * 3][y * 3] = imIn[x * 3][y * 3];
                imOut[x * 3][y * 3 + 1] = imIn[x * 3][y * 3 +1];
                imOut[x * 3][y * 3 + 2] = imIn[x * 3][y * 3 +2];
            }
        }
    }	
    for (int y = 0; y < imOut.getHeight(); y++) {
        for (int x = 1; x < imOut.getWidth(); x++) {
            if(superpixelImageLab[x * 3][y * 3]  != superpixelImageLab[(x-1) * 3][y * 3] or 
                superpixelImageLab[x * 3][y * 3 + 1 ]  != superpixelImageLab[(x-1) * 3][y * 3 +1] or
                superpixelImageLab[x * 3][y * 3 + 2]  != superpixelImageLab[x * 3][(y-1) * 3 + 2] 
            ) {
                imOut[x * 3][y * 3] = 255;
                imOut[x * 3][y * 3 + 1] = 0;
                imOut[x * 3][y * 3 + 2] = 0;
            }
        }
    }	
    
    
    
    // Pour avoir les centres des clusters
    for (auto& cluster : clusterCentres) {
        int x = cluster.x;
        int y = cluster.y;
        imOut[x * 3][y * 3] = 255;
        imOut[x * 3][y * 3 + 1] = 255;
        imOut[x * 3][y * 3 + 2] = 255;
    }
    
}
int main(int argc, char **argv)
{
	char cNomImgLue[250], cNomImgEcrite[250], cNomImgEcrite2[250];
	int K, m, n, nbIter;

	if (argc != 8) 
	{
		printf("Usage: ImageIn.pgm ImageOut.pgm ImageOutwithContours.pgm  Seuil \n"); 
		return 1;
	}
	sscanf (argv[1],"%s", cNomImgLue) ;
	sscanf (argv[2],"%s", cNomImgEcrite);
    sscanf (argv[3],"%s", cNomImgEcrite2);
	// Nombre de superpixels
	sscanf (argv[4],"%d", &K);
	// Compacité
	sscanf (argv[5],"%d", &m);
	// vosisnage pertubation
	sscanf (argv[6],"%d", &n);
	// Nombre d'itérations = treshold
	sscanf (argv[7],"%d", &nbIter);
		
	ImageBase imIn;
	imIn.load(cNomImgLue);

	ImageBase imOut(imIn.getWidth(), imIn.getHeight(), imIn.getColor());


	// Nombre de pixels de image
	int N = imIn.getWidth() * imIn.getHeight();
	//std::cout << N << std::endl;

	// Taille de chaque superpixels
	double tailleSP = static_cast<double>(N)/static_cast<double>(K);
	// Distance entre chaque superpixel
	double S = sqrt(tailleSP);

	// Liste des centres des clusters
	std::vector<Cluster> clusterCentres;

	// Liste des couleurs palettes
	std::vector<Palette> superpixelColors;


	// Calcul du nombre de couleurs de l'image
	std::set<std::tuple<int, int, int>> CouleursUniques;

	for (int x = 0; x < imIn.getHeight(); x++) {
		for (int y = 0; y < imIn.getWidth(); y++) {
			int r = imIn[x*3][y*3];
			int g = imIn[x*3][y*3+1];
			int b = imIn[x*3][y*3+2];
			
			CouleursUniques.insert(std::make_tuple(r, g, b));
		}
	}

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
	for(int x = (int)S; x < imOut.getHeight() - (int)S; x+=(int)S) {
		for(int y = (int)S; y < imOut.getWidth() - (int)S; y+=(int)S) {
			Cluster p = {x, y, imOut[x*3][y*3], imOut[x*3][y*3+1], imOut[x*3][y*3+2]};
			clusterCentres.push_back(p);

			// Compression palette
			Palette color = {imOut[x*3][y*3], imOut[x*3][y*3+1], imOut[x*3][y*3+2]};
			superpixelColors.push_back(color);
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

								/*
				superpixelImage[x * 3][y * 3] = nearestCluster.L;
				superpixelImage[x * 3][y * 3 + 1] = nearestCluster.a;
				superpixelImage[x * 3][y * 3 + 2] = nearestCluster.b;
				*/
				// Avec palette

				superpixelImage[x * 3][y * 3] = superpixelColors[minClusterIndex].r;
				superpixelImage[x * 3][y * 3 + 1] = superpixelColors[minClusterIndex].g;
				superpixelImage[x * 3][y * 3 + 2] = superpixelColors[minClusterIndex].b;
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
    ImageBase superpixelImage_withContour(imIn.getWidth(), imIn.getHeight(), true);
    Contour_image_et_centre(superpixelImage , superpixelImage_withContour  , clusterCentres);


	// Affichage du nombre de couleurs uniques et du nombre de couleurs dans la palette de superpixels
	std::cout << "Nombre de couleurs uniques : " << CouleursUniques.size() << std::endl;
	std::cout << "Taille de la palette de superpixels : " << superpixelColors.size() << std::endl;

	// Calcul de la taille de l'image originale
	uint long tailleOriginale = (uint long)imIn.getWidth() * (uint long)imIn.getHeight() * (uint long)CouleursUniques.size();
	// Calcul de la taille de l'image compressée avec la compression de palette
	long double tailleCompresséePalette = (long double)imIn.getWidth() * (long double)imIn.getHeight() * (long double)superpixelColors.size();

	// Calcul du taux de compression
	long double tauxCompressionPalette = (long double)tailleOriginale / (long double)tailleCompresséePalette;
	std::cout << "Taille originale : " << tailleOriginale << std::endl;
	std::cout << "Taux de compression avec compression de palette : " << tauxCompressionPalette << std::endl;
	std::cout << "PSNR: " << psnr(imIn , superpixelImage) << std::endl;
	superpixelImage.save(cNomImgEcrite);

	superpixelImage_withContour.save(cNomImgEcrite2);
  
}
