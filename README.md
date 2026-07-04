# Delivery Robots — Simulação de Centro de Distribuição (IDP)

Trabalho prático de Programação Concorrente (IDP) — simulação de um centro de distribuição automatizado com robôs coletores/entregadores, esteira transportadora e geração de pacotes.

## Requisitos

- Linux (testado em Ubuntu 24.04)
- gcc
- make
- ncurses (interface de acompanhamento):
  - Ubuntu: `sudo apt install libncurses-dev`
  - Fedora: `sudo dnf install ncurses-devel`

## Build

```bash
make
```

Gera o executável em `build/idp`.

## Execução

```bash
make run
```

Em um terminal interativo, o programa mostra os três cenários e solicita a
escolha. Também é possível selecionar diretamente pelo índice:

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
