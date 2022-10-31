SRC=$1
if [ -z "$1"]; then
	echo "Missing Input osm.pbf";
	exit;
fi

for file in `ls ini | grep '.ini'`; do
	layer=`echo $file | sed -e 's/\.ini//'`;
	wquery=`grep WHEREQUERY ini/${file} | sed -e 's/#WHEREQUERY=//'`
	wselect=`grep VALIDATTRIBUTES ini/${file} | sed -e 's/#VALIDATTRIBUTES=//'`
	wlayers=`grep VALIDLAYERS ini/${file} | sed -e 's/#VALIDLAYERS=//'`
	echo $layer;
	echo $wquery
	ogr2ogr -where "$wquery" -select "$wselect" -f GPKG ${layer}.gpkg ${SRC} -oo CONFIG_FILE=ini/${file} ${wlayers}
done 


