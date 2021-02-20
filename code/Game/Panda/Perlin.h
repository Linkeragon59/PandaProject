#pragma once

#include <vector>
#include <algorithm>
#include <random> // for random_device and mt19937
#include <cmath> // for floor

// Smoothstep function
double fade(double t);
// Linear interpolation
double lerp(double t, double a, double b);
// Gradient
double grad(int hash, double x, double y);

class Perlin
{
public: 
    Perlin(int numberOctaves, double octavePersistence);
    ~Perlin();
    double SimpleNoisePt(double x, double y);
    double FractalNoisePt(double x, double y);
    std::vector<int> GetPerm() const { return myPerm; };

private:
    std::vector<int> myPerm;
    int myNumberOctaves;
    double myOctavePersistence;
};
