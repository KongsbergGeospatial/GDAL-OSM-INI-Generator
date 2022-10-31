# GDAL OSM ini Generator #

The goal of this project was to generate the .ini files needed by GDAL to convert an osm.pbf file to a more useful format for runtime display. Typically, PostGIS is used for this type of data transformation, but PostGIS can be cumbersome to setup for users. As such, a trivially copiable file format like GeoPackage was chosen as the output format.

To do this, a CartoCSS stylesheet was taken and an .ini file was generated for each layer. This requires parsing the SQL query for each layer and breaking it into the components necessary for each attribute within the layer. These ini files will allow GDAL to produce the output data that is suitable to use with the CartoCSS styles.

This project used GeoFabrik's stylesheet as a base.
https://github.com/geofabrik/openstreetmap-carto-vector-tiles

For those looking to skip building the C++ project and to go straight to GDAL, we've included the generated ini files, a bash script, and a PowerShell script for running the conversions in the 'Sample' folder.

## Limitations ##

### Magnacarto
For the pure simplicity of only having to read a single file, this project requires the CartoCSS stylesheet to be converted into a single 'project.xml' file. For this, 'Magnacarto' was used.
https://github.com/omniscale/magnacarto

### Limited SQL
As opposed to the PostGIS queries used by the CartoCSS stylesheets, neither GDAL or the sql parser used by this project can handle anything beyond basic SQL queries. This limitation required this project to create 'good enough' alternatives to these queries.

This means that not all features may get properly parsed. For the Geofabrik's stylesheet, the layers 'turning-circle-casing', 'turning-circle-fill', 'roads-text-ref-low-zoom', and 'roads-text-ref' still aren't properly parsed. Handwritten ini files for 'roads-text-ref-low-zoom' and 'roads-text-ref' exist in the Sample folder.


## 3rd Party Tools ##
https://github.com/hyrise/sql-parser
https://github.com/zeux/pugixml

Copyright (c) Kongsberg Geospatial Ltd. All rights reserved.
Licensed under the [MIT](LICENSE) license.
