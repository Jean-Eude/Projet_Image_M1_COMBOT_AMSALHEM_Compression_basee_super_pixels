#include <opencv2/opencv.hpp>
#include <Window.h>
#include <GUI.h>
#include <ImageBase.h>

#include <string>
#include <iostream>
#include <vector>
#include <utility>
#include <cmath>

using namespace cv;



void InterNN(double imgX, double imgY, cv::Mat& imIn, uint8_t* r, uint8_t* g, uint8_t* b) {
    int xNearest = static_cast<int>(round(imgX));
    int yNearest = static_cast<int>(round(imgY));

    xNearest = std::max(0, std::min(xNearest, imIn.cols - 1));
    yNearest = std::max(0, std::min(yNearest, imIn.rows - 1));

    cv::Vec3b pixel = imIn.at<cv::Vec3b>(yNearest, xNearest);
    *r = pixel[0];  // Bleu
    *g = pixel[1];  // Vert
    *b = pixel[2];  // Rouge
}

void InterBill(double imgX, double imgY, int size, cv::Mat& imIn, uint8_t* r, uint8_t* g, uint8_t* b) {
    double imgX_original = imgX / static_cast<double>(size) * imIn.cols;
    double imgY_original = imgY / static_cast<double>(size) * imIn.rows;

    int x1 = static_cast<int>(imgX_original);
    int y1 = static_cast<int>(imgY_original);
    int x2 = std::min(x1 + 1, imIn.cols - 1);
    int y2 = std::min(y1 + 1, imIn.rows - 1);

    double dx = imgX_original - x1;
    double dy = imgY_original - y1;

    cv::Vec3b p1 = imIn.at<cv::Vec3b>(y1, x1);
    cv::Vec3b p2 = imIn.at<cv::Vec3b>(y1, x2);
    cv::Vec3b p3 = imIn.at<cv::Vec3b>(y2, x1);
    cv::Vec3b p4 = imIn.at<cv::Vec3b>(y2, x2);

    *r = static_cast<uint8_t>((1 - dx) * (1 - dy) * p1[2] + dx * (1 - dy) * p2[2] + (1 - dx) * dy * p3[2] + dx * dy * p4[2]);  // Rouge
    *g = static_cast<uint8_t>((1 - dx) * (1 - dy) * p1[1] + dx * (1 - dy) * p2[1] + (1 - dx) * dy * p3[1] + dx * dy * p4[1]);  // Vert
    *b = static_cast<uint8_t>((1 - dx) * (1 - dy) * p1[0] + dx * (1 - dy) * p2[0] + (1 - dx) * dy * p3[0] + dx * dy * p4[0]);  // Bleu
}


// Fonction pour dessiner un pixel sur la texture SDL
void draw_Pixel(uint8_t r, uint8_t g, uint8_t b, int x, int y, uint32_t* buffer, int width, int height) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        uint8_t ir = r;
        uint8_t ig = g;
        uint8_t ib = b;
        uint32_t pixel = (ir << 16) | (ig << 8) | ib;
        buffer[y * width + x] = pixel;
    }
}

