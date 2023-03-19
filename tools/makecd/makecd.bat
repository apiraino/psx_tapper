set MYPATH=C:\psyq\tools\makecd
::cd C:\psyq\tools\mkpsxiso
mkisofs -xa -o %MYPATH%\psxiso.iso %MYPATH%\ISOROOT
%MYPATH%\mkpsxiso.exe %MYPATH%\psxiso.iso %MYPATH%\psxiso.bin %MYPATH%\infoeur.dat
copy %MYPATH%\psxiso.bin C:\psyq\emulators\epsxe190
copy %MYPATH%\psxiso.bin C:\psyq\emulators\nopsx
::cd C:\psyq\epsxe190
::epsxe.exe -nogui -loadbin psxiso.bin
