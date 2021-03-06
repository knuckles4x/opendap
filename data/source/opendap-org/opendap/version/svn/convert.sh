#!/bin/bash
#
#3> <> prov:specializationOf <https://github.com/tetherless-world/opendap/blob/master/data/source/opendap-org/opendap/version/svn/convert.sh>;
#3>    rdfs:seeAlso <https://github.com/tetherless-world/opendap/wiki/OPeNDAP-Provenance>;
#3> .

mkdir automatic &> /dev/null

for portion in tags trunk; do
   if [[ manual/$portion.log -nt automatic/$portion.log.ttl || ../../src/grddl.xsl -nt automatic/$portion.log.ttl ]]; then
      saxon.sh ../../src/grddl.xsl xml ttl -v `cr-sdv.sh --attribute-value` -in manual/$portion.log > automatic/$portion.log.ttl
      justify.sh manual/$portion.log automatic/$portion.log.ttl ../../src/grddl.xsl
      say `valid-rdf.sh automatic/$portion.log.ttl`
   fi
done

if [[ "$CSV2RDF4LOD_PUBLISH" == 'true' ]]; then
   aggregate-source-rdf.sh automatic/tags.log.ttl automatic/trunk.log.ttl
else
   echo "Avoiding publishing b/c \$CSV2RDF4LOD_PUBLISH is not 'true'; set to 'true' to publish."
fi
