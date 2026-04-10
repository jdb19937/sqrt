#!/bin/bash
# proba_schemata.sh — validate omnia data contra schemata

errores=0
probata=0

proba() {
    local schema="$1"
    local datum="$2"
    probata=$((probata + 1))
    if ! valida_schema "$schema" "$datum" > /dev/null 2>&1; then
        echo "MALUM: $datum"
        valida_schema "$schema" "$datum" 2>&1 | sed 's/^/  /'
        errores=$((errores + 1))
    fi
}

echo "planetae:"
for f in planetae/*.ison; do
    proba schemae/tesserae/planeta-schema.ison "$f"
done

echo "visiones:"
for f in visiones/*.ison; do
    proba schemae/tesserae/visio-schema.ison "$f"
done

echo "campi:"
for f in campi/*.ison; do
    proba schemae/campus-schema.ison "$f"
done

echo ""
echo "$probata probata, $errores errores."
[ "$errores" -eq 0 ]
