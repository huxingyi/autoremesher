flex lex.l
mv lex.yy.c lex.yy.cpp
bison -d parse.y -r all
mv parse.tab.c parse.tab.cpp
