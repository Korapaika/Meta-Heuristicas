// ConsoleApplication1.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.
//
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <vector>
#include <cstring>
#include <string.h>
#include <time.h>
//#include <omp.h>

//#include "ABRKGA.h"


//------DEFINITION OF CONSTANTS, TYPES AND VARIABLES--------

//Tipo Solução
struct TSol
{
	vector<int> sol;
	double fo;
};


struct TSol2
{
	vector < vector <int> >  route;     // matriz com a sequencia de tarefas a serem realizadas por cada equipe
	vector <double> beginTime;          // vetor com o tempo de inicio de cada tarefa
	vector <double> travelTime;          // vetor com o tempo total percorrido na rota
	vector <double> z;                        // vetor com os tempos ociosos das equipes
	vector <int> bt;						//  tarefas não realizadas
};




// Run
char instance[50];                          // name of instance

//ABRKGA
int n;                                      // size of cromossoms
int p;                                      // size of population
int maxGenerations;                         // max number of generations
double pe;                                  // fraction of population to be the elite-set
double pm;                                  // fraction of population to be replaced by mutants
double rhoe;                                // probability that offspring inherit an allele from elite parent
double alfa;                                // threashold of restrict chromossoms list
double beta;                                // perturbation intensity
double sigma;                               // pearson correlation factor
double teta;                                // percentage of generations in an epoch
double gama;                                // cooling rate of the population

vector <TSol> Pop;                        // current population
vector <TSol> PopInter;                       // intermediary population

TSol bestSolution;                          // best solution found in the CS+BRKGA

// computational time
clock_t CPUbegin,                           // initial run time
CPUbest,                            // time to find the best solution in a run
CPUend;                             // end run time

//Problem specific
//(n is the number of tasks)
int m;                       // number of teams   
int M;                       // sum of big initial time windows (tasks and teams) and maximum travel cost
int MW;                    // minimum priority of a task
int MZ;                     // sum of the difference between the end e initial time windows of teams

vector <vector <double> > c;                // matrix with travel cost between tasks
vector <vector <double> > d;                // matrix with travel cost between garage of teams and tasks
vector <vector <int> > q;                       // matrix with skills between tasks and teams

struct TItem                                // struct with tasks informations
{
	int pt;                 //execuction time
	int w;                  //priority
	int a;                  //begin time windows
	int b;                  //end time windows
	double x;            //coordinate x
	double y;            //coordinate y
};

vector <TItem> task;            //tasks vector
vector <TItem> team;           //team vector
TSol2 result;					//results





/************************************************************************************
								FUNCTIONS AREA
*************************************************************************************/
void ReadData(char nameTable[]);
void FreeMemory();
void VNS();
void ILS();
double CalculateFO(TSol s, int print);
TSol TrocarBit(TSol s, int i, int j);
TSol SubidaTrocaBit(TSol s);
TSol Busca(TSol s, int k);
TSol GerarSolucaoInicial();
TSol OrdenaEquipes(TSol s);
TSol TrocarBit(TSol s, int i, int j);
int irandomico(int min, int max);
double randomico(double min, double max);
double RecalculateFO(TSol s);
TSol2 Perturba(int beta);
TSol OrdenaPrioridade(TSol s);
TSol2 Limpa(int pos);
TSol OrdenaArtigo(TSol s);



int main()
{
	//file with test instances
	FILE* arqProblems;
	int pc = 0;
	if (pc == 0)
		arqProblems = fopen("Instances\\arqProblems.txt", "r");
	else
		arqProblems = fopen("/Instances/arqProblems.txt", "r");

	if (arqProblems == NULL)
	{
		printf("\nERROR: File arqProblems.txt not found\n");
		getchar();
		exit(1);
	}

	// best solution that is saved in txt file
	TSol sBest;
	//sBest.sol = { 3,10,9,2,1,0,15,6,8,7,14,11,12,5,13,4 };
	bestSolution.fo = 221.12681;
	//bestSolution.fo = 61.13919;
	// run the method for all test instances
	while (!feof(arqProblems))
	{
		// read the name of instances
		char nameTable[100];
		fscanf(arqProblems, "%s", nameTable);

		//read the informations of the instance
		ReadData(nameTable);
		ILS();
		//VNS();
		//CalculateFO(sBest, 1);

		// free memory
		FreeMemory();
	}

	fclose(arqProblems);
	//system("pause");
	return 0;
}

/************************************************************************************
							  PROBLEM SPECIFIC FUNCTIONS
*************************************************************************************/



