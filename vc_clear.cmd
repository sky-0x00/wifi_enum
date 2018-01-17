@echo off
echo удаление временных файлов vc-проектов...
pause
del /s /f /q *.obj *.sdf *.ipch *.pch *.pdb *.idb *.ilk *.log *.tlog
rd /s /q ipch
pause
