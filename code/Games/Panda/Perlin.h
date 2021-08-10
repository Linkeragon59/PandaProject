#pragma once

#include <array>
#include <algorithm>
#include <random> // for random_device and mt19937
#include <cmath> // for floor

double fade(double t);
double lerp(double t, double a, double b);
double grad(int hash, double x, double y);

class Perlin
{
public: 
    Perlin(int numberOctaves, double octavePersistence);
    ~Perlin();
    double SimpleNoisePt(double x, double y);
    double FractalNoisePt(double x, double y);
    std::array<int, 512> GetPerm() const { return myPerm; };

private:
    std::array<int, 512> myPerm;
    int myNumberOctaves;
    double myOctavePersistence;
};