double CalculateFO(TSol s, int print)
{
	vector < vector <int> >  route;     // matriz com a sequencia de tarefas a serem realizadas por cada equipe
	vector <double> beginTime;          // vetor com o tempo de inicio de cada tarefa
	vector <double> travelTime;          // vetor com o tempo total percorrido na rota
	vector <double> z;                        // vetor com os tempos ociosos das equipes

	// initialize vectors
	beginTime.resize(n, -1);
	route.resize(m);
	travelTime.resize(m, 0);
	z.resize(m);

	// inicialmente as equipes possuem todo o tempo da janela como ociosa
	for (int j = 0; j < m; j++)
		z[j] = team[j].b - team[j].a;

	// find the best team and position to insert task s.sol[i]
	for (int i = 0; i < n; i++)
	{
		int achei = 0;                                          // achar alguma equipe que possa realizar a tarefa sol[i] 

		// global variables
		int bestTeam = 0;                                  // melhor equipe para fazer a tarefa
		int bestPos = 0;                                     // posicao da rota onde inserir a tarefa sol[i] na bestTeam
		double bestTime = 9999999;                  // tempo de viagem da melhor equipe
		double bestZ = 0;                                  // tempo ocioso da melhor equipe
		double bestInicio = 0;                            // tempo de inicio da tarefa sol[i] no melhor team e posicao

		// team variables
		int posStarTeam = 0;                              // melhor posicao para inserir a tarefa i na rota j 
		double tStarTeam = 9999999;                 // melhor tempo de uma rota j com a insercao da tarefa i
		double zStarTeam = -99999999;             // tempo ocioso da melhor posicao de insercao na rota j
		double inicioStarTeam = 0;                     // tempo no qual irei comecar a tarefa sol[i] na rota j

		// verify if is possible to insert task sol[i] in the team j
		for (int j = 0; j < m; j++)
		{
			// team j has skill to perform task s.sol[i]
			if (q[s.sol[i]][j] == 1)
			{
				// *** find best position to insert task s.sol[i]

				// route variables
				int posRoute = 0;                          // define a posicao de insercao da tarefa i na equipe j na posicao k
				double zStarRoute = 0;                 // define o melhor tempo ocioso da equipe j
				double tStarRoute = 0;                 // define a melhor alteracao no tempo de viagem da equipe j
				double inicioStarRoute = 0;           // define o melhor inicio para a tarefa i na rota j

				// iniciar variaveis das equipes
				posStarTeam = 0;                          // melhor posicao na rota j
				tStarTeam = 9999999;                   // melhor tempo na rota j

				int existe = 0;                               // there is an feasible position to insert task i in the route j
				double inicio = 0;                          // define o tempo de inicio da tarefa i na equipe j na posicao k

				// verificar se posso inserir em todas posicoes da rota j (inclusive no fim)
				for (int k = 0; k <= route[j].size(); k++)
				{
					// rota esta vazia
					if (route[j].empty())
					{
						// definir inicio da tarefa
						if (team[j].a + d[j][s.sol[i]] <= task[s.sol[i]].a)       // chegar antes da janela de inicio da tarefa (esperar)
							inicio = task[s.sol[i]].a;
						else                                                                     // chegar depois da janela de inicio da tarefa estar aberta
							inicio = team[j].a + d[j][s.sol[i]];

						// checar a janela de fim da tarefa e da equipe
						if ((inicio + task[s.sol[i]].pt <= task[s.sol[i]].b) && (inicio + task[s.sol[i]].pt + d[j][s.sol[i]] <= team[j].b))
						{
							if (team[j].b - (inicio + task[s.sol[i]].pt + d[j][s.sol[i]]) > zStarRoute)
							{
								zStarRoute = team[j].b - (inicio + task[s.sol[i]].pt + d[j][s.sol[i]]);      // tempo ocioso
								tStarRoute = d[j][s.sol[i]] + d[j][s.sol[i]];                                          // tempo de viagem (rota vazia inicialmente)

								existe = 1;
								posRoute = 0;
								inicioStarRoute = inicio;
							}
						}
					}

					// inserir antes da primeira tarefa (nao altera o tempo ocioso)
					else
						if (k == 0)
						{
							// definir o inicio
							if (team[j].a + d[j][s.sol[i]] <= task[s.sol[i]].a)       // chegar antes da janela de inicio da tarefa (esperar)
								inicio = task[s.sol[i]].a;
							else                                                                     // chegar depois da janela de inicio da tarefa estar aberta
								inicio = team[j].a + d[j][s.sol[i]];

							// verificar a janela de tempo
							if ((inicio + task[s.sol[i]].pt <= task[s.sol[i]].b) && ((inicio + task[s.sol[i]].pt + c[s.sol[i]][route[j][k]]) <= beginTime[route[j][k]]))
							{
								// verificar se eh a melhor posicao para inserir a tarefa nesta rota (maior tempo ocioso z)
								if (z[j] > zStarRoute)
								{
									zStarRoute = z[j];                                                                                                           // tempo ocioso nao muda
									tStarRoute = travelTime[j] + d[j][s.sol[i]] + c[s.sol[i]][route[j][k]] - d[j][route[j][k]];       // tempo de viagem atualizado

									existe = 1;
									posRoute = k;

									inicioStarRoute = inicio;
								}
							}
						}

					// inserir entre duas tarefas (nao altera o tempo ocioso)
						else
							if (k < route[j].size())
							{
								// definir o inicio 
								if (beginTime[route[j][k - 1]] + task[route[j][k - 1]].pt + c[route[j][k - 1]][s.sol[i]] <= task[s.sol[i]].a)       // chegar antes da janela de inicio da tarefa (esperar)
									inicio = task[s.sol[i]].a;
								else                                                                                                            // chegar depois da janela de inicio da tarefa estar aberta
									inicio = beginTime[route[j][k - 1]] + task[route[j][k - 1]].pt + c[route[j][k - 1]][s.sol[i]];

								//verificar a janela de tempo
								if ((inicio + task[s.sol[i]].pt <= task[s.sol[i]].b) && ((inicio + task[s.sol[i]].pt + c[s.sol[i]][route[j][k]]) <= beginTime[route[j][k]]))
								{
									// verificar se eh a melhor posicao para inserir a tarefa nesta rota (maior tempo ocioso z)
									if (z[j] > zStarRoute)
									{
										zStarRoute = z[j];                                                                                                                                          // tempo ocioso nao muda
										tStarRoute = travelTime[j] + c[route[j][k - 1]][s.sol[i]] + c[s.sol[i]][route[j][k]] - c[route[j][k - 1]][route[j][k]];    // tempo de viagem atualizado

										existe = 1;
										posRoute = k;

										inicioStarRoute = inicio;
									}
								}

							}

					// inserir apos a ultima tarefa
							else
								if (k == route[j].size())
								{
									// definir o inicio 
									if (beginTime[route[j][k - 1]] + task[route[j][k - 1]].pt + c[route[j][k - 1]][s.sol[i]] <= task[s.sol[i]].a)       // chegar antes da janela de inicio da tarefa (esperar)
										inicio = task[s.sol[i]].a;
									else                                                                                                            // chegar depois da janela de inicio da tarefa estar aberta
										inicio = beginTime[route[j][k - 1]] + task[route[j][k - 1]].pt + c[route[j][k - 1]][s.sol[i]];

									//verificar a janela de tempo
									if ((inicio + task[s.sol[i]].pt <= task[s.sol[i]].b) && (inicio + task[s.sol[i]].pt + d[j][s.sol[i]] <= team[j].b))
									{
										// verificar se eh a melhor posicao para inserir a tarefa nesta rota (maior tempo ocioso z)
										if (team[j].b - (inicio + task[s.sol[i]].pt + d[j][s.sol[i]]) > zStarRoute)
										{
											zStarRoute = team[j].b - (inicio + task[s.sol[i]].pt + d[j][s.sol[i]]);                                       // tempo ocioso 
											tStarRoute = travelTime[j] + c[route[j][k - 1]][s.sol[i]] + d[j][s.sol[i]] - d[j][route[j][k - 1]];    // tempo de viagem atualizado

											existe = 1;
											posRoute = k;

											inicioStarRoute = inicio;
										}
									}
								}

					// verificar se eh a melhor posicao para inserir a tarefa i na rota j
					if (zStarRoute > zStarTeam && existe == 1)
					{
						zStarTeam = zStarRoute;
						posStarTeam = posRoute;
						tStarTeam = tStarRoute;
						inicioStarTeam = inicioStarRoute;
					}

				} // end route

				// ** ja sei em qual posicao inserir (bestPos), agora tenho que verificar se eh a melhor equipe para realizar

				// verificar se eh a melhor equipe para realizar a tarefa (eh viavel ?)
				if (tStarTeam < bestTime && existe == 1)
				{
					bestTime = tStarTeam;

					bestTeam = j;                               // equipe que ira realizar a tarefa
					bestPos = posStarTeam;                // posicao que a tarefa sera inserida

					bestZ = zStarTeam;
					bestInicio = inicioStarTeam;

					achei = 1;
				}

			} // end if skill
		} // end team

		// insert task sol[i] in the best team and position
		if (achei == 1)
		{
			if (bestPos == route[bestTeam].size())
				route[bestTeam].push_back(s.sol[i]);
			else
				route[bestTeam].insert(route[bestTeam].begin() + bestPos, s.sol[i]);

			// update values of travelTime, z, beginTime
			travelTime[bestTeam] = bestTime;
			z[bestTeam] = bestZ;
			beginTime[s.sol[i]] = bestInicio;
		}

	} // end task


	// *** calculate fitness
	s.fo = 0;

	for (int j = 0; j < m; j++)
	{
		for (int i = 0; i < route[j].size(); i++)
		{
			// priotity prizes
			s.fo += task[route[j][i]].w / MW;

			// idle time
			if (i == route[j].size() - 1)
			{
				s.fo += (team[j].b - (beginTime[route[j][i]] + task[route[j][i]].pt + d[j][route[j][i]])) / MZ;
			}
		}
	}

	// transform in minimization problem
	s.fo = -s.fo;


	// verificar se eh a melhor solucao ate o momento
	if (s.fo < bestSolution.fo)
	{
		bestSolution = s;
		CPUbest = clock();
	}

	// print
	if (print == 1)
	{
		printf("\n\n");
		for (int i = 0; i < n; i++)
			printf("%d ", s.sol[i]);

		printf("\n\n");
		for (int j = 0; j < m; j++)
		{
			printf("\nTeam %d: ", j);
			for (int i = 0; i < route[j].size(); i++)
			{
				printf("%2d (%.2lf) - ", route[j][i], beginTime[route[j][i]]);
			}

			printf("\t [%.2lf] ", z[j]);
		}

		printf("\n\nFO: %.5lf \n\n", -s.fo);
	}

	result.route = route;
	result.beginTime = beginTime;
	result.travelTime = travelTime;
	result.z = z;
	for (int j = 0; j < m; j++)
	{
		for (int i = 0; i < route[j].size(); i++)
		{
			result.bt[route[j][i]] = 1;
		}
	}
	return -s.fo;
}


