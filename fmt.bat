clang-format -i --style=file *.cc *.h||exit /b
sort-c *.cc *.h||exit /b
sort-cases *.cc *.h||exit /b
git diff
