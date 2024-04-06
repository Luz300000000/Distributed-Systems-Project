---- <Informações sobre o Grupo de Trabalho> ----
@ FCUL
- Inês Luz         57552
- Matilde Marques  58164
- Marta Lourenço   58249
---------------------------------------------------------

---- <Informações sobre a Implementação do Projeto> ----

--> Instruções de Compilação e Execução <--
- Abrir o terminal no diretório do projeto ("/grupo27")

- (Instruções de compilação do makefile):
    - Criar apenas a biblioteca estática libtable    --> "make libtable"
    - Criar apenas o objeto relativo ao sdmessage    --> "make sdmessage"
    - Criar apenas a aplicação respetiva do cliente  --> "make table-client"
    - Criar apenas a aplicação respetiva do servidor --> "make table-server"
    - Criar todos os targets mencionados acima       --> "make all"
    - Remover todos os ficheiros de todos os targets --> "make clean"

- (Instruções de execução):
    - Para cada servidor, abrir um novo terminal no diretório do projeto e executar: "./binary/table_server <zookeeperip:port> <port> <n_lists>"
    - Para cada cliente, abrir um novo terminal no diretório do projeto e executar: "./binary/table_client <zookeeperip:port>"

--> Limitações de Implementação <--
- Não foram encontradas limitações na implementação do sistema de replicação em cadeia usando o ZooKeeper.
- Adicionalmente, recorremos ao Valgrind para procurar e resolver possíveis leaks de memória através de alguns testes, dos quais não encontrámos leaks de memória. Contudo, o Valgrind detetou alguns erros que são nativos do ZooKeeper e alguns warnings de conditional jumps que não afetam a funcionalidade do projeto.
