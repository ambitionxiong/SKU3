@echo off
set BASE=C:\My_STM32_Project\UIBUILDWER\first_ui_project\ui_builder
set TARGET=C:\My_STM32_Project\UIBUILDWER\first_ui_project_code\ui_builder

echo Syncing generated files to simulator project...
robocopy "%BASE%" "%TARGET%" /E /XD custom build /NJH /NJS /NP

echo.
echo Done. Remember to:
echo   1. Update custom.c (remove waitmenu_24_create)
echo   2. Update nav.c for new pages if needed
pause
