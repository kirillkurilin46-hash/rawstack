@echo off
chcp 65001 >nul
echo === Rawstack: сборка VST3 ===
where cmake >nul 2>nul || (echo Не найден CMake. Установите Visual Studio 2022 с компонентом "Разработка классических приложений на C++". & pause & exit /b 1)
cmake -B build -G "Visual Studio 17 2022" -A x64 || (pause & exit /b 1)
cmake --build build --config Release --parallel || (pause & exit /b 1)
echo.
echo Готово! Плагин лежит здесь:
echo   %cd%\build\Rawstack_artefacts\Release\VST3\Rawstack.vst3
echo Скопируйте папку Rawstack.vst3 в C:\Program Files\Common Files\VST3\
pause
