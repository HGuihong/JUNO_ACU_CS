#!/bin/bash

filename="testchain.db"

for ((i=1; i<1000; i++))
do
    
cat>>$filename <<EOF
record(ai, "sinch$i") {
   field(SCAN, "Passive") 
   field(INP, "sinch$(($i-1))")
   field(FLNK, "sinch$(($i+1))")
}

EOF


done
