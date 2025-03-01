#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

Vec3b rgbToHsl(Vec3b rgb) {
    float r = rgb[2] / 255.0f, g = rgb[1] / 255.0f, b = rgb[0] / 255.0f;
    float chigh = max(max(r, g), b);
    float clow = min(min(r, g), b);
    float delta = chigh - clow;
    float h = 0, s = 0, l = (chigh + clow) / 2.0f;

    if (delta != 0){
        if (r == chigh) {
            h = 60 * fmod(((g - b) / delta), 6);
        }
        else if (g == chigh) {
            h = 60 * (((b - r) / delta) + 2);
        }
        else {
            h = 60 * (((r - g) / delta) + 4);
        }
        
        if (h < 0) {
            h += 360;
        }

        s = delta / (1 - abs(2 * l - 1));
    }
   
    return Vec3b(h / 2, s * 255, l * 255);
}

Vec3b hslToRgb(Vec3b hsl) {
    float h = (hsl[0] * 2.0f) / 60.0f, s = hsl[1] / 255.0f, l = hsl[2] / 255.0f ;
    float r, g, b;

    float c = (1 - abs(2 * l - 1)) * s;
    float x = c * (1 - abs(fmod(h, 2) - 1));
    float m = l - c / 2;

    if (h < 1) {
        r = c;
        g = x;
        b = 0;
    }
    else if (h < 2) {
        r = x;
        g = c;
        b = 0;
    }
    else if (h < 3) {
        r = 0;
        g = c;
        b = x;
    }
    else if (h < 4) {
        r = 0;
        g = x;
        b = c;
    }
    else if (h < 5) {
        r = x;
        g = 0;
        b = c;
    }
    else {
        r = c;
        g = 0;
        b = x;
    }

    return Vec3b((b + m) * 255, (g + m) * 255, (r + m) * 255);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "Número de argumentos insuficientes" << endl;
        return 1;
    }

    String caminhoImagem = argv[1];
    int luminosidade = stoi(argv[2]);

    Mat img = imread(caminhoImagem);

    if (img.empty()) {
        cout << "Erro ao carregar imagem" << endl;
        return 1;
    }

    // rgb to hsl
    Mat imgHsl = img.clone();
    for (int i = 0; i < img.rows; i++) {
        for (int j = 0; j < img.cols; j++) {
            Vec3b hsl = rgbToHsl(img.at<Vec3b>(i, j));
            int novaLum = min(255, max(0, hsl[2] + luminosidade));
            imgHsl.at<Vec3b>(i, j) = Vec3b(hsl[0], hsl[1], novaLum); 
        }
    }

    // hsl to rgb
    Mat imgRgb = img.clone();
    for (int i = 0; i < img.rows; i++) {
        for (int j = 0; j < img.cols; j++) {
            imgRgb.at<Vec3b>(i, j) = hslToRgb(imgHsl.at<Vec3b>(i, j));
        }
    }

    imwrite("output.png", imgRgb);
    return 0;
}
