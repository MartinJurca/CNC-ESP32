#include <Arduino.h>
#include <cmath>

#ifndef _POINT_
#define _POINT_

struct point
{
    int x;
    int y;
    int z;
};

float Distance(point p1, point p2)
{
    if ((p1.x == p2.x) && (p1.y == p2.y)) return 0.0;
    float x1_ = (float)p1.x, y1_ = (float)p1.y;
    float x2_ = (float)p2.x, y2_ = (float)p2.y;
    float clenx = x2_ - x1_;
    float cleny = y2_ - y1_;
    clenx = pow(clenx, 2);
    cleny = pow(cleny, 2);
    return sqrt(clenx + cleny);
}

point VectorTo(point p1, point p2)
{
    point newp;
    newp.x = p2.x - p1.x;
    newp.y = p2.y - p1.y;
    newp.z = p2.z - p1.z;
    return newp;
}

#endif