int main(int argc, char* argv[]) {
    int width = 1280, height = 720;
    bool imageModif = true;
    int size = 256;
    int previousSize = size;
    uint8_t r = 255, g = 255, b = 255; 

    // Créer une fenêtre et une interface utilisateur
    Window window("Projet Image - Compression basee superpixels", width, height);
    GUI gui(window);


    void* pixels;
    int pitch;
    uint32_t* buffer;
    char inputBuffer[256] = ""; 
    bool imageLoaded = true;
    cv::Mat image = cv::imread("1.ppm");
    if (!image.empty()) {
        cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
    }


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
    // Nombre de pixels de image
    int N = 0;

    // Taille de chaque superpixels
    double tailleSP = 0.;
    // Distance entre chaque superpixel
    int S = 0;
    
    int imgX_N = 0;
    int imgY_N = 0;

    gui.setupGUI();


    // Boucle principale
    while (!window.getDone()) {
        
        ///// Gestion des entrées
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                window.setDone(true);
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window.getWindow()))
                window.setDone(true);
        }

        ///// Traitements

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
                    if (!image.empty()) {
                        int imgX = (x - squareWidth1) * image.cols / size;
                        int imgY = (y - squareHeight) * image.rows / size;

                        if (imgX >= 0 && imgX < image.cols && imgY >= 0 && imgY < image.rows) {
                            cv::Vec3b pixel = image.at<cv::Vec3b>(imgY, imgX);
                            draw_Pixel(pixel[0], pixel[1], pixel[2], x, y, buffer, width, height);
                        }
                    } else {
                        draw_Pixel(255, 255, 255, x, y, buffer, width, height);
                    }
                }
            }

            startX = squareWidth2;
            endX = squareWidth2 + size;
            startY = squareHeight;
            endY = squareHeight + size;

            for (int x = startX; x < endX; x++) {
                int imgX = x - squareWidth2;
                imgXRatio = static_cast<double>(imgX) / size * image.cols;

                for (int y = startY; y < endY; y++) {
                    int imgY = y - squareHeight;
                    imgYRatio = static_cast<double>(imgY) / size * image.rows;

                    if (!image.empty()) {
                        if (imgXRatio >= 0 && imgXRatio < image.cols && imgYRatio >= 0 && imgYRatio < image.rows) {
                            cv::Vec3b pixel = image.at<cv::Vec3b>(imgYRatio, imgXRatio);
                            draw_Pixel(pixel[0], pixel[1], pixel[2], x, y, buffer, width, height);
                        }
                    } else {
                        draw_Pixel(r, g, b, x, y, buffer, width, height);
                    }
                }
            }

            SDL_UnlockTexture(window.getTexture());
            imageModif = false;

        }


        ///// Interface utilisateur
        gui.beforeUpdate();

        ImGui::Begin("Paramètres");
        ImGui::Text("Paramètres généraux :");
        ImGui::Spacing();
        ImGui::InputText("Nom du fichier", inputBuffer, sizeof(inputBuffer));
        previousSize = size;

        if (ImGui::SliderInt("Taille", &size, 0, width/2)) {
            imageModif = true;
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

                cv::Mat loadedImage = cv::imread(cheminNom);

                if (!loadedImage.empty()) {
                    cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
                    cv::resize(loadedImage, image, cv::Size(width, height));
                    
                    SDL_LockTexture(window.getTexture(), NULL, &pixels, &pitch);
                    buffer = static_cast<uint32_t*>(pixels);

                    memcpy(buffer, loadedImage.data, loadedImage.rows * loadedImage.cols * loadedImage.channels());

                    SDL_UnlockTexture(window.getTexture());
                    imageLoaded = true;
                    imageModif = true;
                } else {
                    std::cerr << "Erreur lors du chargement de l'image !" << std::endl;
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

        ImGui::SameLine();
        
        if (ImGui::Button("Save")) {
            cv::Mat imOut_Save = cv::Mat::zeros(size, size, CV_8UC3);
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

                        imOut_Save.at<cv::Vec3b>(imgY, imgX) = cv::Vec3b(b_s, g_s, r_s);
                    }
                }
            }
        
            SDL_UnlockTexture(window.getTexture());
            
            std::string outputFileName;
            if (strlen(inputBuffer) == 0) {
                outputFileName = "1.ppm";
            } else {
                outputFileName = inputBuffer;
            }

            cv::imwrite(outputFileName, imOut_Save);
        }
        ImGui::Spacing();
        ImGui::Separator();
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
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
        ImGui::Render();


        ///// Update
        SDL_RenderClear(window.getRenderer());
        SDL_RenderCopy(window.getRenderer(), window.getTexture(), NULL, NULL);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(window.getRenderer());
    }

    gui.destroy();
    window.destroyWindow();

    return 0;
}
