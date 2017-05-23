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
	- Extract dependencies in destination folder (old dependencies are overwritten)

.PARAMETER dependencies_file
	The manifest file containing the required archives to install.

	Expected format is:
	archive_name;folder_to_unzip;archive_url
	archive_name;folder_to_unzip;archive_url
	...
.PARAMETER dest_dir
	Optional: if unspecified then by default it will be set to $script_root\..\dependencies (i.e. [mensia-test-root]\dependencies])
	Destination directory for extracted archives. Each archive found in the manifest file
	is extracted in 'dest_dir\folder_to_unzip'.

	Note that if no cache is found, archives are also downloaded in 'dest_dir\arch'.
.PARAMETER cache_dir
	Cache directory for extracted archives. Each archive found in the manifest file
	is extracted from cache_dir to 'dest_dir\folder_to_unzip'.

.NOTES
	File Name      : windows-install-dependencies.ps1
	Prerequisite   : Tested with PS v4.0 on windows 10 pro.
.LINK
	Detailed specifications:
	https://jira.mensiatech.com/confluence/pages/viewpage.action?spaceKey=CT&title=Dependency+management
.EXAMPLE
	powershell.exe -NoExit -NoProfile -ExecutionPolicy Bypass -File \absolute\path\to\windows-get-dependencies.ps1 -manifest_file .\windows-dependencies.txt
.EXAMPLE
	powershell.exe -NoExit -NoProfile -ExecutionPolicy Bypass -File \absolute\path\to\windows-get-dependencies.ps1 -manifest_file .\windows-dependencies.txt -dest_dir \absolute\path\to\dep\ -cache_dir \absolute\path\to\cache -proxy_pass user:passwd
#>

#
# script parameters
#

Param(
[parameter(Mandatory=$true)][ValidateScript({Test-Path $_ })][string]$manifest_file,
[parameter(Mandatory=$false)][ValidateScript({Test-Path $_ })][string]$cache_dir,
[parameter(Mandatory=$false)][ValidateNotNullOrEmpty()][string]$dest_dir = ".\..\dependencies",
[parameter(Mandatory=$false)][string]$proxy_pass
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
Write-Host ""

Write-Host "===Parameters==="
Write-Host "Dependencies file = $manifest_file"
Write-Host "Destination directory = $dest_dir"
Write-Host ""

#
# script variables
#

# we keep 2 different variables here because there are 2 different behaviors:
# - cache archives shoud not be overwritten (It is the responsability of the system admin to keep it in sync with remote directories)
# - destination directory archives must be overwritten to ensure a regular update
$Script:arch_dir = ""

# monitoring variables
$Script:download_count = 0
$Script:extract_count = 0

$Script:dependency_server = "UNUSED"

#
# script functions
#

function ExpandZipFile($zip, $dest)
{
	Write-Host "Extract: [" $zip "] -> [" $dest "]"
	$shell = New-Object -ComObject Shell.Application

	# create dest if it does not exists
	if(-Not (Test-Path $dest)) {
		New-Item $dest -itemtype directory | Out-Null
	}

	# expand folders because NameSpace command expects absolute paths
	$zip = (Resolve-Path $zip)
	$dest = (Resolve-Path $dest)

	$folder = $shell.NameSpace("$zip")

	# Progress based on count is pretty dumb. However
	# the sizes retrieved by $folder.GetDetailsOf($item, 2)
	# or $item.Size are unreliable. At least, it allows the script
	# user to see something is going on.
	$count = 0
	$total_count = $folder.Items().Count
	foreach($item in $folder.Items()) {

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

	$Script:extract_count++
}

function InstallDeps($arch, $dir, $dropbox_url)
{
	$zip = $Script:arch_dir + "\" + $arch

	if(-Not (Test-Path $zip)) {
		$WebClient = New-Object System.Net.WebClient
		if($proxy_pass){
			Write-Host "- Credentials are specified. Try to download from server with username [$Username]."
			$url = $Script:dependency_server + "/" + $arch
			$Username, $Password = $proxy_pass.split(':',2)
			$WebClient.Credentials = New-Object System.Net.Networkcredential($Username, $Password)
		}
		elseif($dropbox_url) {
			Write-Host "- Credentials are not specified. Try to download from Dropbox."
			$url = $dropbox_url
		}
		else {
			Write-Host "- Credentials and dropbox link are not specified, can not download dependency."
			return
		}

		Write-Host "Download: [" $url "] -> [" $zip "]."
		$WebClient.DownloadFile( $url, $zip )
		
		if(-Not (Test-Path $zip)) {
			exit
		}
	}

	ExpandZipFile $zip ($Script:dest_dir + "\" + $dir)
}

#
# core script
#

# check if a cache directory is available through DEPENDENCY_CACHE env variable
Write-Host "===Looking for cache==="

# if -cache_dir is specified, use its value, otherwise check if an environment variable exists 
if($cache_dir) {
	Write-Host "Cache directory provided by parameter: $cache_dir"
} elseif ((Test-Path env:\DEPENDENCY_CACHE) -and (Test-Path $env:DEPENDENCY_CACHE)) {
	Write-Host "Found cache directory: "  ($env:DEPENDENCY_CACHE)
	$cache_dir = $env:CV_DEPENDENCY_CACHE
}

if ($cache_dir) {
# check if a cache directory was passed as parameters, and if it exists
	Write-Host "Found cache directory: "  ($cache_dir)
	$arch_dir = $cache_dir
	if(-Not (Test-Path $arch_dir)) {
		New-Item $arch_dir -itemtype directory | Out-Null
		Write-Host "Created archive directory in cache: "  $arch_dir
	}

	Write-Host "Missing archives will be transferred to " $arch_dir

} else {
	Write-Host "Found no cache directory"

	$arch_dir = $dest_dir + "\arch"

	if(-Not (Test-Path $arch_dir)) {
		New-Item $arch_dir -itemtype directory | Out-Null
		Write-Host "Created archive directory in destination: "  $arch_dir
	}

	Write-Host "All archives will be transferred to " $arch_dir
}

Write-Host ""
Write-Host "===Installing dependencies==="

$Script:timer = [System.Diagnostics.Stopwatch]::StartNew()
foreach ($dep in (Get-Content $manifest_file)) {
	$arch, $dir, $dropbox_url = $dep.split(';',3)
	InstallDeps $arch $dir $dropbox_url
}
$timer.Stop()

Write-Host ""
Write-Host "===Install Summary==="
Write-Host "State = Success"
Write-Host "Number of archives downloaded = $download_count"
Write-Host "Number of archives extracted = $extract_count"
Write-Host "Installation time = "  $([string]::Format("{0:d2}h:{1:d2}mn:{2:d2}s", $timer.Elapsed.hours, $timer.Elapsed.minutes, $timer.Elapsed.seconds)) -nonewline
Write-Host ""
