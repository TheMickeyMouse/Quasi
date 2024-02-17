set platform=%1
set config=%2

set outpath=..\out\%config%\%platform%\out

for %%D in (
    "%outpath%\Game\"
    "%outpath%\Testing\"
) do xcopy /y /d "..\bin\%platform%\%config%\*.dll" "%%D"