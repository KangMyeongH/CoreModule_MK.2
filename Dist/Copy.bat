@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

:: 현재 bat 파일이 있는 디렉토리
set "SOURCE_DIR=%~dp0"

:: 제외할 파일 (자기 자신)
set "EXCLUDE_FILE=%~nx0"

:: 복사할 대상 디렉토리
set "TARGET_DIR1=D:\GameEngine_DirectX11\GameEngine\GameEngine\ThirdParty\coremodule"
set "TARGET_DIR2=D:\GameEngine_DirectX11\GameEngine\Client\ThirdParty\coremodule"

:: 대상 디렉토리가 존재하는지 확인하고 없으면 생성
for %%D in ("%TARGET_DIR1%" "%TARGET_DIR2%") do (
    if not exist "%%~D" mkdir "%%~D"
)

:: 파일 복사 실행 (Copy.bat 제외)
for %%F in ("%SOURCE_DIR%*") do (
    if /I not "%%~nxF"=="%EXCLUDE_FILE%" (
        for %%D in ("%TARGET_DIR1%" "%TARGET_DIR2%") do (
            echo 복사 중: %%~nxF → %%~D
            xcopy "%%F" "%%~D\" /Y /Q /I
        )
    )
)

echo 완료!
pause