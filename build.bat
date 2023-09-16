set cc=cl -EHsc -MDd -W2 -nologo -std:c++20
%cc% sort-c.cc setargv.obj
%cc% sort-cases.cc setargv.obj
