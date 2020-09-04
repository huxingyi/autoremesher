#gallery.sh: generates webpages to visualize a collection of 3D models

#******************************************************************************
#usage: - create a directory with all the models in the "models" subdirectory
#       - launch "gallery.sh" in that directory
#
# this will generate thumbnail images (in 'images'), a JavaScript viewer and the
# associated webpage for each model.
#******************************************************************************


SCRIPTDIR=`dirname "$0"`
JSDIR=$SCRIPTDIR/../build/Emscripten-clang-Release/bin/

mkdir -p viewer

cp $SCRIPTDIR/FileSaver.js viewer
cp $JSDIR/vorpaview.js viewer
cp $JSDIR/vorpaview.js.mem viewer

mkdir -p images

# Generate images
for i in `ls models`
do
   $SCRIPTDIR/snapshot.sh models/$i
   mv output.png images/$i.png
done

# Generate js embeddable datafiles
for i in `ls models`
do
   (cd viewer; cp ../models/$i .; python $EMSCRIPTEN/tools/file_packager.py $i'_data.data' --preload $i > $i'_data.js'; rm $i)
done

# Generate viewer pages
for i in `ls models`
do
   cat $SCRIPTDIR/template_emscripten.html | sed -e 's/%EXENAME%/'vorpaview'/g' \
       -e 's|<!-- DATAFILE -->|<script async type="text/javascript" src="'$i'_data.js"></script>|g' \
       > viewer/$i.html
done

# Generate index
> index.html
echo "<table>" >> index.html
#echo "<tr>" >> index.html
for i in `ls models`
do
   echo "<tr>" >> index.html
   echo "<td>" >> index.html
   echo "<a href=\"viewer/$i.html\"> <img src=\"images/$i.png\"/> </a>" >> index.html
   echo "</td>" >> index.html
   echo "<td>" >> index.html
   
   echo "<p><a href=\"viewer/$i.html\"> $i </a></p><br/>" >> index.html
   echo "<p><a href=\"models/$i\"> [download datafile] </a></p><br/>" >> index.html
   echo "</td>" >> index.html
   echo "</tr>" >> index.html   
done
#echo "</tr>" >> index.html
echo "</table>" >> index.html


