// UCCP.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.
//

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <vector>

using namespace std;

#pragma hdrstop

#define MAX(x,y) ((x)<(y) ? (y) : (x))

#define n 150     		 // Iris           
//#define n 178     		 // Wine           
//#define n 47               // Soybean
//#define n 98         		 // BreastA
//#define n 180         		 // DLBCLA e DLBCLB

#define restricao 500		 // restrições must-link e cannot link

//Variáveis Globais

//int n = 0;

//int LIN = 150; 			// Iris
//int LIN = 178;   			// Wine
//int LIN = 47;    			// Soybean
//int LIN = 98;    			// BreastA
//int LIN = 141;    		// DLBCLA
//int LIN = 180;    			// DLBCLB
int LIN;

//int COL = 4;  			// Iris
//int COL = 13;    			// Wine
//int COL = 35;      		// Soybean
//int COL = 1213; 			// BreastA 
//int COL = 661; 				// DLBCLA e DLBCLB
int COL;

//int k = 3; 					// Iris, Wine
//int k = 4;  				// Soybean
//int k = 5;
int k;


double** dist;
//vector <vector <double>> dist;
int* labels;  				// vector solution
//vector <int> labels;
int inviabilidade;
double melhor_custo;
int cont_rest = 0;
int iii = 0;

/***********************************************************************************
*** Estrutura: constraints                                                      ***
*** Funcao: Estrutura com das restrições                                        ***
************************************************************************************/
typedef struct
{
	int p1;
	int p2;
	int tipo;
} constraints;

constraints* constr;

typedef struct
{
	int nnos;
	int* aloca;
	double custo;
	int nmed;
} cluster;

cluster* C;


struct TSolRK
{
	float obj[n];
	//vector <float> obj;
	int labels[n];
	//vector <int> labels;
	double fo;
	double fo_final;
	int inviabilidade;
};


clock_t CPUinicio,
CPUmelhor,
CPUfim;


//Declaração de Funções
void LerArquivos(char nameTable[]);
//void LerArquivos();
int Decoder(TSolRK s);
int NewDecoder(TSolRK s);
double alloca_cost();
int calc_inviabilidade(int vetor[]);
//int calc_inviabilidade(vector<int> vetor);
void BRKGA();

double randomico(double min, double max);
int irandomico(int min, int max);

#pragma argsused
/************************************************************************************
*** Método: Main()                                                                ***
*** Função: Programa principal                                                    ***
*************************************************************************************/
int main()
{
	//file with test instances
	FILE* arqProblems;
	arqProblems = fopen("arqProblems.txt", "r");
	if (arqProblems == NULL)
	{
		printf("\nERROR: File arqProblems.txt not found\n");
		getchar();
		exit(1);
	}
	int opcao = 0, opcao2;
	char nameTable[100];
	//melhor_custo = 99999999999999;
	//labels.clear();
	//labels.resize(LIN, 0);
	//labels = (int*)calloc(LIN, sizeof(int));
	while (!feof(arqProblems))
	{
		// read the name of instances
		//char nameTable[100];
		fscanf(arqProblems, "%s", nameTable);

		if (strcmp(nameTable, "DLBCLB") == 0) {
			//n = 180;
			LIN = 180;
			COL = 661;
			k = 3;
		}
		else if (strcmp(nameTable, "iris") == 0) {
			//n = 150;
			LIN = 150;
			COL = 4;
			k = 3;
		}

		/*cout << n << endl;
		system("pause");*/

		//read the informations of the instance
		//LerArquivos(nameTable);
	}

	



	//int opcao = 0, opcao2;
	//LerArquivos();
	LerArquivos(nameTable);
	//int iii=0; 
	melhor_custo = 99999999999999;
	//labels.clear();
	//labels.resize(LIN, 0);
	labels = (int*)calloc(LIN, sizeof(int));
	// srand((unsigned) time(&t)); // fixa semente %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	srand((unsigned)time(NULL));

	printf("\n-------------------------------------------------------------\n");
	printf("\n BRKGA");

	while (iii < 50) {
		CPUinicio = CPUfim = CPUmelhor = clock();
		printf("\n############## Iteracao %d ######################\n", iii);
		BRKGA();
		iii++;
	}



	//fclose(arqProblems);
	return 0;
}

