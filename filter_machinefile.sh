#!/bin/bash
# Uso: ./filter_machinefile.sh machinefile > machinefile_ok

inputfile="$1"
if [ -z "$inputfile" ]; then
    echo "Uso: $0 machinefile" >&2
    exit 1
fi

first=1
while IFS= read -r host || [ -n "$host" ]; do
    # Ignora líneas vacías y comentarios
    [[ -z "$host" || "$host" =~ ^# ]] && continue
    if [ $first -eq 1 ]; then
        # La primera máquina (master) siempre se incluye
        echo "$host"
        first=0
    else
        # Si hay slots, quítalos solo para el chequeo SSH
        host_clean=$(echo "$host" | awk '{print $1}')
        if ssh -o BatchMode=yes -o ConnectTimeout=2 "$host_clean" "echo OK" 2>/dev/null | grep -q OK; then
            echo "$host"
        fi
    fi
done < "$inputfile"