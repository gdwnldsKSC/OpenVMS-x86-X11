#!/bin/bash

DEST=$1
shift

if [ -z "$HDR" ]; then
    HDR="HDR"
fi

> $DEST

for i in $*; do
  if [ "$i" == "$HDR" ] || [ "$i" == "HDR" ]; then
    echo >> $DEST;
    read hdr
    echo "$hdr" >> $DEST
  else
    cat $i >> $DEST;
  fi
done < $HDR

