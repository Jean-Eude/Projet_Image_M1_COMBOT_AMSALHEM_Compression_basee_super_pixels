#include "ImageBase.h"
#include <stdio.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <set>
#include <tuple>
#include <vector>
#include <fstream>

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


int moyenne(std::vector<int> v){
    double sum;
    for ( int i : v)
        sum += i;
    sum  /= v.size();
    return (int)sum;
} 

void moyenneur(ImageBase & imIn , ImageBase & imMoy) {
     
    for(int x = 0 ; x < imIn.getHeight(); x++){
        for(int y = 0 ; y < imIn.getWidth()  ; y++){
                             
            int _x = std::max(x - 1 , 0);
            int _y = std::max(y - 1 , 0);
            
            int x_ = std::min(x + 1 , imIn.getHeight() - 1);
            int y_ = std::min(y + 1 , imIn.getWidth() - 1);
            int pix_moy;
            if (_x == 0 and _y == 0)
                  pix_moy = moyenne({ imIn[x][y]  ,   imIn[x][y_], 
                                          imIn[x_][y],   imIn[x_][y_]}); 
                 
            else if (_x == 0 and y_ == imIn.getWidth() - 1)
                   pix_moy = moyenne({ imIn[x][_y]  ,   imIn[x][y]  ,   
                                        imIn[x_][_y],   imIn[x_][y]}); 
           
            else if (x_ == imIn.getHeight()  and _y == 0)
                  pix_moy = moyenne({   imIn[_x][y],   imIn[_x][y_],
                                          imIn[x][y]  ,   imIn[x][y_]}); 
           
            else if (x_ == imIn.getHeight() - 1  and _y == imIn.getWidth() - 1)
                  pix_moy = moyenne({imIn[_x][_y],   imIn[_x][y],
                                        imIn[x][_y]  ,   imIn[x][y]   }); 
           
            else if (_x == 0 )
                  pix_moy = moyenne({
                                        imIn[x][_y]  ,   imIn[x][y]  ,   imIn[x][y_], 
                                        imIn[x_][_y],   imIn[x_][y],   imIn[x_][y_]}); 
           
            else if (_y == 0 )
                  pix_moy = moyenne({  imIn[_x][y],   imIn[_x][y_],
                                           imIn[x][y]  ,   imIn[x][y_], 
                                         imIn[x_][y],   imIn[x_][y_]}); 
           
            else if (x_ == imIn.getHeight() - 1)
                  pix_moy = moyenne({imIn[_x][_y],   imIn[_x][y],   imIn[_x][y_],
                                        imIn[x][_y]  ,   imIn[x][y]  ,   imIn[x][y_], 
                                        }); 
           
            else if (y_ == imIn.getWidth() - 1)
                  pix_moy = moyenne({imIn[_x][_y],   imIn[_x][y],  
                                        imIn[x][_y]  ,   imIn[x][y] ,  
                                        imIn[x_][_y],   imIn[x_][y],   }); 
           
            else
             pix_moy = moyenne({imIn[_x][_y],   imIn[_x][y],   imIn[_x][y_],
                                        imIn[x][_y]  ,   imIn[x][y]  ,   imIn[x][y_], 
                                        imIn[x_][_y],   imIn[x_][y],   imIn[x_][y_]}); 
           
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
            
            int x_ = std::min(x + 1 , imIn.getHeight() - 1);
            int y_ = std::min(y + 1 , imIn.getWidth() - 1);
            
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

double PSNR(ImageBase & imIn , ImageBase & imOut) {
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

    return PSNR;
}


// Entropie (RGB)
double Entropy(ImageBase& image, const char* fileName) {
    int width = image.getWidth();
    int height = image.getHeight();
    int totalPixels = width * height;

	int* histoR = new int[256](); 
	int* histoG = new int[256]();
	int* histoB = new int[256]();

    for (int i = 0; i < image.getWidth(); i++) {
        for (int j = 0; j < image.getHeight(); j++) {
			histoR[image[i*3][j*3]]++;
            histoG[image[i*3][j*3+1]]++;
            histoB[image[i*3][j*3+2]]++;
        }    
    }

    std::ofstream outFile(fileName);
    if (outFile.is_open()) {
        for (int i = 0; i < 256; i++) {
            outFile << i << "\t" << histoR[i] << "\t" << histoG[i] << "\t" << histoB[i] << "\n";
        }
        outFile.close();
        std::cout << "Les histogrammes ont été écrits dans le fichier avec succès." << std::endl;
    } else {
        std::cerr << "Erreur lors de l'ouverture du fichier." << std::endl;
    }


    // Calcul des probabilités et de l'entropie
    double entropyR = 0.0;
    double entropyG = 0.0;
    double entropyB = 0.0;

    for (int i = 0; i < 256; ++i) {
        if (histoR[i] != 0) {
            double pi = static_cast<double>(histoR[i]) / totalPixels;
            entropyR -= pi * log2(pi);
        }
    }

    for (int i = 0; i < 256; ++i) {
        if (histoG[i] != 0) {
            double pi = static_cast<double>(histoG[i]) / totalPixels;
            entropyG -= pi * log2(pi);
        }
    }

    for (int i = 0; i < 256; ++i) {
        if (histoB[i] != 0) {
            double pi = static_cast<double>(histoB[i]) / totalPixels;
            entropyB -= pi * log2(pi);
        }
    }

	delete[] histoR;
	delete[] histoG;
	delete[] histoB;

    return (entropyR + entropyG + entropyB) / 3;
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
	char cNomImgLue[250], cNomImgEcrite[250], cNomImgEcrite2[250], cNomImgEcrite3[250];
	int K, m, n, nbIter, comp, RGBouLAB;

	if (argc != 10) 
	{
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
	// Compression (0 : non - 1 : oui)
	sscanf (argv[7],"%s", cNomImgEcrite2);
	sscanf (argv[8],"%s", cNomImgEcrite3);
	sscanf (argv[9],"%d", &RGBouLAB);


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
	
	if(RGBouLAB == 1) {
		RGBtoLab(imOut, Lab, 'A');
	}


	// 2. Initialisation des clusters
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
	RGBtoLab(Lab, L, 'L');	


	// Entropie de l'image originale
	double entropie_originale = Entropy(imIn, "HistoRGB.dat");
	std::cout << "Entropie de l'image originale : " << entropie_originale << " bits/pixel = " << ceil(entropie_originale) << " bits/pixel (entier supérieur)" << std::endl; 


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


	// Initialisation de la palette (couleurs uniques)
	std::vector<Palette> palCouleurs;

	for (Palette c : superpixelColors) {
		bool couleurUnique = true;
		for (Palette d : palCouleurs) {
			if (c.r == d.r && c.g == d.g && c.b == d.b) {
				couleurUnique = false;
				break;
			}
		}
		if (couleurUnique) {
			palCouleurs.push_back(c);
			//std::cout << c.r << " " << c.g << " " << c.b << std::endl;
		}
	}
	
	// Créer de la palette d'indice
	ImageBase paletteIndice(int(sqrt(palCouleurs.size()) + 0.5), int(sqrt(palCouleurs.size()) + 0.5), imIn.getColor());	

	for(int x = 0; x < paletteIndice.getHeight(); x+=1) {
		for(int y = 0; y < paletteIndice.getWidth(); y+=1) {
			paletteIndice[x*3][y*3] = palCouleurs[x * paletteIndice.getWidth() + y].r;
			paletteIndice[x*3][y*3+1] = palCouleurs[x * paletteIndice.getWidth() + y].g;
			paletteIndice[x*3][y*3+2] = palCouleurs[x * paletteIndice.getWidth() + y].b;
		}
	}

	paletteIndice.save("testPal.ppm");


	// Rendu des indices
	ImageBase indices(superpixelImage.getWidth(), superpixelImage.getHeight(), false);	
	for(int x = 0; x < superpixelImage.getHeight(); x+=1) {
		for(int y = 0; y < superpixelImage.getWidth() ; y+=1) {
			for(int k = 0 ; k < paletteIndice.getHeight() ; k++){
				for(int z = 0 ; z < paletteIndice.getWidth() ; z++){
					if(superpixelImage[x*3][y*3]== paletteIndice[k*3][z*3] && 
					superpixelImage[x*3][y*3+1]== paletteIndice[k*3][z*3+1] &&
					superpixelImage[x*3][y*3+2]== paletteIndice[k*3][z*3+2])
					{
						indices[x][y] = round(((k * paletteIndice.getWidth() + z) * 255) / palCouleurs.size());
					}
				}
			}
		}
	}
	indices.save("indice.pgm");


	std::vector<int> index;
	index.resize(superpixelImage.getWidth() * superpixelImage.getHeight());
	for(int x = 0; x < superpixelImage.getHeight(); x+=1) {
		for(int y = 0; y < superpixelImage.getWidth() ; y+=1) {
			for(int k = 0 ; k < paletteIndice.getHeight() ; k++){
				for(int z = 0 ; z < paletteIndice.getWidth() ; z++){
					if(superpixelImage[x*3][y*3]== paletteIndice[k*3][z*3] && 
					superpixelImage[x*3][y*3+1]== paletteIndice[k*3][z*3+1] &&
					superpixelImage[x*3][y*3+2]== paletteIndice[k*3][z*3+2])
					{
						index[x * superpixelImage.getWidth() + y] = k * paletteIndice.getWidth() + z;
					}
				}
			}
		}
	}

	// Codage par plage
	std::vector<int> codageplageIndex;
	int cpt = 0;
	for(int i = 0; i < index.size() - 1; i++) {
		if(index[i+1] != index[i]) {
			codageplageIndex.push_back(cpt);
			codageplageIndex.push_back(index[i]);
		} else {
			cpt++;
		}
	}

	std::cout << codageplageIndex.size() << std::endl;

	// Entropie de l'image superpixelisé
	double entropie_superpixels = Entropy(superpixelImage, "HistoRGB_SP.dat");
	std::cout << "Entropie de l'image superpixelisée : " << entropie_superpixels << " bits/pixel = " << ceil(entropie_superpixels) << " bits/pixel (entier supérieur)" << std::endl; 


	// Affichage du nombre de couleurs uniques et du nombre de couleurs dans la palette de superpixels
	std::cout << "Taille de la palette de superpixels : " << palCouleurs.size() << std::endl;

	// Calcul de la taille de l'image originale
	uint long tailleOriginale = sizeof(imIn.getData()) * imIn.getHeight() * imIn.getWidth();
	// Calcul de la taille de l'image compressée avec la compression de palette
	double tailleCompresséePalette = (sizeof(int)) * codageplageIndex.size() + sizeof(paletteIndice.getData()) * palCouleurs.size();

	// Calcul du taux de compression
	double tauxCompressionPalette = (double)tailleOriginale / (double)tailleCompresséePalette;
	std::cout << "Taille originale : " << tailleOriginale << std::endl;
	std::cout << "Taille compressé : " << tailleCompresséePalette << std::endl;
	std::cout << "Taux de compression avec compression palette : " << tauxCompressionPalette << std::endl;

	double psnrVal = PSNR(imIn , superpixelImage);
	std::cout << "PSNR: " << psnrVal << " dB" << std::endl;


	const char *psnr = "PSNR.dat";
    std::ofstream outFilePSNR(psnr);
    if (outFilePSNR.is_open()) {
        outFilePSNR << psnrVal << "\n";
        outFilePSNR.close();
    } else {
        std::cerr << "Erreur lors de l'ouverture du fichier." << std::endl;
    }

	const char *entroAvant = "EntroAvant.dat";
    std::ofstream outFileEA(entroAvant);
    if (outFileEA.is_open()) {
        outFileEA << entropie_originale << "\n";
        outFileEA.close();
    } else {
        std::cerr << "Erreur lors de l'ouverture du fichier." << std::endl;
    }

	const char *entroApres = "EntroApres.dat";
    std::ofstream outFileEAp(entroApres);
    if (outFileEAp.is_open()) {
        outFileEAp << entropie_superpixels << "\n";
        outFileEAp.close();
    } else {
        std::cerr << "Erreur lors de l'ouverture du fichier." << std::endl;
    }

	const char *TDC = "tdc.dat";
    std::ofstream outFileTDC(TDC);
    if (outFileTDC.is_open()) {
        outFileTDC << tauxCompressionPalette << "\n";
        outFileTDC.close();
    } else {
        std::cerr << "Erreur lors de l'ouverture du fichier." << std::endl;
    }


	superpixelImage.save(cNomImgEcrite);

	Contour_image_et_centre(superpixelImage, superpixelImage, clusterCentres);
	Gradient.save(cNomImgEcrite2);
	superpixelImage.save(cNomImgEcrite3);
}