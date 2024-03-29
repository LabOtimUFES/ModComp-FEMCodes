#include "time_integration.h"

int Predictor_New(ParametersType *Parameters, MatrixDataType *MatrixData, FemStructsType *FemStructs,
		FemFunctionsType *FemFunctions, FemOtherFunctionsType *FemOtherFunctions)
{
	int I, i;
	int nel, neq, passo;
	double t, dt, alpha, norm_a, norm_Da, tol_correction;
	double *a, *aB, *auxVec, *Da, *DaB, *u, *u_old, **R2, *R2aux, *invN2, **M2, *M2aux, *F; //Parametros do Preditor
	double *uB, *delta_old, *delta_old_NMV;
	AuxBuildStructuresType *AuxBuild;
	
	nel = Parameters->nel;
	neq = Parameters->neq;
	
	a = (double*) mycalloc("a of 'Preditor_New'", neq + 1, sizeof(double));
	aB = (double*) mycalloc("aB of 'Preditor_New'",NDOF*nel, sizeof(double));
	Da = (double*) mycalloc("Da of 'Preditor_New'", neq + 1, sizeof(double));
	DaB = (double*) mycalloc("DaB of 'Preditor_New'", NDOF*nel, sizeof(double));
	auxVec = (double*) mycalloc("auxVec of 'Preditor_New'", neq + 1, sizeof(double));
	u_old = (double*) mycalloc("u_old of 'Preditor_New'", neq + 1, sizeof(double));
	R2 = (double**) mycalloc("R2 of 'Preditor_New'", nel, sizeof(double));
	R2aux = (double *) mycalloc("R2aux of 'Preditor_New'", NDOF*nel, sizeof(double));
	for (I = 0; I < nel;I++)
		R2[I] = &R2aux[NDOF*I]; 
	invN2 = (double*) mycalloc("invN2 of 'Preditor_New'", nel, sizeof(double));
	M2 = (double **) mycalloc("M2 of 'Preditor_New'", nel, sizeof(double*));
	M2aux = (double *) mycalloc("M2aux of 'Preditor_New'", NNOEL*NDOF*NDOF*nel, sizeof(double));
	for (I = 0; I < nel;I++)
		M2[I] = &M2aux[NNOEL*NDOF*NDOF*I]; 
	uB = (double*) mycalloc("uB of 'Preditor_New'", nel*NDOF, sizeof(double));
	delta_old = (double*) mycalloc("delta_old of 'Preditor_New'", nel, sizeof(double));
	delta_old_NMV = (double*) mycalloc("delta_old of 'Preditor_New'", nel, sizeof(double));

	u = FemStructs->u;
	F = FemStructs->F;
	dt = Parameters->DeltaT;
	Parameters->DeltaT_Build = dt;
	alpha = Parameters->Alpha;
	Parameters->Alpha_Build = alpha;
	tol_correction = Parameters->NonLinearTolerance;
	AuxBuild = (AuxBuildStructuresType*) mycalloc("AuxBuild of 'Predictor_New'",1,sizeof(AuxBuildStructuresType));
	AuxBuild->tolerance = Parameters->StabilizationTolerance;
	AuxBuild->M2 = M2;
	AuxBuild->R2 = R2;
	AuxBuild->invN2 = invN2;
	AuxBuild->delta_old_NMV = delta_old_NMV;
	FemStructs->AuxBuild = AuxBuild;
	FemStructs->delta_old = delta_old;
	FemStructs->du = a;
	FemStructs->uB = uB;
	FemStructs->duB = aB;

	FemFunctions->InitialSolution(Parameters, FemStructs->Node, u);
	uB_InitialSolution(Parameters, FemStructs, FemFunctions, u, uB);
	
	t = 0.0;
	passo = 0;
	int tag = 1;

	do{
		passo++;
		t += dt;
		
		#ifdef debug
			printf("\n\n Passo: %d\n", passo); 
		#endif
		//PREDICAO
		i = 0;
		
		dmemcpy(neq, u, u_old); // copy u to u_old		
		daxpy(neq, (1.0-alpha)*dt , a, u); // u  = u + (1-alpha)*dt*a
		memsetzero(neq,a); // set 0 in a
	
		daxpy(nel*NDOF, (1.0-alpha)*dt , aB, uB); // uB  = uB + (1-alpha)*dt*aB
		memsetzero(nel*NDOF,aB); // set 0 in aB
	
		// MULTICORRECAO
		do{
			i++;

			FemOtherFunctions->Build(Parameters, MatrixData, FemStructs, FemFunctions);
		
			FemFunctions->scaling(Parameters, MatrixData, FemStructs);

			FemFunctions->precond_setup(Parameters, MatrixData, FemStructs, tag++, F);

			FemOtherFunctions->solver(Parameters, MatrixData, FemStructs, FemFunctions, F, Da);
			
			FemFunctions->unscaling(Parameters, MatrixData, FemStructs, Da);

			calculate_DaB(Parameters, FemStructs, FemFunctions, Da, DaB);

			daxpy(neq, 1, Da, a);
			daxpy(neq, alpha*dt, Da, u);

			daxpy(nel*NDOF, 1, DaB, aB);
			daxpy(nel*NDOF, alpha*dt, DaB, uB);

			norm_a = sqrt(ddot(neq, a, a));
			norm_Da = sqrt(ddot(neq, Da, Da));
			#ifdef debug
				double normF;
				normF = sqrt(ddot(neq, F, F));
				printf("Tol_correction = %lf \t  Norma_Res =%lf \t Norm a = %lf \t  Norma Da = %lf \t t = %lf \t i = %d \n", 
					tol_correction*norm_a, normF, norm_a, norm_Da, t, i);
			#endif
		}while(!FemFunctions->StopCriteria(Parameters,norm_a,norm_Da,i)); // end while multicorrection
		
		
		#ifdef debug
			printf("\n\n"); 
		#endif
	
	}while(!FemFunctions->StopTimeIntegration(Parameters,u,u_old,t)); // end while time

	free(a);
	free(Da);
	free(u_old);
	free(auxVec);
	free(uB);
	free(DaB);
	free(delta_old);
	free(M2aux);
	free(M2);
	free(R2aux);
	free(R2);
	free(invN2);
	free(delta_old_NMV);
	free(AuxBuild);
	
	return 0;

}



