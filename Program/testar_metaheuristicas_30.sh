#!/usr/bin/env bash
set -euo pipefail

# Execute a partir da pasta Program, com o decoder desejado ja compilado:
#   bash /caminho/testar_metaheuristicas_30.sh N3
#   bash /caminho/testar_metaheuristicas_30.sh N4
# Mantem MAXRUNS=5 e o limite de 1 segundo por cliente.

DECODER="${1:-}"
if [[ "$DECODER" != "N3" && "$DECODER" != "N4" ]]; then
  echo 'Uso: bash testar_metaheuristicas_30.sh N3|N4' >&2
  exit 1
fi

CONFIG="config/config_tests.conf"
ORIGEM="../Results"
INSTANCIAS="../Instances/KP"
DESTINO="../Experimentos_TDTSP/Comparacao_N4_MH/${DECODER}"
METODOS=(SA ILS VNS BRKGA BRKGA-CS PSO GA LNS GRASP IPR)

[[ -f "$CONFIG" && -x ./runRKO ]] || { echo 'Execute na pasta Program, com config e runRKO presentes.' >&2; exit 1; }
[[ -d "$INSTANCIAS" ]] || { echo "Pasta de instancias nao encontrada: $INSTANCIAS" >&2; exit 1; }
mkdir -p "$ORIGEM" "$DESTINO"

# Nao misturar arquivos existentes com o novo experimento.
for f in "$ORIGEM/Results_RKO.csv" "$ORIGEM/Solutions_RKO.txt"; do
  if [[ -s "$f" ]]; then
    echo "ERRO: $f nao esta vazio. Arquive-o antes de iniciar." >&2
    exit 1
  fi
done

# Conferir configuracao original sem alterá-la permanentemente.
grep -Eq '^MAXRUNS[[:space:]]+5[[:space:]]*$' "$CONFIG" || {
  echo 'ERRO: MAXRUNS deve ser 5 no arquivo de configuracao.' >&2; exit 1;
}

BACKUP="$(mktemp)"
cp "$CONFIG" "$BACKUP"
restore_config() { cp "$BACKUP" "$CONFIG"; rm -f "$BACKUP"; }
trap restore_config EXIT
# Seleciona 30 instancias: 3 categorias x 10 tamanhos, ID 01.
INST_LIST=()
for categoria in S M L; do
  for clientes in 10 12 20 22 50 52 80 82 100 102; do
    matches=("$INSTANCIAS"/*_customers/"Instance_${categoria}_${clientes}_01.txt")
    if [[ ${#matches[@]} -ne 1 || ! -f "${matches[0]}" ]]; then
      echo "ERRO: nao encontrei uma unica instancia para ${categoria}, ${clientes} clientes." >&2
      exit 1
    fi
    INST_LIST+=("${matches[0]}")
  done
done

[[ ${#INST_LIST[@]} -eq 30 ]] || { echo "ERRO: esperadas 30 instancias" >&2; exit 1; }
printf '%s\n' "${INST_LIST[@]}" > "$DESTINO/instancias_selecionadas.txt"
printf 'decoder=%s\nmaxruns=5\ntime_limit=number_of_customers_seconds\nselection=Instance_{S,M,L}_{clients}_01.txt\n' "$DECODER" > "$DESTINO/protocolo.txt"

for metodo in "${METODOS[@]}"; do
  echo "=== Decoder $DECODER | Metodo $metodo ==="
  # Remove somente linhas que correspondem aos nomes dos metodos.
  awk '$0!="SA" && $0!="ILS" && $0!="VNS" && $0!="BRKGA" && $0!="BRKGA-CS" && $0!="PSO" && $0!="GA" && $0!="LNS" && $0!="GRASP" && $0!="IPR" && $0!="MultiStart" {print}' "$BACKUP" > "$CONFIG"
  printf '\n%s\n' "$metodo" >> "$CONFIG"

  PASTA="$DESTINO/$metodo"
  mkdir -p "$PASTA"
  contador=0
  for instancia in "${INST_LIST[@]}"; do
    nome="$(basename "$instancia" .txt)"
    csv="$PASTA/${nome}.csv"
    sol="$PASTA/${nome}.txt"
    contador=$((contador+1))
    if [[ -s "$csv" && -s "$sol" ]]; then
      echo "Ja concluida: $metodo / $nome"
      continue
    fi
    [[ ! -s "$ORIGEM/Results_RKO.csv" && ! -s "$ORIGEM/Solutions_RKO.txt" ]] || {
      echo 'ERRO: arquivos temporarios contem resultados; verifique antes de prosseguir.' >&2; exit 1;
    }
    echo "Executando: $DECODER | $metodo | $contador/30 | $nome | 5 runs"
    clientes="$(echo "$nome" | cut -d_ -f3)"
    ./runRKO "$instancia" "$clientes"
    [[ -s "$ORIGEM/Results_RKO.csv" && -s "$ORIGEM/Solutions_RKO.txt" ]] || {
      echo "ERRO: saida ausente em $metodo / $nome" >&2; exit 1;
    }
    mv "$ORIGEM/Results_RKO.csv" "$csv"
    mv "$ORIGEM/Solutions_RKO.txt" "$sol"
  done
done

echo "Concluido: $DECODER. Resultados em $DESTINO"
