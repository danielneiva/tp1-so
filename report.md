

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

4. Estruturas de dados

   1. Descreva e justifique as estruturas de dados utilizadas para
     gerência das threads de espaço do usuário (partes 1, 2 e 5).
  
      Foi criado um struct *dccthread* responsável por armazenar informações de uma thread como seu nome, o contexto da sua execução, o contexto para o qual está esperando, a sua pilha, o inteiro de parâmetro e um inteiro para controlar se é yielded.
      
      Há algumas variáveis globais como *manager* que também armazena contexto. Foram definidas listas do tipo dlist para armazenar as threads e as threads que estão como sleep.
      
      A *dccthread_init* recebe um ponteiro para a função func que por sua vez recebe um inteiro, além do inteiro param. 
      
      A *dccthread_create* ...
      
      A *dccthread_self* ...
      
      A *dccthread_name* ...
      
      A *dccthread_exit* ...
      
      A *dccthread_wait* ...
      
      A *dccthread_sleep* ... 
      
      A *wake_thread* ...

   2. Descreva o mecanismo utilizado para sincronizar chamadas de
     dccthread_yield e disparos do temporizador (parte 4).
       
       A *dccthread_yield* ...