void ReadData(char nameTable[])
{
	char name[200] = "";
	int pc = 0;
	if (pc == 0)
		strcpy(name, "Instances\\");
	else
		strcpy(name, "Instances/");

	strcat(name, nameTable);

	FILE * arq;
	arq = fopen(name, "r");

	if (arq == NULL)
	{
		printf("\nERROR: File (%s) not found!\n", name);
		getchar();
		exit(1);
	}

	// => name of instance
	strcpy(instance, nameTable);

	// => read data
	int temp;

	// number of tasks and teams
	fscanf(arq, "%d %d", &n, &m);

	//task and garage information
	task.clear();
	task.resize(n);

	team.clear();
	team.resize(m);

	// priority of tasks 
	fscanf(arq, "%d", &temp); // first value is not considered
	for (int i = 0; i < n; i++)
		fscanf(arq, "%d", &task[i].w);

	// skills between tasks and teams
	q.clear();
	q.resize(n, vector<int>(m));

	//fscanf(arq, "%d %d", &temp, &temp);
	for (int i = 0; i < m; i++) {
		fscanf(arq, "%d", &temp); // first line is not read
	}
	
	for (int i = 0; i < n; i++)
		for (int j = 0; j < m; j++)
			fscanf(arq, "%d", &q[i][j]);

	// processing time of tasks 
	fscanf(arq, "%d", &temp);   // first value is not considered
	for (int i = 0; i < n; i++)
		fscanf(arq, "%d", &task[i].pt);

	// beginning time windows of tasks
	fscanf(arq, "%d", &temp);   // first value is not considered
	for (int i = 0; i < n; i++)
		fscanf(arq, "%d", &task[i].a);

	// end time windows of tasks
	fscanf(arq, "%d", &temp);   // first value is not considered
	for (int i = 0; i < n; i++)
		fscanf(arq, "%d", &task[i].b);

	// beginning time windows of teams
	for (int i = 0; i < m; i++)
		fscanf(arq, "%d", &team[i].a);

	// end time windows of teams
	for (int i = 0; i < m; i++)
		fscanf(arq, "%d", &team[i].b);

	fscanf(arq, "%d %d %d", &M, &MW, &MZ);


	// coordinates of teams (teams have the same location)
	double tempX, tempY;
	fscanf(arq, "%lf %lf ", &tempX, &tempY);
	for (int i = 0; i < m; i++)
	{
		team[i].x = tempX;
		team[i].y = tempY;
	}

	// coordinates of tasks
	for (int i = 0; i < n; i++)
		fscanf(arq, "%lf %lf ", &task[i].x, &task[i].y);

	fclose(arq);

	// calculate the Euclidean distances between tasks
	c.clear();
	c.resize(n, vector<double>(n));

	for (int i = 0; i < n; i++)
	{
		for (int j = i; j < n; j++)
		{
			c[i][j] = c[j][i] = sqrt((task[j].x - task[i].x) * (task[j].x - task[i].x) + (task[j].y - task[i].y) * (task[j].y - task[i].y));
		}
	}

	// calculate the Euclidean distances between task and teams garage
	d.clear();
	d.resize(m, vector<double>(n));

	for (int i = 0; i < m; i++)
	{
		for (int j = 0; j < n; j++)
		{
			d[i][j] = sqrt((task[j].x - team[i].x) * (task[j].x - team[i].x) + (task[j].y - team[i].y) * (task[j].y - team[i].y));
		}
	}

	//print
	/*printf("\n%d \n%d \n", n, m);

	//priority vector
	for (int i=0; i<n; i++)
		printf("%d \n", task[i].w);

	//skill matrix
	for (int i=0; i<n; i++)
	{
		printf("\n");
		for (int j=0; j<m; j++)
		{
			printf("%d ", q[i][j]);
		}
	}

	//processing time vector
	printf("\n");
	for (int i=0; i<n; i++)
		printf("%d ", task[i].pt);

	//beginning time vector
	printf("\n");
	for (int i=0; i<n; i++)
		printf("%d ", task[i].a);

	//end time vector
	printf("\n");
	for (int i=0; i<n; i++)
		printf("%d ", task[i].b);

	//beginning time vector
	printf("\n");
	for (int i=0; i<m; i++)
		printf("%d ", team[i].a);

	//end time vector
	printf("\n");
	for (int i=0; i<m; i++)
		printf("%d ", team[i].b);

	printf("\n\n%d \n%d \n%d", M, MW, MZ);

	//coordinates vector
	printf("\n");
	for (int i=0; i<m; i++)
		printf("%.6lf %.6lf \n", team[i].x, team[i].y);

	//coordinates vector
	printf("\n");
	for (int i=0; i<n; i++)
		printf("%.6lf %.6lf \n", task[i].x, task[i].y);

	//distances
	for (int i=0; i<n; i++)
	{
		printf("\n");
		for (int j=0; j<n; j++)
			printf("%.5lf \t", c[i][j]);
	}

	printf("\n");
	for (int i=0; i<m; i++)
	{
		printf("\n");
		for (int j=0; j<n; j++)
			printf("%.5lf \t", d[i][j]);
	}

	getchar();
	system("pause");*/
}

