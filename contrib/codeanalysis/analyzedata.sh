
function analyze_data
{
  echo -n "$1," >> $2
  awk 'END {printf "%s,", $1}' r$1-codesize.out >> $2
  awk '{printf "%s,", $1}' r$1-numunittests.out >> $2
  awk 'END {printf "%s,", $1}' r$1-unittestsoutput.out >> $2

  awk '{printf "%s,", $1}' r$1-debugbinarysize.out  >> $2
  awk 'NR>1{exit} 1 {printf "%s,", $2}' r$1-debugbuildtime.out >> $2
  awk 'NR>1{exit} 1 {printf "%s,", $2}' r$1-debugprofiletime.out >> $2

  awk '{printf "%s,", $1}' r$1-releasebinarysize.out  >> $2
  awk 'NR>1{exit} 1 {printf "%s,", $2}' r$1-releasebuildtime.out >> $2
  awk 'NR>1{exit} 1 {printf "%s,", $2}' r$1-releasemodulesbuildtime.out >> $2
  awk 'NR>1{exit} 1 {printf "%s,", $2}' r$1-releaseprofiletime.out >> $2
  awk 'NR>1{exit} 1 {printf "%s,", $2}' r$1-releaseunittestruntime.out >> $2

  if [ -x r$1-threadfreebinarysize.out ]
  then
    awk '{printf "%s,", $1}' r$1-threadfreebinarysize.out  >> $2
    awk 'NR>1{exit} 1 {printf "%s,", $2}' r$1-threadfreeprofiletime.out >> $2

    awk 'NR>1{exit} 1 {printf "%s,", $2}' r$1-1threadruntime.out >> $2
    awk 'NR>1{exit} 1 {printf "%s,", $2}' r$1-2threadruntime.out >> $2
    awk 'NR>1{exit} 1 {printf "%s,", $2}' r$1-4threadruntime.out >> $2
    awk 'NR>1{exit} 1 {printf "%s,", $2}' r$1-8threadruntime.out >> $2
  else
    echo -n ",,,,,," >> $2
  fi

  awk -F "\n" 'NR>1{exit} 1 {printf "\"%s\"\n", $1}' r$1-revisionlog.out >> $2
}


filename=output-$1-$2.csv

echo "Revision, LOC, Num Unit Tests, Successful Unit Tests, Debug Binary Size, Debug Build Time, Debug Profile Time, Release Binary Size, Release Core Build Time, Release Modules Build Time, Release Profile Time, Release Unit Tests Time, Threadless Binary Size, Threadless Profile Time, 1 Thread Profile Time, 2 Threads Profile Time, 4 Threads Profile Time, 8 Threads Profile Time, Revision Log" > $filename

for i in `seq $1 $2` 
do
  analyze_data $i $filename
done
