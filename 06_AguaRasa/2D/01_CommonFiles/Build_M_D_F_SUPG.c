#include "ShalowWater.h"
#include "../../../00_CommonFiles/Allocation_Operations/allocations.h"
#include "../../../00_CommonFiles/IO_Operations/io.h"
#include "../../../00_CommonFiles/BLAS_Operations/ourBLAS.h"


int Build_M_D_F_SUPG(ParametersType *Parameters, MatrixDataType *MatrixData, FemStructsType *FemStructs, FemFunctionsType *FemFunctions)
{
	double mu = 0.01;
	double rho = 1000;
	int e, i, j;
	int J1, J2, J3, nel, neq;
	double x1, x2, x3, y1, y2, y3, y23, y31, y12, x32, x13, x21;
	double Area, twoArea, invArea, third = 1.0/3.0, forth = 1.0/4.0, sixth = 1.0/6.0, twelve = 1.0/12.0;
	double c, h, abs_vbeta, betax, betay, betaxy, CFL;
	double tau, delta;
	double *U, *dU;
	double Me[9][9], De[9][9], Ub[3], dUb[3], Ue[9], dUe[9], gradUx[3], gradUy[3];
	double tolerance;
	double alpha = Parameters->Alpha_Build;
	double delta_t = Parameters->DeltaT_Build;
	double *R = FemStructs->F;
	double *delta_old = FemStructs->delta_old;
	int **lm = FemStructs->lm;

	NodeType *Node = FemStructs->Node;
	ElementType *Element = FemStructs->Element;
	
	tolerance = FemStructs->AuxBuild->tolerance;
	nel = Parameters->nel;
	neq = Parameters->neq;
	
	dzero(neq+1, R);
	setzeros(Parameters,MatrixData);

	U = (double*) mycalloc("U of 'Build_M_F_SUPG_Transiente'", 3*Parameters->nnodes, sizeof(double));
	dU = (double*) mycalloc("dU of 'Build_M_F_SUPG_Transiente'", 3*Parameters->nnodes, sizeof(double));
	eval_U_dU(Parameters,FemStructs,FemFunctions,U,dU);

	for (e = 0; e < nel; e++){
		// *** Nos do elemento
		J1 = Element[e].Vertex[0];
		J2 = Element[e].Vertex[1];
		J3 = Element[e].Vertex[2];

		// *** Coordenadas nodais e operador diferencial
		x1 = Node[J1].x;
		x2 = Node[J2].x;
		x3 = Node[J3].x;
		
		y1 = Node[J1].y;
		y2 = Node[J2].y;
		y3 = Node[J3].y;

		y23 = y2 - y3;
		y31 = y3 - y1;
		y12 = y1 - y2;

		x32 = x3 - x2;
		x13 = x1 - x3;
		x21 = x2 - x1;

		// *** Area do elemento
		twoArea = fabs(x21 * y31 - x13 * y12);
		Area = 0.5*twoArea;
		invArea = 1.0/Area;

		// Fill ' Ue ' with the values ​​of the previous solution or workaround for each degree of liberty of the element node
		// height
		Ue[0] = U[3*J1];
		Ue[3] = U[3*J2];
		Ue[6] = U[3*J3];

		dUe[0] = dU[3*J1];
		dUe[3] = dU[3*J2];
		dUe[6] = dU[3*J3];

		// discharge in x
		Ue[1] = U[3*J1+1];
		Ue[4] = U[3*J2+1];
		Ue[7] = U[3*J3+1];

		dUe[1] = dU[3*J1+1];
		dUe[4] = dU[3*J2+1];
		dUe[7] = dU[3*J3+1];

		// discharge in Y
		Ue[2] = U[3*J1+2];
		Ue[5] = U[3*J2+2];
		Ue[8] = U[3*J3+2];

		dUe[2] = dU[3*J1+2];
		dUe[5] = dU[3*J2+2];
		dUe[8] = dU[3*J3+2];

		// Baricentro do triangulo
		for (i = 0; i < 3; i++){
			Ub[i] = (Ue[i] + Ue[i+3] + Ue[i+6]) * third;
			dUb[i] = (dUe[i] + dUe[i+3] + dUe[i+6]) * third;
		}

		// *** Calculo do Gradiente (gradu = Bu)
		for (i = 0; i < 3; i++)
		{
			gradUx[i] = (Ue[i]*y23 + Ue[i+3]*y31 + Ue[i+6]*y12) / twoArea;
			gradUy[i] = (Ue[i]*x32 + Ue[i+3]*x13 + Ue[i+6]*x21) / twoArea;
		}

		// *** A = [A1 A2]
		double A1[3][3];
		double A2[3][3];
	
		FemFunctions->A1_A2_calculations(Ub, A1, A2);

		double axx[3][3], axy[3][3], ayx[3][3], ayy[3][3];

		axx[0][0] = A1[1][0];
		axx[0][1] = A1[1][1];
		axx[0][2] = 0.0;
		axx[1][0] = A1[1][0]*A1[1][0] + A1[1][1]*A1[2][0];
		axx[1][1] = A1[1][0] + A1[1][1]*A1[1][1];
		axx[1][2] = 0.0;
		axx[2][0] = A1[2][1]*A1[1][0] + A1[2][2]*A1[2][0];
		axx[2][1] = A1[2][0] + A1[2][1]*A1[1][1] + A1[2][2]*A1[2][1];
		axx[2][2] = A1[2][2]*A1[2][2];

		axy[0][0] = A2[1][0];
		axy[0][1] = A2[1][1];
		axy[0][2] = A2[1][2];
		axy[1][0] = A1[1][1]*A2[1][0];
		axy[1][1] = A1[1][1]*A2[1][1];
		axy[1][2] = A1[1][0] + A1[1][1]*A2[1][2];
		axy[2][0] = A1[2][1]*A2[1][0] + A1[2][2]*A2[2][0];
		axy[2][1] = A1[2][1]*A2[1][1];
		axy[2][2] = A1[2][0] + A1[2][1]*A2[1][2] + A1[2][2]*A2[2][2];

		ayx[0][0] = A1[2][0];
		ayx[0][1] = A1[2][1];
		ayx[0][2] = A1[2][2];
		ayx[1][0] = A2[1][1]*A1[1][0] + A2[1][2]*A1[2][0];
		ayx[1][1] = A2[1][0] + A2[1][1]*A1[1][1] + A2[2][1]*A1[2][1];
		ayx[1][2] = A2[1][2]*A2[2][2];
		ayx[2][0] = A2[2][2]*A1[2][0];
		ayx[2][1] = A2[2][0] + A2[2][2]*A1[2][1];
		ayx[2][2] = A2[2][2]*A2[2][2];

		ayy[0][0] = A2[2][0];
		ayy[0][1] = 0.0;
		ayy[0][2] = A2[2][2];
		ayy[1][0] = A2[1][1]*A2[1][0] + A2[1][2]*A2[2][0];
		ayy[1][1] = A2[1][1]*A2[1][1];
		ayy[1][2] = A2[1][0] + A2[1][1]*A2[2][0] + A2[2][1]*A2[2][2];
		ayy[2][0] = A2[2][0];
		ayy[2][1] = 0.0;
		axy[2][2] = A2[2][0] + A2[2][2]*A2[2][2];

		//  OPERADOR DE CAPTURA DE DESCONTINUIDADES
		//delta = FemFunctions->ShockCapture(tolerance, delta_old, gradUx, gradUy, Ax, Ay, A0, dUb, y23, y31, y12, x32, x13, x21,
		//									twoArea, e, Parameters->invY, Ub);

		// *** Calculo do parametro de estabilizacao (tau) do SUPG            
		betax = 2.0 * (gradUx[0]*Ub[0] + gradUx[1]*Ub[1] + gradUx[2]*Ub[2] + gradUx[3]*Ub[3]);
		betay = 2.0 * (gradUy[0]*Ub[0] + gradUy[1]*Ub[1] + gradUy[2]*Ub[2] + gradUy[3]*Ub[3]);

		betaxy = sqrt(betax*betax + betay*betay);    

		if (betaxy != 0.0)
		{
			betax = betax / betaxy;
			betay = betay / betaxy;
		}

		h = sqrt (twoArea);

		tau = 1.0; 
		if (tau < 0.0) 
			tau = 0.0;

		delta = 1.0;

		//------------------------------------------------------------------------------
		//  MONTAGENS DAS MATRIZES
		//------------------------------------------------------------------------------

		// *** Matriz de Massa do Galerkin
		double Mg1, Mg2;
		Mg1 = Area * sixth;
		Mg2 = Area * twelve;

		// *** Matriz de Massa do SUPG 
		double ms1[3][3], ms2[3][3], ms3[3][3];

		ms1[0][0] = 0.0;
		ms1[0][1] = y23;
		ms1[0][2] =                x32;
		ms1[1][0] = y23*A1[1][0] + x32*A2[1][0];
		ms1[1][1] = y23*A1[1][1] + x32*A2[1][1];
		ms1[1][2] =                x32*A2[1][2];
		ms1[2][0] = y23*A1[2][0] + x32*A2[2][0];
		ms1[2][1] = y23*A1[2][1];
		ms1[2][2] = y23*A1[2][2] + x32*A2[2][2];

		ms2[0][0] = 0.0;
		ms2[0][1] = y31;
		ms2[0][2] =                x13;
		ms2[1][0] = y31*A1[1][0] + x13*A2[1][0];
		ms2[1][1] = y31*A1[1][1] + x13*A2[1][1];
		ms2[1][2] =                x13*A2[1][2];
		ms2[2][0] = y31*A1[2][0] + x13*A2[2][0];
		ms2[2][1] = y31*A1[2][1];
		ms2[2][2] = y31*A1[2][2] + x13*A2[2][2];

		ms3[0][0] = 0.0;
		ms3[0][1] = y12;
		ms3[0][2] =                x21;
		ms3[1][0] = y12*A1[1][0] + x21*A2[1][0];
		ms3[1][1] = y12*A1[1][1] + x21*A2[1][1];
		ms3[1][2] =                x21*A2[1][2];
		ms3[2][0] = y12*A1[2][0] + x21*A2[2][0];
		ms3[2][1] = y12*A1[2][1];
		ms3[2][2] = y12*A1[2][2] + x21*A2[2][2];
		 

		// *** Coeficientes da Matriz de Massa [Me]

		Me[0][0] = Mg1 + tau*sixth*ms1[0][0];
		Me[0][1] = Mg1 + tau*sixth*ms1[0][1];
		Me[0][2] = Mg1 + tau*sixth*ms1[0][2];
		Me[1][0] = Mg1 + tau*sixth*ms1[1][0];
		Me[1][1] = Mg1 + tau*sixth*ms1[1][1];
		Me[1][2] = Mg1 + tau*sixth*ms1[1][2];
		Me[2][0] = Mg1 + tau*sixth*ms1[2][0];
		Me[2][1] = Mg1 + tau*sixth*ms1[2][1];
		Me[2][2] = Mg1 + tau*sixth*ms1[2][2];

		Me[3][3] = Mg1 + tau*sixth*ms2[0][0];
		Me[3][4] = Mg1 + tau*sixth*ms2[0][1];
		Me[3][5] = Mg1 + tau*sixth*ms2[0][2];
		Me[4][3] = Mg1 + tau*sixth*ms2[1][0];
		Me[4][4] = Mg1 + tau*sixth*ms2[1][1];
		Me[4][5] = Mg1 + tau*sixth*ms2[1][2];
		Me[5][3] = Mg1 + tau*sixth*ms2[2][0];
		Me[5][4] = Mg1 + tau*sixth*ms2[2][1];
		Me[5][5] = Mg1 + tau*sixth*ms2[2][2];

		Me[6][6] = Mg1 + tau*sixth*ms3[0][0];
		Me[6][7] = Mg1 + tau*sixth*ms3[0][1];
		Me[6][8] = Mg1 + tau*sixth*ms3[0][2];
		Me[7][6] = Mg1 + tau*sixth*ms3[1][0];
		Me[7][7] = Mg1 + tau*sixth*ms3[1][1];
		Me[7][8] = Mg1 + tau*sixth*ms3[1][2];
		Me[8][6] = Mg1 + tau*sixth*ms3[2][0];
		Me[8][7] = Mg1 + tau*sixth*ms2[2][1];
		Me[8][8] = Mg1 + tau*sixth*ms3[2][2];

		Me[0][6] = Me[0][3] = Mg2 + tau*sixth*ms1[0][0];
		Me[0][7] = Me[0][4] = Mg2 + tau*sixth*ms1[0][1];
		Me[0][8] = Me[0][5] = Mg2 + tau*sixth*ms1[0][2];
		Me[1][6] = Me[1][3] = Mg2 + tau*sixth*ms1[1][0];
		Me[1][7] = Me[1][4] = Mg2 + tau*sixth*ms1[1][1];
		Me[1][8] = Me[1][5] = Mg2 + tau*sixth*ms1[1][2];
		Me[2][6] = Me[2][3] = Mg2 + tau*sixth*ms1[2][0];
		Me[2][7] = Me[2][4] = Mg2 + tau*sixth*ms1[2][1];
		Me[2][8] = Me[2][5] = Mg2 + tau*sixth*ms1[2][2];

		Me[3][6] = Me[3][0] = Mg2 + tau*sixth*ms2[0][0];
		Me[3][7] = Me[3][1] = Mg2 + tau*sixth*ms2[0][1];
		Me[3][8] = Me[3][2] = Mg2 + tau*sixth*ms2[0][2];
		Me[4][6] = Me[4][0] = Mg2 + tau*sixth*ms2[1][0];
		Me[4][7] = Me[4][1] = Mg2 + tau*sixth*ms2[1][1];
		Me[4][8] = Me[4][2] = Mg2 + tau*sixth*ms2[1][2];
		Me[5][6] = Me[5][0] = Mg2 + tau*sixth*ms2[2][0];
		Me[5][7] = Me[5][1] = Mg2 + tau*sixth*ms2[2][1];
		Me[5][8] = Me[5][2] = Mg2 + tau*sixth*ms2[2][2];

		Me[6][3] = Me[6][0] = Mg2 + tau*sixth*ms3[0][0];
		Me[6][4] = Me[6][1] = Mg2 + tau*sixth*ms3[0][1];
		Me[6][5] = Me[6][2] = Mg2 + tau*sixth*ms3[0][2];
		Me[7][3] = Me[7][0] = Mg2 + tau*sixth*ms3[1][0];
		Me[7][4] = Me[7][1] = Mg2 + tau*sixth*ms3[1][1];
		Me[7][5] = Me[7][2] = Mg2 + tau*sixth*ms3[1][2];
		Me[8][3] = Me[8][0] = Mg2 + tau*sixth*ms3[2][0];
		Me[8][4] = Me[8][1] = Mg2 + tau*sixth*ms3[2][1];
		Me[8][5] = Me[8][2] = Mg2 + tau*sixth*ms3[2][2];


		// *** Matriz de Conveccao de Galerkin
		double d12[2][2], d13[2][2], d21[2][2], d23[2][2];

		d12[0][0] = 0.0;
		d12[0][1] =                  x32*x13*mu/rho;
		d12[1][0] = y23*y31*mu/rho;
		d12[1][1] = y23*y31*mu/rho + x32*x13*mu/rho;

		d13[0][0] = 0.0;
		d13[0][1] =                  x32*x21*mu/rho;
		d13[1][0] = y23*y12*mu/rho;
		d13[1][1] = y23*y12*mu/rho + x32*x21*mu/rho;

		d21[0][0] = 0.0;
		d21[0][1] =                  x13*x32*mu/rho;
		d21[1][0] = y31*y23*mu/rho;
		d21[1][1] = y31*y23*mu/rho + x13*x32*mu/rho;

		d23[0][0] = 0.0;
		d23[0][1] =                  x13*x21*mu/rho;
		d23[1][0] = y31*y12*mu/rho;
		d23[1][1] = y31*y12*mu/rho + x13*x21*mu/rho;

		// *** Matriz de Conveccao do SUPG
		double ds11[3][3], ds12[3][3], ds13[3][3], ds21[3][3], ds22[3][3], ds23[3][3], ds31[3][3], ds32[3][3], ds33[3][3];

		ds11[0][0] = y23*(y23*axx[0][0] + x32*ayx[0][0]) + x32*(y23*axy[0][0] + x32*ayy[0][0]);
		ds11[0][1] = y23*(y23*axx[0][1] + x32*ayx[0][1]) + x32*(y23*axy[0][1] + x32*ayy[0][1]);
		ds11[0][2] = y23*(y23*axx[0][2] + x32*ayx[0][2]) + x32*(y23*axy[0][2] + x32*ayy[0][2]);
		ds11[1][0] = y23*(y23*axx[1][0] + x32*ayx[1][0]) + x32*(y23*axy[1][0] + x32*ayy[1][0]);
		ds11[1][1] = y23*(y23*axx[1][1] + x32*ayx[1][1]) + x32*(y23*axy[1][1] + x32*ayy[1][1]);
		ds11[1][2] = y23*(y23*axx[1][2] + x32*ayx[1][2]) + x32*(y23*axy[1][2] + x32*ayy[1][2]);
		ds11[2][0] = y23*(y23*axx[2][0] + x32*ayx[2][0]) + x32*(y23*axy[2][0] + x32*ayy[2][0]);
		ds11[2][1] = y23*(y23*axx[2][1] + x32*ayx[2][1]) + x32*(y23*axy[2][1] + x32*ayy[2][1]);
		ds11[2][2] = y23*(y23*axx[2][2] + x32*ayx[2][2]) + x32*(y23*axy[2][2] + x32*ayy[2][2]);

		ds12[0][0] = y31*(y23*axx[0][0] + x32*ayx[0][0]) + x13*(y23*axy[0][0] + x32*ayy[0][0]);
		ds12[0][1] = y31*(y23*axx[0][1] + x32*ayx[0][1]) + x13*(y23*axy[0][1] + x32*ayy[0][1]);
		ds12[0][2] = y31*(y23*axx[0][2] + x32*ayx[0][2]) + x13*(y23*axy[0][2] + x32*ayy[0][2]);
		ds12[1][0] = y31*(y23*axx[1][0] + x32*ayx[1][0]) + x13*(y23*axy[1][0] + x32*ayy[1][0]);
		ds12[1][1] = y31*(y23*axx[1][1] + x32*ayx[1][1]) + x13*(y23*axy[1][1] + x32*ayy[1][1]);
		ds12[1][2] = y31*(y23*axx[1][2] + x32*ayx[1][2]) + x13*(y23*axy[1][2] + x32*ayy[1][2]);
		ds12[2][0] = y31*(y23*axx[2][0] + x32*ayx[2][0]) + x13*(y23*axy[2][0] + x32*ayy[2][0]);
		ds12[2][1] = y31*(y23*axx[2][1] + x32*ayx[2][1]) + x13*(y23*axy[2][1] + x32*ayy[2][1]);
		ds12[2][2] = y31*(y23*axx[2][2] + x32*ayx[2][2]) + x13*(y23*axy[2][2] + x32*ayy[2][2]);

		ds13[0][0] = y12*(y23*axx[0][0] + x32*ayx[0][0]) + x21*(y23*axy[0][0] + x32*ayy[0][0]);
		ds13[0][1] = y12*(y23*axx[0][1] + x32*ayx[0][1]) + x21*(y23*axy[0][1] + x32*ayy[0][1]);
		ds13[0][2] = y12*(y23*axx[0][2] + x32*ayx[0][2]) + x21*(y23*axy[0][2] + x32*ayy[0][2]);
		ds13[1][0] = y12*(y23*axx[1][0] + x32*ayx[1][0]) + x21*(y23*axy[1][0] + x32*ayy[1][0]);
		ds13[1][1] = y12*(y23*axx[1][1] + x32*ayx[1][1]) + x21*(y23*axy[1][1] + x32*ayy[1][1]);
		ds13[1][2] = y12*(y23*axx[1][2] + x32*ayx[1][2]) + x21*(y23*axy[1][2] + x32*ayy[1][2]);
		ds13[2][0] = y12*(y23*axx[2][0] + x32*ayx[2][0]) + x21*(y23*axy[2][0] + x32*ayy[2][0]);
		ds13[2][1] = y12*(y23*axx[2][1] + x32*ayx[2][1]) + x21*(y23*axy[2][1] + x32*ayy[2][1]);
		ds13[2][2] = y12*(y23*axx[2][2] + x32*ayx[2][2]) + x21*(y23*axy[2][2] + x32*ayy[2][2]);

		ds21[0][0] = y23*(y31*axx[0][0] + x13*ayx[0][0]) + x32*(y31*axy[0][0] + x13*ayy[0][0]);
		ds21[0][1] = y23*(y31*axx[0][1] + x13*ayx[0][1]) + x32*(y31*axy[0][1] + x13*ayy[0][1]);
		ds21[0][2] = y23*(y31*axx[0][2] + x13*ayx[0][2]) + x32*(y31*axy[0][2] + x13*ayy[0][2]);
		ds21[1][0] = y23*(y31*axx[1][0] + x13*ayx[1][0]) + x32*(y31*axy[1][0] + x13*ayy[1][0]);
		ds21[1][1] = y23*(y31*axx[1][1] + x13*ayx[1][1]) + x32*(y31*axy[1][1] + x13*ayy[1][1]);
		ds21[1][2] = y23*(y31*axx[1][2] + x13*ayx[1][2]) + x32*(y31*axy[1][2] + x13*ayy[1][2]);
		ds21[2][0] = y23*(y31*axx[2][0] + x13*ayx[2][0]) + x32*(y31*axy[2][0] + x13*ayy[2][0]);
		ds21[2][1] = y23*(y31*axx[2][1] + x13*ayx[2][1]) + x32*(y31*axy[2][1] + x13*ayy[2][1]);
		ds21[2][2] = y23*(y31*axx[2][2] + x13*ayx[2][2]) + x32*(y31*axy[2][2] + x13*ayy[2][2]);

		ds22[0][0] = y31*(y31*axx[0][0] + x13*ayx[0][0]) + x13*(y31*axy[0][0] + x13*ayy[0][0]);
		ds22[0][1] = y31*(y31*axx[0][1] + x13*ayx[0][1]) + x13*(y31*axy[0][1] + x13*ayy[0][1]);
		ds22[0][2] = y31*(y31*axx[0][2] + x13*ayx[0][2]) + x13*(y31*axy[0][2] + x13*ayy[0][2]);
		ds22[1][0] = y31*(y31*axx[1][0] + x13*ayx[1][0]) + x13*(y31*axy[1][0] + x13*ayy[1][0]);
		ds22[1][1] = y31*(y31*axx[1][1] + x13*ayx[1][1]) + x13*(y31*axy[1][1] + x13*ayy[1][1]);
		ds22[1][2] = y31*(y31*axx[1][2] + x13*ayx[1][2]) + x13*(y31*axy[1][2] + x13*ayy[1][2]);
		ds22[2][0] = y31*(y31*axx[2][0] + x13*ayx[2][0]) + x13*(y31*axy[2][0] + x13*ayy[2][0]);
		ds22[2][1] = y31*(y31*axx[2][1] + x13*ayx[2][1]) + x13*(y31*axy[2][1] + x13*ayy[2][1]);
		ds22[2][2] = y31*(y31*axx[2][2] + x13*ayx[2][2]) + x13*(y31*axy[2][2] + x13*ayy[2][2]);

		ds23[0][0] = y12*(y31*axx[0][0] + x13*ayx[0][0]) + x21*(y31*axy[0][0] + x13*ayy[0][0]);
		ds23[0][1] = y12*(y31*axx[0][1] + x13*ayx[0][1]) + x21*(y31*axy[0][1] + x13*ayy[0][1]);
		ds23[0][2] = y12*(y31*axx[0][2] + x13*ayx[0][2]) + x21*(y31*axy[0][2] + x13*ayy[0][2]);
		ds23[1][0] = y12*(y31*axx[1][0] + x13*ayx[1][0]) + x21*(y31*axy[1][0] + x13*ayy[1][0]);
		ds23[1][1] = y12*(y31*axx[1][1] + x13*ayx[1][1]) + x21*(y31*axy[1][1] + x13*ayy[1][1]);
		ds23[1][2] = y12*(y31*axx[1][2] + x13*ayx[1][2]) + x21*(y31*axy[1][2] + x13*ayy[1][2]);
		ds23[2][0] = y12*(y31*axx[2][0] + x13*ayx[2][0]) + x21*(y31*axy[2][0] + x13*ayy[2][0]);
		ds23[2][1] = y12*(y31*axx[2][1] + x13*ayx[2][1]) + x21*(y31*axy[2][1] + x13*ayy[2][1]);
		ds23[2][2] = y12*(y31*axx[2][2] + x13*ayx[2][2]) + x21*(y31*axy[2][2] + x13*ayy[2][2]);

		ds31[0][0] = y23*(y12*axx[0][0] + x21*ayx[0][0]) + x32*(y12*axy[0][0] + x21*ayy[0][0]);
		ds31[0][1] = y23*(y12*axx[0][1] + x21*ayx[0][1]) + x32*(y12*axy[0][1] + x21*ayy[0][1]);
		ds31[0][2] = y23*(y12*axx[0][2] + x21*ayx[0][2]) + x32*(y12*axy[0][2] + x21*ayy[0][2]);
		ds31[1][0] = y23*(y12*axx[1][0] + x21*ayx[1][0]) + x32*(y12*axy[1][0] + x21*ayy[1][0]);
		ds31[1][1] = y23*(y12*axx[1][1] + x21*ayx[1][1]) + x32*(y12*axy[1][1] + x21*ayy[1][1]);
		ds31[1][2] = y23*(y12*axx[1][2] + x21*ayx[1][2]) + x32*(y12*axy[1][2] + x21*ayy[1][2]);
		ds31[2][0] = y23*(y12*axx[2][0] + x21*ayx[2][0]) + x32*(y12*axy[2][0] + x21*ayy[2][0]);
		ds31[2][1] = y23*(y12*axx[2][1] + x21*ayx[2][1]) + x32*(y12*axy[2][1] + x21*ayy[2][1]);
		ds31[2][2] = y23*(y12*axx[2][2] + x21*ayx[2][2]) + x32*(y12*axy[2][2] + x21*ayy[2][2]);

		ds32[0][0] = y31*(y12*axx[0][0] + x21*ayx[0][0]) + x13*(y12*axy[0][0] + x21*ayy[0][0]);
		ds32[0][1] = y31*(y12*axx[0][1] + x21*ayx[0][1]) + x13*(y12*axy[0][1] + x21*ayy[0][1]);
		ds32[0][2] = y31*(y12*axx[0][2] + x21*ayx[0][2]) + x13*(y12*axy[0][2] + x21*ayy[0][2]);
		ds32[1][0] = y31*(y12*axx[1][0] + x21*ayx[1][0]) + x13*(y12*axy[1][0] + x21*ayy[1][0]);
		ds32[1][1] = y31*(y12*axx[1][1] + x21*ayx[1][1]) + x13*(y12*axy[1][1] + x21*ayy[1][1]);
		ds32[1][2] = y31*(y12*axx[1][2] + x21*ayx[1][2]) + x13*(y12*axy[1][2] + x21*ayy[1][2]);
		ds32[2][0] = y31*(y12*axx[2][0] + x21*ayx[2][0]) + x13*(y12*axy[2][0] + x21*ayy[2][0]);
		ds32[2][1] = y31*(y12*axx[2][1] + x21*ayx[2][1]) + x13*(y12*axy[2][1] + x21*ayy[2][1]);
		ds32[2][2] = y31*(y12*axx[2][2] + x21*ayx[2][2]) + x13*(y12*axy[2][2] + x21*ayy[2][2]);

		ds33[0][0] = y12*(y12*axx[0][0] + x21*ayx[0][0]) + x21*(y12*axy[0][0] + x21*ayy[0][0]);
		ds33[0][1] = y12*(y12*axx[0][1] + x21*ayx[0][1]) + x21*(y12*axy[0][1] + x21*ayy[0][1]);
		ds33[0][2] = y12*(y12*axx[0][2] + x21*ayx[0][2]) + x21*(y12*axy[0][2] + x21*ayy[0][2]);
		ds33[1][0] = y12*(y12*axx[1][0] + x21*ayx[1][0]) + x21*(y12*axy[1][0] + x21*ayy[1][0]);
		ds33[1][1] = y12*(y12*axx[1][1] + x21*ayx[1][1]) + x21*(y12*axy[1][1] + x21*ayy[1][1]);
		ds33[1][2] = y12*(y12*axx[1][2] + x21*ayx[1][2]) + x21*(y12*axy[1][2] + x21*ayy[1][2]);
		ds33[2][0] = y12*(y12*axx[2][0] + x21*ayx[2][0]) + x21*(y12*axy[2][0] + x21*ayy[2][0]);
		ds33[2][1] = y12*(y12*axx[2][1] + x21*ayx[2][1]) + x21*(y12*axy[2][1] + x21*ayy[2][1]);
		ds33[2][2] = y12*(y12*axx[2][2] + x21*ayx[2][2]) + x21*(y12*axy[2][2] + x21*ayy[2][2]);


		// *** Matriz de Conveccao da captura de choque
		double sh12, sh13, sh23;

		sh12 = y23*y31 + x32*x13;
		sh13 = y23*y12 + x32*x21;
		sh23 = y31*y12 + x13*x21;

		// *** Coeficientes da Matriz de Rigidez [De]

		De[0][0] = sixth*ms1[0][0] + forth*invArea*(-d12[0][0] -d13[0][0]) + tau*forth*invArea*ds11[0][0] + delta*forth*invArea*(-sh12 -sh13);
		De[0][1] = sixth*ms1[0][1] + forth*invArea*(-d12[0][1] -d13[0][1]) + tau*forth*invArea*ds11[0][1] + delta*forth*invArea*(-sh12 -sh13);
		De[0][2] = sixth*ms1[0][2] + forth*invArea*(-d12[0][2] -d13[0][2]) + tau*forth*invArea*ds11[0][2] + delta*forth*invArea*(-sh12 -sh13);
		De[1][0] = sixth*ms1[1][0] + forth*invArea*(-d12[1][0] -d13[1][0]) + tau*forth*invArea*ds11[1][0] + delta*forth*invArea*(-sh12 -sh13);
		De[1][1] = sixth*ms1[1][1] + forth*invArea*(-d12[1][1] -d13[1][1]) + tau*forth*invArea*ds11[1][1] + delta*forth*invArea*(-sh12 -sh13);
		De[1][2] = sixth*ms1[1][2] + forth*invArea*(-d12[1][2] -d13[1][2]) + tau*forth*invArea*ds11[1][2] + delta*forth*invArea*(-sh12 -sh13);
		De[2][0] = sixth*ms1[2][0] + forth*invArea*(-d12[2][0] -d13[2][0]) + tau*forth*invArea*ds11[2][0] + delta*forth*invArea*(-sh12 -sh13);
		De[2][1] = sixth*ms1[2][1] + forth*invArea*(-d12[2][1] -d13[2][1]) + tau*forth*invArea*ds11[2][1] + delta*forth*invArea*(-sh12 -sh13);
		De[2][2] = sixth*ms1[2][2] + forth*invArea*(-d12[2][2] -d13[2][2]) + tau*forth*invArea*ds11[2][2] + delta*forth*invArea*(-sh12 -sh13);

		De[0][3] = sixth*ms2[0][0] + forth*invArea*d12[0][0] + tau*forth*invArea*ds12[0][0] + delta*forth*invArea*sh12;
		De[0][4] = sixth*ms2[0][1] + forth*invArea*d12[0][1] + tau*forth*invArea*ds12[0][1] + delta*forth*invArea*sh12;
		De[0][5] = sixth*ms2[0][2] + forth*invArea*d12[0][2] + tau*forth*invArea*ds12[0][2] + delta*forth*invArea*sh12;
		De[1][3] = sixth*ms2[1][0] + forth*invArea*d12[1][0] + tau*forth*invArea*ds12[1][0] + delta*forth*invArea*sh12;
		De[1][4] = sixth*ms2[1][1] + forth*invArea*d12[1][1] + tau*forth*invArea*ds12[1][1] + delta*forth*invArea*sh12;
		De[1][5] = sixth*ms2[1][2] + forth*invArea*d12[1][2] + tau*forth*invArea*ds12[1][2] + delta*forth*invArea*sh12;
		De[2][3] = sixth*ms2[2][0] + forth*invArea*d12[2][0] + tau*forth*invArea*ds12[2][0] + delta*forth*invArea*sh12;
		De[2][4] = sixth*ms2[2][1] + forth*invArea*d12[2][1] + tau*forth*invArea*ds12[2][1] + delta*forth*invArea*sh12;
		De[2][5] = sixth*ms2[2][2] + forth*invArea*d12[2][2] + tau*forth*invArea*ds12[2][2] + delta*forth*invArea*sh12;

		De[0][6] = sixth*ms3[0][0] + forth*invArea*d13[0][0] + tau*forth*invArea*ds13[0][0] + delta*forth*invArea*sh13;
		De[0][7] = sixth*ms3[0][1] + forth*invArea*d13[0][1] + tau*forth*invArea*ds13[0][1] + delta*forth*invArea*sh13;
		De[0][8] = sixth*ms3[0][2] + forth*invArea*d13[0][2] + tau*forth*invArea*ds13[0][2] + delta*forth*invArea*sh13;
		De[1][6] = sixth*ms3[1][0] + forth*invArea*d13[1][0] + tau*forth*invArea*ds13[1][0] + delta*forth*invArea*sh13;
		De[1][7] = sixth*ms3[1][1] + forth*invArea*d13[1][1] + tau*forth*invArea*ds13[1][1] + delta*forth*invArea*sh13;
		De[1][8] = sixth*ms3[1][2] + forth*invArea*d13[1][2] + tau*forth*invArea*ds13[1][2] + delta*forth*invArea*sh13;
		De[2][6] = sixth*ms3[2][0] + forth*invArea*d13[2][0] + tau*forth*invArea*ds13[2][0] + delta*forth*invArea*sh13;
		De[2][7] = sixth*ms3[2][1] + forth*invArea*d13[2][1] + tau*forth*invArea*ds13[2][1] + delta*forth*invArea*sh13;
		De[2][8] = sixth*ms3[2][2] + forth*invArea*d13[2][2] + tau*forth*invArea*ds13[2][2] + delta*forth*invArea*sh13;

		De[3][0] = sixth*ms1[0][0] + forth*invArea*ds21[0][0] + tau*forth*invArea*ds12[0][0] + delta*forth*invArea*sh12;
		De[3][1] = sixth*ms1[0][1] + forth*invArea*ds21[0][1] + tau*forth*invArea*ds12[0][1] + delta*forth*invArea*sh12;
		De[3][2] = sixth*ms1[0][2] + forth*invArea*ds21[0][2] + tau*forth*invArea*ds12[0][2] + delta*forth*invArea*sh12;
		De[4][0] = sixth*ms1[1][0] + forth*invArea*ds21[1][0] + tau*forth*invArea*ds12[1][0] + delta*forth*invArea*sh12;
		De[4][1] = sixth*ms1[1][1] + forth*invArea*ds21[1][1] + tau*forth*invArea*ds12[1][1] + delta*forth*invArea*sh12;
		De[4][2] = sixth*ms1[1][2] + forth*invArea*ds21[1][2] + tau*forth*invArea*ds12[1][2] + delta*forth*invArea*sh12;
		De[5][0] = sixth*ms1[2][0] + forth*invArea*ds21[2][0] + tau*forth*invArea*ds12[2][0] + delta*forth*invArea*sh12;
		De[5][1] = sixth*ms1[2][1] + forth*invArea*ds21[2][1] + tau*forth*invArea*ds12[2][1] + delta*forth*invArea*sh12;
		De[5][2] = sixth*ms1[2][2] + forth*invArea*ds21[2][2] + tau*forth*invArea*ds12[2][2] + delta*forth*invArea*sh12;

		De[3][3] = sixth*ms2[3][3] + forth*invArea*(-d21[3][3] -d23[3][3]) + tau*forth*invArea*ds22[3][3] + delta*forth*invArea*(-sh12 -sh23);
		De[3][4] = sixth*ms2[3][4] + forth*invArea*(-d21[3][4] -d23[3][4]) + tau*forth*invArea*ds22[3][4] + delta*forth*invArea*(-sh12 -sh23);
		De[3][5] = sixth*ms2[3][5] + forth*invArea*(-d21[3][5] -d23[3][5]) + tau*forth*invArea*ds22[3][5] + delta*forth*invArea*(-sh12 -sh23);
		De[4][3] = sixth*ms2[4][3] + forth*invArea*(-d21[4][3] -d23[4][3]) + tau*forth*invArea*ds22[4][3] + delta*forth*invArea*(-sh12 -sh23);
		De[4][4] = sixth*ms2[4][4] + forth*invArea*(-d21[4][4] -d23[4][4]) + tau*forth*invArea*ds22[4][4] + delta*forth*invArea*(-sh12 -sh23);
		De[4][5] = sixth*ms2[4][5] + forth*invArea*(-d21[4][5] -d23[4][5]) + tau*forth*invArea*ds22[4][5] + delta*forth*invArea*(-sh12 -sh23);
		De[5][3] = sixth*ms2[5][3] + forth*invArea*(-d21[5][3] -d23[5][3]) + tau*forth*invArea*ds22[5][3] + delta*forth*invArea*(-sh12 -sh23);
		De[5][4] = sixth*ms2[5][4] + forth*invArea*(-d21[5][4] -d23[5][4]) + tau*forth*invArea*ds22[5][4] + delta*forth*invArea*(-sh12 -sh23);
		De[5][5] = sixth*ms2[5][5] + forth*invArea*(-d21[5][5] -d23[5][5]) + tau*forth*invArea*ds22[5][5] + delta*forth*invArea*(-sh12 -sh23);

		De[3][6] = sixth*ms3[0][0] + forth*invArea*d23[0][0] + tau*forth*invArea*ds23[0][0] + delta*forth*invArea*sh23;
		De[3][7] = sixth*ms3[0][1] + forth*invArea*d23[0][1] + tau*forth*invArea*ds23[0][1] + delta*forth*invArea*sh23;
		De[3][8] = sixth*ms3[0][2] + forth*invArea*d23[0][2] + tau*forth*invArea*ds23[0][2] + delta*forth*invArea*sh23;
		De[4][6] = sixth*ms3[1][0] + forth*invArea*d23[1][0] + tau*forth*invArea*ds23[1][0] + delta*forth*invArea*sh23;
		De[4][7] = sixth*ms3[1][1] + forth*invArea*d23[1][1] + tau*forth*invArea*ds23[1][1] + delta*forth*invArea*sh23;
		De[4][8] = sixth*ms3[1][2] + forth*invArea*d23[1][2] + tau*forth*invArea*ds23[1][2] + delta*forth*invArea*sh23;
		De[5][6] = sixth*ms3[2][0] + forth*invArea*d23[2][0] + tau*forth*invArea*ds23[2][0] + delta*forth*invArea*sh23;
		De[5][7] = sixth*ms3[2][1] + forth*invArea*d23[2][1] + tau*forth*invArea*ds23[2][1] + delta*forth*invArea*sh23;
		De[5][8] = sixth*ms3[2][2] + forth*invArea*d23[2][2] + tau*forth*invArea*ds23[2][2] + delta*forth*invArea*sh23;

		De[6][0] = sixth*ms1[0][0] + forth*invArea*d13[0][0] + tau*forth*invArea*ds31[0][0] + delta*forth*invArea*sh13;
		De[6][1] = sixth*ms1[0][1] + forth*invArea*d13[0][1] + tau*forth*invArea*ds31[0][1] + delta*forth*invArea*sh13;
		De[6][2] = sixth*ms1[0][2] + forth*invArea*d13[0][2] + tau*forth*invArea*ds31[0][2] + delta*forth*invArea*sh13;
		De[7][0] = sixth*ms1[1][0] + forth*invArea*d13[1][0] + tau*forth*invArea*ds31[1][0] + delta*forth*invArea*sh13;
		De[7][1] = sixth*ms1[1][1] + forth*invArea*d13[1][1] + tau*forth*invArea*ds31[1][1] + delta*forth*invArea*sh13;
		De[7][2] = sixth*ms1[1][2] + forth*invArea*d13[1][2] + tau*forth*invArea*ds31[1][2] + delta*forth*invArea*sh13;
		De[8][0] = sixth*ms1[2][0] + forth*invArea*d13[2][0] + tau*forth*invArea*ds31[2][0] + delta*forth*invArea*sh13;
		De[8][1] = sixth*ms1[2][1] + forth*invArea*d13[2][1] + tau*forth*invArea*ds31[2][1] + delta*forth*invArea*sh13;
		De[8][2] = sixth*ms1[2][2] + forth*invArea*d13[2][2] + tau*forth*invArea*ds31[2][2] + delta*forth*invArea*sh13;

		De[6][3] = sixth*ms2[0][0] + forth*invArea*d23[0][0] + tau*forth*invArea*ds32[0][0] + delta*forth*invArea*sh23;
		De[6][4] = sixth*ms2[0][1] + forth*invArea*d23[0][1] + tau*forth*invArea*ds32[0][1] + delta*forth*invArea*sh23;
		De[6][5] = sixth*ms2[0][2] + forth*invArea*d23[0][2] + tau*forth*invArea*ds32[0][2] + delta*forth*invArea*sh23;
		De[7][3] = sixth*ms2[1][0] + forth*invArea*d23[1][0] + tau*forth*invArea*ds32[1][0] + delta*forth*invArea*sh23;
		De[7][4] = sixth*ms2[1][1] + forth*invArea*d23[1][1] + tau*forth*invArea*ds32[1][1] + delta*forth*invArea*sh23;
		De[7][5] = sixth*ms2[1][2] + forth*invArea*d23[1][2] + tau*forth*invArea*ds32[1][2] + delta*forth*invArea*sh23;
		De[8][3] = sixth*ms2[2][0] + forth*invArea*d23[2][0] + tau*forth*invArea*ds32[2][0] + delta*forth*invArea*sh23;
		De[8][4] = sixth*ms2[2][1] + forth*invArea*d23[2][1] + tau*forth*invArea*ds32[2][1] + delta*forth*invArea*sh23;
		De[8][5] = sixth*ms2[2][2] + forth*invArea*d23[2][2] + tau*forth*invArea*ds32[2][2] + delta*forth*invArea*sh23;

		De[6][6] = sixth*ms3[6][6] + forth*invArea*(-d21[6][6] -d23[6][6]) + tau*forth*invArea*ds33[6][6] + delta*forth*invArea*(-sh13 -sh23);
		De[6][7] = sixth*ms3[6][7] + forth*invArea*(-d21[6][7] -d23[6][7]) + tau*forth*invArea*ds33[6][7] + delta*forth*invArea*(-sh13 -sh23);
		De[6][8] = sixth*ms3[6][8] + forth*invArea*(-d21[6][8] -d23[6][8]) + tau*forth*invArea*ds33[6][8] + delta*forth*invArea*(-sh13 -sh23);
		De[7][6] = sixth*ms3[7][6] + forth*invArea*(-d21[7][6] -d23[7][6]) + tau*forth*invArea*ds33[7][6] + delta*forth*invArea*(-sh13 -sh23);
		De[7][7] = sixth*ms3[7][7] + forth*invArea*(-d21[7][7] -d23[7][7]) + tau*forth*invArea*ds33[7][7] + delta*forth*invArea*(-sh13 -sh23);
		De[7][8] = sixth*ms3[7][8] + forth*invArea*(-d21[7][8] -d23[7][8]) + tau*forth*invArea*ds33[7][8] + delta*forth*invArea*(-sh13 -sh23);
		De[8][6] = sixth*ms3[8][6] + forth*invArea*(-d21[8][6] -d23[8][6]) + tau*forth*invArea*ds33[8][6] + delta*forth*invArea*(-sh13 -sh23);
		De[8][7] = sixth*ms3[8][7] + forth*invArea*(-d21[8][7] -d23[8][7]) + tau*forth*invArea*ds33[8][7] + delta*forth*invArea*(-sh13 -sh23);
		De[8][8] = sixth*ms3[8][8] + forth*invArea*(-d21[8][8] -d23[8][8]) + tau*forth*invArea*ds33[8][8] + delta*forth*invArea*(-sh13 -sh23);

		//*****************************************

		//Fill local Ae and Re and Global R
		double Re[9], MedUe[9], KeUe[9], Ae[9][9];
		
		for (i=0; i<9; i++){
			MedUe[i] = 0;
			KeUe[i] = 0;
			for (j=0; j<9; j++){
				MedUe[i] += Me[i][j]*dUe[j];
				KeUe[i] += De[i][j]*Ue[j];
				Ae[i][j] = Me[i][j] + alpha*delta_t*De[i][j];
			}
			Re[i] = - MedUe[i] - KeUe[i];
		}


		for (i=0; i<9; i++)
			R[lm[e][i]] += Re[i];
		R[neq] = 0;
		
		FemFunctions->assembly(Parameters, MatrixData, FemStructs, e, Ae);

	}

	myfree(U);
	myfree(dU);

	return 0;
}

int main() {
	return 0;
}