/************************************************************************************
*** Método: LerArquivos()                                                         ***
*** Função: Ler os arquivos de beneficios e pesos dos objetos                     ***
*************************************************************************************/
void LerArquivos(char nameTable[])
//void LerArquivos()
{
	if (nameTable == NULL) {
		exit(1);
	}
	
	double valor = 0.0;
	FILE* arquivo;

	//char nomearqB[50] = "dados\\iris.txt";
   // char open_const[50] = "0.25.25.txt";
    //char open_const[50] = "Restricoes\\iris_11.constraints";

  //char nomearqB[50] = "wine.txt";
  //char open_const[50] = "wine_18.constraints";
  //  char nomearqB[50] = "soybean.txt";
  // char open_const[50] = "soybean_1.constraints";
  //char nomearqB[50] = "BreastA.txt";
   //char open_const[50] = "BreasA_1.constraints";
  //  char nomearqB[50] = "DLBCLA.txt";
   // char open_const[50] = "DLACLA_1.constraints";
	//char nomearqB[50] = "dados\\DLBCLB.txt";
	//char open_const[50] = "Restricoes\\DLBCLB\\DLBCLB_2.constraints";
	char nomearqB[50] = "dados\\";
	strcat(nomearqB, nameTable);
	strcat(nomearqB, ".txt");

	/*cout << nomearqB << endl;
	system("pause");*/

	char open_const[50] = "Restricoes\\";
	strcat(open_const, nameTable);
	strcat(open_const, "\\");
	strcat(open_const, nameTable);
	strcat(open_const, "_500.constraints");
	//strcat(open_const, "_50.constraints");

	/*cout << open_const << endl;
	system("pause");*/

	// Alocar memória para o conjunto de instâncias
	double** xy;
	//vector <vector <double>> xy;
	xy = (double**)calloc(LIN, sizeof(double*));
	/*xy.clear();
	xy.resize(LIN);
	for (int i = 0; i < COL; i++) {
		xy[i].resize(COL, 0);
	}*/
	
	if (xy == NULL)
	{
		printf("\n>>> Memoria insuficiente para *xy[%d]\n", n);
		exit(1);
	}
	for (int i = 0; i < LIN; i++)
	{
		xy[i] = (double*)calloc(COL, sizeof(double));
		if (xy[i] == NULL)
		{
			printf("\n>>> Memoria insuficiente para xy[%d][%d]\n", i, LIN);
			exit(1);
		}
	}

	// alocar memória para as distância

	dist = (double**)calloc(LIN, sizeof(double*));
	/*dist.clear();
	dist.resize(LIN);
	for (int i = 0; i < LIN; i++) {
		dist[i].resize(LIN, -1);
	}*/
	if (dist == NULL) {
		printf("\n>>> Memoria insuficiente para *dist[%d]\n", LIN);
		exit(1);
	}
	for (int i = 0; i < LIN; i++) {
		dist[i] = (double*)calloc(LIN, sizeof(double));
		if (dist[i] == NULL) {
			printf("\n>>> Memoria insuficiente para dist[%d][%d]\n", i, LIN);
			exit(1);
		}
	}

	// Preparar matriz de distancias

	for (int i = 0; i < LIN; i++)
		for (int j = 0; j < LIN; j++)
			dist[i][j] = -1;

	//Ler os dados dos objetos 
	arquivo = fopen(nomearqB, "r");
	if (!arquivo) {
		printf("O Arquivo %s nao pode ser aberto.\n", nomearqB);
		getchar();
		exit(1);
	}

	for (int a = 0; a < LIN; a++) {
		for (int b = 0; b < COL; b++) {
			fscanf(arquivo, "%lf", &valor);
			xy[a][b] = valor;
		}
	}

	/*
	for(int a=0; a < LIN; a++){
		 for(int b=0; b < COL; b++) {
		printf("%f ",xy[a][b]);
	}
	printf("\n ");
	}
	*/

	// Cálculo das distâncias

	for (int i = 0; i < LIN; i++) {
		for (int j = 0; j < LIN; j++) {
			double value = 0.0;
			for (int u = 0; u < COL; u++) {
				value += (xy[i][u] - xy[j][u]) * (xy[i][u] - xy[j][u]);
			}
			// value = sqrt(value);
			dist[i][j] = value;
			// dist[i][j] = floor(value*1000.0)/1000.0; //
		  //  dist[i][j] = (ceil(value*100.0))/100.0;  //com arredondamento para cima - 2 casas decimais
			dist[j][i] = dist[i][j];
			//  printf("%lf ", dist[i][j]);
		}
		dist[i][i] = 0;
	}
	/*
	for(int i = 0; i < LIN; i++){
	   for(int j = 0; j < LIN; j++){
		   printf("%f ", dist[i][j] );
	   }
	   printf("\n");
	}
	 */
	fclose(arquivo);

	//Ler as restrições
	arquivo = fopen(open_const, "r");
	if (!arquivo) {
		printf("O Arquivo %s nao pode ser aberto.\n", open_const);
		getchar();
		exit(1);
	}

	// Quantidade de restrições 
	constr = (constraints*)calloc(500, sizeof(constraints));
	if (constr == NULL) {
		printf("\n>>> Memoria insuficiente para constraints[%d]\n");
		exit(1);
	}

	int p1, p2, p3;
	int i = 0;

	while (!feof(arquivo))	// Enquanto não se chegar no final do arquivo /   
	{
		fscanf(arquivo, "%d %d %d", &p1, &p2, &p3);
		constr[i].p1 = p1;
		constr[i].p2 = p2;
		constr[i].tipo = p3;
		//	 printf("%d %d %d\n ", constr[i].p1,  constr[i].p2,  constr[i].tipo);
		i++;
	}

	fclose(arquivo);
	//for(int j = 0; j < 500; j++)
	//printf("%d %d %d\n ", constr[j].p1,  constr[j].p2,  constr[j].tipo);

	C = (cluster*)calloc(k, sizeof(cluster));
	if (C == NULL) {
		printf("\n>>> Memoria insuficiente para C[%d]\n");
		exit(1);
	}

	for (int i = 0; i < k; i++) {
		C[i].aloca = (int*)calloc(n, sizeof(int)); //n-p
		if (C[i].aloca == NULL) {
			printf("\n>>> Memoria insuficiente para C[%d][%d]\n", i, k);
			exit(1);
		}
	}

	for (int i = 0; i < n; i++)
		free(xy[i]);
	free(xy);
	//xy.clear();
}

