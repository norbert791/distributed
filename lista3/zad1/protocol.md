# Wymagania techniczne
## Master
- monitoruje działanie node'ów
- werbuje node'y
- zarządza statusem primary

## Primary
- Przyjmuje **wszystkie** operacje read i write
- Wszystkie operacje są przekazywane do backupu

## Secondary
- Przyjmuje operacje od Primary

## Idle
- Oczekuje na przydzielenie roli
- Przy inicjalizacji przyjmuje snapshoty