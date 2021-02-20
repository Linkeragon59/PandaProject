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
    Perlin();
    ~Perlin();
    double NoisePt(double x, double y);
    std::vector<int> GetPerm() const { return myPerm; };

private:
    std::vector<int> myPerm;
};
