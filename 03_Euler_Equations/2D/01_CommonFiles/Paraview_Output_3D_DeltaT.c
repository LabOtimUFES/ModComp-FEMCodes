#include "EulerEquations.h"
#include "../../../00_CommonFiles/IO_Operations/io.h"
#include "../../../00_CommonFiles/Allocation_Operations/allocations.h"

int Paraview_Output_3D_DeltaT(double *U, NodeType *Node, ElementType *Element, ParametersType *Parameters, double (*rhopresc)(double, double), double (*v1presc)(double, double),
                    double (*v2presc)(double, double), double (*epresc)(double, double), double (*Gamma)(double, double), double (*CV)(double, double), double t)
{
	int I, eq1, eq2, eq3, eq4, nnodes, nel;
	char FileName[2000];
	FILE *OutFile;
	double *rho, *v1, *v2, *e, *temp, *pres, aux, cv, gamma, X, Y;

	nnodes = Parameters->nnodes;
	nel = Parameters->nel;

	rho = (double*) mycalloc("rho of 'Paraview_Output_3D'", nnodes, sizeof(double));
	v1 = (double*) mycalloc("v1 of 'Paraview_Output_3D'", nnodes, sizeof(double));
	v2 = (double*) mycalloc("v2 of 'Paraview_Output_3D'", nnodes, sizeof(double));
	e = (double*) mycalloc("e of 'Paraview_Output_3D'", nnodes, sizeof(double));
	temp = (double*) mycalloc("temp of 'Paraview_Output_3D'", nnodes, sizeof(double));
	pres = (double*) mycalloc("pres of 'Paraview_Output_3D'", nnodes, sizeof(double));

	for (I = 0; I < nnodes; I++){
		eq1 = Node[I].id[0];
		eq2 = Node[I].id[1];
		eq3 = Node[I].id[2];
		eq4 = Node[I].id[3];
		X = Node[I].x;
		Y = Node[I].y;
		cv = CV(X, Y);
		gamma = Gamma(X, Y);

		if (eq1 >= 0)
			rho[I] = U[eq1];
		else
			rho[I] = rhopresc(X, Y);

		if (eq2 >= 0)
			v1[I] = U[eq2];
		else
			v1[I] = v1presc(X, Y);

	   	if (eq3 >= 0)
	   		v2[I] = U[eq3];
	   	else
			v2[I] = v2presc(X, Y);

		if (eq4 >= 0)
			e[I] = U[eq4];
		else
			e[I] = epresc(X, Y);

		aux = e[I] - (v1[I]*v1[I] + v2[I]*v2[I])/(2.0*rho[I]);
		temp[I] = aux/(rho[I]*cv);
		pres[I] = (gamma - 1)*aux;
	}
	sprintf(FileName,"../03_output/%s_%s_%s_%s_%s_%s_%s_N%d_E%d_T%lf_3D.vtu", Parameters->Experiments, Parameters->ProblemTitle, Parameters->StabilizationForm, Parameters->ShockCapture,
			Parameters->TimeIntegration,Parameters->MatrixVectorProductScheme, Parameters->Preconditioner, Parameters->nnodes, Parameters->nel, t);
	OutFile = myfopen(FileName,"w");

	fprintf(OutFile,"<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"BigEndian\">\n");
	fprintf(OutFile,"\t<UnstructuredGrid>\n");
	fprintf(OutFile,"\t\t<Piece NumberOfPoints=\"%d\" NumberOfCells=\"%d\">\n", nnodes, nel);
	fprintf(OutFile,"\t\t\t<PointData Scalars=\"scalars\">\n");
	fprintf(OutFile,"\t\t\t\t<DataArray type=\"Float32\" Name=\"Density\" Format=\"ascii\">\n");

	for (I = 0; I < nnodes; I++)
		fprintf(OutFile,"\t\t\t\t   %.12lf\n", rho[I]);

	fprintf(OutFile,"\t\t\t\t</DataArray>\n");
	fprintf(OutFile,"\t\t\t\t<DataArray type=\"Float32\" Name=\"Temperature\" Format=\"ascii\">\n");

	for (I = 0; I < nnodes; I++)
		fprintf(OutFile,"\t\t\t\t   %.12lf\n", temp[I]);

	fprintf(OutFile,"\t\t\t\t</DataArray>\n");
	fprintf(OutFile,"\t\t\t\t<DataArray type=\"Float32\" Name=\"Pressure\" Format=\"ascii\">\n");

	for (I = 0; I < nnodes; I++)
		fprintf(OutFile,"\t\t\t\t   %.12lf\n", pres[I]);

	fprintf(OutFile,"\t\t\t\t</DataArray>\n");
	fprintf(OutFile,"\t\t\t</PointData>\n");
	fprintf(OutFile,"\t\t\t<Points>\n");
	fprintf(OutFile,"\t\t\t\t<DataArray type=\"Float32\" NumberOfComponents=\"3\" Format=\"ascii\">\n");

	for (I = 0; I < nnodes; I++)
		fprintf(OutFile,"\t\t\t\t   %.12lf\t%.12lf\t%.12lf\n", Node[I].x,Node[I].y, rho[I]);

	fprintf(OutFile,"\t\t\t\t</DataArray>\n");
	fprintf(OutFile,"\t\t\t</Points>\n");
	fprintf(OutFile,"\t\t\t<Cells>\n");
	fprintf(OutFile,"\t\t\t\t<DataArray type=\"Int32\" Name=\"connectivity\" Format=\"ascii\">\n");

	for (I = 0; I < nel; I++)
		fprintf(OutFile,"\t\t\t\t   %d\t%d\t%d\n", Element[I].Vertex[0], Element[I].Vertex[1], Element[I].Vertex[2]);

	fprintf(OutFile,"\t\t\t\t</DataArray>\n");
	fprintf(OutFile,"\t\t\t\t<DataArray type=\"Int32\" Name=\"offsets\" Format=\"ascii\">\n");

	for (I = 0; I < nel; I++)
		fprintf(OutFile,"\t\t\t\t   %d\n",(I+1)*3);

	fprintf(OutFile,"\t\t\t\t</DataArray>\n");
	fprintf(OutFile,"\t\t\t\t<DataArray type=\"Int32\" Name=\"types\" Format=\"ascii\">\n");

	for (I = 0; I < nel; I++)
			fprintf(OutFile,"\t\t\t\t   %d\n",5);

	fprintf(OutFile,"\t\t\t\t</DataArray>\n");
	fprintf(OutFile,"\t\t\t</Cells>\n");
	fprintf(OutFile,"\t\t</Piece>\n");
	fprintf(OutFile,"\t</UnstructuredGrid>\n");
	fprintf(OutFile,"</VTKFile>\n");

	fclose(OutFile);

	myfree(rho);
	myfree(v1);
	myfree(v2);
	myfree(e);
	myfree(temp);
	myfree(pres);

	return 0;
}
