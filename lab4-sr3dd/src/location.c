#include "location.h"

#include <math.h>

struct coordinates get_coordinates(int location, int rows, int cols){

    //location out of grid
    if(location > rows*cols){
        return (struct coordinates){.x = rows, .y = cols};
    }

    struct coordinates grid_position = {
                                            .x = (location-1)/cols, 
                                            .y = ((location-1)%cols)
                                        };

    return grid_position;
}

unsigned int get_euclidean_distance(struct coordinates source, 
                                        struct coordinates target){
    double euclidean_dist = floor(sqrt((pow(target.x - source.x, 2) + 
                                    pow(target.y - source.y, 2))));

    return (unsigned int)euclidean_dist;
}

int get_location(int cols, struct coordinates grid_position){
    int location = (grid_position.x*cols) + grid_position.y + 1;

    return location;
}

bool is_in_range(int rows, int cols, int source_location, int target_location){

    unsigned int euclidean_dist = 
            get_euclidean_distance(get_coordinates(source_location, rows, cols),
                                   get_coordinates(target_location, rows, cols));

    return euclidean_dist <= RANGE_DISTANCE;
}
