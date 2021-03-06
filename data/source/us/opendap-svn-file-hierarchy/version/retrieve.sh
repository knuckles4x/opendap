#!/bin/bash
#
#
#
# WARNING: DO NOT EDIT THIS FILE 
#
#   ... if it appears as a retrieval trigger in a Prizms instance's data root.
#   Editing this file will also edit the system default in the Prizms installation, e.g. ~/opt/prizms/bin/,
#     which may lead to conflicts when updating Prizms' installation.
#
#
#
#3> <> a conversion:RetrievalTrigger; #, conversion:Idem___potent;
#3>    prov:specializationOf <https://github.com/tetherless-world/opendap/blob/master/data/source/us/opendap-svn-file-hierarchy/version/retrieve.sh>;
#3>    prov:wasDerivedFrom   <https://github.com/timrdf/prizms/blob/master/bin/dataset/pr-neighborlod.sh>;
#3>    rdfs:seeAlso          <https://github.com/timrdf/csv2rdf4lod-automation/wiki/Secondary-Derivative-Datasets>;
#3> .
#
# This script sets up a new version of a dataset when given a URL to a tabular file and some options
# describing its structure (comment character, header line, and delimter).
#
# If you have a non-tabular file, or custom software to retrieve data, then this script can be 
# used as a template for the retrieve.sh that is placed in the version directory.
#
# See:
# https://github.com/timrdf/csv2rdf4lod-automation/wiki/Automated-creation-of-a-new-Versioned-Dataset
#

#                              e.g. /home/lebot/opt/prizms/bin/dataset/pr-neighborlod.sh
#                               |
#                               |                    e.g. ./retrieve.sh
#                               |                     |
#                              \./                   \./
#[ -n "`readlink $0`" ] && this=`readlink $0` || this=$0
#echo $this
#HOME=$(cd ${this%/*/*/*} && pwd)
#export PATH=$PATH`$HOME/bin/install/paths.sh`
#export CLASSPATH=$CLASSPATH`$HOME/bin/install/classpaths.sh`

see="https://github.com/timrdf/csv2rdf4lod-automation/wiki/CSV2RDF4LOD-not-set"
CSV2RDF4LOD_HOME=${CSV2RDF4LOD_HOME:?"not set; source csv2rdf4lod/source-me.sh or see $see"}

# cr:data-root cr:source cr:directory-of-datasets cr:dataset cr:directory-of-versions cr:conversion-cockpit
ACCEPTABLE_PWDs="cr:directory-of-versions cr:conversion-cockpit"
if [ `${CSV2RDF4LOD_HOME}/bin/util/is-pwd-a.sh $ACCEPTABLE_PWDs` != "yes" ]; then
   ${CSV2RDF4LOD_HOME}/bin/util/pwd-not-a.sh $ACCEPTABLE_PWDs
   exit 1
fi

if [[ `cr-pwd-type.sh` == "cr:conversion-cockpit" ]]; then
   pushd ../ &> /dev/null
fi

TEMP="_"`basename $0``date +%s`_$$.tmp

if [[ "$1" == "--help" ]]; then
   echo "usage: `basename $0` [version-identifier] [URL]"
   echo "   version-identifier: conversion:version_identifier for the VersionedDataset to create. Can be '', 'cr:auto', 'cr:today', 'cr:force'."
   echo "   URL               : URL to use during retrieval."
   exit 1
fi


