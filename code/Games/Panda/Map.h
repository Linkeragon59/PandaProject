#pragma once

#include <vector>
#include <iostream>
#include <string>
#include "Perlin.h"

enum class biomeType
{
    sea,
    land
};

class Map
{
public:
    Map();
    ~Map();
    void Print();
    biomeType BiomePt(double x, double y);

private:
    double myXmin, myXmax, myYmin, myYmax;
    std::vector<Perlin> myHabitatVars; 
};
