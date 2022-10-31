#If powershell doesn't like this script being unsigned, wrap the call with:
#powershell -executionpolicy ByPass -File .\convertToOSM.ps1 .\your_osm_file.osm.pbf
$SRC=$args[0]
if (!$SRC)
{
	Write-Output "Missing Input osm.pbf";
	return;
}

foreach ($file in Get-ChildItem ini -Name -Filter *.ini)
{
	$layer = $file -replace ".ini";
	$temp = Get-Content .\ini\$file | where {$_ -match "WHEREQUERY"}
	$wquery = $temp -replace "#WHEREQUERY="
	$temp = Get-Content .\ini\$file | where {$_ -match "VALIDATTRIBUTES"}
	$wselect = $temp -replace "#VALIDATTRIBUTES="
	$temp = Get-Content .\ini\$file | where {$_ -match "VALIDLAYERS"}
	$wlayers = $temp -replace "#VALIDLAYERS="
	Write-Output $layer;
	Invoke-Expression "ogr2ogr.exe -where `"$wquery`" -select `"$wselect`" -f GPKG $layer.gpkg $SRC -oo CONFIG_FILE=ini\$file $wlayers"
}