#-#-#-#-#-#-#-#-#
version="$1"
version_reason=""
url="$2"
if [[ "$1" == "cr:auto" && ${#url} -gt 0 ]]; then
   version=`urldate.sh $url`
   #echo "Attempting to use URL modification date to name version: $version"
   version_reason="(URL's modification date)"
fi
if [[ ${#version} -eq 0                        || \
      ${#version} -ne 11 && "$1" == "cr:auto"  || \
                            "$1" == "cr:today" || \
                            "$1" == "cr:force" ]]; then
   # We couldn't determine the date from the URL (11 length from e.g. "2013-Aug-12")
   # Or, there was no URL given.
   # Or, we're told to use today's date.
   version=`cr-make-today-version.sh 2>&1 | head -1`
   #echo "Using today's date to name version: $version"
   version_reason="(Today's date)"
fi
if [[ -e "$version" && "$1" == "cr:force"  ]]; then
   version=`date +%Y-%b-%d-%H-%M_%s`
fi
if [ ${#version} -gt 0 -a `echo $version | grep ":" | wc -l | awk '{print $1}'` -gt 0 ]; then
   # No colons allowed?
   echo "Version identifier invalid."
   exit 1
fi
iteration=`find -mindepth 1 -maxdepth 1 -name "$version*" | wc -l | awk '{print $1}'`
if [[ "$iteration" -gt 0 ]]; then
   let "iteration=$iteration+1"
   iteration="_$iteration"
   version=$version$iteration
   version_reason="$version_reason (expanding to iteration)"
fi
shift 2

echo "INFO version   : $version $version_reason"
echo "INFO iteration : $iteration"
echo "INFO url       : $url"

#
# This script is invoked from a cr:directory-of-versions, 
# e.g. source/contactingthecongress/directory-for-the-112th-congress/version
#
#if [[ ! -d $version || ! -d $version/source || `find $version -empty -type d -name source` ]]; then

   see='https://github.com/timrdf/csv2rdf4lod-automation/wiki/CSV2RDF4LOD-environment-variables'
   endpoint=${CSV2RDF4LOD_PUBLISH_VIRTUOSO_SPARQL_ENDPOINT:?"not set; source csv2rdf4lod/source-me.sh or see $see"}

   # Create the directory for the new version.
   mkdir -p $version/source

   rq='../../../src/svn-files.rq'
   # Go into the directory that stores the original data obtained from the source organization.
   echo INFO `cr-pwd.sh`/$version/source
   pushd $version/source &> /dev/null
      touch .__CSV2RDF4LOD_retrieval # Make a timestamp so we know what files were created during retrieval.
      # - - - - - - - - - - - - - - - - - - - - Replace below for custom retrieval  - - - \
      us="$CSV2RDF4LOD_BASE_URI"
      if [[ "$us" =~ http* ]]; then
         our_redirect=`curl -sLI $CSV2RDF4LOD_BASE_URI | grep "Location:" | head -1 | sed 's/^\s*//;s/\s*$//' | awk '{print $2}'`
      fi
      #rq2=`basename $rq`
      #cat $rq | awk -f ../../../src/unknown-domain.awk -v ns1="$us" ns2="$our_redirect" > $rq2
      if [[ `which cache-queries.sh` && "$endpoint" =~ http* && -e $rq ]]; then
         cache-queries.sh "$endpoint" -o ttl -q $rq --limit-offset -od .
         #csv="$rq2.csv"
         #if [[ `wc -l $csv | awk '{print $1}'` -lt 2 ]]; then
         #   echo "No results from $rq2:"
         #   cat $rq2 $csv
         #   rm $csv
         #   echo "Changing from subject-based to object-based query."
         #   cat $rq | awk -f ../../../src/unknown-domain.awk -v ns1="$us" ns2="$our_redirect" o=o > $rq2
         #   cache-queries.sh "$endpoint" -o csv -q $rq2 -od .
         #fi
      else
         echo "   ERROR: Failed to create dataset `basename $0`:"                        
         echo "      CSV2RDF4LOD_PUBLISH_VIRTUOSO_SPARQL_ENDPOINT: $endpoint"        
         echo "      cache-queries.sh path: `which cache-queries.sh`"
         echo "      $rq:"
         ls -lt $rq $rq2
      fi
      if [ "$CSV2RDF4LOD_RETRIEVE_DROID_SOURCES" != "false" ]; then                     # |
         sleep 1                                                                        # |
         cr-droid.sh . > cr-droid.ttl                                                   # |
      fi                                                                                # |
      # - - - - - - - - - - - - - - - - - - - - Replace above for custom retrieval - - - -/
   popd &> /dev/null

   # Go into the conversion cockpit of the new version.
   worthwhile="no"
   pushd $version &> /dev/null

      if [ ! -e automatic ]; then
         mkdir automatic
      fi

      retrieved_files=`find source -newer source/.__CSV2RDF4LOD_retrieval -type f | grep -v "pml.ttl$" | grep -v "cr-droid.ttl$"`

      # uri-nodes.sh source/svn-files.rq.ttl | grep "^https://scm" | sort -u
      bte="$DATAFAQS_HOME/services/sadi/faqt/naming/between-the-edges.py"
      python $bte source/svn-files.rq.ttl text/turtle automatic/svn-files.bte.ttl
      justify.sh source/svn-files.rq.ttl automatic/svn-files.bte.ttl https://github.com/timrdf/DataFAQs/blob/master/services/sadi/faqt/naming/between-the-edges.py
      find source -name "svn-files.rq.[0-9].ttl"           -print0 | xargs -0 -I CHUNK -P ${CSV2RDF4LOD_CONCURRENCY:-1} python $bte CHUNK text/turtle -od automatic
      find source -name "svn-files.rq.[0-9][0-9].ttl"      -print0 | xargs -0 -I CHUNK -P ${CSV2RDF4LOD_CONCURRENCY:-1} python $bte CHUNK text/turtle -od automatic
      find source -name "svn-files.rq.[0-9][0-9][0-9].ttl" -print0 | xargs -0 -I CHUNK -P ${CSV2RDF4LOD_CONCURRENCY:-1} python $bte CHUNK text/turtle -od automatic
      # TODO: find a complete query that covers all of them.
      # TODO: justify the chunks

      if [[ `void-triples.sh source/svn-files.rq.ttl` -gt 0 && `void-triples.sh automatic/svn-files.bte.ttl` -gt 0 ]]; then
         worthwhile="yes"
      fi

      #if [[ "$worthwhile" == 'yes' ]]; then
      #   aggregate-source-rdf.sh source/svn-files.rq.ttl automatic/svn-files.bte.ttl
      #fi

   popd &> /dev/null

   if [[ "$worthwhile" != 'yes' ]]; then
      echo
      echo "Note: version $version of dataset `cr-dataset-id.sh` did not become worthwhile; removing retrieval attempt."
      echo
      rm -rf $version
   else
      # Recursive step; process another chunk into a new version.
      cr-retrieve.sh -w --skip-if-exists
   fi
#else
#   echo "Version exists; skipping."
#fi

if [[ `cr-pwd-type.sh` == "cr:conversion-cockpit" ]]; then
   popd ../ &> /dev/null
fi
