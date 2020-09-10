$pythonVersion = $Args[0]
$pythonMajor = ($pythonVersion -split '\.')[0]

Write-Host "Installing python version ${pythonVersion}"

if ($pythonMajor -eq "3")
{
    Invoke-WebRequest "https://www.python.org/ftp/python/${pythonVersion}/python-${pythonVersion}-amd64.exe" -OutFile "python-${pythonVersion}-amd64.exe"
    & "./python-${pythonVersion}-amd64.exe" /quiet /l* _python.log TARGETDIR=_python
}
else
{
    Invoke-WebRequest "https://www.python.org/ftp/python/${pythonVersion}/python-${pythonVersion}.amd64.msi" -OutFile "python-${pythonVersion}-amd64.msi"
    msiexec /i "python-${pythonVersion}-amd64.msi" /quiet /l* _python.log TARGETDIR=_python
}

Write-Host "##vso[task.prependpath]_python"
Write-Host "##vso[task.prependpath]_python\Scripts"
