#include "../01_CommonFiles/TranspEquation.h"
#include "pudim.h" 

int PUDIM_InitialSolution(ParametersType *Parameters, NodeType* Node, double *u)
{
	int I, nnodes;
	double X, Y;
	
	nnodes = Parameters->nnodes;

	for(I=0; I<nnodes; I++){
		if (Node[I].id>=0){
			X = Node[I].x;
			Y = Node[I].y;
			if (fabs(X)<=1e-14 && Y<=0.)
				u[Node[I].id] = -sin(2*PI*Y);
			else
				u[Node[I].id] = 0;
		}
	}
	return 0;
}


