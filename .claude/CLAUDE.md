# Instruções do projeto — Delivery Robots (IDP)

Trabalho prático de Programação Concorrente (IDP). Simulação de um centro de
distribuição com robôs coletores/entregadores em C, usando threads,
mutexes/cond vars, interface ncurses (raylib é bônus). Issues no GitHub
seguem o backlog do grupo (Samuel, Andreia, Genaro).

## Fluxo de trabalho com issues

Antes de começar a resolver qualquer issue:

0. **Ler `docs/requisitos.md` e entender o funcionamento geral da aplicação
   antes de fazer qualquer coisa.** Reler as partes relevantes do
   enunciado, olhar o estado atual do código (`src/`) e entender como a
   issue se encaixa no todo — mapa, esteira, estações, robôs, sincronização
   — antes de escrever uma linha. Nunca implementar no escuro.
1. **Verificar independência da issue.** Ler o campo `Depends on` no corpo
   da issue (`gh issue view <N> --json body`). Se ela depender de outra
   issue, confirmar que a dependência está `CLOSED` antes de prosseguir.
   Nunca implementar uma issue com dependência ainda em aberto sem avisar
   o usuário primeiro.
2. **Verificar PRs anteriores e o status da issue.** Antes de tocar em
   código:
   - `gh issue view <N> --json state` — a issue precisa estar `OPEN`. Se já
     estiver `CLOSED`, parar e perguntar antes de continuar.
   - `gh pr list --state open` — confirmar que não existe PR aberto pendente
     para essa issue, e que o PR da issue anterior já foi mergeado ou
     fechado (`gh pr view <N> --json state,mergedAt`).
3. Trabalhar sempre numa branch de feature dedicada à issue — nunca direto
   na `main`.
4. Implementar e testar localmente antes de considerar a issue pronta:
   `make` sem warnings, e para headers isolados
   `gcc -Wall -Wextra -fsyntax-only -x c arquivo.h`.
5. **Nunca commitar ou dar `git push` sem pedido explícito do usuário na
   mensagem atual.** "Resolver a issue" significa escrever e testar o
   código — parar aí até o usuário pedir commit/push.
6. Quando pedirem commit (`/commit`): commits atômicos, mensagem de uma
   linha só, padrão Angular (`tipo(escopo): resumo`), **sem
   `Co-Authored-By` e sem corpo/descrição**.
7. **Regra estrita:** antes de qualquer `git push`, rodar
   `git branch --show-current` e `git status` para confirmar que a branch é
   a correta (nunca push direto pra `main`) e que só há o que deveria estar
   ali prestes a subir.
8. Ao abrir PR: usar `.github/pull_request_template.md`, sempre referenciar
   `Closes #<numero>`. PR e commits não devem conter nenhuma menção a
   Claude/IA/assistente.

## Uso do `gh`

- Ver issue completa: `gh issue view <N> --json number,title,body,labels,assignees,milestone,state`
- Listar issues abertas: `gh issue list --state open`
- Listar PRs abertos: `gh pr list --state open`
- Ver se um PR já foi mergeado: `gh pr view <N> --json state,mergedAt`
- Abrir PR: `gh pr create --title "..." --body "..." --base main --head <branch>`
- Fechar PR: `gh pr close <N>`
- Assignees do grupo: `samuka7abr` (Samuel), `andreiabtiveron` (Andreia),
  `joaogenaro11` (Genaro)

## Padrões de código (C)

- Zero warnings com `-Wall -Wextra`.
- Evitar alocação desnecessária: preferir os tamanhos fixos já definidos em
  `idp.h` (`MAX_*`) a `malloc` solto. Todo `malloc` tem que ter um `free`
  correspondente.
- Todo `pthread_mutex_init`/`pthread_cond_init` precisa do `_destroy`
  correspondente antes do programa encerrar.
- Toda `pthread_create` precisa de `pthread_join` (ou detach explícito e
  justificado no código).
- Sem abstração prematura: implementar exatamente o que a issue pede, nada
  de preparar estrutura para funcionalidade futura que ainda não existe.
- Comentário só quando o porquê não é óbvio pelo código — não comentar o
  que já está claro pelo nome da função/variável.
- Nomes de domínio (structs, campos, funções da simulação) em português,
  consistente com o enunciado e o restante do código já escrito.

## Commits

- Sem `Co-Authored-By`, sem corpo — mensagem de uma linha, padrão Angular.
- Nunca commitar ou dar push por conta própria — só quando pedido
  explicitamente na mensagem do usuário.
