#ifndef LOCATION_H
#define LOCATION_H

#include <stdbool.h>

#define RANGE_DISTANCE 2

struct coordinates{
    int x;
    int y;
};

// returns (x, y) grid coordinates of location given number of rows, cols
struct coordinates get_coordinates(int, int , int);
// returns Euclidean distance between two points with given coords
unsigned int get_euclidean_distance(struct coordinates, struct coordinates);
// returns location given (x, y) grid coords and number of cols
int get_location(int, struct coordinates);
// returns true if two locations within range (floor(euclidean)<=RANGE_DISTANCE)
bool is_in_range(int, int, int, int);

#endif
