<#
.SYNOPSIS
	This script aims at installing dependencies from a manifest file.
.DESCRIPTION
	Script workflow is as follows:
	- Check for a cache directory
	- Parse input dependencies manifest
	- Download dependencies if:
		* cache found but dependency not found in the cache
		* cache not found (old archives are overwritten)
	- Extract dependencies in destination folder (old dependencies are overwritten)

	Search for the cache directory is made through CV_DEPENDENCY_CACHE environment
	variable. If you maintain such a cache to synchronize local data with remote ones,
	please set CV_DEPENDENCY_CACHE.

	Expected hierarchy of the cache directory is:
	- '\dependencies' directory used to store project dependencies (binaries etc.)
	- '\test' used to store test data
.PARAMETER dependencies_file
	The manifest file containing the required archives to install.

	Expected format is:
	archive_name;folder_to_unzip;archive_url
	archive_name;folder_to_unzip;archive_url
	...
.PARAMETER data_type
	The type of data to install. This parameter is mainly used to look at the right directory
	in the cache.

	Expected value: 'dependencies' or 'test'.
.PARAMETER dest_dir
	Destination directory for extracted archives. Each archive found in the manifest file
	is extracted in 'dest_dir\folder_to_unzip'.

	Note that if no cache is found, archives are also downloaded in 'dest_dir\arch'.
.NOTES
	File Name      : windows-install-dependencies.ps1
	Prerequisite   : Tested with PS v4.0 on windows 8.1 pro.
.LINK
	Detailed specifications:
	https://jira.mensiatech.com/confluence/pages/viewpage.action?spaceKey=CT&title=Dependency+management
.EXAMPLE
	.\windows-install-dependencies.ps1 -dependencies_file \absolute\path\to\dep\dependencies.txt -data_type dependencies -dest_dir \absolute\path\to\dep\
.EXAMPLE
	powershell.exe -NoExit -NoProfile -ExecutionPolicy Bypass -File \absolute\path\to\windows-install-dependencies.ps1 -dependencies_file \absolute\path\to\dep\dependencies.txt -data_type dependencies -dest_dir \absolute\path\to\dep\
#>

#
# script parameters
#

Param(
[parameter(Mandatory=$true)][ValidateNotNullOrEmpty()][string]$dependencies_file,
[parameter(Mandatory=$true)][ValidateSet('test','dependencies', ignorecase=$False)][string]$data_type,
[parameter(Mandatory=$true)][ValidateNotNullOrEmpty()][string]$dest_dir
)

#
# header used to handle permission restrictions
#

