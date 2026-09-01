set currentDir=%cd%
set "pathToUnreal=Binaries\Win64\"

copy /y "..\thirdparty\build\geographiclib\vs2022-x64\bin\Release\GeographicLib.dll" "%pathToUnreal%"
copy /y "..\thirdparty\build\poco\vs2022-x64\bin\PocoNet.dll" "%pathToUnreal%"
copy /y "..\thirdparty\build\poco\vs2022-x64\bin\PocoFoundation.dll" "%pathToUnreal%"
