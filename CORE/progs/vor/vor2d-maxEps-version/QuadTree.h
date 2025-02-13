/* **************************************

   File: QuadTree.h
   $Id: QuadTree.h 1149 2012-06-25 06:13:37Z jmlien $

   Description: 

   HISTORY: March, 2012: Created by Cong Wang, Chee Yap and Yi-Jen Chiang
            May, 2012: Updated by Jyh-Ming Lien for Voronoi diagram

   Since Core Library  Version 2.1


 ************************************** */

#pragma once
#include <iostream>
#include "Box.h"
//#include "UnionFind.h"
#include "PriorityQueue.h"

extern int freeCount;
extern int stuckCount;
extern int mixCount;
extern int mixSmallCount;

class QuadTree
{
private:


	//insert Node to PQ
	void insertNode(Box* b)
	{
		switch (b->getStatus())
		{
		case Box::OUT:
			++freeCount;
			break;
		case Box::ON:
			++stuckCount;
			if (b->height > maxEpsilon) PQ->push(b);
			break;
		case Box::IN:
			++mixCount;
			if (b->height < epsilon || b->width < epsilon)
				++mixSmallCount;
			PQ->push(b);
			break;
		case Box::UNKNOWN:
			std::cout << "UNKNOWN not handled?" << std::endl;
			break;
		}
	}

public:

	BoxQueue* PQ;
	Box* pRoot;
	double epsilon;		// minimum size
	double maxEpsilon;	// maximum size (if candidate box)

	QuadTree(Box* root, double e, double f):
	    pRoot(root), epsilon(e), maxEpsilon(f)
	{
		PQ = new seqQueue();
		assert(PQ);

		//Chee: pRoot->updateStatus(maxEpsilon);
		pRoot->updateStatus();
		insertNode(pRoot);
	}

	void subdividePhase ()
	{
	    int ct = 0;
	    while(PQ->empty()==false)
	    {
	        expand();
	        ct++;
	    }
	}//subdividePhase

	//
	// balance the quadtree so that the depth difference of leaves
	// is at most 1
	//
	void balancePhase ()
	{
	    list<Box*> leaves;
	    pRoot->getLeaves(leaves);

	    //we use a heap here to retrieve the deepest leaves
	    vector< pair<int,Box*> > box_pq;
	    for(list<Box*>::iterator i=leaves.begin();i!=leaves.end();i++)
	    {
	        Box * box=*i;
	        pair<int,Box*> tmp(box->depth,box);
	        box_pq.push_back(tmp);
	        push_heap(box_pq.begin(),box_pq.end());
	    }


	    //loop until the heap is empty
	    while(box_pq.empty()==false)
	    {

	        pair<int,Box*> tmp=box_pq.front();
	        Box * box=tmp.second;
	        pop_heap(box_pq.begin(),box_pq.end());
	        box_pq.pop_back();

	        //check the neighbors
	        for(int i=0;i<4;i++){

                Box * nei=box->pChildren[i]; //for leaves, neighbors are stored in pChildren

	            if(nei==NULL) continue;

	            //visiting all neighbors
	            if(nei->depth < box->depth-1)
	            {
	                if(nei->status==Box::OUT) continue; //no need to split

	                Box::Status backup_nei_status=nei->status;
	                nei->status=Box::IN; //force to split
	                bool results=nei->split(epsilon, maxEpsilon); //ask neighbor to slip
	                nei->status=backup_nei_status;

	                if(results) //the neighbor did split
	                {
                        for(int k=0;k<4;k++){ //enqueue neighbors' kid

                            Box * nei_kid=nei->pChildren[k];

                            //Chee: nei_kid->updateStatus(maxEpsilon);
                            nei_kid->updateStatus();
                            if(nei_kid->status==Box::IN)
                            {
                                PQ->push(nei_kid);
                            }

                            pair<int,Box*> tmp(nei_kid->depth,nei_kid);
                            box_pq.push_back(tmp);
                            push_heap(box_pq.begin(),box_pq.end());
                        }//end for k
	                }
	                else{
	                    cout<<"! Warning: QuadTree::balancePhase: split failed"<<endl;
	                }
	            }
	        }//end for i
	    }//end while
	}//balancePhase

	//
	// call each box to construct itself
	//
	void constructPhase ()
	{
        list<Box*> leaves;
        pRoot->getLeaves(leaves);

        for(list<Box*>::iterator i=leaves.begin();i!=leaves.end();i++)
        {
            Box * box=*i;
            box->buildVor();
        }
	}//constructPhase

	Box* getBox(Box* root, double x, double y)
	{
		if (x > root->x + root->width / 2 || x < root->x - root->width / 2
			|| y > root->y + root->height / 2 || y < root->y - root->height / 2)
		{
			return 0;
		}

		Box* b = root;
		while (!b->isLeaf)
		{
			double dx = x - b->x;
			double dy = y - b->y;
			if (dx <= 0 && dy >= 0)
			{
				b = b->pChildren[0];
			}
			else if (dx >= 0 && dy >= 0)
			{
				b = b->pChildren[1];
			}
			else if (dx >= 0 && dy <= 0)
			{
				b = b->pChildren[2];
			}
			else if (dx <= 0 && dy <= 0)
			{
				b = b->pChildren[3];
			}
		}
		return b;
	}

	Box* getBox(double x, double y)
	{
		return getBox(pRoot, x, y);
	}

	// Chee: I restored this version of expand...
	/*
	bool expand (Box* b)
	{
		if (!b->split(epsilon, maxEpsilon))
		{
			return false;
		}

		for (int i = 0; i < 4; ++i)
		{
			b->pChildren[i]->updateStatus();
			insertNode(b->pChildren[i]);
		}

		return true;
	}//expand (Box* b)
	*/

	// THIS expand() is funny: why does it have a loop?
	// 	Why not use expand(box* b) above?
	/**/
	bool expand ()
	{
		while(!PQ->empty())
		{
			Box* b = PQ->extract();
			//b might not be a leaf since it could already be split in expand(Box* b), and PQ is not updated there
			if (b->isLeaf && b->split(epsilon, maxEpsilon))
			{
			    // Chee: The following assertion seems wrong now: we could
			    // have split even though status is not equal to IN.
			    //
			    // WHY?  Because box status could be equal to ON
			    //        and box height is greated than maxEps!
				//Chee: assert(b->status == Box::IN);
				assert(b->status == Box::IN || b->status == Box::ON);
				for (int i = 0; i < 4; ++i)
				{
					//chee: b->pChildren[i]->updateStatus(maxEpsilon);
					b->pChildren[i]->updateStatus();
					insertNode(b->pChildren[i]);
				}			
				return true;
			}			
		}
		return false;
	}//expand
	/**/
	
	~QuadTree(void)
	{
	}
};
