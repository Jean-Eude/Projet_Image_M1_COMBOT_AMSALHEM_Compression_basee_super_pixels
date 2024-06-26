#include <Window.h>
#include <GUI.h>
#include <ImageBase.h>

#include <string>
#include <iostream>
#include <vector>
#include <utility>
#include <cmath>


// Lecture
float readDatafromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erreur : Impossible d'ouvrir le fichier " << filename << std::endl;
        return 0.0f; 
    }
    std::string line;
    if (!std::getline(file, line)) {
        std::cerr << "Erreur : Aucune donnée lue dans le fichier " << filename << std::endl;
        return 0.0f; 
    }
    float psnrValue;
    std::istringstream iss(line);
    if (!(iss >> psnrValue)) {
        std::cerr << "Erreur : Format de données incorrect dans le fichier " << filename << std::endl;
        return 0.0f; 
    }
    return psnrValue;
}

void InterNN(double imgX, double imgY, ImageBase& imIn, uint8_t* r, uint8_t* g, uint8_t* b) {
    int xNearest = static_cast<int>(round(imgX));
    int yNearest = static_cast<int>(round(imgY));

    xNearest = std::max(0, std::min(xNearest, imIn.getWidth() - 1));
    yNearest = std::max(0, std::min(yNearest, imIn.getHeight() - 1));

    *r = imIn[yNearest * 3][xNearest * 3];
    *g = imIn[yNearest * 3][xNearest * 3 + 1];
    *b = imIn[yNearest * 3][xNearest * 3 + 2];
}

void InterBill(double imgX, double imgY, int size, ImageBase& imIn, uint8_t* r, uint8_t* g, uint8_t* b) {
    double imgX_original = static_cast<double>(imgX) / static_cast<double>(size) * imIn.getWidth();
    double imgY_original = static_cast<double>(imgY) / static_cast<double>(size) * imIn.getHeight();

    int x1 = static_cast<int>(imgX_original);
    int y1 = static_cast<int>(imgY_original);
    int x2 = std::min(x1 + 1, imIn.getWidth() - 1);
    int y2 = std::min(y1 + 1, imIn.getHeight() - 1);

    double dx = imgX_original - x1;
    double dy = imgY_original - y1;

    uint8_t p1_r = imIn[y1 * 3][x1 * 3];
    uint8_t p1_g = imIn[y1 * 3][x1 * 3 + 1];
    uint8_t p1_b = imIn[y1 * 3][x1 * 3 + 2];

    uint8_t p2_r = imIn[y1 * 3][x2 * 3];
    uint8_t p2_g = imIn[y1 * 3][x2 * 3 + 1];
    uint8_t p2_b = imIn[y1 * 3][x2 * 3 + 2];

    uint8_t p3_r = imIn[y2 * 3][x1 * 3];
    uint8_t p3_g = imIn[y2 * 3][x1 * 3 + 1];
    uint8_t p3_b = imIn[y2 * 3][x1 * 3 + 2];

    uint8_t p4_r = imIn[y2 * 3][x2 * 3];
    uint8_t p4_g = imIn[y2 * 3][x2 * 3 + 1];
    uint8_t p4_b = imIn[y2 * 3][x2 * 3 + 2];

    *r = static_cast<uint8_t>((1 - dx) * (1 - dy) * p1_r + dx * (1 - dy) * p2_r + (1 - dx) * dy * p3_r + dx * dy * p4_r);
    *g = static_cast<uint8_t>((1 - dx) * (1 - dy) * p1_g + dx * (1 - dy) * p2_g + (1 - dx) * dy * p3_g + dx * dy * p4_g);
    *b = static_cast<uint8_t>((1 - dx) * (1 - dy) * p1_b + dx * (1 - dy) * p2_b + (1 - dx) * dy * p3_b + dx * dy * p4_b);
}

#pragma omp parallel
void draw_Pixel(uint8_t r, uint8_t g, uint8_t b, int x, int y, uint32_t *buffer, int width, int height) {
    uint8_t ir = r;
    uint8_t ig = g;
    uint8_t ib = b;

    uint32_t pixel = (ir << 16) | (ig << 8) | ib;
    buffer[y * width + x] = pixel;
}
#pragma omp end

