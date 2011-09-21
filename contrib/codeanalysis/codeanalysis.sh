
function run_test
{
  echo "****Getting r$1 from SVN"
  svn export --quiet -r$1 http://chaiscript.googlecode.com/svn/trunk chaiscript-r$1

  pushd chaiscript-r$1

  echo "****Getting svn revision log"
  svn propget svn:log --revprop -r$1  http://chaiscript.googlecode.com/svn/trunk > ../r$1-revisionlog.out

  echo "****Editing CMakeLists.txt to allow for build type switching"
  # Clean up CMakeLists.txt so that we can set the build type at configure time
  sed -i -e "s/SET (CMAKE_BUILD_TYPE.*//"  CMakeLists.txt

  echo "****Compiling Debug Build"
  cmake -D CMAKE_BUILD_TYPE=Debug CMakeLists.txt
  /usr/bin/time -p make chaiscript_eval 2> ../r$1-debugbuildtime.out

  echo "****Analyzing Debug Build"
  /usr/bin/time -p ./chaiscript_eval ../profile.chai 2>../r$1-debugprofiletime.out
  ls -s --block-size=1 ./chaiscript_eval > ../r$1-debugbinarysize.out

  make clean

  echo "****Compiling Release Build"
  cmake -D CMAKE_BUILD_TYPE=Release CMakeLists.txt
  /usr/bin/time -p make chaiscript_eval 2> ../r$1-releasebuildtime.out

  echo "****Analyzing Release Build"
  /usr/bin/time -p ./chaiscript_eval ../profile.chai 2>../r$1-releaseprofiletime.out
  ls -s --block-size=1 ./chaiscript_eval > ../r$1-releasebinarysize.out

  echo "****Analyzing Code Size"
  find ./include -name "*.hpp" | xargs wc > ../r$1-codesize.out

  echo "****Building Remaining Modules"
  /usr/bin/time -p make 2> ../r$1-releasemodulesbuildtime.out

  echo "****Running unit tests"
  /usr/bin/time -p ./run_unit_tests.sh 2> ../r$1-releaseunittestruntime.out > ../r$1-unittestsoutput.out

  echo "****Counting number of unit tests"
  find unittests/ -name "*.chai" | wc | awk '{print $1}' > ../r$1-numunittests.out


  echo "****Running multithreaded tests"
  if [ -e src/multithreaded.cpp ]
  then
    # Run multithreaded tests
    echo "****Building multithreaded test"
    pushd src
    g++ multithreaded.cpp -ldl -omultithreaded -I../include -O3
    echo "****Testing 1 thread runtime"
    /usr/bin/time -p ./multithreaded 1 2> ../../r$1-1threadruntime.out
    echo "****Testing 2 thread runtime"
    /usr/bin/time -p ./multithreaded 1 1 2> ../../r$1-2threadruntime.out
    echo "****Testing 4 thread runtime"
    /usr/bin/time -p ./multithreaded 1 1 1 1 2> ../../r$1-4threadruntime.out
    echo "****Testing 8 thread runtime"
    /usr/bin/time -p ./multithreaded 1 1 1 1 1 1 1 1 2> ../../r$1-8threadruntime.out


    echo "****Compiling thread-free version"
    g++ main.cpp -ldl -othreadfree -I../include -O3 -DCHAISCRIPT_NO_THREADS
    echo "****Analyzing thread-free version"
    /usr/bin/time -p ./threadfree ../../profile.chai 2>../../r$1-threadfreeprofiletime.out
    ls -s --block-size=1 ./threadfree > ../../r$1-threadfreebinarysize.out

    popd
  else
    echo "Multithreaded test non-existent"
  fi

  popd
}

for i in `seq $1 $2` 
do
  run_test $i
done