/*****************************************************************************
Método: GRAVAR SAIDA
Funcao: gravar as soluções em um arquivo .TXT
******************************************************************************/
void GravarSaida(TSolRK s)
{
	FILE* arquivo;

	arquivo = fopen("Resultados.txt", "a");

	if (!arquivo)
	{
		printf("\n\nErro ao abrir o arquivo .txt!!!");
		getchar();
		exit(1);
	}
	/*
		fprintf(arquivo,"\nSol: ");
		for (int i=0; i<n; i++)
			fprintf(arquivo,"%d ", s.sol[i]);

		fprintf(arquivo,"\n");
		*/
	fprintf(arquivo, "\n######################## %d ###################", restricao);

	fprintf(arquivo, "\n\nVetor de solucoes: \n");
	for (int i = 0; i < LIN; i++)
		fprintf(arquivo, "%d ", s.labels[i]);

	fprintf(arquivo, "\n\n Inviabilidades: ");
	fprintf(arquivo, "%d ", s.inviabilidade);



	fprintf(arquivo, "\nFO: %f", s.fo_final);

	//	fprintf(arquivo,"\nTempo melhor: %.3f",(float)(CPUmelhor - CPUinicio)/CLOCKS_PER_SEC);
	fprintf(arquivo, "\nTempo total:%.3f", (float)(CPUfim - CPUinicio) / CLOCKS_PER_SEC);
	fprintf(arquivo, "\n######################## FIM ###################\n");

	fclose(arquivo);
}

/*****************************************************************************
Método: GRAVAR SAIDA
Funcao: gravar as soluções em um arquivo .TXT de todas as iterações
******************************************************************************/
void GravarSaidaTotal(TSolRK s)
{
	FILE* arquivo;

	arquivo = fopen("Resultados_total.txt", "a");

	if (!arquivo)
	{
		printf("\n\nErro ao abrir o arquivo .txt!!!");
		getchar();
		exit(1);
	}
	/*
		fprintf(arquivo,"\nSol: ");
		for (int i=0; i<n; i++)
			fprintf(arquivo,"%d ", s.sol[i]);

		fprintf(arquivo,"\n");
		*/
	fprintf(arquivo, "\n ######################## %d ###################", restricao);

	fprintf(arquivo, "\n\nVetor de solucoes: \n");
	for (int i = 0; i < LIN; i++)
		fprintf(arquivo, "%d ", s.labels[i]);

	fprintf(arquivo, "\n\n Inviabilidades: ");
	fprintf(arquivo, "%d ", s.inviabilidade);



	fprintf(arquivo, "\nFO: %f", s.fo_final);

	//	fprintf(arquivo,"\nTempo melhor: %.3f",(float)(CPUmelhor - CPUinicio)/CLOCKS_PER_SEC);
	fprintf(arquivo, "\nTempo total:%.3f", (float)(CPUfim - CPUinicio) / CLOCKS_PER_SEC);
	fprintf(arquivo, "\n######################## FIM ###################\n");

	fclose(arquivo);
}

void GravarSaidalista(TSolRK s)
{
	FILE* arquivo;
	if (s.inviabilidade == 0)
		cont_rest = cont_rest + 1;

	arquivo = fopen("Resultados_lista.txt", "a");

	if (!arquivo)
	{
		printf("\n\nErro ao abrir o arquivo .txt!!!");
		getchar();
		exit(1);
	}
	/*
		fprintf(arquivo,"\nSol: ");
		for (int i=0; i<n; i++)
			fprintf(arquivo,"%d ", s.sol[i]);

		fprintf(arquivo,"\n");
		*/
		//  fprintf(arquivo,"\n######################## %d ###################",restricao);


	fprintf(arquivo, "\n\n Inviabilidades: ");
	fprintf(arquivo, "%d ", s.inviabilidade);
	fprintf(arquivo, " FO: %f", s.fo_final);

	//	fprintf(arquivo,"\nTempo melhor: %.3f",(float)(CPUmelhor - CPUinicio)/CLOCKS_PER_SEC);
	fprintf(arquivo, " Tempo total: %.3f", (float)(CPUfim - CPUinicio) / CLOCKS_PER_SEC);
	//	fprintf(arquivo,"\n######################## FIM ###################\n");
	fprintf(arquivo, " cont_rest: %d", cont_rest);
	fclose(arquivo);
}



