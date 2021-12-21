
0 modulo atualmente funciona para qualquer tipo de grafo e não somente mais para licitações. O conceito de pesos dos vértices também foi adicionado. São necessários 3 arquivos csv e o nome 
dos campos que serão usados em cada arquivo

- arquivo1 : informaçoes sobre todos os nós
- nome do campo que identifica cada nó
- arquivo2: arquivo com 2 nós , o peso entre esses nós e o grafo que esses nós pertencem
- nome dos campos do nó 1 , nó 2, peso e grafo 
- arquivo3: arquivo que mostra qual nó está em qual grafo
- nome dos campos que serao usados no arquivo 3
- pasta de output 

Foi feito a abstração dos nomes dos campos  para generalizar o uso do módulo.
A saida do módulo é um grafo em networkX que representa os arquivos de input e
que foi serializado.