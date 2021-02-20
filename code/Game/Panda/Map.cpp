#include "Map.h"

// Get biome from habitat variables
biomeType getBiome(std::vector<double> habitatVars)
{
    // double temperature = habitatVars.at(0);
	// double humidity = habitatVars.at(1);
	double elevation = habitatVars.at(2);

    // Very basic rule for now
    if(elevation > 0) 
    {
        return biomeType::land;
    }
    else
    {
        return biomeType::sea;
    }
}

// Constructor
Map::Map()
{
    myHabitatVars = std::vector<Perlin>(3);
}

// Destructor
Map::~Map()
{
}

// Get biome at point (x, y)
biomeType Map::BiomePt(double x, double y)
{
    int nVars = myHabitatVars.size();
    std::vector<double> habitatVars(nVars);
    for(int i = 0; i < nVars; i++)
    {
        habitatVars.at(i) = myHabitatVars.at(i).NoisePt(x, y);
    }
    biomeType biome = getBiome(habitatVars);
    return biome;
}

// Print map (for testing purposes only...)
void Map::Print()
{
    std::vector<double> gridX = {0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0};
    std::vector<double> gridY = {0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0};

    for(auto itY = gridY.begin(); itY != gridY.end(); itY++) 
    {
        for(auto itX = gridX.begin(); itX != gridX.end(); itX++) 
        {
            biomeType b = BiomePt(*itX, *itY);
            if(b == biomeType::land)
                std::cout << "#";
            else
                std::cout << ".";
        }
        std::cout << std::endl;
    }
}