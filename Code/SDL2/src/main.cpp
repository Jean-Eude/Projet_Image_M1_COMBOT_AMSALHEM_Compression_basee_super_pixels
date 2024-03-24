#include <Window.h>
#include <GUI.h>
#include <ImageBase.h>
#include <SLIC.h>

#include <string>
#include <iostream>
#include <vector>
#include <utility>
#include <cmath>


std::mutex mtx;

void draw_Pixel(uint8_t r, uint8_t g, uint8_t b, int x, int y, uint32_t *buffer, int width, int height) {
    uint8_t ir = r;
    uint8_t ig = g;
    uint8_t ib = b;

    uint32_t pixel = (ir << 16) | (ig << 8) | ib;
    buffer[(height - 1 - y) * width + x] = pixel;
}

double calculate_entropy(int *histogram, int size) {
    double entropy = 0.0;
    for (int i = 0; i < size; ++i) {
        if (histogram[i] != 0) {
            double probability = static_cast<double>(histogram[i]) / size;
            entropy = probability * std::log2(probability);
        }
    }
    return entropy;
}

// Fonction pour calculer l'entropie pour chaque canal de couleur
void calculate_entropy_window(uint32_t *buffer, int width, int height, double &entroR, double &entroG, double &entroB) {
    int histogramR[256] = {0};
    int histogramG[256] = {0};
    int histogramB[256] = {0};

    for (int i = 0; i < width * height; ++i) {
        uint8_t r = (buffer[i] >> 16) & 0xFF;
        uint8_t g = (buffer[i] >> 8) & 0xFF;
        uint8_t b = buffer[i] & 0xFF;

        histogramR[r]++;
        histogramG[g]++;
        histogramB[b]++;
    }

    entroR = calculate_entropy(histogramR, 256);
    entroG = calculate_entropy(histogramG, 256);
    entroB = calculate_entropy(histogramB, 256);
}

