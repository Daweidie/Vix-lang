#! /bin/bash
# file="main.ll"


set -e
echo "rm build and runtime bin files..."
cd src
../build/vixc main.vix -ll
mv main.ll ../seed/
cd ..
cd seed
rm vixc.ll
mv main.ll vixc.ll 
cd ..
rm -rf build runtime
cd ..
ls bootstrap/
sleep 5
git add .
git commit -m "$1"
git push

