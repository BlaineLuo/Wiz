
DEL /F /Q /S *.aps *.ncb *.user

for /r %1 %%R in (Debug) do if exist "%%R" (rd /s /q "%%R")
for /r %1 %%R in (Release) do if exist "%%R" (rd /s /q "%%R")
for /r %1 %%R in (ReleaseCE) do if exist "%%R" (rd /s /q "%%R")
for /r %1 %%R in (ReleaseXP) do if exist "%%R" (rd /s /q "%%R")
