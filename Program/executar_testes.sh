#!/bin/bash
set -euo pipefail

RESULTADOS="../Experimentos_TDTSP/Nivel_4_GPT/Resultados"
ORIGEM="../Results"

mkdir -p "$RESULTADOS"

# Evita misturar resultados anteriores
if [[ -s "$ORIGEM/Results_RKO.csv" ]]; then
    echo "ERRO: Results_RKO.csv já contém resultados."
    echo "Faça uma cópia antes de iniciar os testes."
    exit 1
fi

total=0

for instancia in ../Instances/KP/*_customers/Instance_*.txt; do
    nome=$(basename "$instancia" .txt)

    # Extrai o número de clientes do nome
    clientes=$(echo "$nome" | cut -d_ -f3)

    echo "Executando: $nome | ${clientes}s | 5 runs"

    ./runRKO "$instancia" "$clientes"

    total=$((total + 1))

    cp "$ORIGEM/Results_RKO.csv" "$RESULTADOS/Results_RKO.csv"
    cp "$ORIGEM/Solutions_RKO.txt" "$RESULTADOS/Solutions_RKO.txt"

    echo "Concluídas: $total/225"
done

# Preserva os resultados deste nível
cp "$ORIGEM/Results_RKO.csv" "$RESULTADOS/Results_RKO.csv"
cp "$ORIGEM/Solutions_RKO.txt" "$RESULTADOS/Solutions_RKO.txt"

echo "Testes concluídos! Total: $total instâncias."