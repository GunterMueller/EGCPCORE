/*
 * file: Feature.h
 *
 * 	This is the main file that controls the CORE_LEVEL...
 * 		Choose CORE_LEVEL to be 1, 2 or 3.
 * 		CORE_LEVEL 0 will default to CORE_LEVEL 1.
 * 	We prefer to default to LEVEL 1
 * 		as it is slower for LEVEL 2 or 3.
 *
 * 	Author: Jyh-Ming Lien
 * 		Chee Yap
 * 	Since Core 2.1
 * 	$Id:$
 * ***************************************************/

#pragma once

#ifndef CORE_LEVEL
	#define CORE_LEVEL 1
#endif

#if (CORE_LEVEL==0)
	#undef CORE_LEVEL
	#define CORE_LEVEL 1
#endif

#include <stdlib.h>
#include "CORE.h"

#include "Vector.h"
#include "Point.h"

class Set; //defined in "UnionFind.h"
class Box; //defined in "Box.h"

class Feature
{
public:

    static int LEVEL;

    static int corelevel()
    {

#if CORE_LEVEL==1
        LEVEL=1;
#elif CORE_LEVEL==2
        LEVEL=2;
#elif CORE_LEVEL==3
        LEVEL=3;
#else
        LEVEL=4;
#endif
        return LEVEL;
    }

    Feature() { pSet=NULL; }
    virtual ~Feature(){}

    void static showLevel()
    {
        std::cout << "Core Level = " << LEVEL << std::endl;
    }

    //
    virtual double distance(const Point2d& p)=0;

    //
    virtual bool inZone(const Point2d& p)=0;
    virtual bool inZone_star(const Point2d& p)=0;

    //
    virtual bool inZone(Box * b)=0;
    virtual bool inZone_star(Box * b)=0;

	Set* pSet;   //?
};
