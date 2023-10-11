clang-format -i --style=file *.cc *.h||exit /b
sort-c -i *.cc *.h||exit /b
sort-cases -i *.cc *.h||exit /b
git diff