void FreeMemory()
{
	Pop.clear();
	PopInter.clear();
	c.clear();
	d.clear();
	q.clear();
	task.clear();
	team.clear();
}

double randomico(double min, double max)
{

	return ((double)(rand() % 10000) / 10000.0) * (max - min) + min;
}

int irandomico(int min, int max)
{

	return (int)randomico(0, max - min + 1.0) + min;
}









/************************************************************************************
*** Método: TrocarBit(TSol s, int i, int j)                                       ***
*** Função: Troca um bit da solução s                                             ***
*************************************************************************************/
TSol TrocarBit(TSol s, int i, int j)
{
	swap(s.sol[i], s.sol[j]);

	return s;
}




/************************************************************************************
*** Método: GerarSolucaoInicial()                                                 ***
*** Função: Gerar Solucao Inicial				                                  ***
*************************************************************************************/
TSol GerarSolucaoInicial()
{
	TSol s;
	s.sol.clear();
	s.sol.resize(n);
	result.bt.clear();
	result.bt.resize(n);
	for (int j = 0; j < n; j++) {
		s.sol[j] = j;
		result.bt[j] = 0;
	}
	/*Adicionar novas ordenações*/
	//s = OrdenaEquipes(s);
	//s = OrdenaPrioridade(s);
	s = OrdenaArtigo(s);
	s.fo = CalculateFO(s, 0);
	
	return s;
}

TSol OrdenaEquipes(TSol s)
{
	TSol novo;
	novo.sol.resize(n);
	vector <int> contador;
	contador.clear();
	contador.resize(n, 0);
	// conta equipes para cada tarefa
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++) {
			if (q[i][j] == 1) {
				contador[i]++;
			}
		}
	}
	// bubble sort
	for (int i = 0; i < n - 1; i++) {
		for (int j = i; j < n - 1; j++) {
			if (contador[j] > contador[j + 1]) {
				swap(contador[j], contador[j + 1]);
				swap(s.sol[j], s.sol[j + 1]);
			}
		}
	}
	// retorno ordenado por quantidade de equipes
	return s;
}

