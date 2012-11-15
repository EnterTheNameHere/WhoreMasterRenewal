#!/bin/sh

rootDirectory=`dirname $0`;
wmrProjectDirectory="WhoreMasterRenewal"
resourcesDirectory="Resources"
dependenciesDirectory="Dependencies"
savesDirectory="Saves"
binDirectory="bin"

# Libraries (directories) in Dependencies where to search for files
# (searching only in "bin" folder, not "lib")
libraries=("freetype" "jpeg" "lua" "SDL" "SFGUI" "SFML" "TinyXML")



# Check for arguments
saves=0
onlyNew=0
verbose=0
onlyTest=0

for arg in "$@"
do
	if [ $arg == "saves" ]; then
		saves=1
	elif [ $arg == "onlyNew" ]; then
		onlyNew=1
	elif [ $arg == "verbose" ]; then
		verbose=1
	elif [ $arg == "onlyTest" ]; then
		onlyTest=1
		verbose=1
	fi
done



echo "Copying libraries..."

# Copy found files from libraries
[ ! -d "$rootDirectory/$binDirectory" ] && mkdir "$rootDirectory/$binDirectory"
for library in ${libraries[@]}
do
	[ -d "$rootDirectory/$wmrProjectDirectory/$dependenciesDirectory/$library/bin" ] && find "$rootDirectory/$wmrProjectDirectory/$dependenciesDirectory/$library/bin" -type f -print0 | while read -d $'\0' file;
		do
			
			sourceFile=$file
			targetFile=$rootDirectory/$binDirectory/$(basename $file)
			
			if [ $onlyNew -eq 1 ]; then
				if [ -f "$targetFile" ]; then
					if `diff "$sourceFile" "$targetFile" > /dev/null`; then
						continue
					fi
				fi
			fi
			
			if [ $verbose -eq 1 ]; then
				echo "copy \"$sourceFile\" to \"$targetFile\""
			fi
			
			if [ $onlyTest -eq 0 ]; then
				cp "$sourceFile" "$targetFile"
			fi
			
		done
done

echo "Done..."
echo "Copying resources..."

# Create directories found in Resources in bin directory, if needed
find $rootDirectory/$wmrProjectDirectory/$resourcesDirectory/ -type d -print0 | while read -d $'\0' directory;
do
	
	sourceDirectory=$directory
	replace=$rootDirectory/$wmrProjectDirectory/$resourcesDirectory
	replaceWith=$rootDirectory/$binDirectory/$resourcesDirectory
	targetDirectory=${directory//$replace/$replaceWith}

	if [ ! -d "$targetDirectory" ]; then
		
		if [ $verbose -eq 1 ]; then
			echo "Directory: $sourceDirectory"
		fi
		
		mkdir "$targetDirectory"
	fi
	
done

# Files in Resources

find $rootDirectory/$wmrProjectDirectory/$resourcesDirectory/ -type f -print0 | while read -d $'\0' file;
do
	
	sourceFile=$file
	replace=$rootDirectory/$wmrProjectDirectory/$resourcesDirectory
	replaceWith=$rootDirectory/$binDirectory/$resourcesDirectory
	targetFile=${file//$replace/$replaceWith}
	
	if [ $onlyNew -eq 1 ]; then
		if [ -f "$targetFile" ]; then
			if `diff "$sourceFile" "$targetFile" > /dev/null`; then
				continue
			fi
		fi
	fi
	
	if [ $verbose -eq 1 ]; then
		echo "copy \"$sourceFile\" to \"$targetFile\""
	fi

	if [ $onlyTest -eq 0 ]; then
		cp "$sourceFile" "$targetFile"
	fi
done

echo "Done..."

# Saves, if we want them
[ ! -d "$rootDirectory/$binDirectory/$savesDirectory" ] && mkdir "$rootDirectory/$binDirectory/$savesDirectory" # Create Saves directory
if [ $saves -eq 1 ]; then

	echo "Copying saves..."

	find $rootDirectory/$wmrProjectDirectory/$savesDirectory -type f -print0 | while read -d $'\0' file;
	do
		
		sourceFile=$file
		targetFile=$rootDirectory/$binDirectory/$savesDirectory/$(basename $file)
		
		if [ $onlyNew -eq 1 ]; then
			if [ -f "$targetFile" ]; then
				if `diff "$sourceFile" "$targetFile" > /dev/null`; then
					continue
				fi
			fi
		fi
		
		if [ $verbose -eq 1 ]; then
			echo "copy \"$sourceFile\" to \"$targetFile\""
		fi
		
		if [ $onlyTest -eq 0 ]; then
			cp "$sourceFile" "$targetFile"
		fi
	done

	echo "Done..."

fi

echo "Copying licenses..."

copyTo=$rootDirectory/$binDirectory
if [ $onlyTest -eq 0 ]; then
	cp $rootDirectory/$wmrProjectDirectory/licenses.txt $copyTo
fi

echo "Done..."

echo "Finished."