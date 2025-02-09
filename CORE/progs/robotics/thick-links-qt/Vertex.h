/*
 *  Vertex.h
 *  
 *  Author: Ching-Hsiang (Tom) Hsu
 *  Email: chhsu@nyu.edu
 *  Modified: Ching-Hsiang (Tom) Hsu, Oct. 31, 2016
 */

#ifndef VERTEX
#define VERTEX

#include "Pose.h"
#include "Color.h"

class Vertex{
public:
    Pose pos;
    Color clr;

    Vertex(Pose p, Color c):pos(p), clr(c){}
    Vertex(Pose p):pos(p), clr(Color(1,1,1,1)){}
};

#endif // VERTEX

