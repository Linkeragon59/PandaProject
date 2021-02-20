#include "Map.h"

namespace
{
    // Some characteristics of the map
    int locNumberHabitatVars = 3;
    int locNumberOctaves = 4;
    double locOctavePersistence = 0.6;
}

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
    // Reserve memory
    myHabitatVars.reserve(locNumberHabitatVars);
    // Create habitat variables and store in vector
    for(int i = 0; i < locNumberHabitatVars; i++)
    {
        myHabitatVars.push_back(
            Perlin(locNumberOctaves, locOctavePersistence)
        );
    }
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
        habitatVars.at(i) = myHabitatVars.at(i).FractalNoisePt(x, y);
    }
    biomeType biome = getBiome(habitatVars);
    return biome;
}

// Print map (for testing purposes only...)
void Map::Print()
{
    // Define grids over x and y dimensions
    int nX = 60;
    int nY = 20;
    std::vector<double> gridX(nX);
    std::vector<double> gridY(nY);
    for(int i = 0; i < nX; i++)
        gridX.at(i) = 0.1 * i;
    for(int i = 0; i < nY; i++)
        gridY.at(i) = 0.1 * i;

    // Loop over grids and print biome at (x, y)
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