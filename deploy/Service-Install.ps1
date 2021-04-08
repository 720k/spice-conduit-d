
$Manifest = Get-Content $PSScriptRoot\manifest.json | ConvertFrom-Json
New-Service -name $Manifest.ProjectName -Description $Manifest.ProjectDescription -BinaryPathName "$PSScriptRoot\$($Manifest.ProjectName).exe --backend windows" -StartupType Manual
# Get-CimInstance -ClassName Win32_Service -Filter "Name='$($Manifest.Name)'" -Property *
