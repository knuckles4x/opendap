#!/bin/bash

for portion in tags trunk; do
   echo manual/$portion.log
   if [[ -e source/$portion && source/$portion -nt manual/$portion.log ]]; then
      mkdir manual &> /dev/null
      pushd source/$portion
         svn log -v --xml > ../../manual/$portion.log
      popd
   else
      echo "   (Skipping svn log because svn repository is not checked out.)"
   fi
done