/************************************************************************************
*** Método: ImprimirMelhorSol2()                                                   ***
*** Função: Imprimir a melhor solucao encontrada                                  ***
*************************************************************************************/
void ImprimirMelhorSolRK(TSolRK s)
{
	printf("\n\n\nMELHOR SOLUCAO ENCONTRADA");
	printf("\n*************************");
	printf("\n\nFO: %f", s.fo_final);
	/*
	printf("\n\nObjetos selecionados: \n");
	printf("--------------------\n");
	for (int i=0; i<n; i++)
	  if (s.obj[i] > 0.5)
		printf(" %d ",i);

	printf("\n\nObjetos nao selecionados: \n");
	printf("------------------------\n");
	for (int i=0; i<n; i++)
	  if (s.obj[i] <= 0.5)
		printf(" %d ",i);
   */
	printf("\n\nVetor de solucoes: \n");
	for (int i = 0; i < LIN; i++)
		printf("%d ", s.labels[i]);

	printf("\n\nInviabilidades: ");
	printf("%d ", s.inviabilidade);
	//printf("\n\n\nPressione alguma tecla para retornar ao menu...");
	//fflush(stdin);
  //  getchar();
}


/************************************************************************************
*** Método: GerarSolucaoInicial2()                                                ***
*** Função: Gerar Solucao Inicial Aleatorimante float                             ***
*************************************************************************************/
TSolRK GerarSolucaoInicialRK()
{
	TSolRK s;
	s.fo_final = 0.0;

	//s.obj.clear();
	//s.labels.clear();
	//s.obj.resize(n);
	//s.labels.resize(n);

	for (int j = 0; j < n; j++)
		s.obj[j] = randomico(0, 1);
	//  for (int j=0; j<n; j++)
   //  printf("%f ", s.obj[j]); 
	s.fo = NewDecoder(s);
   //s.fo = Decoder(s);
	s.inviabilidade = inviabilidade;
	for (int i = 0; i < LIN; i++)
		s.labels[i] = labels[i];

	for (int i = 0; i < k; i++)
		s.fo_final += C[i].custo;
	return s;
}



/************************************************************************************
*** Método: limpar_solucao2()                                                      ***
*** Função: Inicializa todos os indivíduos de uma população como "nulos"          ***
*************************************************************************************/
void limpar_populacaoRK(TSolRK Pop[], int tamPop)
//void limpar_populacaoRK(vector<TSolRK> Pop, int tamPop)
{
	for (int i = 0; i < tamPop; i++)
	{
		for (int j = 0; j < n; j++)
			Pop[i].obj[j] = 0;
		/*Pop[i].obj.clear();
		Pop[i].obj.resize(n, 0);
		Pop[i].labels.clear();
		Pop[i].labels.resize(n);*/

		Pop[i].fo = 0;
	}
	return;
}

/************************************************************************************
*** Método: alloca_cost()                                                             ***
*** Função: Aloca os ponto da solucao nos clusters***
*************************************************************************************/

double alloca_cost() {

	for (int q = 0; q < k; q++) {
		C[q].nnos = 0;
		C[q].custo = 0.0;
	}

	for (int i = 0; i < n; i++) {
		C[labels[i]].aloca[C[labels[i]].nnos] = i;
		C[labels[i]].nnos++;
	}
	/*
	for(int i = 0; i < k; i++)
		printf("%d ", C[i].nnos);
		printf("\n");
		printf("\n");

	for(int i = 0; i < k; i++){
		for(int j =0; j < C[i].nnos; j++){
			printf("%d ", C[i].aloca[j]);
		}
		printf("\n");
	}
	*/
	// calcular os custos

	double custo, custoTotal;

	for (int i = 0; i < k; i++) {
		custoTotal = 0.0;
		for (int h = 0; h < C[i].nnos; h++) {
			custo = 0;
			for (int j = h + 1; j < C[i].nnos; j++) {
				custo += dist[C[i].aloca[h]][C[i].aloca[j]];
			}
			custoTotal += custo;
		}
		if (C[i].nnos == 0)
			C[i].custo = custoTotal;
		else
			C[i].custo = custoTotal / C[i].nnos;
	}

	custo = 0.0;
	for (int p = 0; p < k; p++)
		custo += C[p].custo;

	return custo;
}
/************************************************************************************
*** Método: calc_inviabilidade()                                                             ***
*** Função: Calcula as inviabilidades  ***
*************************************************************************************/