TSol OrdenaArtigo(TSol s) 
{
	vector <double> prioridade;
	prioridade.clear();
	prioridade.resize(n);

	// critério para n/m < 10
	if (n / m < 10) {
		vector <int> NST;
		NST.clear();
		NST.resize(n, 0);
		// conta equipes para cada tarefa
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < m; j++) {
				if (q[i][j] == 1) {
					NST[i]++;
				}
			}
		}
		// calcula a prioridade de ordem de cada tarefa
		for (int i = 0; i < n; i++) {
			prioridade[i] = task[i].w - (NST[i] / (m + 1));
		}
	}
	// critério para n/m > 10
	else {
		for (int i = 0; i < n; i++) {
			prioridade[i] = task[i].w + (task[i].w / task[i].pt) + (task[i].pt / (task[i].b - task[i].a));
		}
	}

	// ordena as tarefas de acordo com o novo critério
	for (int i = 0; i < n; i++) {
		int p = 0, pos = 0;
		for (int j = i; j < n; j++) {
			if (prioridade[j] > p) {
				pos = j;
				p = prioridade[j];
			}
		}
		swap(prioridade[i], prioridade[pos]);
		swap(s.sol[i], s.sol[pos]);
	}

	return s;
}







/************************************************************************************
*** Método: SubidaTrocaBit()                                                      ***
*** Função: Heuristica de subida que examina a troca de todos os bits             ***
*************************************************************************************/
/*TSol SubidaTrocaBit(TSol s, double min, double max)
{
	int melhorou = 1;
	int pos = irandomico(0, n - 1);
	TSol sLocal = s;       //armazena a melhor solucao vizinha

	TSol2 novo = Perturba(min, max);
	TSol2 anterior;

	while (melhorou)
	{
		melhorou = 0;

		for (int i = 0; i < n; i++)
		{
			if (i != pos) {
				anterior = result;
				//trocar um bit
				//s = TrocarBit(s, i);
				s = TrocarBit(s, i, pos);
				result = novo;
				s.fo = RecalculateFO(s, -1);
				//s.fo = CalculateFO(s, 0);

				//armazenar a melhor troca se a solucao melhorar
				if (s.fo > sLocal.fo)
				{
					sLocal = s;
					melhorou = 1;
					anterior = result;
				}

				//desfazer a troca e continuar a busca
				s = TrocarBit(s, pos, i);
			}
		}

		//continuar a busca a partir da melhor solucao
		s = sLocal;
	}

	result = anterior;

	return s;
}*/



TSol SubidaTrocaBit(TSol s)
{
	int melhorou = 1;
	TSol sLocal = s;
	TSol2 rLocal = result;       //armazena a solução corrente
	TSol2 rViz, rAnterior = result;		//armazena melhor solução vizinha
	int cont = 0;

	while (melhorou)
	{
		melhorou = 0;
		for (int i = 0; i < m; i++)
		{
			//trocar uma equipe
			rViz = Limpa(i);
			result = rViz;
			s.fo = RecalculateFO(s);

			//armazenar a melhor troca se a solucao melhorar
			if (s.fo > sLocal.fo)
			{
				sLocal = s;
				melhorou = 1;
				rLocal = result;
			}

			result = rAnterior;

		}

		//continuar a busca a partir da melhor solucao
		s = sLocal;
		rAnterior = rLocal;
		rViz = rAnterior;
		
		/*if (melhorou == 0 && cont < 3) {
			melhorou = 1;
			switch (cont) {
			case 0:
				s = OrdenaPrioridade(s);
				break;
			case 1:
				s = OrdenaEquipes(s);
				break;
			case 2:
				for (int i = 0; i < rand() % n; i++)
				{
					int pos1 = rand() % n;
					int pos2 = rand() % n;
					s = TrocarBit(s, pos1, pos2);
				}
				break;
			}
			sLocal = s;
			cont++;
		}*/
	}
	result = rLocal; //melhor solução encontrada
	return s;
}




/************************************************************************************
*** Método: Busca(TSol s, int k)                                                  ***
*** Função: Busca local de intervalo restrito                                     ***
*************************************************************************************/
TSol Busca(TSol s, int k)
{
	int melhorou = 1;
	//int pos = irandomico((0+k), (n - 1 - k));
	int pos = rand() % n;
	TSol sLocal = s;       //armazena a melhor solucao vizinha
	TSol2 anterior;

	anterior = result;

	while (melhorou)
	{
		melhorou = 0;

		//for (int i = (pos-k); i < (pos+k); i++)
		for(int i = 0; i < n; i++)
		{
			if (i != pos) {
				//trocar um bit
				//s = TrocarBit(s, i);
				s = TrocarBit(s, i, pos);
				//s.fo = CalculateFO(s, 0);
				//int pos2 = irandomico(0, m - 1);
				//s.fo = RecalculateFO(s);
				s.fo = CalculateFO(s, 0);

				//armazenar a melhor troca se a solucao melhorar
				if (s.fo > sLocal.fo)
				{
					sLocal = s;
					melhorou = 1;
					anterior = result;
				}

				//desfazer a troca e continuar a busca
				s = TrocarBit(s, pos, i);
			}
		}

		//continuar a busca a partir da melhor solucao
		s = sLocal;
		result = anterior;
	}

	return s;
}





