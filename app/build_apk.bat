@echo off
set "JAVA_HOME=C:\Program Files\Android\Android Studio\jbr"
set "PATH=%JAVA_HOME%\bin;%PATH%"
cd /d C:\Users\3C5C~1\SomnaCore\app
call C:\Users\3C5C~1\SomnaCore\app\gradlew.bat assembleDebug
