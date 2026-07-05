#!/bin/sh

bibs="
geogram.bib
"

cat <<EOF
/*
 * This file was automatically generated, do not edit.
 */

#include <geogram/bibliography/bibliography.h>

void register_embedded_bib_file(void);
   
void register_embedded_bib_file() {
EOF

for bib in $bibs
do
    echo "     GEO::Biblio::register_references("
    cat $bib | sed    -e 's|\\|\\\\|g' \
		      -e 's|"|\\\"|g' \
                      -e 's|\\n|\\\\n|' \
		      -e 's|^|        \"|' \
		      -e 's| *$| \\n\"|'     
    echo "     );"
    echo
done

echo "}"