int main(int, char**) {
    ImageBase imIn;
    imIn.load("lena.ppm");

    int width = 1280, height = 720;
    int Orwidth = width, Orheight = height;

    Window window("Projet Image - Compression basée superpixels", width, height);
    GUI gui(window);

    void* pixels;
    int pitch;

    int curr_Redim = 0;
    int curr_Algo = 0;
    int curr_Comp = 0;

    // Initialisation de ImGui
    gui.setupGUI();

    // Booléens
    bool adaptationImg = false;

    bool imageModif = true;
    bool interPPV = true;
    bool interBil = false;

    bool SPP_rien = true;
    bool SPP_SLIC = false;
    bool SPP_QCT = false;
    bool SPP_TASP = false;

    bool Comp_rien = true;
    bool Comp_pal = false;

    // Champ de texte
    char inputBuffer[256] = ""; 

    int S = 4;
    int N = 1;
    int m = 1;


    uint8_t r, g, b;
    double entroR, entroG, entroB;

    // Boucle de rendu
    while (!window.getDone())
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                window.setDone(true);
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window.getWindow()))
                window.setDone(true);
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_r) {
                SPP_SLIC = true;
            }
        }

        if (imageModif)
        {
            SDL_LockTexture(window.getTexture(), NULL, &pixels, &pitch);
            uint32_t* buffer = static_cast<uint32_t*>(pixels);

            int windowWidth = window.getWidth();
            int windowHeight = window.getHeight();

            double echelleX = static_cast<double>(windowWidth) / imIn.getWidth();
            double echelleY = static_cast<double>(windowHeight) / imIn.getHeight();


            for (int y = 0; y < windowHeight; ++y)
            {
                for (int x = 0; x < windowWidth; ++x)
                {
                    // Plus proche voisins 
                    if(interPPV) {
                        double srcX = x / echelleX;
                        double srcY = imIn.getHeight() - (y / echelleY) - 1;

                        int srcXInt = static_cast<int>(srcX);
                        int srcYInt = static_cast<int>(srcY);
                        srcXInt = std::min(std::max(srcXInt, 0), imIn.getWidth() - 1);
                        srcYInt = std::min(std::max(srcYInt, 0), imIn.getHeight() - 1);

                        uint8_t r = imIn[srcYInt * 3][srcXInt * 3];
                        uint8_t g = imIn[srcYInt * 3][srcXInt * 3 + 1];
                        uint8_t b = imIn[srcYInt * 3][srcXInt * 3 + 2];

                        draw_Pixel(r, g, b, x, y, buffer, windowWidth, windowHeight);
                    }

                    // Interpolation billinéaire 
                    else if (interBil) {
                        double srcX = x / echelleX;
                        double srcY = imIn.getHeight() - (y / echelleY) - 1;

                        int srcXInt = static_cast<int>(srcX);
                        int srcYInt = static_cast<int>(srcY);

                        srcXInt = std::min(std::max(srcXInt, 0), imIn.getWidth() - 1);
                        srcYInt = std::min(std::max(srcYInt, 0), imIn.getHeight() - 1);

                        int srcXIntNext = std::min(srcXInt + 1, imIn.getWidth() - 1);
                        int srcYIntNext = std::min(srcYInt + 1, imIn.getHeight() - 1);

                        double deltaX = srcX - srcXInt;
                        double deltaY = srcY - srcYInt;

                        uint8_t r00 = imIn[srcYInt * 3][srcXInt * 3];
                        uint8_t g00 = imIn[srcYInt * 3][srcXInt * 3 + 1];
                        uint8_t b00 = imIn[srcYInt * 3][srcXInt * 3 + 2];

                        uint8_t r01 = imIn[srcYInt * 3][srcXIntNext * 3];
                        uint8_t g01 = imIn[srcYInt * 3][srcXIntNext * 3 + 1];
                        uint8_t b01 = imIn[srcYInt * 3][srcXIntNext * 3 + 2];

                        uint8_t r10 = imIn[srcYIntNext * 3][srcXInt * 3];
                        uint8_t g10 = imIn[srcYIntNext * 3][srcXInt * 3 + 1];
                        uint8_t b10 = imIn[srcYIntNext * 3][srcXInt * 3 + 2];

                        uint8_t r11 = imIn[srcYIntNext * 3][srcXIntNext * 3];
                        uint8_t g11 = imIn[srcYIntNext * 3][srcXIntNext * 3 + 1];
                        uint8_t b11 = imIn[srcYIntNext * 3][srcXIntNext * 3 + 2];

                        double r = (1 - deltaX) * (1 - deltaY) * r00 + deltaX * (1 - deltaY) * r01 +
                                (1 - deltaX) * deltaY * r10 + deltaX * deltaY * r11;

                        double g = (1 - deltaX) * (1 - deltaY) * g00 + deltaX * (1 - deltaY) * g01 +
                                (1 - deltaX) * deltaY * g10 + deltaX * deltaY * g11;

                        double b = (1 - deltaX) * (1 - deltaY) * b00 + deltaX * (1 - deltaY) * b01 +
                                (1 - deltaX) * deltaY * b10 + deltaX * deltaY * b11;

                        uint8_t r_final = static_cast<uint8_t>(r);
                        uint8_t g_final = static_cast<uint8_t>(g);
                        uint8_t b_final = static_cast<uint8_t>(b);

                        draw_Pixel(r_final, g_final, b_final, x, y, buffer, windowWidth, windowHeight);
                    }

                    if(SPP_SLIC) {
                        std::vector<Pixel> image;
                        image.resize(window.getWidth() * window.getHeight());
                        std::vector<SuperPixel> clustercentres;
    
                        int hsizec = 0;
                        int wsizec = 0;
    
                        for (int x = 0; x < window.getHeight(); x+=S) {
                            hsizec++;
                            wsizec = 0;
                            for (int y = 0; y < window.getWidth(); y+=S) {
                                wsizec++;
                                SuperPixel sp(window.getHeight() * window.getWidth());
                                for (int i = 0; i<S ; i++){
                                    for (int j = 0; j<S ; j++){
                                        Pixel p((x+i) , (y+j) , imIn[(x+i)*3][ (y+j)*3] , imIn[(x+i)*3][ (y+j)*3+1]  , imIn[(x+i)*3][ (y+j)*3+2]);
                                        RGBtoLab(p);
                                        image[(x+i) * window.getWidth() + (y+j)] = p;
                                        sp.indicespixels[(x+i) * window.getWidth() + (y+j)] = (x + i) * window.getWidth() + y + j;
                                    }
                                }
                                calculMoyenne(sp, image);
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
                                for (int x = 0; x <window.getHeight() ; x++){
                                    for (int y = 0 ; y < window.getWidth(); y++){
                                        if (c.indicespixels[x * window.getWidth() + y] != -1){
                                            double minDist = FLT_MAX;
                                            SuperPixel* minsp;
                                            for (int i  : c.indice_adj){
                                                double dist = calculDistances(image[x * window.getWidth() + y] , clustercentres[i] , S , m);
                                                if (dist < minDist) { 
                                                    minDist = dist;
                                                    minsp = &clustercentres[i];
                                                }
                                            } if (minsp != &c){
                                                c.indicespixels[x * window.getWidth() + y] = -1;
                                                minsp->indicespixels[x * window.getWidth() + y] = x * window.getWidth() + y;
                                            }
                                        }
                                    }
                                }
                                calculMoyenne(c, image);
                                RGBtoLab(c);
                            }
                        }   
    
                        for (SuperPixel & c : clustercentres) {
                            for (int i : c.indicespixels) {
                                if (i != -1) {
                                    draw_Pixel((int)c.R, (int)c.G, (int)c.B, i % window.getWidth(), i / window.getWidth(), buffer, windowWidth, windowHeight);
                                }
                            }
                        }
                    }
                }
            }

            //calculate_entropy_window(buffer, width, height, entroR, entroG, entroB);

            SDL_UnlockTexture(window.getTexture());
            imageModif = false;
        }

        gui.beforeUpdate();

        ImGui::Begin("Paramètres");
        ImGui::Text("Paramètres généraux :");
        ImGui::Spacing();
        if (ImGui::Button("Load")) {
            IGFD::FileDialogConfig cfg;
            cfg.path = ".";
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Ouvrir une image", ".ppm", cfg);
        }
        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
            if (ImGuiFileDialog::Instance()->IsOk()) { 
                std::string cheminNom = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string cheminAccess = ImGuiFileDialog::Instance()->GetCurrentPath();

                std::cout << (char*)cheminNom.c_str() << std::endl;
                imIn.load((char*)cheminNom.c_str());

                interPPV = true;
                interBil = false;

                imageModif = true;
            }
            ImGuiFileDialog::Instance()->Close();
        }

        ImGui::SameLine();
        if (ImGui::Button("Save")) {
            void* pixels;
            int pitch;
            SDL_LockTexture(window.getTexture(), NULL, &pixels, &pitch);
            uint32_t* buffer = static_cast<uint32_t*>(pixels);

            ImageBase imOut(window.getWidth(), window.getHeight(), imIn.getColor());

            for (int y = 0; y < window.getHeight(); y++) {
                for (int x = 0; x < window.getWidth(); x++) {
                    uint32_t pixel = buffer[y * window.getWidth() + x];
                    uint8_t r = (pixel >> 16) & 0xFF;
                    uint8_t g = (pixel >> 8) & 0xFF;
                    uint8_t b = pixel & 0xFF;
                    imOut[y*3][x*3] = r;
                    imOut[y*3][x*3+1] = g;
                    imOut[y*3][x*3+2] = b;
                }
            }

            SDL_UnlockTexture(window.getTexture());
            if (strlen(inputBuffer) == 0) {
                imOut.save("1.ppm");
            } else {
                imOut.save(inputBuffer);
            }
        }

        ImGui::InputText("Nom du fichier", inputBuffer, sizeof(inputBuffer));
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Redimensionnement :");

        if(window.getWidth() == Orwidth && window.getHeight() == Orheight) {
            const char* itemsRedim[] = { "Plus proche voisins", "Interpolation billinéaire"};
            if(ImGui::Combo("Algorithme", &curr_Redim, itemsRedim, IM_ARRAYSIZE(itemsRedim))) {
                std::string selected_item(itemsRedim[curr_Redim]);

                if(curr_Redim == 0) {
                    interPPV = true;
                    interBil = false;
                } else if(curr_Redim == 1) {
                    interPPV = false;
                    interBil = true;
                }
                imageModif = true;
            }
        }

        if(ImGui::Checkbox("Adapter la fenêtre à l'image", &adaptationImg)) {
            if(adaptationImg) {
                window.setWindowSize(imIn.getWidth(), imIn.getHeight());
            } else {
                window.setWindowSize(width, height);
            }
        }


        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Algorithme basée superpixels :");
        const char* itemsAlgo[] = { "Sans algorithme", "SLIC", "Quickshift", "TASP" };
        if(ImGui::Combo("Algorithme superpixels", &curr_Algo, itemsAlgo, IM_ARRAYSIZE(itemsAlgo))) {
            std::string selected_item(itemsAlgo[curr_Algo]);

            if(curr_Algo == 0) {
                SPP_rien = true;
                SPP_SLIC = false;
                SPP_QCT = false;
                SPP_TASP = false;        
            } else if(curr_Algo == 1) {
                SPP_rien = false;
                SPP_SLIC = true;
                SPP_QCT = false;
                SPP_TASP = false;   
            } else if(curr_Algo == 2) {
                SPP_rien = false;
                SPP_SLIC = false;
                SPP_QCT = true;
                SPP_TASP = false;   
            } else if(curr_Algo == 3) {
                SPP_rien = false;
                SPP_SLIC = false;
                SPP_QCT = false;
                SPP_TASP = true;   
            }
            imageModif = true;
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Méthode de compression :");
        const char* itemsComp[] = { "Sans compression", "Compression par palette" };
        if(ImGui::Combo("Méthode de compression", &curr_Comp, itemsComp, IM_ARRAYSIZE(itemsComp))) {
            std::string selected_item(itemsComp[curr_Comp]);

            if(curr_Comp == 0) {

            } else if(curr_Comp == 1) {

            }
            imageModif = true;
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Informations :");
        ImGui::Spacing();
        ImGui::Text("Entropie : %f", (entroR + entroG + entroB) / 3);

        ImGui::Text("PSNR :");
        ImGui::Text("EQM :");
        ImGui::Text("Taux de compression :");
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

    window.destroyWindow();

    return 0;
}
