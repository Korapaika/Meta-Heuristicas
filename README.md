# Meta-Heuristicas

O projeto inteiro está na solução ConsoleApplication1.
A pasta Instances fora da pasta do projeto não é utilizada.
As funções estão comentadas dentro do código.
Apenas alguns exemplos da pasta Instances estão no repositório, devido ao limite de arquivos.
As alterações foram feitas apenas para o ILS, o VNS não está sendo usado.

A busca local (SubidaTrocaBit()) remove sequencialmente todas as tarefas de 1 equipe por vez,
realocando as tarefas e armazenando o melhor vizinho.

A solução inicial já organiza as tarefas por prioridade.

A perturbação do ILS limpa as tarefas alocadas de equipes aleatoriamente e também permuta
a ordem das tarefas aleatoriamente.


23/04/2019

Adicionada nova perturbação que não insere as tarefas removidas de volta no vetor de tarefas
não realizadas.

Atualizado os parâmetros betaMin e betaMax.

Adicionado critério de aceitação de solução pior:
  - Após 100 iterações sem melhora

Adicionado novo método de ordenação:
  - Por distância

16/04/2019

Adicionados métodos de ordenação:
  - Por quantidade de equipes
  - Pelos critérios do artigo


