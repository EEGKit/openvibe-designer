<#
.SYNOPSIS
	This script aims at installing dependencies from a manifest file.
.DESCRIPTION
	Script workflow is as follows:
	- Check for a cache directory
	- Parse input dependencies manifest
	- Download dependencies if: (Currently does not download!)
		* cache found but dependency not found in the cache
		* cache not found (old archives are overwritten)
	- Extract dependencies in destination folder (old dependencies are overwritten) if:
		* dependencies were never unzipped
		* the version of unzipped dependency is correct (read from manifest version file)
	- write manifest version file, named after the manifest file name and suffixed with "-version", 
	it contains the version of lastly unzipped dependencies such as:
		folder_to_unzip=version_number

.PARAMETER dependencies_file
	The manifest file containing the required archives to install.

	First line is the base URL of the dependencies, following lines are formatted as:
	archive_name;folder_to_unzip;version_number
	archive_name;folder_to_unzip;version_number
	...
.PARAMETER dest_dir
	Optional: if unspecified then by default it will be set to $script_root\..\dependencies
	Destination directory for extracted archives. Each archive found in the manifest file
	is extracted in 'dest_dir\folder_to_unzip'.

	Note that if no cache is found, archives are also downloaded in 'dest_dir\arch'.
.PARAMETER cache_dir
	Cache directory for extracted archives. Each archive found in the manifest file
	is extracted from cache_dir to 'dest_dir\folder_to_unzip'.

.NOTES
	File Name      : windows-get-dependencies.ps1
	Prerequisite   : Tested with PS v4.0 on windows 10 pro.
	
	Environment variables will be used if set:
		* PROXYPASS: Used to set credentials, should be formed as user:passwd
		* ZIP_EXECUTABLE: should be set to the path to 7zip executable. If unset, script will try to read registry keys.
.LINK
	Detailed specifications:
	https://jira.mensiatech.com/confluence/pages/viewpage.action?spaceKey=CT&title=Dependency+management
.EXAMPLE
	powershell.exe -NoExit -NoProfile -ExecutionPolicy Bypass -File \absolute\path\to\windows-get-dependencies.ps1 -manifest_file .\windows-dependencies.txt
.EXAMPLE
	powershell.exe -NoExit -NoProfile -ExecutionPolicy Bypass -File \absolute\path\to\windows-get-dependencies.ps1 -manifest_file .\windows-dependencies.txt -dest_dir \absolute\path\to\dep\ -cache_dir \absolute\path\to\cache
#>

#
# script parameters
#
Param(
[parameter(Mandatory=$true)][ValidateScript({Test-Path $_ })][string]$manifest_file,
[parameter(Mandatory=$false)][string]$cache_dir,
[parameter(Mandatory=$false)][ValidateNotNullOrEmpty()][string]$dest_dir = ".\..\dependencies"
)

$manifest_file = [System.IO.Path]::GetFullPath($manifest_file)
$dest_dir = [System.IO.Path]::GetFullPath($dest_dir)

#
# input validation
#
Write-Host "===Input validation==="
if (Test-Path $dest_dir){
	Write-Host "Destination directory found"
} else {
	Write-Host "Destination directory not found"
	New-Item $dest_dir -itemtype directory | Out-Null
	Write-Host "Created destination directory: "  $dest_dir
}

# if -cache_dir is specified, use its value, otherwise check if an environment variable exists 
if($cache_dir) {
	Write-Host "Cache directory provided by parameter: $cache_dir"
} elseif ($env:DEPENDENCY_CACHE -and (Test-Path $env:DEPENDENCY_CACHE)) {
	Write-Host "Found cache directory: " ($env:DEPENDENCY_CACHE)
	$cache_dir = $env:DEPENDENCY_CACHE
} else {
	$cache_dir = $dest_dir + "\arch"
	Write-Host "Found no cache directory. Set it to default value: $cache_dir"
}
if(-Not (Test-Path $cache_dir)) {
	New-Item $cache_dir -itemtype directory | Out-Null
	Write-Host "Created archive directory in destination: "  $cache_dir
}
Write-Host "All archives will be downloaded in " $cache_dir
Write-Host ""

Write-Host "Configure Web client"

# Base URL of dependency server is read in environment variable
$Script:dependency_server = $env:URL

$WebClient = New-Object System.Net.WebClient
if($env:PROXYPASS){
	$Username, $Password = $env:PROXYPASS.split(':',2)
	Write-Host "Credentials are provided. Try to download from server with username [$Username]."
	$WebClient.Credentials = New-Object System.Net.Networkcredential($Username, $Password)
} else {
	Write-Host "Credentials were not provided. If your dependencies are not up-to-date, you won't be able to download new files."
}
Write-Host ""

