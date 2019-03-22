	
#include <iostream>
using namespace std;
 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <vector>
#include <cstring>
#include <string.h>
//#include <omp.h>
 
#include "ABRKGA.h"
 
 
//------DEFINITION OF CONSTANTS, TYPES AND VARIABLES--------
 
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
 
 
/************************************************************************************
                                FUNCTIONS AREA
*************************************************************************************/
int main()
{
    //file with test instances
    FILE *arqProblems;
     
    if (pc == 0)
        arqProblems = fopen ("C:/Users/Chaves/Dropbox/ACADEMICO/PESQUISA/2017/FTSP/Instances/arqProblems.txt", "r");
    else
        arqProblems = fopen ("/Users/antoniochaves/Dropbox/ACADEMICO/PESQUISA/2017/FTSP/Instances/arqProblems.txt", "r");
     
    if (arqProblems == NULL)
    {
        printf("\nERROR: File arqProblems.txt not found\n");
        getchar();
        exit(1);
    }
 
    // best solution that is saved in txt file
    TSol sBest; 
 
    // run the method for all test instances
    while (!feof(arqProblems))
    {
        // read the name of instances
        char nameTable[100];
        fscanf(arqProblems,"%s", nameTable);
 
        //read the informations of the instance
        ReadData(nameTable);
 
        
 
        // free memory
        FreeMemory();
    }
 
    fclose(arqProblems);
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
    for (int j=0; j<m; j++)
        z[j] = team[j].b - team[j].a;       
     
    // find the best team and position to insert task s.sol[i]
    for (int i=0; i<n; i++)
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
        for (int j=0; j<m; j++)
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
                for (int k=0; k<=route[j].size(); k++)
                {
                    // rota esta vazia
                    if (route[j].empty())
                    {
                        // definir inicio da tarefa
                        if (team[j].a+d[j][s.sol[i]] <= task[s.sol[i]].a)       // chegar antes da janela de inicio da tarefa (esperar)
                            inicio = task[s.sol[i]].a;
                        else                                                                     // chegar depois da janela de inicio da tarefa estar aberta
                            inicio = team[j].a+d[j][s.sol[i]]; 
 
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
                    if (k==0)
                    {
                        // definir o inicio
                        if (team[j].a+d[j][s.sol[i]] <= task[s.sol[i]].a)       // chegar antes da janela de inicio da tarefa (esperar)
                            inicio = task[s.sol[i]].a;
                        else                                                                     // chegar depois da janela de inicio da tarefa estar aberta
                            inicio = team[j].a+d[j][s.sol[i]]; 
                         
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
                    if (k<route[j].size())
                    {
                        // definir o inicio 
                        if (beginTime[route[j][k-1]]+task[route[j][k-1]].pt+c[route[j][k-1]][s.sol[i]] <= task[s.sol[i]].a)       // chegar antes da janela de inicio da tarefa (esperar)
                            inicio = task[s.sol[i]].a;
                        else                                                                                                            // chegar depois da janela de inicio da tarefa estar aberta
                            inicio = beginTime[route[j][k-1]]+task[route[j][k-1]].pt+c[route[j][k-1]][s.sol[i]]; 
                         
                        //verificar a janela de tempo
                        if ((inicio + task[s.sol[i]].pt <= task[s.sol[i]].b) && ((inicio + task[s.sol[i]].pt + c[s.sol[i]][route[j][k]]) <= beginTime[route[j][k]]))
                        {
                            // verificar se eh a melhor posicao para inserir a tarefa nesta rota (maior tempo ocioso z)
                            if (z[j] > zStarRoute)
                            {
                                zStarRoute = z[j];                                                                                                                                          // tempo ocioso nao muda
                                tStarRoute = travelTime[j] + c[route[j][k-1]][s.sol[i]] + c[s.sol[i]][route[j][k]] - c[route[j][k-1]][route[j][k]];    // tempo de viagem atualizado
                                 
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
                        if (beginTime[route[j][k-1]]+task[route[j][k-1]].pt+c[route[j][k-1]][s.sol[i]] <= task[s.sol[i]].a)       // chegar antes da janela de inicio da tarefa (esperar)
                            inicio = task[s.sol[i]].a;
                        else                                                                                                            // chegar depois da janela de inicio da tarefa estar aberta
                            inicio = beginTime[route[j][k-1]]+task[route[j][k-1]].pt+c[route[j][k-1]][s.sol[i]]; 
                         
                        //verificar a janela de tempo
                        if ((inicio + task[s.sol[i]].pt <= task[s.sol[i]].b) && (inicio + task[s.sol[i]].pt + d[j][s.sol[i]] <= team[j].b))
                        {
                            // verificar se eh a melhor posicao para inserir a tarefa nesta rota (maior tempo ocioso z)
                            if (team[j].b - (inicio + task[s.sol[i]].pt + d[j][s.sol[i]]) > zStarRoute)
                            {
                                zStarRoute = team[j].b - (inicio + task[s.sol[i]].pt + d[j][s.sol[i]]);                                       // tempo ocioso 
                                tStarRoute = travelTime[j] + c[route[j][k-1]][s.sol[i]] + d[j][s.sol[i]] - d[j][route[j][k-1]];    // tempo de viagem atualizado
                                 
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
                route[bestTeam].insert(route[bestTeam].begin()+bestPos, s.sol[i]);
             
            // update values of travelTime, z, beginTime
            travelTime[bestTeam] = bestTime;
            z[bestTeam] = bestZ;
            beginTime[s.sol[i]] = bestInicio;
        }
     
    } // end task
     
     
    // *** calculate fitness
    s.fo = 0; 
      
    for (int j=0; j<m; j++)
    {
        for (int i=0; i<route[j].size(); i++)
        {
            // priotity prizes
            s.fo += task[route[j][i]].w/MW;
             
            // idle time
            if (i == route[j].size()-1)
            {
                s.fo += (team[j].b - (beginTime[route[j][i]] + task[route[j][i]].pt + d[j][route[j][i]]))/MZ;
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
        for (int i=0; i<n; i++)
            printf("%d ", s.sol[i]);
         
        printf("\n\n");
        for (int j=0; j<m; j++)
        {
            printf("\nTeam %d: ", j);
            for(int i=0; i<route[j].size(); i++)
            {
                printf("%2d (%.2lf) - ", route[j][i], beginTime[route[j][i]]);
            }
             
            printf("\t [%.2lf] ", z[j]);
        }
         
        printf("\n\nFO: %.2lf \n\n", -s.fo);
    }
 
    return s.fo;
}
 
 
void ReadData(char nameTable[])
{
    char name[200] = "";
     
    if (pc == 0)
        strcpy(name, "C:/Users/Chaves/Dropbox/ACADEMICO/PESQUISA/2017/FTSP/Instances/"); 
    else
        strcpy(name, "/Users/antoniochaves/Dropbox/ACADEMICO/PESQUISA/2017/FTSP/Instances/"); 
     
    strcat(name,nameTable);
 
    FILE *arq;
    arq = fopen(name,"r");
 
    if (arq == NULL)
    {
        printf("\nERROR: File (%s) not found!\n",name);
        getchar();
        exit(1);
    }
 
    // => name of instance
    strcpy(instance,nameTable);
 
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
    for (int i=0; i<n; i++)
        fscanf(arq, "%d", &task[i].w);
     
    // skills between tasks and teams
    q.clear();
    q.resize(n, vector<int>(m));
     
    fscanf(arq, "%d %d", &temp, &temp); // first line is not read
    for (int i=0; i<n; i++)
        for (int j=0; j<m; j++)
            fscanf(arq, "%d", &q[i][j]);
     
    // processing time of tasks 
    fscanf(arq, "%d", &temp);   // first value is not considered
    for (int i=0; i<n; i++)
        fscanf(arq, "%d", &task[i].pt);
     
    // beginning time windows of tasks
    fscanf(arq, "%d", &temp);   // first value is not considered
    for (int i=0; i<n; i++)
        fscanf(arq, "%d", &task[i].a);
         
    // end time windows of tasks
    fscanf(arq, "%d", &temp);   // first value is not considered
    for (int i=0; i<n; i++)
        fscanf(arq, "%d", &task[i].b);
         
    // beginning time windows of teams
    for (int i=0; i<m; i++)
        fscanf(arq, "%d", &team[i].a);
         
    // end time windows of teams
    for (int i=0; i<m; i++)
        fscanf(arq, "%d", &team[i].b);
 
    fscanf(arq, "%d %d %d", &M, &MW, &MZ);
     
     
    // coordinates of teams (teams have the same location)
    double tempX, tempY;
    fscanf(arq, "%lf %lf ", &tempX, &tempY);
    for (int i=0; i<m; i++)
    {
        team[i].x = tempX;
        team[i].y = tempY;
    }
     
    // coordinates of tasks
     for (int i=0; i<n; i++)
        fscanf(arq, "%lf %lf ", &task[i].x, &task[i].y);
     
    fclose(arq);
     
    // calculate the Euclidean distances between tasks
    c.clear();
    c.resize(n, vector<double>(n));
 
    for (int i=0; i<n; i++)
    {
        for (int j=i; j<n; j++)
        {
            c[i][j] = c[j][i] = sqrt( (task[j].x - task[i].x) * (task[j].x - task[i].x) + (task[j].y - task[i].y) * (task[j].y - task[i].y) );
        }
    }
     
    // calculate the Euclidean distances between task and teams garage
    d.clear();
    d.resize(m, vector<double>(n));
 
    for (int i=0; i<m; i++)
    {
        for (int j=0; j<n; j++)
        {
            d[i][j] = sqrt( (task[j].x - team[i].x) * (task[j].x - team[i].x) + (task[j].y - team[i].y) * (task[j].y - team[i].y) );
        }
    }
 
    //print
/*   printf("\n%d \n%d \n", n, m);
     
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
 
    getchar(); */
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
 
    return ((double)(rand()%10000)/10000.0)*(max-min)+min;
}
 
int irandomico(int min, int max)
{
     
    return (int)randomico(0,max-min+1.0) + min;
}

