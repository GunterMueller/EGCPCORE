/* **************************************
   File: Wall.cpp

   Description: 

   HISTORY: March, 2012: Cong Wang, Chee Yap and Yi-Jen Chiang

   Since Core Library  Version 2.1
   $Id: $
 ************************************** */

#include "Wall.h"
#include "Corner.h"
#include "Box.h"

#include <float.h>

Wall::Wall(Corner* s, Corner* d):src(s), dst(d)
{
	src->nextWall = this;
	dst->preWall = this;
}

//	distance to line segment,
//	follows http://local.wasp.uwa.edu.au/~pbourke/geometry/pointline/
double Wall::distance(double x, double y)
{
	double x1 = src->x;
	double x2 = dst->x;
	double y1 = src->y;
	double y2 = dst->y;
	double u = ( (x-x1)*(x2-x1) + (y-y1)*(y2-y1) ) / ( (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) );
	u = u < 0 ? 0 : u;
	u = u > 1 ? 1 : u;
	double x0 = u*x2 + (1-u)*x1;
	double y0 = u*y2 + (1-u)*y1;
	return sqrt( (x-x0)*(x-x0) + (y-y0)*(y-y0) );
}

double Wall::distance_star(double x, double y)
{
    double x1 = src->x;
    double x2 = dst->x;
    double y1 = src->y;
    double y2 = dst->y;
    double u = ( (x-x1)*(x2-x1) + (y-y1)*(y2-y1) ) / ( (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) );

    if(u>0 && u<1){
        double x0 = u*x2 + (1-u)*x1;
        double y0 = u*y2 + (1-u)*y1;
        return sqrt( (x-x0)*(x-x0) + (y-y0)*(y-y0) );
    }

    return FLT_MAX;
}

short Wall::distance_sign(double x, double y)
{
    double x1 = src->x;
    double x2 = dst->x;
    double y1 = src->y;
    double y2 = dst->y;
    double u = ( (x-x1)*(x2-x1) + (y-y1)*(y2-y1) ) / ( (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) );

    if(u>0 && u<1){
        return 0;
    }

    if(u>=0) return 1;
    return -1;
}

bool Wall::inZone(Box * b)
{
    //center x,y
    double x=b->x;
    double y=b->y;
    double w2=b->width/2;  //half of width
    double h2=b->height/2; //half of height

    double corner1[2]={x-w2,y-h2};
    double corner2[2]={x+w2,y-h2};
    double corner3[2]={x+w2,y+h2};
    double corner4[2]={x-w2,y+h2};

    //check with the Zone of the wall
    short s1=distance_sign(corner1[0],corner1[1]);
    short s2=distance_sign(corner2[0],corner2[1]);
    short s3=distance_sign(corner3[0],corner3[1]);
    short s4=distance_sign(corner4[0],corner4[1]);


    if(s1==0 || s2==0 || s3==0 || s4==0)
        return true;

    if(s1!=s2 || s2!=s3 || s3!=s4 || s4!=s1 || s1==0)
        return true;

    return false;
}

bool Wall::inZone_star(Box * b)
{
    if(inZone(b)==false) return false;

    //center x,y
    double x=b->x;
    double y=b->y;
    double w2=b->width/2;  //half of width
    double h2=b->height/2; //half of height

    double corner1[2]={x-w2,y-h2};
    double corner2[2]={x+w2,y-h2};
    double corner3[2]={x+w2,y+h2};
    double corner4[2]={x-w2,y+h2};

    //check the side of the wall
    bool r1=isRight(corner1[0],corner1[1]);
    bool r2=isRight(corner2[0],corner2[1]);
    bool r3=isRight(corner3[0],corner3[1]);
    bool r4=isRight(corner4[0],corner4[1]);

    int count=0;
    if(r1) count++;
    if(r2) count++;
    if(r3) count++;
    if(r4) count++;

    if(count==0) return false; //all on the left side
    if(count>1) return true;   //at least two points on the right side

    //count==1

    //check with the Zone of the wall
    short s1=distance_sign(corner1[0],corner1[1]);
    short s2=distance_sign(corner2[0],corner2[1]);
    short s3=distance_sign(corner3[0],corner3[1]);
    short s4=distance_sign(corner4[0],corner4[1]);

    if(r1 && s1==0) return true; //in the zone and on the right
    if(r2 && s2==0) return true;
    if(r3 && s3==0) return true;
    if(r4 && s4==0) return true;

    //otherwise, check if the end points of this wall are in the box
    if( b->in(src->x,src->y) || b->in(dst->x,dst->y) )
        return true;

    return false;
}

bool Wall::isRight(double x, double y)
{
	double x1 = src->x;
	double x2 = dst->x;
	double y1 = src->y;
	double y2 = dst->y;
	return (x2-x1)*(y-y1) - (y2-y1)*(x-x1) < 0;
}