if ($env:ZIP_EXECUTABLE) {
	$Script:7zip_executable=$env:ZIP_EXECUTABLE
} else {
	Try {
		$Script:7zip_executable=(Get-ItemProperty HKLM:\Software\7-Zip).Path + "\7z.exe"
	} Catch {
		Write-Host "7-Zip was not found in register keys. Install it will fasten unzip."
	}
}
if ($Script:7zip_executable -and (Test-Path $Script:7zip_executable)) {
	Write-Host "Found 7-Zip in registry keys. It will be used to unzip archives."
} else {
	Write-Host "7-Zip was not found in register keys. Install it will quicken the unzipping."
	$Script:7zip_executable=""
}

Write-Host "===Parameters==="
Write-Host "Dependencies file = $manifest_file"
Write-Host "Destination directory = $dest_dir"
Write-Host ""

#
# script variables
#

# monitoring variables
$Script:extract_count = 0
$Script:download_count = 0
$Script:manifest_file_name = (Get-Item $manifest_file).Basename

Try { 
	$Script:old_dep_versions = import-csv $dest_dir\$Script:manifest_file_name-versions.txt -Delimiter '=' -Header Dependency,Version
} Catch {
	Write-Host "Found no manifest version file."
}
$Script:dep_versions = ""

#
# script functions
#

function ExpandZipFile($zip, $dest)
{
	Write-Host "Extract: [" $zip "] -> [" $dest "]"
	$shell = New-Object -ComObject Shell.Application
	
	if ([string]::IsNullOrEmpty($Script:7zip_executable)){
		# create dest if it does not exists
		if(-Not (Test-Path $dest)) {
			New-Item $dest -itemtype directory | Out-Null
		}

		$folder = $shell.NameSpace("$zip")

		# Progress based on count is pretty dumb. However
		# the sizes retrieved by $folder.GetDetailsOf($item, 2)
		# or $item.Size are unreliable. At least, it allows the script
		# user to see something is going on.
		$count = 0
		$total_count = $folder.Items().Count
		ForEach($item in $folder.Items()) {

			$item_name = $folder.GetDetailsOf($item, 0)
			$percent_complete = [System.Math]::Floor(100 * $count / $total_count)
			Write-Progress `
				-Status "Progress: $percent_complete%" `
				-Activity ("Extracting " + (Split-Path -Leaf $zip) + " to " + (Split-Path -Leaf $dest)) `
				-CurrentOperation "Copying $item_name (this can take a long time...)" `
				-PercentComplete $percent_complete

			# 0x14 = sum of options 4 (donnot display windows box) and 16 (answer yes to all)
			$shell.Namespace("$dest").CopyHere($item, 0x14)

			$count++
		}
	} else {
		$7z_Arguments = @(
			'x'            ## extract files with full paths
			'-y'           ## assume Yes on all queries
			$zip           ## archive path
			"`"-o$dest`""  ## dest path        

		)
		& $7zip_executable $7z_Arguments
	}
	$Script:extract_count++

}

function InstallDeps($arch, $dir, $version)
{
	$zip = $Script:cache_dir + "\" + $arch

	if(-Not (Test-Path $zip)) {
		if($Username -and $Script:dependency_server){
			$url = $Script:dependency_server + "/" + $arch
		} else {
			Write-Host "- Credentials or $Script:dependency_server are not specified, can not download dependency. "
			Write-Host "- They have to be set through environment variables: PROXYPASS and URL."
			exit
		}

		Write-Host "Download: [" $url "] -> [" $zip "]."
		$Script:WebClient.DownloadFile( $url, $zip )
		$Script:download_count++
	}
	if(-Not (Test-Path $zip)) {
		exit
	}

	$old_dep_version=$($Script:old_dep_versions | Where-Object  {$_.dependency -eq $dir}).version
	if((Test-Path ($Script:dest_dir + "\" + $dir)) -and ($old_dep_version -eq $version)) {
		Write-Host "No need to unzip dependency. Already at the good version: " $old_dep_version "."
	} else {
		Write-Host "Dependency version is not the good one: [" $old_dep_version "]. Should be " $version
		ExpandZipFile $zip ($Script:dest_dir + "\" + $dir)
	}
	$Script:dep_versions += $dir + "=" + $version + "`n"
}

#
# core script
#

Write-Host ""
Write-Host "===Installing dependencies==="

$Script:timer = [System.Diagnostics.Stopwatch]::StartNew()
foreach ($dep in Get-Content $manifest_file) {
	$arch, $dir, $version = $dep.split(';',3)
	InstallDeps $arch $dir $version
}
$timer.Stop()

$Script:dep_versions > $dest_dir\$Script:manifest_file_name-versions.txt

Write-Host ""
Write-Host "===Install Summary==="
Write-Host "State = Success"
Write-Host "Number of archives downloaded = $Script:download_count"
Write-Host "Number of archives extracted = $Script:extract_count"
Write-Host "Installation time = "  $([string]::Format("{0:d2}h:{1:d2}mn:{2:d2}s", $timer.Elapsed.hours, $timer.Elapsed.minutes, $timer.Elapsed.seconds)) -nonewline
Write-Host ""
