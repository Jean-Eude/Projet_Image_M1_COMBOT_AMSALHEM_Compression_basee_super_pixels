#pragma once

#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <utility>


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

void OriginalImage(cv::Mat &imIn, cv::Mat &imOut, int x, int y, uint32_t *buffer, int size, int width) {
    for (int px = x; px < x + size; px++) {
        for (int py = y; py < y + size; py++) {
            int imgX_N = px - x;
            int imgY_N = py - y;

            if (imgX_N >= 0 && imgX_N < size && imgY_N >= 0 && imgY_N < size) {
                uint32_t pixel = buffer[py * width + px];
                uint8_t r_s = (pixel >> 16) & 0xFF;
                uint8_t g_s = (pixel >> 8) & 0xFF;
                uint8_t b_s = pixel & 0xFF;

                imOut.at<cv::Vec3b>(imgY_N, imgX_N) = cv::Vec3b(b_s, g_s, r_s);
            }
        }
    }
}

// RGB --> Lab 
void RGBtoLab(cv::Mat &imIn, cv::Mat &imOut, char c) {
    cv::Mat temp;
    cv::cvtColor(imIn, temp, cv::COLOR_RGB2Lab);
    std::vector<cv::Mat> labChannels;
    cv::split(temp, labChannels);

    for (int x = 0; x < temp.rows; x++) {
        for (int y = 0; y < temp.cols; y++) {
            if(c == 'L') {
                imOut.at<uchar>(x, y) = labChannels[0].at<uchar>(x, y);
            } else if(c == 'a') {
                imOut.at<uchar>(x, y) = labChannels[1].at<uchar>(x, y);
            } else if(c == 'b') {
                imOut.at<uchar>(x, y) = labChannels[2].at<uchar>(x, y);
            }
        }
    }
}

void Convert2Gradient(cv::Mat &imIn, cv::Mat &imOut) {
    int rows = imIn.rows;
    int cols = imIn.cols;

    // Gradient en direction X
    cv::Mat gradX(rows, cols, CV_64F);
    for (int x = 0; x < rows; ++x) {
        for (int y = 0; y < cols; ++y) {
            double dx = 0.0;
            if (y > 0 && y < cols - 1) {
                dx = (imIn.at<double>(x, y + 1) - imIn.at<double>(x, y - 1)) / 2.0;
            } else if (y == 0) {
                dx = imIn.at<double>(x, y + 1) - imIn.at<double>(x, y);
            } else {
                dx = imIn.at<double>(x, y) - imIn.at<double>(x, y - 1);
            }
            gradX.at<double>(x, y) = dx;
        }
    }

    // Gradient en direction Y
    cv::Mat gradY(rows, cols, CV_64F);
    for (int x = 0; x < rows; ++x) {
        for (int y = 0; y < cols; ++y) {
            double dy = 0.0;
            if (x > 0 && x < rows - 1) {
                dy = (imIn.at<double>(x + 1, y) - imIn.at<double>(x - 1, y)) / 2.0;
            } else if (x == 0) {
                dy = imIn.at<double>(x + 1, y) - imIn.at<double>(x, y);
            } else {
                dy = imIn.at<double>(x, y) - imIn.at<double>(x - 1, y);
            }
            gradY.at<double>(x, y) = dy;
        }
    }

    // Magnitude du gradient
    imOut.create(rows, cols, CV_64F);
    for (int x = 0; x < rows; ++x) {
        for (int y = 0; y < cols; ++y) {
            double gx = gradX.at<double>(x, y);
            double gy = gradY.at<double>(x, y);
            double mag = std::sqrt(gx * gx + gy * gy);
            imOut.at<double>(x, y) = mag;
        }
    }
}


void perturbClusterCenters(cv::Mat &Lab, cv::Mat &imIn, cv::Mat &gradient, int n, std::vector<Cluster> &clusterCentres) {
    for (int i = 0; i < clusterCentres.size(); ++i) {
        int x = clusterCentres[i].x;
        int y = clusterCentres[i].y;

        double minGradient = gradient.at<uchar>(x, y);
        int newX = x, newY = y;
        
        for (int dx = -n/2; dx <= n/2; ++dx) {
            for (int dy = -n/2; dy <= n/2; ++dy) {
                int nx = x + dx;
                int ny = y + dy;

                if (nx >= 0 && nx < gradient.rows && ny >= 0 && ny < gradient.cols) {
                    if (gradient.at<uchar>(nx, ny) < minGradient) {
                        minGradient = gradient.at<uchar>(nx, ny);
                        newX = nx;
                        newY = ny;
                    }
                }
            }
        }

        clusterCentres[i].x = newX;
        clusterCentres[i].y = newY;
        clusterCentres[i].L = Lab.at<uchar>(newX, newY);
        clusterCentres[i].a = Lab.at<uchar>(newX, newY + 1);
        clusterCentres[i].b = Lab.at<uchar>(newX, newY + 2);
    }
}