/*****************************************************************************
*** Método: ITERATED LOCAL SEARCH                                          ***
*** Funcao: ILS retirado do problema da mochila                            ***
******************************************************************************/
void ILS()
{
	int beta = 0;                            //taxa de perturbação
	int Iter = 0;
	int IterMax = 2000;
	int IterMelhora = 0;
	int delta = 0;

	TSol s,
		sMelhor,
		sViz,
		sMelhorViz;

	TSol2 anterior,
		perturbado;

	//srand(time(NULL)); //seed para numeros aleatorios
	
	//***Criar a solucao inicial do ILS
	s = GerarSolucaoInicial();

	//aplicar busca local
	sMelhor = SubidaTrocaBit(s);
	//sMelhor = Busca(s, 0);

	//double betaMax = 1, betaMin = 0;
	int betaMax = 10, betaMin = 1;



	/*printf("\nDigite a intensidade minima da perturbacao: ");
	scanf("%d", &betaMin);

	printf("\nDigite a intensidade maxima da perturbacao: ");
	scanf("%d", &betaMax);*/


	while (Iter < IterMax)
		// while(Iter-IterMelhora < IterMax) // alteracao Galeno
	{
		Iter++;

		//s' <- perturbar a melhor solucao s*
		//beta = randomico(betaMin, betaMax);
		beta = rand() % betaMax + betaMin;

		//perturbar a solucao corrente (sMelhor)
		sViz = sMelhor;
		anterior = result;

		//perturba a ordem das tarefas

		//int aux = rand() % 3;
		/*switch (beta % 3) {
		case 0: 
			sViz = OrdenaPrioridade(sViz);
			break;
		case 1:
			sViz = OrdenaEquipes(sViz);
			break;
		case 2:
			for (int i = 0; i < beta; i++)
			{
				int pos1 = rand() % n;
				int pos2 = rand() % n;
				sViz = TrocarBit(sViz, pos1, pos2);
			}
			break;
		}*/

		// perturba a ordem do vetor de tarefas
		for (int i = 0; i < beta; i++)
		{
			int pos1 = rand() % n;
			int pos2 = rand() % n;
			sViz = TrocarBit(sViz, pos1, pos2);
		}
		// perturba a matriz de solução do problema
		perturbado = Perturba(beta);
		result = perturbado;
		sViz.fo = RecalculateFO(sViz);

		//s*' <- busca local (s')
		//sMelhorViz = SubidaTrocaBit(sViz, betaMin, betaMax);
		sMelhorViz = SubidaTrocaBit(sViz);
		//sMelhorViz = Busca(sViz, 0);

		//s* <- criterio de aceitação (s*,s*', historico)
		delta = sMelhorViz.fo - sMelhor.fo;
		if (delta > 0)
		{
			sMelhor = sMelhorViz;
			IterMelhora = Iter;
			anterior = result;
		}
		else {
			result = anterior;
		}
		printf("\nIter: %d \t fo: %.5f ", Iter, sMelhor.fo);

	} //fim-while_ILS


	printf("\n\n");
	for (int i = 0; i < n; i++)
		printf("%d ", sMelhor.sol[i]);

	printf("\n\n");
	for (int j = 0; j < m; j++)
	{
		printf("\nTeam %d: ", j);
		for (int i = 0; i < result.route[j].size(); i++)
		{
			printf("%2d (%.2lf) - ", result.route[j][i], result.beginTime[result.route[j][i]]);
		}

		printf("\t [%.2lf] ", result.z[j]);
	}

	printf("\n\nFO: %.5lf \n\n", sMelhor.fo);



	system("pause");
	system("cls");
}




/************************************************************************************
*** Método: VNS()                                                                 ***
*** Função: Aplica a metaheuristica VNS no FTSP                                   ***
*************************************************************************************/
void VNS()
{
	TSol s, sViz, sMelhor;

	int k, r;

	//numero de estruturas de vizinhança
	r = 16;
	//printf("\nDigite o numero de vizinhancas do VNS: ");
	//scanf("%d", &r);


	//GerarSolucaoInicial
	s = GerarSolucaoInicial();
	sMelhor = s;

	int numIteracoes = 0;
	while (numIteracoes <= 500)
	{
		//tipo de estrurura de vizinhança
		k = 1;

		while (k <= r && numIteracoes <= 500)
		{
			//comecar a busca a partir da melhor solucao
			sViz = sMelhor;

			//gerar o vizinho aleatoriamente na vizinhança k
			numIteracoes++;


			for (int i = 0; i < k; i++)
			{
				//int team = irandomico(0, m - 1);
				//int pos1 = irandomico(0, n - 1);
				int pos1 = rand() % n;
				//int pos2 = irandomico(0, n - 1);
				int pos2 = rand() % n;
				sViz = TrocarBit(sViz, pos1, pos2);
			}
			sViz.fo = CalculateFO(sViz, 0);
			//aplicar a busca local no vizinho gerado
			if (k > n) {
				sViz = Busca(sViz, n);
			}
			else {
				sViz = Busca(sViz, k);
			}
			//sViz = SubidaTrocaBit(sViz);
			sViz.fo = CalculateFO(sViz, 0);
			//se melhorar, continuar a busca a partir da solucao vizinha e retornar para 1a vizinhança
			if (sViz.fo > sMelhor.fo)
			{
				s = sViz;
				sMelhor = s;
				k = 1;
			}
			//senao, continuar a busca a partir da solucao anterior e partir para proxima vizinhança
			else
			{
				k++;
				
			}

			printf("\nVizinho: %d  fo: %.4f  Melhor Solucao = %.4f ", numIteracoes, sMelhor.fo, s.fo);
		}
	}
	system("cls");
	CalculateFO(s, 1);
	//printf("\nSolucao Otima = %.5f ", -bestSolution.fo);
	//ImprimirMelhorSol(sMelhor);
}








/************************************************************************************
*** Método: Limpa(int pos)                                                        ***
*** Função: Limpa as tarefas de uma equipe específica                             ***
*************************************************************************************/
TSol2 Limpa(int pos)
{
	TSol2 novo;
	novo = result;
	novo.z[pos] = team[pos].b - team[pos].a;
	for (int i = 0; i < novo.route[pos].size(); i++) {
		novo.bt[novo.route[pos][i]] = 0;
	}
	novo.route[pos].clear();

	return novo;
}



/************************************************************************************
*** Método: Perturba(int beta)				                                      ***
*** Função: Realiza uma perturbação nas equipes                                   ***
*************************************************************************************/
TSol2 Perturba(int beta)
{
	//srand(time(NULL)); //seed para numeros aleatorios

	TSol2 novo;
	novo = result;
	for (int j = 0; j < beta; j++) {
		int pos = rand() % m;
		novo.z[pos] = team[pos].b - team[pos].a;
		for (int i = 0; i < novo.route[pos].size(); i++) {
			novo.bt[novo.route[pos][i]] = 0;
		}
		novo.route[pos].clear();
	}
	
	
	
	return novo;
}






