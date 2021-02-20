#include "Perlin.h"

// Smoothstep function
double fade(double t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

// Linear interpolation
double lerp(double t, double a, double b) {
    return a + t * (b - a);
}

// Gradient
double grad(int hash, double x, double y) {
    // Use bitwise operations to create "4 gradient directions"
    int check1 = hash & 1;
    int check2 = hash & 2;
    double a = check1 ? x : -x;
    double b = check2 ? y : -y;
    return a + b;
}

// Constructor
Perlin::Perlin()
{
    // Define vector (0, ..., 255)
    std::vector<int> perm(256);
    for(int i = 0; i < 256; i++) 
    {
        perm.at(i) = i;
    }
    // Shuffle vector randomly
    std::random_device rd;
    // The argument is the random seed (here random itself)
    std::mt19937 g(rd());
    std::shuffle(perm.begin(), perm.end(), g);

    // Duplicate permutation for use in Perlin algorithm
    myPerm = std::vector<int>(512);
    for(int i = 0; i < 256; i++)
    {
        myPerm.at(i) = perm.at(i);
        myPerm.at(256 + i) = perm.at(i);
    }

    myNumberOctaves = 4;
    myOctavePersistence = 0.6;
}

// Destructor
Perlin::~Perlin()
{
}

// Evaluate simple Perlin field at point (x, y)
double Perlin::SimpleNoisePt(double x, double y)
{
    // Integer part of x and y modulo 255
    int xFloor = (int)floor(x) & 255;
    int yFloor = (int)floor(y) & 255;
    // Decimal part of x and y
    double xRelative = x - floor(x);
    double yRelative = y - floor(y);
    // Faded decimal parts
    double u = fade(xRelative);
    double v = fade(yRelative);                              
    
    // Get 4 pseudo-random numbers between 0 and 255
    int i00 = myPerm[myPerm[xFloor] + yFloor];
    int i01 = myPerm[myPerm[xFloor] + yFloor + 1];
    int i10 = myPerm[myPerm[xFloor + 1] + yFloor];
    int i11 = myPerm[myPerm[xFloor + 1] + yFloor + 1];
    
    // Get "gradients" from random numbers
    double g00 = grad(i00, xRelative, yRelative);
    double g01 = grad(i01, xRelative, yRelative - 1);
    double g10 = grad(i10, xRelative - 1, yRelative);
    double g11 = grad(i11, xRelative - 1, yRelative - 1);

    // Linear interpolation of gradients
    double m1 = lerp(u, g00, g10);
    double m2 = lerp(u, g01, g11);
    double m3 = lerp(v, m1, m2);
    return m3;
}

// Evaluate fractal Perlin field at point (x, y)
double Perlin::FractalNoisePt(double x, double y)
{
    double noise = 0.0;
    double ampli = 1.0;
    double freq = 1.0;
    for(int i = 0; i < myNumberOctaves; i++) 
    {
        noise = noise + ampli * SimpleNoisePt(freq * x, freq * y);
        ampli = ampli * myOctavePersistence;
        freq = freq * 2;
    }
    return noise;
}
