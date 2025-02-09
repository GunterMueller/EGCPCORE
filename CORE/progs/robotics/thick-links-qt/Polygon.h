/*
 * Polygon.h
 *
 *  Created on: Jul 20, 2013
 *  Author: Zhongdi Luo
 */

#ifndef POLYGON_H_
#define POLYGON_H_

#include "Corner.h"

#include <vector>
#include <stdio.h>
#include <math.h>
using std::vector;

const int MAX = 101;

extern vector<int> srcInPolygons;

class Polygon {
public:
    vector<Corner*> corners;
};

struct point {
    int x, y;
};

extern vector<Polygon> polygons;

bool pointInPolygon(double x, double y, Polygon polygon);

#endif /* POLYGON_H_ */
