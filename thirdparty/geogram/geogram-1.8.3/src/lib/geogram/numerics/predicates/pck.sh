# MCCFLAGS can be set to "-semistatic" to generate 
#  semistatic predicates if need be
#   (but default mode is better according to FPG's documentation)

MCCFLAGS=
OUTFILENAME=`basename $1 .pck`.h
echo Generating $OUTFILENAME ...
echo "/* Automatically generated code, do not edit */" > $OUTFILENAME
echo "/* Generated from source file:" $1 "*/" >> $OUTFILENAME

for i in DIM2 DIM3 DIM4 DIM6 DIM7 DIM8
do
   echo instancing $i
   cp $1 /tmp/pck_input.c
   cc -E -D$i -I. /tmp/pck_input.c | egrep -v '#' > /tmp/pck_preproc.mcc
   mcc $MCCFLAGS /tmp/pck_preproc.mcc | \
       sed -e 's|double \([pq].\)_0\(, double \1_.\)\+|const double* \1|g'  \
           -e 's|\([^_]\)\([pq]\)\(.\)_\(.\)\([^_]\)|\1\2\3[\4]\5|g'        \
       >> $OUTFILENAME
done

#echo instancing DIMN_EXACT
#cp $1 /tmp/pck_input.c
#cc -E -DDIMN_EXACT -I. /tmp/pck_input.c | egrep -v '#' >> $OUTFILENAME

#The sed command replaces multiple parameters with arrays:
#  First  sed command replaces in the prototype of the function 
#     'double p1_0, double p1_1, ...,' with 'const double* p1'
#  Second sed command replaces all occurences of pi_j with pi[j]