/************************************************************************************
*** Método: TSol OrdenaPrioridade(TSol s)                                         ***
*** Função: Ordena a lista de tarefas por prioridade                              ***
*************************************************************************************/
TSol OrdenaPrioridade(TSol s) {
	int aux, pos;
	for (int i = 0; i < n - 1; i++) {
		aux = task[s.sol[i]].w;
		pos = i;
		for (int j = i+1; j < n; j++) {
			if (task[s.sol[j]].w > aux) {
				aux = task[s.sol[j]].w;
				pos = j;
			}
		}
		swap(s.sol[i], s.sol[pos]);
	}
	return s;
}









/************************************************************************************
*** Método: double RecalculateFO(TSol s)                                          ***
*** Função: Recalcula FO de uma solução perturbada                                ***
*************************************************************************************/
double RecalculateFO(TSol s)
{
	vector < vector <int> >  route;     // matriz com a sequencia de tarefas a serem realizadas por cada equipe
	vector <double> beginTime;          // vetor com o tempo de inicio de cada tarefa
	vector <double> travelTime;          // vetor com o tempo total percorrido na rota
	vector <double> z;                        // vetor com os tempos ociosos das equipes


	beginTime = result.beginTime;
	route = result.route;
	travelTime = result.travelTime;
	z = result.z;

	// find the best team and position to insert task s.sol[i]
	for (int i = 0; i < n; i++)
	{
		int achei = 0;                                          // achar alguma equipe que possa realizar a tarefa sol[i] 

		// global variables
		int bestTeam = 0;                                  // melhor equipe para fazer a tarefa
		int bestPos = 0;                                     // posicao da rota onde inserir a tarefa sol[i] na bestTeam
		double bestTime = 9999999;                  // tempo de viagem da melhor equipe
		double bestZ = 0;                                  // tempo ocioso da melhor equipe
		double bestInicio = 0;                            // tempo de inicio da tarefa sol[i] no melhor team e posicao

		// team variables
		int posStarTeam = 0;                              // melhor posicao para inserir a tarefa i na rota j 
		double tStarTeam = 9999999;                 // melhor tempo de uma rota j com a insercao da tarefa i
		double zStarTeam = -99999999;             // tempo ocioso da melhor posicao de insercao na rota j
		double inicioStarTeam = 0;                     // tempo no qual irei comecar a tarefa sol[i] na rota j

		// verify if is possible to insert task sol[i] in the team j
		for (int j = 0; j < m; j++)
		{
			// team j has skill to perform task s.sol[i]
			if (q[s.sol[i]][j] == 1 && result.bt[s.sol[i]] == 0)
			{
				// *** find best position to insert task s.sol[i]

				// route variables
				int posRoute = 0;                          // define a posicao de insercao da tarefa i na equipe j na posicao k
				double zStarRoute = 0;                 // define o melhor tempo ocioso da equipe j
				double tStarRoute = 0;                 // define a melhor alteracao no tempo de viagem da equipe j
				double inicioStarRoute = 0;           // define o melhor inicio para a tarefa i na rota j

				// iniciar variaveis das equipes
				posStarTeam = 0;                          // melhor posicao na rota j
				tStarTeam = 9999999;                   // melhor tempo na rota j

				int existe = 0;                               // there is an feasible position to insert task i in the route j
				double inicio = 0;                          // define o tempo de inicio da tarefa i na equipe j na posicao k

				// verificar se posso inserir em todas posicoes da rota j (inclusive no fim)
				for (int k = 0; k <= route[j].size(); k++)
				{
					// rota esta vazia
					if (route[j].empty())
					{
						// definir inicio da tarefa
						if (team[j].a + d[j][s.sol[i]] <= task[s.sol[i]].a)       // chegar antes da janela de inicio da tarefa (esperar)
							inicio = task[s.sol[i]].a;
						else                                                                     // chegar depois da janela de inicio da tarefa estar aberta
							inicio = team[j].a + d[j][s.sol[i]];

						// checar a janela de fim da tarefa e da equipe
						if ((inicio + task[s.sol[i]].pt <= task[s.sol[i]].b) && (inicio + task[s.sol[i]].pt + d[j][s.sol[i]] <= team[j].b))
						{
							if (team[j].b - (inicio + task[s.sol[i]].pt + d[j][s.sol[i]]) > zStarRoute)
							{
								zStarRoute = team[j].b - (inicio + task[s.sol[i]].pt + d[j][s.sol[i]]);      // tempo ocioso
								tStarRoute = d[j][s.sol[i]] + d[j][s.sol[i]];                                          // tempo de viagem (rota vazia inicialmente)

								existe = 1;
								posRoute = 0;
								inicioStarRoute = inicio;
							}
						}
					}

					// inserir antes da primeira tarefa (nao altera o tempo ocioso)
					else
						if (k == 0)
						{
							// definir o inicio
							if (team[j].a + d[j][s.sol[i]] <= task[s.sol[i]].a)       // chegar antes da janela de inicio da tarefa (esperar)
								inicio = task[s.sol[i]].a;
							else                                                                     // chegar depois da janela de inicio da tarefa estar aberta
								inicio = team[j].a + d[j][s.sol[i]];

							// verificar a janela de tempo
							if ((inicio + task[s.sol[i]].pt <= task[s.sol[i]].b) && ((inicio + task[s.sol[i]].pt + c[s.sol[i]][route[j][k]]) <= beginTime[route[j][k]]))
							{
								// verificar se eh a melhor posicao para inserir a tarefa nesta rota (maior tempo ocioso z)
								if (z[j] > zStarRoute)
								{
									zStarRoute = z[j];                                                                                                           // tempo ocioso nao muda
									tStarRoute = travelTime[j] + d[j][s.sol[i]] + c[s.sol[i]][route[j][k]] - d[j][route[j][k]];       // tempo de viagem atualizado

									existe = 1;
									posRoute = k;

									inicioStarRoute = inicio;
								}
							}
						}

					// inserir entre duas tarefas (nao altera o tempo ocioso)
						else
							if (k < route[j].size())
							{
								// definir o inicio 
								if (beginTime[route[j][k - 1]] + task[route[j][k - 1]].pt + c[route[j][k - 1]][s.sol[i]] <= task[s.sol[i]].a)       // chegar antes da janela de inicio da tarefa (esperar)
									inicio = task[s.sol[i]].a;
								else                                                                                                            // chegar depois da janela de inicio da tarefa estar aberta
									inicio = beginTime[route[j][k - 1]] + task[route[j][k - 1]].pt + c[route[j][k - 1]][s.sol[i]];

								//verificar a janela de tempo
								if ((inicio + task[s.sol[i]].pt <= task[s.sol[i]].b) && ((inicio + task[s.sol[i]].pt + c[s.sol[i]][route[j][k]]) <= beginTime[route[j][k]]))
								{
									// verificar se eh a melhor posicao para inserir a tarefa nesta rota (maior tempo ocioso z)
									if (z[j] > zStarRoute)
									{
										zStarRoute = z[j];                                                                                                                                          // tempo ocioso nao muda
										tStarRoute = travelTime[j] + c[route[j][k - 1]][s.sol[i]] + c[s.sol[i]][route[j][k]] - c[route[j][k - 1]][route[j][k]];    // tempo de viagem atualizado

										existe = 1;
										posRoute = k;

										inicioStarRoute = inicio;
									}
								}

							}

					// inserir apos a ultima tarefa
							else
								if (k == route[j].size())
								{
									// definir o inicio 
									if (beginTime[route[j][k - 1]] + task[route[j][k - 1]].pt + c[route[j][k - 1]][s.sol[i]] <= task[s.sol[i]].a)       // chegar antes da janela de inicio da tarefa (esperar)
										inicio = task[s.sol[i]].a;
									else                                                                                                            // chegar depois da janela de inicio da tarefa estar aberta
										inicio = beginTime[route[j][k - 1]] + task[route[j][k - 1]].pt + c[route[j][k - 1]][s.sol[i]];

									//verificar a janela de tempo
									if ((inicio + task[s.sol[i]].pt <= task[s.sol[i]].b) && (inicio + task[s.sol[i]].pt + d[j][s.sol[i]] <= team[j].b))
									{
										// verificar se eh a melhor posicao para inserir a tarefa nesta rota (maior tempo ocioso z)
										if (team[j].b - (inicio + task[s.sol[i]].pt + d[j][s.sol[i]]) > zStarRoute)
										{
											zStarRoute = team[j].b - (inicio + task[s.sol[i]].pt + d[j][s.sol[i]]);                                       // tempo ocioso 
											tStarRoute = travelTime[j] + c[route[j][k - 1]][s.sol[i]] + d[j][s.sol[i]] - d[j][route[j][k - 1]];    // tempo de viagem atualizado

											existe = 1;
											posRoute = k;

											inicioStarRoute = inicio;
										}
									}
								}

					// verificar se eh a melhor posicao para inserir a tarefa i na rota j
					if (zStarRoute > zStarTeam && existe == 1)
					{
						zStarTeam = zStarRoute;
						posStarTeam = posRoute;
						tStarTeam = tStarRoute;
						inicioStarTeam = inicioStarRoute;
					}

				} // end route

				// ** ja sei em qual posicao inserir (bestPos), agora tenho que verificar se eh a melhor equipe para realizar

				// verificar se eh a melhor equipe para realizar a tarefa (eh viavel ?)
				if (tStarTeam < bestTime && existe == 1)
				{
					bestTime = tStarTeam;

					bestTeam = j;                               // equipe que ira realizar a tarefa
					bestPos = posStarTeam;                // posicao que a tarefa sera inserida

					bestZ = zStarTeam;
					bestInicio = inicioStarTeam;

					achei = 1;
				}

			} // end if skill
		} // end team

		// insert task sol[i] in the best team and position
		if (achei == 1)
		{
			if (bestPos == route[bestTeam].size())
				route[bestTeam].push_back(s.sol[i]);
			else
				route[bestTeam].insert(route[bestTeam].begin() + bestPos, s.sol[i]);

			// update values of travelTime, z, beginTime
			travelTime[bestTeam] = bestTime;
			z[bestTeam] = bestZ;
			beginTime[s.sol[i]] = bestInicio;
		}

	} // end task


	// *** calculate fitness
	s.fo = 0;

	for (int j = 0; j < m; j++)
	{
		for (int i = 0; i < route[j].size(); i++)
		{
			// priotity prizes
			s.fo += task[route[j][i]].w / MW;

			// idle time
			if (i == route[j].size() - 1)
			{
				s.fo += (team[j].b - (beginTime[route[j][i]] + task[route[j][i]].pt + d[j][route[j][i]])) / MZ;
			}
		}
	}

	// transform in minimization problem
	s.fo = -s.fo;


	// verificar se eh a melhor solucao ate o momento
	if (s.fo < bestSolution.fo)
	{
		bestSolution = s;
		CPUbest = clock();
	}


	result.route = route;
	result.beginTime = beginTime;
	result.travelTime = travelTime;
	result.z = z;
	for (int j = 0; j < m; j++)
	{
		for (int i = 0; i < route[j].size(); i++)
		{
			result.bt[route[j][i]] = 1;
		}
	}
	return -s.fo;
}



