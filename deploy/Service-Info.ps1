
$Manifest = Get-Content $PSScriptRoot\manifest.json | ConvertFrom-Json
$Service = Get-CimInstance -ClassName Win32_Service -Filter "Name='$($Manifest.ProjectName)'" -Property *
if ($null -ne $Service) {
    $Service
} else {
    Write-Verbose "[NOT FOUND] Service $($Manifest.ProjectName)"
}
