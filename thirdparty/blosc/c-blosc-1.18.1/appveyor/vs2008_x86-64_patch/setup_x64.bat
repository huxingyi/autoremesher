regedit /s %~dp0VC_OBJECTS_PLATFORM_INFO.reg

regedit /s %~dp0600dd186-2429-11d7-8bf6-00b0d03daa06.reg
regedit /s %~dp0600dd187-2429-11d7-8bf6-00b0d03daa06.reg
regedit /s %~dp0600dd188-2429-11d7-8bf6-00b0d03daa06.reg
regedit /s %~dp0600dd189-2429-11d7-8bf6-00b0d03daa06.reg
regedit /s %~dp0656d875f-2429-11d7-8bf6-00b0d03daa06.reg
regedit /s %~dp0656d8760-2429-11d7-8bf6-00b0d03daa06.reg
regedit /s %~dp0656d8763-2429-11d7-8bf6-00b0d03daa06.reg
regedit /s %~dp0656d8766-2429-11d7-8bf6-00b0d03daa06.reg

copy "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcpackages\AMD64.VCPlatform.config" "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcpackages\AMD64.VCPlatform.Express.config"
copy "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcpackages\Itanium.VCPlatform.config" "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcpackages\Itanium.VCPlatform.Express.config"
copy "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\bin\vcvars64.bat" "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\bin\amd64\vcvarsamd64.bat"
