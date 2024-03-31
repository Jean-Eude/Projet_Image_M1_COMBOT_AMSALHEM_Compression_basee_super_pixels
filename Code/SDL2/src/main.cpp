#include <Window.h>
#include <GUI.h>
#include <ImageBase.h>

#include <string>
#include <iostream>
#include <vector>
#include <utility>
#include <cmath>


#include <SLIC.h>


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


void draw_Pixel(uint8_t r, uint8_t g, uint8_t b, int x, int y, uint32_t *buffer, int width, int height) {
    uint8_t ir = r;
    uint8_t ig = g;
    uint8_t ib = b;

    uint32_t pixel = (ir << 16) | (ig << 8) | ib;
    buffer[y * width + x] = pixel;
}

int main(int argc, char* argv[]) {
    int width = 1280, height = 720;
    bool imageModif = true;
    int size = 256;
    int previousSize = size;
    uint8_t r = 255, g = 255, b = 255; 

    ImageBase imIn;
    imIn.load("../Assets/lena.ppm");
    bool imageLoaded = true;

    Window window("Projet Image - Compression basee superpixels", width, height);
    GUI gui(window);

    // Méthodes d'interpolation
    bool interNN = true;
    bool interBi = false;
    int curr_Redim = 0;

    // Algo superpixels
    bool algoRien = true;
    bool algoSlic = false;
    int curr_Algo = 0;

    // Nombre de superpixels
    int K = 1;
    // compacité
    int m = 1;
    // Voisinage
    int n = 3;  // 3 de base
    int nbIter = 1;

    ImageBase imOut(size, size, imIn.getColor());
    ImageBase L(size, size, imIn.getColor());


    // Méthodes de compression
    bool compRien = true;
    bool compPal = true;
    int curr_comp = 0;



    void* pixels;
    int pitch;
    uint32_t* buffer;
    char inputBuffer[256] = ""; 
    gui.setupGUI();

    #pragma omp parallel while
    while (!window.getDone()) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                window.setDone(true);
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window.getWindow()))
                window.setDone(true);
        }

        int squareHeight = (height - size) / 2;
        int offset = (width - 2 * size) / 4;

        int squareWidth1 = offset;
        int squareWidth2 = width / 2 + offset;

        if (imageModif) {
            window.destroyTexture();
            window.createTexture(SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);

            SDL_LockTexture(window.getTexture(), NULL, &pixels, &pitch);
            buffer = static_cast<uint32_t*>(pixels);

            #pragma omp parallel for
            for (int x = squareWidth1; x < squareWidth1 + size; x++) {
                for (int y = squareHeight; y < squareHeight + size; y++) {
                    int imgX = x - squareWidth1;
                    int imgY = y - squareHeight;

                    if (imgX >= 0 && imgX < size && imgY >= 0 && imgY < size) {
                        if(interNN == true) {
                            InterNN(static_cast<double>(imgX) / size * imIn.getWidth(), static_cast<double>(imgY) / size * imIn.getHeight(), imIn, &r, &g, &b);
                        } else if(interBi == true) {
                            InterBill(static_cast<double>(imgX), static_cast<double>(imgY), size, imIn, &r, &g, &b);
                        }
                    }

                    draw_Pixel(r, g, b, x, y, buffer, width, height);
                }
            }

            #pragma omp parallel for collapse(2) schedule(static)
            int startX = squareWidth2;
            int endX = squareWidth2 + size;
            int startY = squareHeight;
            int endY = squareHeight + size;

            for (int x = startX; x < endX; x++) {
                int imgX = x - squareWidth2;
                double imgXRatio = static_cast<double>(imgX) / size * imIn.getWidth();

                for (int y = startY; y < endY; y++) {
                    int imgY = y - squareHeight;
                    double imgYRatio = static_cast<double>(imgY) / size * imIn.getHeight();

                    if (imgX >= 0 && imgX < size && imgY >= 0 && imgY < size) {
                        if(interNN == true) {
                            InterNN(static_cast<double>(imgX) / size * imIn.getWidth(), static_cast<double>(imgY) / size * imIn.getHeight(), imIn, &r, &g, &b);

                            if(algoRien == true) {
                                draw_Pixel(r, g, b, x, y, buffer, width, height);
                            } else if(algoSlic == true) {
                                // Nombre de pixels de image
	                            int N = imIn.getWidth() * imIn.getHeight();
                                // Taille de chaque superpixels
                                double tailleSP = static_cast<double>(N)/static_cast<double>(K);
                                // Distance entre chaque superpixel
                                int S = sqrt(tailleSP);
                                // Liste des centres des clusters
                                std::vector<Cluster> clusterCentres;


                                for (int x = squareWidth1; x < squareWidth1 + size; x++) {
                                    for (int y = squareHeight; y < squareHeight + size; y++) {
                                        int imgX_N = x - squareWidth2;
                                        int imgY_N = y - squareHeight;

                                        if (imgX_N >= 0 && imgX_N < size && imgY_N >= 0 && imgY_N < size) {
                                            uint32_t pixel = buffer[y * width + x];  
                                            uint8_t r_s = (pixel >> 16) & 0xFF;     
                                            uint8_t g_s = (pixel >> 8) & 0xFF; 
                                            uint8_t b_s = pixel & 0xFF; 

                                            imOut[imgY_N * 3][imgX_N * 3] = r_s;
                                            imOut[imgY_N * 3][imgX_N * 3 + 1] = g_s;
                                            imOut[imgY_N * 3][imgX_N * 3 + 2] = b_s;
                                        }
                                    }
                                }

                                RGBtoLab(imOut, L, 'L');

                                draw_Pixel(255 - r, 255 - g, 255 - b, x, y, buffer, width, height);
                            }
                        } else if(interBi == true) {
                            InterBill(static_cast<double>(imgX), static_cast<double>(imgY), size, imIn, &r, &g, &b);
                            
                            if(algoRien == true) {
                                draw_Pixel(r, g, b, x, y, buffer, width, height);
                            } else if(algoSlic == true) {
                                draw_Pixel(255 - r, 255 - g, 255 - b, x, y, buffer, width, height);
                            }
                        }
                    }                    
                }
            } 


            SDL_UnlockTexture(window.getTexture());
            imageModif = false;
        } 

        gui.beforeUpdate();

        ImGui::Begin("Parametres");
        ImGui::Text("Paramètres généraux :");
        ImGui::Spacing();
        ImGui::InputText("Nom du fichier", inputBuffer, sizeof(inputBuffer));
        if (ImGui::SliderInt("Taille", &size, 0, width/2)) {
            if (size != previousSize) {
                previousSize = size;
                imageModif = true;
            }
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
                buffer = static_cast<uint32_t*>(pixels);

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

            uint32_t* buffere = static_cast<uint32_t*>(pixels);
            SDL_LockTexture(window.getTexture(), NULL, &pixels, &pitch);

            for (int x = squareWidth2; x < squareWidth2 + size; x++) {
                for (int y = squareHeight; y < squareHeight + size; y++) {
                    int imgX = x - squareWidth2;
                    int imgY = y - squareHeight;

                    if (imgX >= 0 && imgX < size && imgY >= 0 && imgY < size) {
                        uint32_t pixel = buffere[y * width + x];  
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
            } else if(curr_Redim == 1) {
                interNN = false;
                interBi = true;
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
            } else if(curr_Algo == 1) { 
                algoRien = false;
                algoSlic = true;  
            }

            imageModif = true;
        }

        if(algoSlic) {
            ImGui::SliderInt("Nombre de superpixels", &K, 1, 500);
            ImGui::SliderInt("Compacité", &m, 1, 500);
            ImGui::SliderInt("Voisinage (Gradient)", &n, 1, 10);
            ImGui::SliderInt("Nombre d'itérations", &nbIter, 1, 10);
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
                compRien = true;
                compPal = false; 
            }

            imageModif = true;
        }

        ImGui::Spacing();
        ImGui::Separator();

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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

imOut.save("2.ppm");
L.save("3.ppm");

    window.destroyWindow();

    return 0;
}