#!/bin/bash
# Uso: ./filter_machinefile.sh machinefile > machinefile_ok

inputfile="$1"
if [ -z "$inputfile" ]; then
    echo "Uso: $0 machinefile"
    exit 1
fi

while read -r host; do
    # Ignora líneas vacías y comentarios
    [[ -z "$host" || "$host" =~ ^# ]] && continue
    # Si hay slots, quítalos (ej: nodo1 slots=4 -> nodo1)
    host_clean=$(echo "$host" | awk '{print $1}')
    echo -n "Probing $host_clean... "
    if ssh -o BatchMode=yes -o ConnectTimeout=2 "$host_clean" "echo OK" 2>/dev/null | grep -q OK; then
        echo "$host"
    else
        echo "DOWN"
    fi
done < "$inputfile"