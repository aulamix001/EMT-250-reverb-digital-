@echo off
if exist "%TEMP%\EMT250-ui.log" (
    notepad "%TEMP%\EMT250-ui.log"
) else (
    echo Abra primeiro o plugin EMT-250 Reverb 0.4.1 no REAPER.
    echo O diagnostico sera criado em %%TEMP%%\EMT250-ui.log.
    pause
)
