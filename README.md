## Introdução ao Tema do Projeto
- O objetivo geral deste projeto focou-se na concretização de um serviço de armazenamento de pares (chave,valor) (nos moldes da interface java.util.Map da API Java) similar ao mesmo utilizado pela Amazon para dar suporte aos seus serviços Web. Neste sentido, as estruturas de dados utilizadas para armazenar esta informação são uma lista encadeada simples e uma tabela hash, dada a sua elevada eficiência ao nível da pesquisa.

## Descrição das Etapas de Implementação
**O projeto consistiu em 4 etapas de implementação:**
- Na **etapa 1** foram definidas estruturas de dados e implementadas várias funções para lidar
com a manipulação dos dados que vão ser armazenados na tabela hash. Também foi construído
um módulo para serialização de dados, que teve como objetivo destacar a
necessidade de serializar dados para a comunicação em aplicações distribuídas.
- Na **etapa 2** implementou-se um sistema cliente-servidor simples, no qual o servidor ficou
responsável por manter uma tabela de hash e o cliente responsável por comunicar com o
servidor para realizar operações na tabela. Foram também utilizados os **Protocol Buffers da
Google** para automatizar a serialização e de-serialização dos dados, tendo por base um
ficheiro sdmessage.proto com a definição da mensagem a ser usada na comunicação,
tanto para os pedidos como para as respostas.
- Na **etapa 3** foi criado um sistema concorrente que aceita e processa pedidos de múltiplos
clientes em simultâneo através do uso de múltiplas threads. Este sistema também garante a
gestão da concorrência no acesso a dados partilhados no servidor, concretizando uma
funcionalidade adicional para obtenção de estatísticas do servidor.
- Na **etapa 4** o sistema passou a suportar tolerância a falhas através de replicação do estado do servidor, seguindo o modelo Chain Replication e usando o serviço de coordenação **ZooKeeper**.
