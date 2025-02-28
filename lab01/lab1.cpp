#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

Vec3b rgbToHsl(Vec3b rgb) {
    float r = rgb[2] / 255.0f, g = rgb[1] / 255.0f, b = rgb[0] / 255.0f;
    float chigh = max(max(r, g), b);
    float clow = min(min(r, g), b);
    float delta = chigh - clow;
    float h = 0, s = 0, l = (chigh + clow) / 2;

    if (delta != 0){
        if (r == chigh) {
            float aux = (g - b) / delta;
            h = 60 * fmod(delta, 6); 
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

float hueToRgb(float p, float q, float t) {
    if (t < 0) {
        t += 1;
    }
    if (t > 1) {
        t -= 1;
    }
    if (t < 1 / 6.0f) {
        return p + (q - p) * 6 * t;
    }
    if (t < 1 / 2.0f) {
        return q;
    }
    if (t < 2 / 3.0f) {
        return p + (q - p) * (2 / 3.0f - t) * 6;
    }
    
    return p;
}

Vec3b hslToRgb(Vec3b hsl) {
    float h = hsl[0] * 2 / 360.0f, s = hsl[1] / 255.0f, l = hsl[2] / 255.0f ;
    float r, g, b;

    if (s == 0) {
        r = g = b = l;
    } else {
        float q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        float p = 2 * l - q;
        r = hueToRgb(p, q, h + 1 / 3.0f);
        g = hueToRgb(p, q, h);
        b = hueToRgb(p, q, h - 1 / 3.0f);
    }

    return Vec3b(b * 255, g * 255, r * 255);
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
            float novaLum = min(255, max(0, hsl[2] + luminosidade));
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
