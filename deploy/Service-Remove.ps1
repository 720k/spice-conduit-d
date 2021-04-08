
$Manifest = Get-Content $PSScriptRoot\manifest.json | ConvertFrom-Json
Remove-Service -Name $Manifest.Name
