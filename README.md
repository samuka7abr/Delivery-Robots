# Delivery Robots — Simulação de Centro de Distribuição (IDP)

Trabalho prático de Programação Concorrente (IDP) — simulação de um centro de distribuição automatizado com robôs coletores/entregadores, esteira transportadora e geração de pacotes.

## Requisitos

- Linux (testado em Ubuntu 24.04)
- gcc
- make
- libncurses-dev (interface de acompanhamento)

## Build

```bash
make
```

Gera o executável em `build/idp`.

## Execução

```bash
make run
```

ou diretamente, escolhendo o cenário (0 a 2):

```bash
./build/idp 1
```

## Testes

```bash
make test
```

## Limpeza

```bash
make clean
```

## Estrutura do projeto

- `include/` — headers (contratos dos módulos)
- `src/` — implementação em C
- `build/` — saída da compilação (objetos e executável)
- `tests/` — testes dos módulos
- `docs/` — documentação do trabalho e materiais de apoio