int main(int argc, char* argv[]) {
    int width = 1280, height = 720;
    bool imageModif = true;
    bool launchAlgo = false;
    bool boolSP = false;
    int size = 256;
    int previousSize = size;
    uint8_t r = 255, g = 255, b = 255; 

    ImageBase imIn;
    bool imageLoaded = false;
    ImageBase afterSLIC;


    Window window("Projet Image - Compression basee superpixels", width, height);
    GUI gui(window);


    void* pixels;
    int pitch;
    uint32_t* buffer;
    char inputBuffer[256] = ""; 
    gui.setupGUI();


    // Toutes variables
    int squareHeight = 0;
    int offset = 0;
    int squareWidth1 = 0;
    int squareWidth2 = 0;
    int imgX = 0;
    int imgY = 0;
    int startX = 0;
    int endX = 0;
    int startY = 0;
    int endY = 0;
    double imgXRatio = 0.;
    double imgYRatio = 0.;

    // Méthodes d'interpolation
    bool interNN = true;
    bool interBi = false;
    int curr_Redim = 0;

    // Algo superpixels
    bool algoRien = true;
    bool algoSlic = false;
    int curr_Algo = 0;

    // Méthodes de compression
    bool compRien = true;
    bool compPal = true;
    int curr_comp = 0;

    // Espace couleur
    bool spaceRGB = true;
    bool spaceLAB = true;
    int curr_space = 0;

    // Images
    bool imgNorm = true;
    bool imgGrad = false;
    bool imgCtr = false;
    int curr_Img = 0; 

    // Saves
    bool once = false;

    // Nombre de pixels de image
    int N = 0;
    // Taille de chaque superpixels
    double tailleSP = 0.;
    // Distance entre chaque superpixel
    int S = 0;


    // Nombre de superpixels
    int K = 100;
    // compacité
    int m = 10;
    // Voisinage
    int n = 3;  // 3 de base
    int nbIter = 1;

    int nbMaxK = 10000;
    int nbMaxm = 1000;
    int nbMaxn = 10;
    int nbMaxIter = 10;

    
    int imgX_N = 0;
    int imgY_N = 0;
    int imgx = 0;
    int imgy = 0;
    int prevK = 0;
    int prevm = 0;
    int prevn = 0;
    int prevIter = 0;

    // Métriques
    float PSNR = .0f;
    float EntropieAvant = .0f;
    float EntropieApres = .0f;
    float tdc = .0f;


    while (!window.getDone()) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                window.setDone(true);
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window.getWindow()))
                window.setDone(true);
        }

        squareHeight = (height - size) / 2;
        offset = (width - 2 * size) / 4;

        squareWidth1 = offset;
        squareWidth2 = width / 2 + offset;


        if (imageModif) {
            window.destroyTexture();
            window.createTexture(SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);

            SDL_LockTexture(window.getTexture(), NULL, &pixels, &pitch);
            buffer = static_cast<uint32_t*>(pixels);

            for (int x = squareWidth1; x < squareWidth1 + size; x++) {
                for (int y = squareHeight; y < squareHeight + size; y++) {
                    imgX = x - squareWidth1;
                    imgY = y - squareHeight;


                    if(imageLoaded == false) {
                        draw_Pixel(255, 255, 255, x, y, buffer, width, height);
                    } else {
                        if (interNN == true) {
                            InterNN(static_cast<double>(imgX) / size * imIn.getWidth(), static_cast<double>(imgY) / size * imIn.getHeight(), imIn, &r, &g, &b);
                            draw_Pixel(r, g, b, x, y, buffer, width, height);                        
                        } else if(interBi == true) {
                            InterBill(static_cast<double>(imgX), static_cast<double>(imgY), size, imIn, &r, &g, &b);
                            draw_Pixel(r, g, b, x, y, buffer, width, height);
                        }
                    }
                }
            }

            startX = squareWidth2;
            endX = squareWidth2 + size;
            startY = squareHeight;
            endY = squareHeight + size;

            for (int x = startX; x < endX; x++) {
                imgX = x - squareWidth2;
                imgXRatio = static_cast<double>(imgX) / size * imIn.getWidth();

                for (int y = startY; y < endY; y++) {
                    int imgY = y - squareHeight;
                    imgYRatio = static_cast<double>(imgY) / size * imIn.getHeight();

                    if (imgX >= 0 && imgX < size && imgY >= 0 && imgY < size) {
                        if(imageLoaded == false) {
                            draw_Pixel(255, 255, 255, x, y, buffer, width, height);
                        } else {
                            if (interNN == true) {
                                InterNN(static_cast<double>(imgX) / size * imIn.getWidth(), static_cast<double>(imgY) / size * imIn.getHeight(), imIn, &r, &g, &b);
                                
                                // Partie algo SP
                                if(algoRien == true) {
                                    draw_Pixel(r, g, b, x, y, buffer, width, height);   
                                    launchAlgo = false;
                                    boolSP = false;
                                } 
                                
                                if(algoSlic == true) {
                                    if(launchAlgo == true) {
                                        ImageBase imOut_Save(size, size, imIn.getColor());
                                        for (int py = squareHeight; py < squareHeight + size; py++) {
                                            for (int px = squareWidth1; px < squareWidth1 + size; px++) {
                                                int imgXa = px - squareWidth1;
                                                int imgYa = py - squareHeight;
                                                if (imgXa >= 0 && imgXa < size && imgYa >= 0 && imgYa < size) {
                                                    uint32_t pixel = buffer[py * width + px];  
                                                    uint8_t r_s = (pixel >> 16) & 0xFF;     
                                                    uint8_t g_s = (pixel >> 8) & 0xFF; 
                                                    uint8_t b_s = pixel & 0xFF; 
                                                    imOut_Save[imgYa * 3][imgXa * 3] = r_s;
                                                    imOut_Save[imgYa * 3][imgXa * 3 + 1] = g_s;
                                                    imOut_Save[imgYa * 3][imgXa * 3 + 2] = b_s;
                                                }
                                            }
                                        }
                                        imOut_Save.save("avantSLIC.ppm");
                                        std::string commandeCompilation;
                                        if(spaceRGB == true) {
                                            commandeCompilation = "g++ ../others/SLIC.cpp ../others/ImageBase.cpp -Ofast && ./a.out ../bin/avantSLIC.ppm apresSLIC.ppm " + std::to_string(K) + " " + std::to_string(m) + " " + std::to_string(n) + " " + std::to_string(nbIter) + " " + "Gradient.pgm" + " " + "Centres.ppm" + " " + std::to_string(0);
                                        } 
                                        if(spaceLAB == true) {
                                            commandeCompilation = "g++ ../others/SLIC.cpp ../others/ImageBase.cpp -Ofast && ./a.out ../bin/avantSLIC.ppm apresSLIC.ppm " + std::to_string(K) + " " + std::to_string(m) + " " + std::to_string(n) + " " + std::to_string(nbIter) + " " + "Gradient.pgm" + " " + "Centres.ppm" + " " + std::to_string(1);
                                        }                                        
                                        int resultatCompilation = system(commandeCompilation.c_str());
                                        launchAlgo = false;
                                        boolSP = true;
                                    }

                                    if(boolSP == true) {
                                        launchAlgo = false;                 
                                        imgx = x - squareWidth2;
                                        imgy = y - squareHeight;

                                        if(imgNorm == true) {
                                            afterSLIC.load("apresSLIC.ppm");
                                            if (imgx >= 0 && imgx < afterSLIC.getWidth() && imgy >= 0 && imgy < afterSLIC.getHeight()) {
                                                r = afterSLIC[imgy * 3][imgx * 3];
                                                g = afterSLIC[imgy * 3][imgx * 3 + 1];
                                                b = afterSLIC[imgy * 3][imgx * 3 + 2];
                                            }                                      
                                            draw_Pixel(r, g, b, x, y, buffer, width, height);
                                        }
                                        if(imgGrad == true) {
                                            afterSLIC.load("Gradient.pgm");
                                            if (imgx >= 0 && imgx < afterSLIC.getWidth() && imgy >= 0 && imgy < afterSLIC.getHeight()) {
                                                r = afterSLIC[imgy][imgx];
                                                g = afterSLIC[imgy][imgx];
                                                b = afterSLIC[imgy][imgx];
                                            }                                      
                                            draw_Pixel(r, g, b, x, y, buffer, width, height);
                                        }
                                        if(imgCtr == true) {
                                            afterSLIC.load("Centres.ppm");
                                            if (imgx >= 0 && imgx < afterSLIC.getWidth() && imgy >= 0 && imgy < afterSLIC.getHeight()) {
                                                r = afterSLIC[imgy * 3][imgx * 3];
                                                g = afterSLIC[imgy * 3][imgx * 3 + 1];
                                                b = afterSLIC[imgy * 3][imgx * 3 + 2];
                                            }                                      
                                            draw_Pixel(r, g, b, x, y, buffer, width, height);
                                        }
                                        
                                        boolSP == false;                       
                                    } else {
                                        launchAlgo = false; 
                                        draw_Pixel(r, g, b, x, y, buffer, width, height);
                                        boolSP = false;  
                                    }
                                }

                                launchAlgo = false;
                            } else if(interBi == true) {
                                InterBill(static_cast<double>(imgX), static_cast<double>(imgY), size, imIn, &r, &g, &b);
                                
                                
                                // Partie algo SP
                                if(algoRien == true) {
                                    draw_Pixel(r, g, b, x, y, buffer, width, height);   
                                    launchAlgo = false;
                                    boolSP = false;
                                } 
                                
                                if(algoSlic == true) {
                                    if(launchAlgo == true) {
                                        ImageBase imOut_Save(size, size, imIn.getColor());
                                        for (int py = squareHeight; py < squareHeight + size; py++) {
                                            for (int px = squareWidth1; px < squareWidth1 + size; px++) {
                                                int imgXa = px - squareWidth1;
                                                int imgYa = py - squareHeight;
                                                if (imgXa >= 0 && imgXa < size && imgYa >= 0 && imgYa < size) {
                                                    uint32_t pixel = buffer[py * width + px];  
                                                    uint8_t r_s = (pixel >> 16) & 0xFF;     
                                                    uint8_t g_s = (pixel >> 8) & 0xFF; 
                                                    uint8_t b_s = pixel & 0xFF; 
                                                    imOut_Save[imgYa * 3][imgXa * 3] = r_s;
                                                    imOut_Save[imgYa * 3][imgXa * 3 + 1] = g_s;
                                                    imOut_Save[imgYa * 3][imgXa * 3 + 2] = b_s;
                                                }
                                            }
                                        }
                                        imOut_Save.save("avantSLIC.ppm");
                                        std::string commandeCompilation;
                                        if(spaceRGB == true) {
                                            commandeCompilation = "g++ ../others/SLIC.cpp ../others/ImageBase.cpp -Ofast && ./a.out ../bin/avantSLIC.ppm apresSLIC.ppm " + std::to_string(K) + " " + std::to_string(m) + " " + std::to_string(n) + " " + std::to_string(nbIter) + " " + "Gradient.pgm" + " " + "Centres.ppm" + " " + std::to_string(0);
                                        } 
                                        if(spaceLAB == true) {
                                            commandeCompilation = "g++ ../others/SLIC.cpp ../others/ImageBase.cpp -Ofast && ./a.out ../bin/avantSLIC.ppm apresSLIC.ppm " + std::to_string(K) + " " + std::to_string(m) + " " + std::to_string(n) + " " + std::to_string(nbIter) + " " + "Gradient.pgm" + " " + "Centres.ppm" + " " + std::to_string(1);
                                        }                                        
                                        int resultatCompilation = system(commandeCompilation.c_str());
                                        launchAlgo = false;
                                        boolSP = true;
                                    }

                                    if(boolSP == true) {
                                        launchAlgo = false;                 
                                        imgx = x - squareWidth2;
                                        imgy = y - squareHeight;

                                        if(imgNorm == true) {
                                            afterSLIC.load("apresSLIC.ppm");
                                            if (imgx >= 0 && imgx < afterSLIC.getWidth() && imgy >= 0 && imgy < afterSLIC.getHeight()) {
                                                r = afterSLIC[imgy * 3][imgx * 3];
                                                g = afterSLIC[imgy * 3][imgx * 3 + 1];
                                                b = afterSLIC[imgy * 3][imgx * 3 + 2];
                                            }                                      
                                            draw_Pixel(r, g, b, x, y, buffer, width, height);
                                        }
                                        if(imgGrad == true) {
                                            afterSLIC.load("Gradient.pgm");
                                            if (imgx >= 0 && imgx < afterSLIC.getWidth() && imgy >= 0 && imgy < afterSLIC.getHeight()) {
                                                r = afterSLIC[imgy][imgx];
                                                g = afterSLIC[imgy][imgx];
                                                b = afterSLIC[imgy][imgx];
                                            }                                      
                                            draw_Pixel(r, g, b, x, y, buffer, width, height);
                                        }
                                        if(imgCtr == true) {
                                            afterSLIC.load("Centres.ppm");
                                            if (imgx >= 0 && imgx < afterSLIC.getWidth() && imgy >= 0 && imgy < afterSLIC.getHeight()) {
                                                r = afterSLIC[imgy * 3][imgx * 3];
                                                g = afterSLIC[imgy * 3][imgx * 3 + 1];
                                                b = afterSLIC[imgy * 3][imgx * 3 + 2];
                                            }                                      
                                            draw_Pixel(r, g, b, x, y, buffer, width, height);
                                        }
                                        
                                        boolSP == false;                       
                                    } else {
                                        launchAlgo = false; 
                                        draw_Pixel(r, g, b, x, y, buffer, width, height);
                                        boolSP = false;  
                                    }
                                }

                                launchAlgo = false;
                            }
                        }
                    }                
                }
            } 

            SDL_UnlockTexture(window.getTexture());
            imageModif = false;
        }

        gui.beforeUpdate();

        ImGui::Begin("Paramètres");
        ImGui::Text("Paramètres généraux :");
        ImGui::Spacing();
        ImGui::InputText("Nom du fichier", inputBuffer, sizeof(inputBuffer));
        previousSize = size;

        if (ImGui::SliderInt("Taille", &size, 0, width/2)) {
            imageModif = true;
            launchAlgo = false;
            boolSP = false;
        }
        ImGui::Spacing();
        if (ImGui::InputInt("// Taille", &size, 1)) {
            if(size > width/2) {
                size = width/2;
            } else if(size < 0) {
                size = 0;
            }
            
            imageModif = true;
            launchAlgo = false;
            boolSP = false;
        }
    
        if (ImGui::Button("Load")) {
            IGFD::FileDialogConfig cfg;
            cfg.path = ".";
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Ouvrir une image", ".ppm", cfg);
        }
        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
            if (ImGuiFileDialog::Instance()->IsOk()) { 
                std::string cheminNom = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string cheminAccess = ImGuiFileDialog::Instance()->GetCurrentPath();
                SDL_LockTexture(window.getTexture(), NULL, &pixels, &pitch);
                uint32_t* buffer = static_cast<uint32_t*>(pixels);

                std::cout << (char*)cheminNom.c_str() << std::endl;
                imIn.load((char*)cheminNom.c_str());
                imageLoaded = true;
                imageModif = true; 
            }
            ImGuiFileDialog::Instance()->Close();
        }

        ImGui::SameLine();

        if (ImGui::Button("Save")) {
            ImageBase imOut_Save(size, size, imIn.getColor());

            SDL_LockTexture(window.getTexture(), NULL, &pixels, &pitch);
            uint32_t* buffer = static_cast<uint32_t*>(pixels);

            for (int x = squareWidth2; x < squareWidth2 + size; x++) {
                for (int y = squareHeight; y < squareHeight + size; y++) {
                    int imgX = x - squareWidth2;
                    int imgY = y - squareHeight;

                    if (imgX >= 0 && imgX < size && imgY >= 0 && imgY < size) {
                        uint32_t pixel = buffer[y * width + x];  
                        uint8_t r_s = (pixel >> 16) & 0xFF;     
                        uint8_t g_s = (pixel >> 8) & 0xFF; 
                        uint8_t b_s = pixel & 0xFF; 

                        imOut_Save[imgY * 3][imgX * 3] = r_s;
                        imOut_Save[imgY * 3][imgX * 3 + 1] = g_s;
                        imOut_Save[imgY * 3][imgX * 3 + 2] = b_s;
                    }
                }
            }
        
            SDL_UnlockTexture(window.getTexture());
            
            if (strlen(inputBuffer) == 0) {
                imOut_Save.save("1.ppm");
            } else {
                imOut_Save.save(inputBuffer);
            }
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Méthodes d'interpolation :");
        const char* itemsRedim[] = { "Plus proche voisins", "Interpolation billinéaire"};
        if(ImGui::Combo("Algorithme RD", &curr_Redim, itemsRedim, IM_ARRAYSIZE(itemsRedim))) {
            std::string selected_item(itemsRedim[curr_Redim]);

            if(curr_Redim == 0) {
                interNN = true;
                interBi = false;

                launchAlgo = false;
                boolSP = false;
            } else if(curr_Redim == 1) {
                interNN = false;
                interBi = true;
            
                launchAlgo = false;
                boolSP = false;
            }
            imageModif = true;
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Algorithmes superpixels :");
        const char* itemsAlgo[] = { "Rien", "SLIC"};
        if(ImGui::Combo("Algorithme SP", &curr_Algo, itemsAlgo, IM_ARRAYSIZE(itemsAlgo))) {
            std::string selected_item(itemsAlgo[curr_Algo]);

            if(curr_Algo == 0) {
                algoRien = true;
                algoSlic = false;

                launchAlgo = false;
                boolSP = false;
            } else if(curr_Algo == 1) { 
                algoRien = false;
                algoSlic = true; 

                launchAlgo = false;
                boolSP = false;
            }
        }
        ImGui::Spacing();

        if(algoSlic) {
            prevK = K;
            prevm = m;
            prevn = n;
            prevIter = nbIter;
            ImGui::SliderInt("Nombre de superpixels", &K, 1, nbMaxK);
            if(ImGui::InputInt("// Nombre de superpixels", &K, 1)) {
                if(K > nbMaxK) {
                    K = nbMaxK;
                } else if(K < 0) {
                    K = 0;
                }
            }

            ImGui::SliderInt("Compacité", &m, 1, nbMaxm);
            if(ImGui::InputInt("// Compacité", &m, 1)) {
               if(m > nbMaxm) {
                    m = nbMaxm;
                } else if(m < 0) {
                    m = 0;
                }
            }
            ImGui::SliderInt("Voisinage (Gradient)", &n, 1, nbMaxn);
            ImGui::SliderInt("Nombre d'itérations", &nbIter, 1, nbMaxIter);

            const char* itemsSpace[] = { "RGB", "La*b*"};
            if(ImGui::Combo("Espace couleur", &curr_space, itemsSpace, IM_ARRAYSIZE(itemsSpace))) {
                std::string selected_item(itemsSpace[curr_space]);

                if(curr_space == 0) {
                    spaceRGB = true;
                    spaceLAB = false;
                } else if(curr_space == 1) { 
                    spaceRGB = false;
                    spaceLAB = true;
                }
            }

            const char* itemsImage[] = { "Basique", "Gradient", "Centres et contours"};
            if(ImGui::Combo("Type de l'image", &curr_Img, itemsImage, IM_ARRAYSIZE(itemsImage))) {
                std::string selected_item(itemsImage[curr_Img]);

                if(curr_Img == 0) {
                    imgNorm = true;
                    imgGrad = false;
                    imgCtr = false;
                } else if(curr_Img == 1) { 
                    imgNorm = false;
                    imgGrad = true;
                    imgCtr = false;
                } else if(curr_Img == 2) { 
                    imgNorm = false;
                    imgGrad = false;
                    imgCtr = true;
                }
            }
        }

        ImGui::Spacing();
        if(ImGui::Button("Lancer l'algorithme")) {
            launchAlgo = true;
            imageModif = true;
        }
        ImGui::Spacing();
        ImGui::Separator();

        ImGui::Text("Méthodes de compression :");
        const char* itemsComp[] = { "Rien", "Compression palette"};
        if(ImGui::Combo("Algorithmes Comp", &curr_comp, itemsComp, IM_ARRAYSIZE(itemsComp))) {
            std::string selected_item(itemsComp[curr_comp]);

            if(curr_comp == 0) {
                compRien = true;
                compPal = false;
            } else if(curr_Algo == 1) { 
                compRien = false;
                compPal = true;
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Métriques :");
        ImGui::Spacing();
        PSNR = readDatafromFile("PSNR.dat");
        EntropieAvant = readDatafromFile("EntroAvant.dat");
        EntropieApres = readDatafromFile("EntroApres.dat");
        tdc = readDatafromFile("tdc.dat");

        ImGui::Text("PSNR = %f dB", PSNR);
        ImGui::Text("Entropie (image originale) = %.2f bits / pixel = %.2f bits / pixel (entier sup°)", EntropieAvant, ceil(EntropieAvant));
        ImGui::Text("Entropie (image superpixelisée) = %.2f bits / pixel = %.2f bits / pixel (entier sup°)", EntropieApres, ceil(EntropieApres));
        
        if(algoSlic == true && compPal == true && boolSP == true) {
            ImGui::Text("Taux de compression = %.3f", tdc);
        }

        //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        ImGui::Render();
        SDL_RenderClear(window.getRenderer());
        SDL_RenderCopy(window.getRenderer(), window.getTexture(), NULL, NULL);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(window.getRenderer());
    }

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();


    window.destroyWindow();

    return 0;
}