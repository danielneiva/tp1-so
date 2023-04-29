

# RELATÓRIO

1. Termo de compromisso

   Os membros do grupo afirmam que todo o código desenvolvido para este
 trabalho é de autoria própria.  Exceto pelo material listado no item
 3 deste relatório, os membros do grupo afirmam não ter copiado
 material da Internet nem ter obtido código de terceiros.

2. Membros do grupo e alocação de esforço

  * Daniel Neiva da Silva <daniel.neiva@dcc.ufmg.br> 50%
  * Euller Saez Lage Silva <eullersaez@dcc.ufmg.br> 50%

3. Referências bibliográficas

    GNU.  **Complete Context Control**. Disponível em: https://www.gnu.org/software/libc/manual/html_node/System-V-contexts.html. Acesso em: 24 abr. 2023.
    
    THE OPEN GROUP. **Ucontext - user context**. Disponível em: https://pubs.opengroup.org/onlinepubs/7908799/xsh/ucontext.h.html. Acesso em: 24 abr. 2023.
  
    LEMODA.  **Library functions - Section 3 - Manual Pages**. Disponível em: https://nxmnpg.lemoda.net/3/. Acesso em: 24 abr. 2023.

    Linux manual page. **sigaction**. Disponível em: https://man7.org/linux/man-pages/man2/sigaction.2.html. Acesso em 24 abr. 2023


    Linux manual page. **timer_crate**. Disponível em https://man7.org/linux/man-pages/man2/timer_create.2.html. Acesso em 24 abr. 2023

4. Estruturas de dados

   1. Descreva e justifique as estruturas de dados utilizadas para
     gerência das threads de espaço do usuário (partes 1, 2 e 5).
  
      O struct dccthread provido no cabeçalho dccthread.h foi implementando contendo as variáveis necessárias para a thread. Nele, foram incluídas:
      
      - uma propriedade ucontext_t para armazenar o contexto de execução
      - uma propriedade dccthread_t *waiting_for para armazenar o endereço de memória da thread que está sendo esperada
      - uma propriedade char name[128] para armazenar o nome da thread
      - uma propriedade char stack[STACKSIZE] para armazenar o stack do contexto de execução
      - uma propriedade (*func)(int) para armazenar a função que a thread executa
      - uma propriedade int param para armazenar a variável que deve ser passada para a thread
      - uma variável int is_yielded para indicar que a thread deu licença da CPU
      - uma variável int is_sleeping para indicar se a thread está dormindo

      Para o armazenamento das threads, foi utilizada uma lista encadeada, cuja implementação foi fornecida no pacote do trabalho. A lista, *active_threads*, armazena as threads prontas para execução.
      Para aleḿ disso, foi declarada uma variável global *ucontext_t manager* para armazenar a thread gerente.

   2. Descreva o mecanismo utilizado para sincronizar chamadas de
     dccthread_yield e disparos do temporizador (parte 4).
       
      Para evitar condições de corrida, foram utilizados os signals do linux. Foi criada uma variável *sigset_t mask*, para armazenar os sinais do programa. Aliado à isso, em cada função do programa que pode tirar e colocar threads em execução, foi utilizada a função *sigprocmask* para bloquear os sinais do temporizador no começo da execução e libera-los ao final.

      Adicionalmente, foi criada um set de sinais para as threads que estão "dormindo", *sigset_t sleeping_mask*, para emitir o sinal de acordar as threads. No inicio de cada ciclo do escalonador, os sinais deste set são liberados e logo em seguida bloqueados, para permitir que as threads acordem.
