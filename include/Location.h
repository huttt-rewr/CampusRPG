#ifndef LOCATION_H
#define LOCATION_H

#include <string>
#include <vector>
#include <map>

class Location {
public:
    std::string name;
    std::string description;
    int x, y;                     // Map coordinates
    std::vector<int> connections; // Connected location indices
    std::vector<int> enemyPool;   // Which enemies can appear here
    std::string icon;             // Emoji icon
    bool hasRest;                 // Can rest here to recover HP
    bool hasShop;                 // Has a shop

    Location() : x(0), y(0), hasRest(false), hasShop(false) {}
    Location(const std::string& n, const std::string& d, int px, int py,
             const std::string& ic, bool rest = false, bool shop = false)
        : name(n), description(d), x(px), y(py), icon(ic),
          hasRest(rest), hasShop(shop) {}
};

class GameMap {
public:
    std::vector<Location> locations;
    int currentLocation;

    GameMap();
    void initCampusMap();

    // Movement
    std::vector<int> getReachableLocations() const;
    std::string moveTo(int locationIndex);
    Location& getCurrentLocation();
    std::string getCurrentLocationName() const;

    // Random encounter
    int getRandomEncounter() const;  // Returns enemy index or -1

    // Rest
    std::string rest();
};

#endif
