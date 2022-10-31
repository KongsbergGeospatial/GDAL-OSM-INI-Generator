These scripts allow you to use GDAL to extract osm.pbf files to GeoPackages.
The scripts expect you to have GDAL on your path.
The "convertOSM.sh" script can be run from a bash terminal such as 'git-bash', and the 'convertOSM.ps1' can be run from powershell. In Windows, you can also drag the osm file onto the provided shortcut 'DragAnOsmFileOnThisToExtractIt' to run the script.
NOTE: When running the script from powershell you'll likely get caught by the fact that the script isn't signed. See the comment in 'convertOSM.ps1' on how to get around this.