int calc_inviabilidade(int vetor[]) {
//int calc_inviabilidade(vector<int> vetor) {
	int penalidade = 0;

	for (int i = 0; i < restricao; i++) {
		if (constr[i].tipo == 1)
			if (vetor[constr[i].p1] != vetor[constr[i].p2])
				penalidade++;
		if (constr[i].tipo == -1)
			if (vetor[constr[i].p1] == vetor[constr[i].p2])
				penalidade++;
	}
	return penalidade;
}


















void SetLabel() 
{
	for (int i = 0; i < n; i++) {
		labels[i] = -1;
	}
}
















void MustLinkChain(int p) 
{
	for (int i = 0; i < restricao; i++) {
		if (constr[i].tipo == -1) {
			if (constr[i].p1 == p && labels[constr[i].p2] == -1) {
				labels[constr[i].p2] = labels[p];
				MustLinkChain(constr[i].p2);
			}
			else if (constr[i].p2 == p && labels[constr[i].p1] == -1) {
				labels[constr[i].p1] = labels[p];
				MustLinkChain(constr[i].p1);
			}
		}
	}
}



void MustLinkCluster(int* medianas)
{
	for (int i = 0; i < restricao; i++) {
		if (constr[i].p1 == medianas[0] || constr[i].p1 == medianas[1] || constr[i].p1 == medianas[2]) {
			if (labels[constr[i].p2] == -1 && constr[i].tipo == -1) {
				labels[constr[i].p2] = labels[constr[i].p1];
				MustLinkChain(constr[i].p2);
			}
			
		}
		else if (constr[i].p2 == medianas[0] || constr[i].p2 == medianas[1] || constr[i].p2 == medianas[2]) {
			if (labels[constr[i].p1] == -1 && constr[i].tipo == -1) {
				labels[constr[i].p1] = labels[constr[i].p2];
				MustLinkChain(constr[i].p1);
			}
			
		}
	}
}



bool CannotLinkCheck(int pos, int label)
{
	for (int i = 0; i < restricao; i++) {
		if (constr[i].tipo == 1) {
			if ((constr[i].p1 == pos || constr[i].p2 == pos) && (labels[constr[i].p1] == label || labels[constr[i].p2] == label)) {
				return false;
			}
		}
	}
	return true;
}

void CannotLinkCluster()
{
	bool loop = false;
	for (int i = 0; i < restricao; i++) {
		if (constr[i].tipo == 1) {
			if (labels[constr[i].p1] == labels[constr[i].p2]) {
				cout << "ERRO: " << constr[i].p1 << ", " << constr[i].p2 << endl;
				system("pause");
			}
		}
	}
}

void MLCheck()
{
	bool loop = false;
	for (int i = 0; i < restricao; i++) {
		if (constr[i].tipo == -1) {
			if (labels[constr[i].p1] == -1 && labels[constr[i].p2] != -1) {
				labels[constr[i].p1] = labels[constr[i].p2];
				loop = true;
			}
			else if (labels[constr[i].p2] == -1 && labels[constr[i].p1] != -1) {
				labels[constr[i].p2] = labels[constr[i].p1];
				loop = true;
			}
		}
	}
	if (loop) {
		MLCheck();
	}
}



bool MustLinkCheck(int pos) {
	for (int i = 0; i < restricao; i++) {
		if (constr[i].tipo == -1) {
			if (constr[i].p1 == pos && labels[constr[i].p2] != -1) {
				labels[constr[i].p1] = labels[constr[i].p2];
				return true;
			}
			if (constr[i].p2 == pos && labels[constr[i].p1] != -1) {
				labels[constr[i].p2] = labels[constr[i].p1];
				return true;
			}
		}
	}
	return false;
}



void ClusterML(int label)
{
	for (int i = 0; i < restricao; i++) {
		if (constr[i].tipo == -1) {
			if (labels[constr[i].p1] == -1 && labels[constr[i].p2] == -1) {
				labels[constr[i].p1] = label;
				labels[constr[i].p2] = label;
				MustLinkChain(constr[i].p1);
				MustLinkChain(constr[i].p2);
				return;
			}
		}
	}
}