If (!([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]"Administrator"))
{
	# manually forward the parameters
	# generic way of forwarding with parsing of $myinvocation.Line failed
	# because $myinvocation.Line is empty when the script is called from command line with powershell.exe
	$argumentList = (" -dependencies_file", "`"$dependencies_file`"")
	$argumentList += (" -data_type", "`"$data_type`"")
	$argumentList += (" -dest_dir", "`"$dest_dir`"")

	$command = $myinvocation.MyCommand.Definition + $argumentList
	Start-Process powershell.exe "-NoExit -NoProfile -ExecutionPolicy Bypass -File $command" -Verb RunAs

	Exit
}

#
# input validation
#

Write-Host "===Input validation==="
If (Test-Path $dependencies_file){
	Write-Host "Dependencies file found"
} Else {
	Throw New-Object System.IO.FileNotFoundException "$dependencies_file not found"
}

If (Test-Path $dest_dir){
	Write-Host "Destination directory found"

} Else {
	Write-Host "Destination directory not found"
	New-Item $dest_dir -itemtype directory | Out-Null
	Write-Host "Created destination directory: "  $dest_dir
}
Write-Host ""

Write-Host "===Parameters==="
Write-Host "Dependencies file = $dependencies_file"
Write-Host "Destination directory = $dest_dir"
Write-Host "Data type = $data_type"
Write-Host ""

#
# script variables
#

# we keep 2 different variables here because there are 2 different behaviors:
# - cache archives shoud not be overwritten (It is the responsability of the system admin to keep it in sync with remote directories)
# - destination directory archives must be overwritten to ensure a regular update
$Script:cache_dir = ""
$Script:arch_dir = ""

# monitoring variables
$Script:download_count = 0
$Script:extract_count = 0

#
# script functions
#

# taken and modified from:
# https://blogs.msdn.microsoft.com/jasonn/2008/06/13/downloading-files-from-the-internet-in-powershell-with-progress/
function DownloadDeps($url, $dest)
{
	Write-Host "Download: [" $url "] -> [" $dest "]"

	Try {
		$uri = New-Object System.Uri $url
		$request = [System.Net.HttpWebRequest]::Create($uri)
		$request.Timeout = 15000 #15 second timeout
		$response = $request.GetResponse()
	} Catch {
		 $error = $_.Exception.Message
		 Write-Host "Failed to download resources: "  $error
		 return $false
	}

	If($request.HaveResponse) {
		# retrieve total size of data to download
		$total_len = $response.ContentLength

		If($total_len -eq -1) {
			Write-Host "Failed to download resources: content length = -1"
			return $false
		}

		$response_stream = $response.GetResponseStream()
		$target_stream = New-Object -TypeName System.IO.FileStream -ArgumentList $dest, Create

		$buffer = new-object byte[] 10KB
		$bytes_read = 0

		Do {
			#transfer data from response stream to target stream
			$count = $response_stream.Read($buffer, 0, $buffer.length)
			$target_stream.Write($buffer, 0, $count)
			$bytes_read += $count

			$percent_complete = [System.Math]::Floor(100 * $bytes_read / $total_len)

			Write-Progress -Status "Progress: $percent_complete%" -Activity ("Downloading " + (Split-Path -Leaf $dest)) -PercentComplete $percent_complete

		} While ($count -gt 0)

		$target_stream.Flush()
		$target_stream.Close()
		$target_stream.Dispose()
		$response_stream.Close()
		$response_stream.Dispose()

		$Script:download_count++

	} Else {
		Write-Host "Failed to retrieve response from the server"
		return $false
	}

	return $true
}

function ExpandZipFile($zip, $dest)
{
	Write-Host "Extract: [" $zip "] -> [" $dest "]"
	$shell = New-Object -ComObject Shell.Application

	# create dest if it does not exists
	If(-Not (Test-Path $dest)) {
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

	$Script:extract_count++
}

function InstallDeps($arch, $dir, $url)
{
	$zip = $Script:arch_dir + "\" + $arch

	$do_extract = $true

	If(($Script:cache_dir -and -Not (Test-Path $zip)) -or (-Not $Script:cache_dir)) {
		$do_extract = DownloadDeps $url $zip
	}

	if($do_extract) {
		ExpandZipFile $zip ($Script:dest_dir + "\" + $dir)
	}
}

#
# core script
#

# check if a cache directory is available through CV_DEPENDENCY_CACHE env variable
Write-Host "===Looking for cache==="
If ((Test-Path env:\CV_DEPENDENCY_CACHE) -and (Test-Path $env:CV_DEPENDENCY_CACHE)) {

	Write-Host "Found cache directory: "  ($env:CV_DEPENDENCY_CACHE)
	$cache_dir = $env:CV_DEPENDENCY_CACHE + "\" + $data_type
	$arch_dir = $cache_dir

	If(-Not (Test-Path $cache_dir)) {
		New-Item $cache_dir -itemtype directory | Out-Null
		Write-Host "Created archive directory in cache: "  $cache_dir
	}

	Write-Host "Missing archives will be transferred to " $arch_dir

} else {
	Write-Host "Found no cache directory"

	$arch_dir = $dest_dir + "\arch"

	If(-Not (Test-Path $arch_dir)) {
		New-Item $arch_dir -itemtype directory | Out-Null
		Write-Host "Created archive directory in destination: "  $arch_dir
	}

	Write-Host "All archives will be transferred to " $arch_dir
}

Write-Host ""
Write-Host "===Installing dependencies==="

$Script:timer = [System.Diagnostics.Stopwatch]::StartNew()
ForEach ($dep in (Get-Content $dependencies_file)) {
	$arch, $dir, $url = $dep.split(';',3)
	InstallDeps $arch $dir $url
}
$timer.Stop()

Write-Host ""
Write-Host "===Install Summary==="
Write-Host "State = Success"
Write-Host "Number of archives downloaded = $download_count"
Write-Host "Number of archives extracted = $extract_count"
Write-Host "Installation time = "  $([string]::Format("{0:d2}h:{1:d2}mn:{2:d2}s", $timer.Elapsed.hours, $timer.Elapsed.minutes, $timer.Elapsed.seconds)) -nonewline
Write-Host ""
