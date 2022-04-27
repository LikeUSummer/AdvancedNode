clang++ test.cpp -I../../node/include/ -L../../node/lib/x64/ -lnode -shared -o out/test.node

Remove-Item out/*.exp
Remove-Item out/*.lib

../../node/env/node.exe test.js