/************************************************************************************
*** Método: Decoder()                                                             ***
*** Função: Trasnforma uma solucao de chaves aleatorias em uma solucao do problema***
*************************************************************************************/
int NewDecoder(TSolRK s)
{
	int pc = 1;
	if (pc == 0) {
		return Decoder(s);
	}
	
	SetLabel(); //Atribuir -1 para todas as labels

	//modelos de decoders:
	//1) menor distância com penalidade (OK)
	//2) lista de candidatos com função gulosa (distância + restrições)
		//2.1) Agrupar todos Must-Link das medianas primeiro
	//3) ordenar os pontos
	//fase construtiva grasp (OK)
	//ordenar vetor de chaves (OK)
	//alocar somente as p-medianas primeiro (OK)

	int* medianas;
	int pos;
	float val;
	medianas = (int*)calloc(k, sizeof(int));

	// --------------------------------------------------------------------------------------------------------
	/*for (int i = 0; i < k; i++) {
		ClusterML(i);
		medianas[i] = -1;
	}

	for (int i = 0; i < k; i++) {
		for (int j = 0; j < n; j++) {
			if (medianas[i] == -1 && labels[j] == i) {
				medianas[i] = j;
			}
		}
	}*/ //NAO DEU CERTO
	// --------------------------------------------------------------------------------------------------------


	// SELECIONAR AS MEDIANAS ---------------------------------------------------------------------------------
	for (int i = 0; i < k; i++) {
		pos = -1;
		val = 9999999;
		for (int j = 0; j < n; j++) {
			if (s.obj[j] < val && labels[j] == -1) {
				val = s.obj[j];
				pos = j;
			}
		}
		if (pos != -1) {
			medianas[i] = pos;
			labels[pos] = i;
		}
	}
	// --------------------------------------------------------------------------------------------------------
	


	//cout << "Medianas: " << medianas[0] << ", " << medianas[1] << ", " << medianas[2] << endl;
	//cout << "Labels: " << labels[medianas[0]] << ", " << labels[medianas[1]] << ", " << labels[medianas[2]] << endl;
	//system("pause");
	//MLCheck();
	//CannotLinkCheck();
	//MustLinkCluster(medianas); //Agrupar todos os Must-Link com as p-medianas
	//MLCheck();
	//cout << "passou 1" << endl;
	//CannotLinkCluster();


	double fo;
	inviabilidade = 0;

	
	//CannotLinkCheck();
	//MLCheck();

	// CLASSIFICA POR ORDEM DE CRITÉRIO: ML, CL, DISTÂNCIA -----------------------------------------------------
	/*for (int i = 0; i < n; i++) {
		if (labels[i] == -1) {
			if (MustLinkCheck(i)) {

			}
			else {
				val = 9999999999;
				pos = -1;
				for (int j = 0; j < k; j++) {
					if (dist[i][medianas[j]] < val && CannotLinkCheck(i, j)) {
						//if (dist[i][medianas[j]] < val) {
							//if (dist[i][j] < val && labels[j] != -1 && i != j) {
						val = dist[i][medianas[j]];
						pos = medianas[j];
					}
				}
				if (pos == -1) {
					for (int j = 0; j < n; j++) {
						if (dist[i][j] < val && labels[j] != -1) {
							val = dist[i][j];
							pos = j;
						}
					}
				}
				labels[i] = labels[pos];
			}
		}
	}*/
	// --------------------------------------------------------------------------------------------------------


	/*
	// CLASSIFICA POR MENOR DISTÂNCIA DE QUALQUER PONTO -------------------------------------------------------
	for (int i = 0; i < n; i++) {
		if (labels[i] == -1) {
			val = 9999999999;
			pos = -1;
			for (int j = 0; j < n; j++) {
				if (dist[i][j] < val && labels[j] != -1) {
					val = dist[i][j];
					pos = j;
				}
			}
			labels[i] = labels[pos];
		}
	}
	// --------------------------------------------------------------------------------------------------------
	*/
	




	// CLASSIFICA POR MENOR DISTÂNCIA DA MEDIANA --------------------------------------------------------------
	for (int i = 0; i < n; i++) {
		if (labels[i] == -1) {
			val = 9999999999;
			pos = -1;
			for (int j = 0; j < k; j++) {
				//if (dist[i][medianas[j]] < val && CannotLinkCheck(i, labels[medianas[j]]) == 0) {
				if (dist[i][medianas[j]] < val) {
				//if (dist[i][j] < val && labels[j] != -1 && i != j) {
					val = dist[i][medianas[j]];
					pos = medianas[j];
				}
			}
			labels[i] = labels[pos];
		}
	}
	// --------------------------------------------------------------------------------------------------------
	


	//MLCheck();

	for (int i = 0; i < LIN; i++)
		s.labels[i] = labels[i];

	//fo = alloca_cost();
	//s.fo_final = fo;
	//inviabilidade = calc_inviabilidade(labels);
	//s.inviabilidade = inviabilidade; 

	// s = buscaloc(s);  // aplica busca local troca cluster
	 //s = buscaloc1(s); 	 // aplica busca local swap vetor solução	   
	// s = buscaloc2(s);

	fo = alloca_cost();
	//s.fo_final = fo;
	inviabilidade = calc_inviabilidade(labels);
	fo = fo + (10000 * LIN * inviabilidade);

	return fo;
}








