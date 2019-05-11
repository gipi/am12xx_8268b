#!/bin/bash

###### define array
platform=(windows android iOS chrome macOS)
language=(cht en)

###### function
printRed()
{
    printf "\E[0;31;40m"
    echo ${1}
    printf "\E[0m"
}

printYellow()
{
    printf "\E[0;33;40m"
    echo ${1}
    printf "\E[0m"
}

printPurple()
{
    printf "\E[0;35;40m"
    echo ${1}
    printf "\E[0m"
}

printGreen()
{
    printf "\E[0;32;40m"
    echo ${1}
    printf "\E[0m"
}

###### main
OUTPUT_FOLDER=apptheme_pack

if [ ! -d ${OUTPUT_FOLDER} ]; then
    printRed "=> ${OUTPUT_FOLDER} not exist"
    mkdir ${OUTPUT_FOLDER}
else
    printYellow "remove & make a new one"
    rm -rf ${OUTPUT_FOLDER}
    mkdir ${OUTPUT_FOLDER}
fi

pack_json_function()
{
    PLATFORM_TYPE=$1
    LANGUAGE_TYPE=$2
    
    sh makeAppthemeJsonFile.sh ${PLATFORM_TYPE} ${LANGUAGE_TYPE}
    MV_FILE=apptheme_${PLATFORM_TYPE}_${LANGUAGE_TYPE}.json
    mv ${MV_FILE} ./${OUTPUT_FOLDER}
}

pack_data_function()
{
    PLATFORM_TYPE=$1
    
    cp -r ${PLATFORM_TYPE} ./${OUTPUT_FOLDER}
}

##### generate json and data for each platfom/language
for i in ${platform[@]}; do
        for j in ${language[@]}; do
#####           (1) generate json file & mv it to output folder
                pack_json_function ${i} ${j}
        done
######  (2) mv data folder to ouput folder
        pack_data_function ${i}
done


