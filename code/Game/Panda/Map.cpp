#include "Map.h"

namespace
{
    // Some characteristics of the map
    // (Should eventually be set at a higher level; e.g. in game settings)
    int locNumberHabitatVars = 3;
    int locNumberOctaves = 4;
    double locOctavePersistence = 0.6;
    double locXmin = 0;
    double locXmax = 10;
    double locYmin = 0;
    double locYmax = 10;
}

// Get biome from habitat variables
biomeType getBiome(std::vector<double> habitatVars)
{
    double elevation = habitatVars.at(0);
    // double temperature = habitatVars.at(1);
	// double humidity = habitatVars.at(2);

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

    // Set map limits
    myXmin = locXmin;
    myXmax = locXmax;
    myYmin = locYmin;
    myYmax = locYmax;
}

// Destructor
Map::~Map()
{
}

// Get biome at point (x, y)
biomeType Map::BiomePt(double x, double y)
{
    size_t nVars = myHabitatVars.size();
    std::vector<double> habitatVars(nVars);

    // Elevation includes a penalty for distance to centre
    double xCentre = (myXmin + myXmax)/2,
        yCentre = (myYmin + myYmax)/2;
    double distToCentre = 
        sqrt((x - xCentre)*(x - xCentre) + (y - yCentre)*(y - yCentre));
    distToCentre = distToCentre - (myXmax - myXmin)/2;
    habitatVars.at(0) = myHabitatVars.at(0).FractalNoisePt(x, y) - distToCentre;
    
    // Get other habitat variables (temperature...)
    for(size_t i = 1; i < nVars; i++)
    {
        habitatVars.at(i) = myHabitatVars.at(i).FractalNoisePt(x, y);
    }

    // Get biome from habitat variables
    biomeType biome = getBiome(habitatVars);
    return biome;
}

// Print map (for testing purposes only...)
void Map::Print()
{
    // Define grids over x and y dimensions
    int nX = 40;
    int nY = 20;
    std::vector<double> gridX(nX);
    std::vector<double> gridY(nY);
    for(int i = 0; i < nX; i++)
        gridX.at(i) = myXmin + (1.0 * i)/(nX - 1.0) * (myXmax - myXmin);
    for(int i = 0; i < nY; i++)
        gridY.at(i) = myYmin + (1.0 * i)/(nY - 1.0) * (myYmax - myYmin);

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