/************************************************************************************
*** Método: Decoder()                                                             ***
*** Função: Trasnforma uma solucao de chaves aleatorias em uma solucao do problema***
*************************************************************************************/
int Decoder(TSolRK s)
{
	double fo;
	inviabilidade = 0;

	for (int i = 0; i < n; i++) {

		//  if (s.obj[i] < 0.25) {
		if (s.obj[i] <= 0.333) {
			labels[i] = 0;
			//counts[labels[i]]++;      
		}
		//  if (s.obj[i] > 0.2 && s.obj[i] < 0.4){
		else if (s.obj[i] > 0.333 && s.obj[i] <= 0.666) {
			// if (s.obj[i] > 0.25 && s.obj[i] < 0.5){

			labels[i] = 1;
			//	counts[labels[i]]++;
		}

		// if (s.obj[i] > 0.4 && s.obj[i] < 0.6){
		//if (s.obj[i] > 0.666 && s.obj[i] <= 1) {
		else {
			// if (s.obj[i] > 0.5 && s.obj[i] < 0.75){
			labels[i] = 2;
			//	counts[labels[i]]++;
		}
		/*
		  if (s.obj[i] > 0.75 && s.obj[i] <= 1){
			  labels[i] = 3;
		   }

		  if (s.obj[i] > 0.8 && s.obj[i] <= 1){
			  labels[i] = 4;
		   }
	   */
	}

	for (int i = 0; i < LIN; i++)
		s.labels[i] = labels[i];

	//fo = alloca_cost();
	//s.fo_final = fo;
	//inviabilidade = calc_inviabilidade(labels);
	//s.inviabilidade = inviabilidade; 

	// s = buscaloc(s);  // aplica busca local troca cluster
	 //s = buscaloc1(s); 	 // aplica busca local swap vetor solução	   
	// s = buscaloc2(s);

	fo = alloca_cost();
	//s.fo_final = fo;
	inviabilidade = calc_inviabilidade(labels);
	fo = fo + (10000 * LIN * inviabilidade);

	return fo;
}

