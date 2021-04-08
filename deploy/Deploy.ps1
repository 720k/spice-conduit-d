param(
    [string]$ProjectName,
    [string]$ProjectPath,
    [string]$ProjectBuildPath,
    [string]$QtBinPath,
    [string]$TargetPath,
    [string]$ProjectBuildType
)

function FixSlash([string]$path) {
    return $path.Replace("/","\");
}
Function Get-PrintableParameterValue($Type, $Value) {
    if ( $Type.Equals([String]) )       { return "`"$Value`"" }
    if ( $Type.Equals([Int32]) )        { return $Value }
    if ( $Type.Equals([Switch]) )       { return $Value }
    return $Value
 }
 function Write-Paramaters() {
    $ParametersInfo=@{}
    $AllParameters.Values | ForEach-Object { 
        $ParametersInfo[$_.Name] = @{ Type = $_.ParameterType; Help = $_.ParameterSets.__AllParameterSets.HelpMessage } 
    }
    $ParamList=@()
    $AllParameters.Keys | ForEach-Object {
        if ($CmdletParamters.Contains($_))     { return }   # skip [cmdlet()] appended variables
        $val = (Get-Variable -Name $_ -EA SilentlyContinue).Value
        $IsSet = $Paramters.Keys -contains $_.ToString()
        $ParamList += @{Name = $_; Value =$val; IsSet = $IsSet; Type=$ParametersInfo[$_].Type; Description = $ParametersInfo[$_].Help}
    }
    $ParamList | ForEach-Object {
        $Value = Get-PrintableParameterValue -Type $_.Type -Value $_.Value
        Write-Output "-$($_.Name) = $Value" 
    }
 }

$CmdletParamters=@("Verbose","Debug","ErrorVariable","WarningVariable","InformationVariable","OutVariable","OutBuffer","PipelineVariable","ErrorAction","WarningAction","InformationAction")
$ScriptName = $MyInvocation.MyCommand.Name
$AllParameters = $MyInvocation.MyCommand.Parameters
$Paramters = $PSBoundParameters

$ProjectPath = FixSlash $ProjectPath
$FileNameOfManifest ="manifest.pri"
$FileNameOfProjectBinary ="$ProjectName.exe"
$DeployPath = "$TargetPath\$ProjectName"

Write-Output "*** $ScriptName"
Write-Paramaters
# copy 3rd party
$null = Copy-Item -Recurse "$ProjectPath\3rdparty\QtService\msvc2019_64\plugins" -Destination $DeployPath -Force -Confirm:$false
# copy Service script 
$null = Copy-Item  "$ProjectPath\Deploy\Service*.ps1" -Destination $DeployPath -Force -Confirm:$false
$global:ProjectLibs = "$ProjectPath\3rdparty\QtService\msvc2019_64\bin"