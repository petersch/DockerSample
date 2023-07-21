mkdir "build/win64_vs2019"
cd "build/win64_vs2019"
conan install ../..
cmake ../.. -G "Visual Studio 16 2019" -A "x64"