/************************************************************************************
*** Método: BRKGA()                                                               ***
*** Função: Aplica a metaheuristica BRKGA no Problema da Mochila                  ***
*************************************************************************************/
void BRKGA()
{
	//int n = 50;		    // size of chromosomes
	int p = 1000;	        // size of population
	double pe = 0.2;	    // fraction of population to be the elite-set
	double pm = 0.2;      // fraction of population to be replaced by mutants
	double rhoe = 0.6;	// probability that offspring inherit an allele from elite parent  
	double fo_final = 0.0;
	int cont = 0;
	int Tpe;
	int Tpm;


	//calcular o tamanho das populacoes elite e mutantes
	Tpe = (int)(p * pe);
	Tpm = (int)(p * pm);

	// TSolRK Pop[p];  //populacao do BRKGA
	 //TSolRK PopInter[p];

	TSolRK* Pop;  //populacao do BRKGA
	//vector <TSolRK> Pop;
	Pop = (TSolRK*)calloc(p, sizeof(TSolRK));
	/*Pop.clear();
	Pop.resize(p);

	for (int i = 0; i < p; i++) {
		Pop[i].obj.clear();
		Pop[i].obj.resize(n);
		Pop[i].labels.clear();
		Pop[i].labels.resize(n);
	}*/

	limpar_populacaoRK(Pop, p);
	//TSolRK *PopInter;
	//PopInter = (TSolRK *)calloc(p,sizeof(TSolRK)); 
   // limpar_populacaoRK(PopInter, p);
	TSolRK melhorInd;
	melhorInd.fo = 99999999999;

	//Gerar a população inicial
	for (int i = 0; i < p; i++)
	{
		TSolRK ind = GerarSolucaoInicialRK();
		Pop[p - i - 1] = ind;
	}

	//ordenar a populacao inicial (quick sort)   
	//QuickSort(0,p-1);   

	//ordenar a populacao inicial (bubble sort)
	for (int i = p - 1; i > 0; i--)
	{
		for (int j = 0; j < i; j++)
		{
			if (Pop[j + 1].fo < Pop[j].fo) // alterei pra minimização
			{
				TSolRK aux = Pop[j];
				Pop[j] = Pop[j + 1];
				Pop[j + 1] = aux;
			}
		}
	}

	/*
   for (int i=0; i<p; i++){
	   printf("i: %d \n", i);
	   for(int j = 0; j < n; j++){
	   printf("%d ", Pop[i].labels[j]);
	   }
	   printf("\n\n");
   }
	*/
	int numGeracoes = 0;
	while (numGeracoes < 1000)
	{
		TSolRK* PopInter;
		//vector <TSolRK> PopInter;
		PopInter = (TSolRK*)calloc(p, sizeof(TSolRK));
		/*PopInter.clear();
		PopInter.resize(p);

		for (int i = 0; i < p; i++) {
			PopInter[i].obj.clear();
			PopInter[i].obj.resize(n);
			PopInter[i].labels.clear();
			PopInter[i].labels.resize(n);
		}*/

		limpar_populacaoRK(PopInter, p);

		numGeracoes++;

		// We now will set every chromosome of 'current', iterating with 'i':
		int i = 0;	// Iterate chromosome by chromosome
		int j = 0;	// Iterate allele by allele

		// The 'pe' best chromosomes are maintained, so we just copy these into 'current':
		while (i < Tpe) {
			PopInter[i] = Pop[i];
			++i;
		}

		/*
		for (int i=0; i<p; i++){
			printf("i: %d \n", i);
			for(int j = 0; j < n; j++){
			printf("%d ", PopInter[i].labels[j]);
			}
			printf("\n\n");
		}
		*/
		//for (int i=0; i<Tpe; i++)
		//	printf("%f ", PopInter[i].fo);

			// We'll mate 'p - pe - pm' pairs; initially, i = pe, so we need to iterate until i < p - pm:
		while (i < p - Tpm) {
			// Select an elite parent:
			int eliteParent = irandomico(0, Tpe - 1);

			// Select a non-elite parent:
			int noneliteParent = Tpe + irandomico(0, p - Tpe - 1);

			// Mate:
			for (j = 0; j < n; ++j) {
				int sourceParent;
				if (randomico(0, 1) < rhoe)
					sourceParent = eliteParent;
				else
					sourceParent = noneliteParent;

				PopInter[i].obj[j] = Pop[sourceParent].obj[j];
			}

			++i;
		}

		// We'll introduce 'pm' mutants:
		while (i < p) {
			for (j = 0; j < n; ++j)
			{
				PopInter[i].obj[j] = randomico(0, 1);
			}
			++i;
		}

		//Calculate fitness
		for (i = 0; i < p; i++) {

			//PopInter[i].fo = Decoder(PopInter[i]);
			PopInter[i].fo = NewDecoder(PopInter[i]);

			PopInter[i].fo_final = 0.0;
			for (int ii = 0; ii < k; ii++)
				PopInter[i].fo_final += C[ii].custo;

			PopInter[i].inviabilidade = inviabilidade;
			for (int ii = 0; ii < LIN; ii++)
				PopInter[i].labels[ii] = labels[ii];

			//encontrar o melhor filho do BRKGA
			if (PopInter[i].fo < melhorInd.fo) // alterei
			{
				melhorInd = PopInter[i];
				/*
				 for(int jj=0; jj<n; jj++){
					 printf("%d ", PopInter[i].labels[jj]);
					}
					printf("\n%f ", PopInter[i].fo);
					printf("\n%d \n\n", PopInter[i].inviabilidade);

					for (int ii=0; ii<n; ii++)
					printf("%d ", melhorInd.labels[ii]);
					printf("\n%f ", melhorInd.fo);
					printf("\n%d \n", melhorInd.inviabilidade);
					*/
			}
		}

		//for (int i=0; i<p; i++)
	   //	printf("%f ", melhorInd.fo);	 

		printf("Geracao: %d  melhor individuo = %f inviabilidade = %d \n", numGeracoes, melhorInd.fo_final, melhorInd.inviabilidade);

		//Limpar Pop
		limpar_populacaoRK(Pop, p);

		//substituir a população pelos filhos gerados
		for (int i = 0; i < p; i++)
			Pop[i] = PopInter[i];

		//ordenar a populacao 
		//QuickSort(0,p-1);  
		for (int i = p - 1; i > 0; i--)
		{
			for (int j = 0; j < i; j++)
			{
				if (Pop[j + 1].fo < Pop[j].fo)  // alterei
				{
					TSolRK aux = Pop[j];
					Pop[j] = Pop[j + 1];
					Pop[j + 1] = aux;
				}
			}
		}

		//Limpar a PopInter   
	  //  limpar_populacaoRK(PopInter, p);  
		free(PopInter);
		//PopInter.clear();

		if (cont == 0)
			fo_final = melhorInd.fo_final;

		if (fo_final == melhorInd.fo_final) {
			cont = cont + 1;
		}
		else
			fo_final = melhorInd.fo_final;

		//	if(cont ==100)
		//	numGeracoes = 1000;
		/*	if (melhorInd.fo_final < 93.85 ){
				if(melhorInd.inviabilidade == 0){
				CPUfim = clock();
				GravarSaidalista(melhorInd);
				iii++;
				break;
				}
			}
			*/

	}

	free(Pop);
	//Pop.clear();
	//free(PopInter);

	CPUfim = clock();
	GravarSaidaTotal(melhorInd);
	GravarSaidalista(melhorInd);

	if (melhorInd.fo < melhor_custo) {
		melhor_custo = melhorInd.fo;

		ImprimirMelhorSolRK(melhorInd);
		GravarSaida(melhorInd);
	}
	//free(labels);
}


/************************************************************************************
*** Método: randomico(double min, double max)                                     ***
*** Função: Gera um numero aleatorio entre min e max                              ***
*************************************************************************************/
double randomico(double min, double max)
{
	return ((double)(rand() % 10000) / 10000.0) * (max - min) + min;
}

/************************************************************************************
*** Método: irandomico(int min, int max)                                          ***
*** Função: Gera um numero inteiro aleatorio entre min e max                      ***
*************************************************************************************/
int irandomico(int min, int max)
{
	return (int)randomico(0, max - min + 1.0) + min;
}
