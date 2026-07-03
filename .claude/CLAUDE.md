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
- **Guardas `NULL` consistentes.** Toda função pública de módulo que
  desreferencia um ponteiro valida `ptr == NULL` no topo e retorna o
  resultado "vazio" correspondente (`false` / `NULL` / `0`), como
  `esteira_inicializar` e `gerador_inicializar`. Não guardar um argumento e
  deixar outro solto na mesma função.
- **Nunca sobrecarregar um sentinela.** Um único `NULL`/`-1` não pode
  significar duas coisas diferentes (ex.: "acabou" e "sem espaço"). Quando há
  mais de um motivo de parada/falha, usar `enum` de resultado + out-param
  (padrão adotado em `ResultadoGeracao`). Esse bug já apareceu 2x no repo
  (diagonal do robô na #3, `NULL` do gerador na #5) — vigiar em cada módulo.
- **Comentário/doc não pode contradizer o código.** Header que descreve
  comportamento diferido pra outra issue deve deixar claro que é diferido —
  não afirmar que já faz (ex.: mutex do `mapa`/`esteira` fica pra #9; o
  doc-comment não deve dizer que a init já cuida disso).
- **Cortes de escopo diferidos são intencionais.** Não implementar por
  antecipação o que pertence a issue futura (ex.: sync mutex/cond da esteira
  só entra na #9; `esteira_inicializar` não inicializa o mutex de propósito).

## Testes e verificação

Antes de considerar qualquer código pronto, rodar (nesta ordem):

1. **Build sem warnings:** `make` com `-Wall -Wextra` (já no Makefile). O
   Makefile usa wildcard em `src/` e `tests/`; `make test` compila e roda
   todos os `tests/*.c` (cada binário retorna != 0 se algum check falhar).
2. **Header isolado:** `gcc -Wall -Wextra -fsyntax-only -x c include/foo.h -Iinclude`
   — garante que o header compila sozinho (includes próprios corretos).
3. **Análise estática (`-fanalyzer`, GCC):** pega leak, double-free,
   use-after-free, null-deref e uso de não-inicializado.
   `for f in src/*.c; do gcc -fanalyzer -Wall -Wextra -Iinclude -c "$f" -o /dev/null; done`
4. **UBSan trap-mode (runtime, dispensa a runtime lib):** pega OOB de array,
   overflow de inteiro e null-deref; aborta via SIGILL (exit 132).
   `gcc -Wall -Wextra -Iinclude -fsanitize=undefined -fsanitize-undefined-trap-on-error -g -o bin <srcs>`
   e rodar o binário (exit 0 = sem UB).

`valgrind` e as runtime libs de ASan/UBSan (`libasan`/`libubsan`) **não estão
disponíveis** neste ambiente. Corretude de memória se verifica por
pareamento manual `malloc`/`free` + `-fanalyzer`. Prefira pools fixos em
stack (`MAX_*`) a `malloc` — o gerador/esteira não têm nenhum `malloc`.

Regras que vêm junto:

- **Bug encontrado → teste de regressão junto da correção**, no mesmo PR
  (ex.: colisão de estações na #5, NULL-safety da esteira na #6).
- **Repro pra qualquer coisa suspeita.** Escrever um programinha descartável
  que exercite o caso de borda e observar o comportamento — não confiar só
  na leitura do diff nem só no "compila".

## Revisão de PR

Ao revisar PR de outra pessoa, **não revisar só lendo o diff**:

1. `git worktree add <dir> origin/<branch>` — trabalhar numa cópia isolada,
   nunca na cópia principal.
2. `make` + `make test` + os analisadores acima + repro pros casos suspeitos.
3. Se achar bug na branch de outro contribuidor: aplicar a correção no
   worktree, gerar o `.patch` (`git diff > fix.patch`) e **perguntar** antes
   de pushar — o classificador de permissão bloqueia push em branch alheia
   sem confirmação nomeando a ação (push direto / só comentar / entregar o
   diff).

## Commits

- Sem `Co-Authored-By`, sem corpo — mensagem de uma linha, padrão Angular.
- Nunca commitar ou dar push por conta própria — só quando pedido
  explicitamente na mensagem do usuário.
