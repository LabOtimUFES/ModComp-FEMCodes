CC=gcc
#CC=tau_cc.sh
IDIR=
CFLAGS= -DSSTranspEquation2D -Ddebug -c -lm -lrt -Wall -Ofast -march=native
GCFLAGS= -DSSTranspEquation2D -Ddebug -lm -lrt -Wall -lrt -lm -Wall -Ofast -march=native
LDFLAGS=
SOURCES= main.c \
Preprocess.c \
Process.c \
Postprocess.c \
Fill_LM.c \
ede_Initialization.c \
csr_Initialization.c \
eval_U.c \
F_assembly.c \
Build_Galerkin.c \
Build_K_F_SUPG.c \
Build_K_F_DD.c \
ebe_assembly.c \
ede_assembly.c \
csr_assembly.c \
Paraview_Output.c \
Data_Output.c \
AproximationInEachNode.c \
CAU_ShockCapture.c \
CAU_DD_ShockCapture.c \
YZBeta_ShockCapture.c \
YZBeta_ShockCapture2.c \
h_shock_2sqrtArea.c \
h_shock_sqrtArea.c \
h_shock_Option1.c \
h_shock_Option2.c \
setStabilizationForm.c \
setSolver.c \
setPreconditioner.c \
setScaling.c \
setProblem.c \
setMatrixVectorProductType.c \
setzeros.c \
../CARTOLA/CARTOLA_Condutivity.c \
../CARTOLA/CARTOLA_Font.c \
../CARTOLA/CARTOLA_Reaction.c \
../CARTOLA/CARTOLA_Velocity.c \
../CARTOLA/CARTOLA_upresc.c \
../CONVECTION/CONVECTION_Condutivity.c \
../CONVECTION/CONVECTION_Font.c \
../CONVECTION/CONVECTION_Reaction.c \
../CONVECTION/CONVECTION_Velocity.c \
../CONVECTION/CONVECTION_upresc.c \
../HEMKER/HEMKER_Condutivity.c \
../HEMKER/HEMKER_Font.c \
../HEMKER/HEMKER_Reaction.c \
../HEMKER/HEMKER_Velocity.c \
../HEMKER/HEMKER_upresc.c \
../PUDIM/PUDIM_Condutivity.c \
../PUDIM/PUDIM_Font.c \
../PUDIM/PUDIM_Reaction.c \
../PUDIM/PUDIM_Velocity.c \
../PUDIM/PUDIM_upresc.c \
../REACTION/REACTION_Condutivity.c \
../REACTION/REACTION_Font.c \
../REACTION/REACTION_Reaction.c \
../REACTION/REACTION_Velocity.c \
../REACTION/REACTION_upresc.c \
../TESTE/TESTE_Condutivity.c \
../TESTE/TESTE_Font.c \
../TESTE/TESTE_Reaction.c \
../TESTE/TESTE_Velocity.c \
../TESTE/TESTE_upresc.c \
../RAMPA/RAMPA_Condutivity.c \
../RAMPA/RAMPA_Font.c \
../RAMPA/RAMPA_Reaction.c \
../RAMPA/RAMPA_Velocity.c \
../RAMPA/RAMPA_upresc.c \
../RAMPA2/RAMPA2_Condutivity.c \
../RAMPA2/RAMPA2_Font.c \
../RAMPA2/RAMPA2_Reaction.c \
../RAMPA2/RAMPA2_Velocity.c \
../RAMPA2/RAMPA2_upresc.c \
../VALIDACAO/VALIDACAO_Condutivity.c \
../VALIDACAO/VALIDACAO_Font.c \
../VALIDACAO/VALIDACAO_Reaction.c \
../VALIDACAO/VALIDACAO_Velocity.c \
../VALIDACAO/VALIDACAO_upresc.c \
../VALIDACAO/VALIDACAO_hflux.c \
../../../00_CommonFiles/Allocation_Operations/mycalloc.c \
../../../00_CommonFiles/BLAS_Operations/dtrsvUP.c \
../../../00_CommonFiles/BLAS_Operations/daxpy.c \
../../../00_CommonFiles/BLAS_Operations/dcopy.c \
../../../00_CommonFiles/BLAS_Operations/ddot.c \
../../../00_CommonFiles/BLAS_Operations/dscal.c \
../../../00_CommonFiles/BLAS_Operations/dzero.c \
../../../00_CommonFiles/BLAS_Operations/dmax.c \
../../../00_CommonFiles/CPU_Time_Operations/calculateTime.c \
../../../00_CommonFiles/IO_Operations/myfopen.c \
../../../00_CommonFiles/MatrixVector_Operations/EBE/ebemv.c \
../../../00_CommonFiles/MatrixVector_Operations/EDE/edemv.c \
../../../00_CommonFiles/MatrixVector_Operations/CSR/csrmv.c \
../../../00_CommonFiles/Solvers_and_Preconditioners/pgmres.c \
../../../00_CommonFiles/Solvers_and_Preconditioners/pcg.c \
../../../00_CommonFiles/Solvers_and_Preconditioners/NO_precond_setup.c \
../../../00_CommonFiles/Solvers_and_Preconditioners/CSR/ILUp_precond_setup.c \
../../../00_CommonFiles/Solvers_and_Preconditioners/CSR/Diag_precond_CSR_setup.c \
../../../00_CommonFiles/Solvers_and_Preconditioners/EBE/Diag_precond_EBE_setup.c \
../../../00_CommonFiles/Solvers_and_Preconditioners/EDE/Diag_precond_EDE_setup.c \
../../../00_CommonFiles/Solvers_and_Preconditioners/EBE/LU_precond_EBE_setup.c \
../../../00_CommonFiles/Solvers_and_Preconditioners/EDE/LU_precond_EDE_setup.c \
../../../00_CommonFiles/Solvers_and_Preconditioners/EBE/SGS_precond_EBE_setup.c \
../../../00_CommonFiles/Solvers_and_Preconditioners/EDE/SGS_precond_EDE_setup.c \
../../../00_CommonFiles/Solvers_and_Preconditioners/EBE/SSOR_precond_EBE_setup.c \
../../../00_CommonFiles/Solvers_and_Preconditioners/EBE/SSOR_precond_EBE_setup2.c \
../../../00_CommonFiles/Solvers_and_Preconditioners/EDE/SSOR_precond_EDE_setup.c \
../../../00_CommonFiles/Solvers_and_Preconditioners/EBE/SGS_precond_EBE.c \
../../../00_CommonFiles/Solvers_and_Preconditioners/EDE/SGS_precond_EDE.c \
../../../00_CommonFiles/Solvers_and_Preconditioners/EBE/SSOR_precond_EBE.c \
../../../00_CommonFiles/Solvers_and_Preconditioners/EDE/SSOR_precond_EDE.c \
../../../00_CommonFiles/Solvers_and_Preconditioners/EBE/LU_precond_EBE.c \
../../../00_CommonFiles/Solvers_and_Preconditioners/EDE/LU_precond_EDE.c \
../../../00_CommonFiles/Solvers_and_Preconditioners/NO_precond.c \
../../../00_CommonFiles/Solvers_and_Preconditioners/Diag_precond.c \
../../../00_CommonFiles/Solvers_and_Preconditioners/CSR/ILUp_precond.c \
../../../00_CommonFiles/Solvers_and_Preconditioners/scaling.c \
../../../00_CommonFiles/Reordering/reordering.c \
../../../00_CommonFiles/Reordering/symrcm.c \

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=SSTranspEquation2D

all: $(SOURCES) $(EXECUTABLE)

debug: CFLAGS= -DSSTranspEquation2D -Ddebug -c -lm -lrt -Wall -O0 -g
debug: GCFLAGS= -DSSTransEquation2D -Ddebug -lm -lrt -Wall -g
debug: all

fast: CFLAGS= -DSSTranspEquation2D -c -lm -lrt -Wall -Ofast -march=native
fast: GCFLAGS= -DSSTransEquation2D -lm -lrt -Wall -Ofast -march=native
fast: all

gprof: CFLAGS+=-pg -Ddebug
gprof: LDFLAGS+=-pg -Ddebug
gprof: all

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(GCFLAGS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
