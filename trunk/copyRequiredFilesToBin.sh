#!/bin/sh

rootDirectory=`dirname $0`;
wmrProjectDirectory="WhoreMasterRenewal"
resourcesDirectory="WhoreMasterRenewal/Resources"
dependenciesDirectory="WhoreMasterRenewal/Dependencies"
savesDirectory="WhoreMasterRenewal/Saves"
binDirectory="bin"
libraries=("freetype" "jpeg" "lua" "SDL" "SFGUI" "SFML" "TinyXML")

echo "Copying libraries..."

for library in ${libraries[@]}
do
	[ -d "$rootDirectory/$dependenciesDirectory/$library/bin" ] && find $rootDirectory/$dependenciesDirectory/$library/bin -type f -print0 | while read -d $'\0' file;
				do
					filename=$(basename $file)
					cp "$file" "$rootDirectory/$binDirectory/$filename"
				done
done

echo "Done..."
echo "Copying resources..."

find $rootDirectory/$resourcesDirectory/ -type d -print0 | while read -d $'\0' directory;
do
	directory=${directory//"$resourcesDirectory"/"$binDirectory/Resources"}
	[ ! -d "$directory" ] && mkdir "$directory"
done

find $rootDirectory/$resourcesDirectory/ -type f -print0 | while read -d $'\0' file;
do
	copyTo=${file//"$resourcesDirectory"/"$binDirectory/Resources"}
	cp "$file" "$copyTo"
done

echo "Done..."

if [ "$1" == "Saves" ]; then

	echo "Copying saves..."

	[ ! -d "$rootDirectory/$binDirectory/Saves" ] && mkdir "$rootDirectory/$binDirectory/Saves"
	find $rootDirectory/$savesDirectory -type f -print0 | while read -d $'\0' file;
	do
		copyTo=${file//"$savesDirectory"/"$binDirectory/Saves"}
		cp "$file" "$copyTo"
	done

	echo "Done..."

fi

echo "Copying licenses..."

copyTo=$rootDirectory/$binDirectory
cp $rootDirectory/$wmrProjectDirectory/licenses.txt $copyTo

echo "Done..."

echo "Finished."