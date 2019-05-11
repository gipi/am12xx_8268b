#!/bin/bash
# Program:
#     Generate json file
#
# History:
#     2017/4/6    Yen
#     2018/1/19   Chris
#


DIR_ARG1=$1
STRINGS_ARG2=$2
ZIP_FILENAME=./$1.zip
HTML_FOLDER=apptheme
CURRENT_PATH=`pwd`
OUTPUT_FILE=${CURRENT_PATH}/apptheme_${DIR_ARG1}_${STRINGS_ARG2}.json
echo "Input arg=${DIR_ARG1}"
echo "current path=${CURRENT_PATH}"

### decompress zip file first
### check file 
if [ ! -e ${ZIP_FILENAME} ]; then
    printRed "=> ${ZIP_FILENAME} not exist"
    exit 1
fi
if [ ! -e ${DIR_ARG1} ]; then
    unzip -o $ZIP_FILENAME
fi

### function
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

generateMainBackground()
{
    if [ ! "$(ls -A ${MAIN_BACKGROUND_PATH})" ]; then
        echo "${MAIN_BACKGROUND_PATH} is empty"
        return 0
    fi
    echo "    \"${MAIN_BACKGROUND_STR}\":{" >> ${OUTPUT_FILE}

    ###### image
    echo "        \"image\":{" >> ${OUTPUT_FILE}

    cd ${MAIN_BACKGROUND_PATH}
    OBJ_NUM=`ls . | wc -l`
    COUNT=0
    #echo "main background ${OBJ_NUM}"
    for ENTRY in "."/*
    do
        COUNT=$(($COUNT+1))

        echo "=>${ENTRY}"
        if [ ! -d ${ENTRY} ]; then
            printPurple "${ENTRY} not a directory"
            continue
        fi
        
        OBJECT_NAME=`echo "${ENTRY}" | sed 's/^..//'`
        
        cd ${ENTRY}
        PNG=`find -name *.png`
        PNG_NAME=`echo "${PNG}" | sed 's/^..//'`

        OUTPUT_STR="            \"${OBJECT_NAME}\":\"${HTML_FOLDER}/${DIR_ARG1}/${MAIN_BACKGROUND_STR}/${OBJECT_NAME}/${PNG_NAME}\""
        if [ "${COUNT}" == "${OBJ_NUM}" ]; then
            echo "${OUTPUT_STR}" >> ${OUTPUT_FILE}
        else
            echo "${OUTPUT_STR}," >> ${OUTPUT_FILE}
        fi

        cd ..
    done

    ###### color
    COLOR_FILE=color.properties
    COLOR_HEX="0"
    COLOR_HEX=`[ -e ${COLOR_FILE} ] && cat ${COLOR_FILE}`

    echo "${MAIN_BACKGROUND_STR}=>${COLOR_HEX/=/:}"

    if [ ${#COLOR_HEX} == "0" ]; then
        printYellow "=> ${MAIN_BACKGROUND_STR} <= No color hex"
        echo "        }" >> ${OUTPUT_FILE}
    else
        echo "        }," >> ${OUTPUT_FILE}
        echo "        ${COLOR_HEX/=/:}" >> ${OUTPUT_FILE}
    fi

    printf "    }" >> ${OUTPUT_FILE}
}

generateLeftBackground()
{
    if [ ! "$(ls -A ${LEFT_BACKGROUND_PATH})" ]; then
        echo "${LEFT_BACKGROUND_PATH} is empty"
        return 0
    fi
    echo "    \"${LEFT_BACKGROUND_STR}\":{" >> ${OUTPUT_FILE}

    ###### image
    echo "        \"image\":{" >> ${OUTPUT_FILE}

    cd ${LEFT_BACKGROUND_PATH}
    OBJ_NUM=`ls . | wc -l`
    COUNT=0
    #echo "left background ${OBJ_NUM}"
    for ENTRY in "."/*
    do
        COUNT=$(($COUNT+1))

        echo "${LEFT_BACKGROUND_STR} => ${ENTRY}"
        if [ ! -d ${ENTRY} ]; then
            printPurple "${ENTRY} not a directory"
            continue
        fi
        
        OBJECT_NAME=`echo "${ENTRY}" | sed 's/^..//'`
        
        cd ${ENTRY}
        PNG=`find -name *.png`
        PNG_NAME=`echo "${PNG}" | sed 's/^..//'`
        
        OUTPUT_STR="            \"${OBJECT_NAME}\":\"${HTML_FOLDER}/${DIR_ARG1}/${LEFT_BACKGROUND_STR}/${OBJECT_NAME}/${PNG_NAME}\""
        if [ "${COUNT}" == "${OBJ_NUM}" ]; then
            echo "${OUTPUT_STR}" >> ${OUTPUT_FILE}
        else
            echo "${OUTPUT_STR}," >> ${OUTPUT_FILE}
        fi
        
        cd ..
    done

    ###### color
    COLOR_FILE=color.properties
    COLOR_HEX="0"
    COLOR_HEX=`[ -e ${COLOR_FILE} ] && cat ${COLOR_FILE} | grep -w color`

    ###### color_second
    COLOR_SECOND_HEX="0"
    COLOR_SECOND_HEX=`[ -e ${COLOR_FILE} ] && cat ${COLOR_FILE} | grep -w color_second`


    echo "${LEFT_BACKGROUND_STR} color hex =>${COLOR_HEX/=/:}"
    if [ ${#COLOR_HEX} == "0" ]; then
        printYellow "=> ${LEFT_BACKGROUND_STR} <= No color hex"
        echo "        }" >> ${OUTPUT_FILE}
    else
        echo "        }," >> ${OUTPUT_FILE}
        if [ ${#COLOR_SECOND_HEX} == "0" ]; then
            echo "        ${COLOR_HEX/=/:}" >> ${OUTPUT_FILE}
        else
            echo "        ${COLOR_HEX/=/:}," >> ${OUTPUT_FILE}
        fi
    fi

    echo "${LEFT_BACKGROUND_STR} color second hex =>${COLOR_SECOND_HEX/=/:}"
    if [ ${#COLOR_SECOND_HEX} == "0" ]; then
        printYellow "=> ${LEFT_BACKGROUND_STR} <= No color second hex"
    else
        echo "        ${COLOR_SECOND_HEX/=/:}" >> ${OUTPUT_FILE}
    fi

    printf "    }" >> ${OUTPUT_FILE}
}

generateLeftTopRectImage()
{
    if [ ! "$(ls -A ${LEFT_TOP_RECT_IMAGE_PATH})" ]; then
        echo "${LEFT_TOP_RECT_IMAGE_PATH} is empty"
        return 0
    fi
    echo "    \"${LEFT_TOP_RECT_IMAGE_STR}\":{" >> ${OUTPUT_FILE}

    ###### image
    echo "        \"image\":{" >> ${OUTPUT_FILE}

    cd ${LEFT_TOP_RECT_IMAGE_PATH}
    OBJ_NUM=`ls . | wc -l`
    COUNT=0
    #echo "left background ${OBJ_NUM}"
    for ENTRY in "."/*
    do
        COUNT=$(($COUNT+1))

        echo "${LEFT_TOP_RECT_IMAGE_STR} => ${ENTRY}"
        if [ ! -d ${ENTRY} ]; then
            printPurple "${ENTRY} not a directory"
            continue
        fi
        
        OBJECT_NAME=`echo "${ENTRY}" | sed 's/^..//'`
        
        cd ${ENTRY}
        PNG=`find -name *.png`
        PNG_NAME=`echo "${PNG}" | sed 's/^..//'`
        
        OUTPUT_STR="            \"${OBJECT_NAME}\":\"${HTML_FOLDER}/${DIR_ARG1}/${LEFT_TOP_RECT_IMAGE_STR}/${OBJECT_NAME}/${PNG_NAME}\""
        if [ "${COUNT}" == "${OBJ_NUM}" ]; then
            echo "${OUTPUT_STR}" >> ${OUTPUT_FILE}
        else
            echo "${OUTPUT_STR}," >> ${OUTPUT_FILE}
        fi
           
        cd ..
    done
    
    ###### color
    COLOR_FILE=color.properties
    COLOR_HEX="0"
    COLOR_HEX=`[ -e ${COLOR_FILE} ] && cat ${COLOR_FILE} | grep -w color`
    ###### color_second
    COLOR_SECOND_HEX="0"
    COLOR_SECOND_HEX=`[ -e ${COLOR_FILE} ] && cat ${COLOR_FILE} | grep -w color_second`

    echo "${LEFT_BACKGROUND_STR} color hex =>${COLOR_HEX/=/:}"
    if [ ${#COLOR_HEX} == "0" ]; then
        printYellow "=> ${LEFT_BACKGROUND_STR} <= No color hex"
        echo "        }" >> ${OUTPUT_FILE}
    else
        echo "        }," >> ${OUTPUT_FILE}
        if [ ${#COLOR_SECOND_HEX} == "0" ]; then
            echo "        ${COLOR_HEX/=/:}" >> ${OUTPUT_FILE}
        else
            echo "        ${COLOR_HEX/=/:}," >> ${OUTPUT_FILE}
        fi
    fi

    echo "${LEFT_BACKGROUND_STR} color second hex =>${COLOR_SECOND_HEX/=/:}"
    if [ ${#COLOR_SECOND_HEX} == "0" ]; then
        printYellow "=> ${LEFT_BACKGROUND_STR} <= No color second hex"
    else
        echo "        ${COLOR_SECOND_HEX/=/:}" >> ${OUTPUT_FILE}
    fi
    printf "    }" >> ${OUTPUT_FILE}
}

generateDiscoveryIcon()
{
    if [ ! "$(ls -A ${DISCOVERY_ICON_PATH})" ]; then
        echo "${DISCOVERY_ICON_PATH} is empty"
        return 0
    fi
    echo "    \"${DISCOVERY_ICON_STR}\":{" >> ${OUTPUT_FILE}

    cd ${DISCOVERY_ICON_PATH}
    OBJ_NUM_1=`ls . | wc -l`
    COUNT_1=0
    for ENTRY_1 in "."/*
    do
        COUNT_1=$(($COUNT_1+1))

        OBJECT_NAME_1=`echo "${ENTRY_1}" | sed 's/^..//'`
        echo "        \"${OBJECT_NAME_1}\":{" >> ${OUTPUT_FILE}
        
        cd ${ENTRY_1}
        OBJ_NUM_2=`ls . | wc -l`
        COUNT_2=0
        echo "${ENTRY_1} => ${OBJ_NUM_2}"
        for ENTRY_2 in "."/*
        do
            COUNT_2=$(($COUNT_2+1))

            OBJECT_NAME_2=`echo "${ENTRY_2}" | sed 's/^..//'`

            cd ${ENTRY_2}
            PNG=`find -name *.png`
            PNG_NAME=`echo "${PNG}" | sed 's/^..//'`
            
            OUTPUT_STR="            \"${OBJECT_NAME_2}\":\"${HTML_FOLDER}/${DIR_ARG1}/${DISCOVERY_ICON_STR}/${OBJECT_NAME_1}/${OBJECT_NAME_2}/${PNG_NAME}\""
            if [ "${COUNT_2}" == "${OBJ_NUM_2}" ]; then
                echo "${OUTPUT_STR}" >> ${OUTPUT_FILE}
            else
                echo "${OUTPUT_STR}," >> ${OUTPUT_FILE}
            fi

            cd ..
        done

        if [ "${COUNT_1}" == "${OBJ_NUM_1}" ]; then
            echo "        }" >> ${OUTPUT_FILE}
        else
            echo "        }," >> ${OUTPUT_FILE}
        fi
        
        cd ..
    done

    printf "    }" >> ${OUTPUT_FILE}
    cd ..
}

generateAppFunctions()
{
    if [ ! "$(ls -A ${APP_FUNCTIONS_PATH})" ]; then
        echo "${APP_FUNCTIONS_PATH} is empty"
        return 0
    fi
    echo "    \"${APP_FUNCTIONS_STR}\":{" >> ${OUTPUT_FILE}

    cd ${APP_FUNCTIONS_PATH}
    OBJ_NUM=`ls . | wc -l`
    COUNT=0
    echo "app functions ${OBJ_NUM}"
    for ENTRY in "."/*
    do
        COUNT=$(($COUNT+1))

        #echo "YenTest app function dir======>${ENTRY}"
        if [ ! -d ${ENTRY} ]; then
            printPurple "${ENTRY} not a directory"
            continue
        fi
        
        OBJECT_NAME=`echo "${ENTRY}" | sed 's/^..//'`
        
        echo "        \"${OBJECT_NAME}\":{" >> ${OUTPUT_FILE}
        
        cd ${ENTRY}
        PNG=`find -name *.png`
        PNG_NAME=`echo "${PNG}" | sed 's/^..//'`
        
        #echo "            \"image\":\"${HTML_FOLDER}/${DIR_ARG1}/${APP_FUNCTIONS_STR}/${OBJECT_NAME}/${PNG_NAME}\"," >> ${OUTPUT_FILE}
        
        ###### name
        STRINGS_FILE="../../localizedStrings/${STRINGS_ARG2}.strings"
        STRINGS_NAME="0"
        STRINGS_NAME=`[ -e ${STRINGS_FILE} ] && cat ${STRINGS_FILE} | grep -w ${OBJECT_NAME} | grep =`
        STRINGS_NAME_OUTPUT="${STRINGS_NAME/${OBJECT_NAME}/name}"
        
        #echo "app function strings ${OBJECT_NAME} =>${STRINGS_NAME} ------ ${#STRINGS_NAME}"
        #echo "${STRINGS_NAME_OUTPUT}"
        
        HAS_STRINGS_NAME=0
        if [ ${#STRINGS_NAME} == "0" ]; then
            printYellow "=> ${OBJECT_NAME} <= No custom name"
        else
            HAS_STRINGS_NAME=1
            #echo "            ${STRINGS_NAME_OUTPUT/=/:}," >> ${OUTPUT_FILE}
        fi

        ###### color
        COLOR_FILE="../color.properties"
        COLOR_HEX="0"
        COLOR_HEX=`[ -e ${COLOR_FILE} ] && cat ${COLOR_FILE} | grep -w ${OBJECT_NAME} | awk '{print $3}'`
        #echo "app function color =>${COLOR_HEX/=/:}"
        #echo "app function color =>${COLOR_HEX}"
        
        HAS_COLOR_HEX=0
        if [ ${#COLOR_HEX} == "0" ]; then
            printYellow "=> ${OBJECT_NAME} <= No color hex"
        else
            HAS_COLOR_HEX=1
            #echo "            \"color\":${COLOR_HEX}" >> ${OUTPUT_FILE}
        fi

        ###### hoverColor
        HOVER_COLOR_FILE="../hoverColor.properties"
        HOVER_COLOR_HEX="0"
        HOVER_COLOR_HEX=`[ -e ${HOVER_COLOR_FILE} ] && cat ${HOVER_COLOR_FILE} | grep -w ${OBJECT_NAME} | awk '{print $3}'`
        #echo "app function hover color =>${HOVER_COLOR_HEX/=/:}"
        #echo "app function hover color =>${HOVER_COLOR_HEX}"
        
        HAS_HOVER_COLOR_HEX=0
        if [ ${#HOVER_COLOR_HEX} == "0" ]; then
            printYellow "=> ${OBJECT_NAME} <= No hoverColor hex"
        else
            HAS_HOVER_COLOR_HEX=1
            #echo "            \"color\":${COLOR_HEX}" >> ${OUTPUT_FILE}
        fi

        
        if [ ${HAS_STRINGS_NAME} == "1" ]; then
            echo "            ${STRINGS_NAME_OUTPUT/=/:}," >> ${OUTPUT_FILE}
        fi

        if [ ${HAS_COLOR_HEX} == "1" ]; then
            echo "            \"color\":${COLOR_HEX}," >> ${OUTPUT_FILE}
        fi

        if [ ${HAS_HOVER_COLOR_HEX} == "1" ]; then
            echo "            \"hoverColor\":${HOVER_COLOR_HEX}," >> ${OUTPUT_FILE}
        fi
    
        echo "            \"image\":\"${HTML_FOLDER}/${DIR_ARG1}/${APP_FUNCTIONS_STR}/${OBJECT_NAME}/${PNG_NAME}\"" >> ${OUTPUT_FILE}
    

        if [ "${COUNT}" == "${OBJ_NUM}" ]; then
            echo "        }" >> ${OUTPUT_FILE}
        else
            echo "        }," >> ${OUTPUT_FILE}
        fi
        
        cd ..
    done

    printf "    }" >> ${OUTPUT_FILE}
}

### check folder 
if [ ! -d ${DIR_ARG1} ]; then
    printRed "=> ${DIR_ARG1} not exist"
    exit 1
fi

### start generate json file
for ENTRY in "${DIR_ARG1}"/*
do
    echo "${ENTRY}"
done

TOTAL_DIRS=`ls -l ${DIR_ARG1} | grep "^d" | wc -l`
PROCESSED_DIRS=0
if [ -d ${CURRENT_PATH}/${DIR_ARG1}/localizedStrings ]; then
    TOTAL_DIRS=$(($TOTAL_DIRS-1))
fi

### (0)
echo "{" > ${OUTPUT_FILE}

### (0) md5sum
###    "md5":"md5 checksum", //驗證檔案版本
###
FILE_MD5SUM=`md5sum ${DIR_ARG1}.zip | awk '{print $1}'`
echo ${FILE_MD5SUM}
echo "    \"md5\":\"${FILE_MD5SUM}\"," >> ${OUTPUT_FILE}

### (1) - START
###
###    "main_background":{
###      "image":{
###        "default":"none",
###        "landscape":"iOS/main_background/landscape/main_bg_landscape.png",
###        "portrait":"iOS/main_background/portrait/main_bg_portrait.png"
###      },
###      "color" : "color hex" // ??????
###    },
###
MAIN_BACKGROUND_STR=main_background
MAIN_BACKGROUND_PATH=${CURRENT_PATH}/${DIR_ARG1}/${MAIN_BACKGROUND_STR}

if [ -d ${MAIN_BACKGROUND_PATH} ]; then
    PROCESSED_DIRS=$(($PROCESSED_DIRS+1))
    generateMainBackground
    if [ "${PROCESSED_DIRS}" == "${TOTAL_DIRS}" ]; then
        echo "" >> ${OUTPUT_FILE}
    else
        echo "," >> ${OUTPUT_FILE}
    fi
else
    printRed "=> ${MAIN_BACKGROUND_PATH} not exist"
fi

### (1) - END

### (2) - START
###
###    "left_background":{
###      "image":{
###        "default":"iOS/left_background/default/left_bg.png"
###      },
###      "color" : "color hex", // ??????
###      "color_second" : "color hex" //需要漸層色時使用 ??????
###    },
###
LEFT_BACKGROUND_STR=left_background
LEFT_BACKGROUND_PATH=${CURRENT_PATH}/${DIR_ARG1}/${LEFT_BACKGROUND_STR}

if [ -d ${LEFT_BACKGROUND_PATH} ]; then
    PROCESSED_DIRS=$(($PROCESSED_DIRS+1))
    generateLeftBackground
    if [ "${PROCESSED_DIRS}" == "${TOTAL_DIRS}" ]; then
        echo "" >> ${OUTPUT_FILE}
    else
        echo "," >> ${OUTPUT_FILE}
    fi
else
    printRed "=> ${LEFT_BACKGROUND_PATH} not exist"
fi


### (2) - END

### (3) - START
###
###     "left_top_rect_image" : { //20171107 新增 、20171113修改（desktop平台專用）
###      "image" : {
###        "default":"image url"
###      }
###    },
###
LEFT_TOP_RECT_IMAGE_STR=left_top_rect_image
LEFT_TOP_RECT_IMAGE_PATH=${CURRENT_PATH}/${DIR_ARG1}/${LEFT_TOP_RECT_IMAGE_STR}

if [ $DIR_ARG1 == "windows" ] || [ $DIR_ARG1 == "chrome" ] || [ $DIR_ARG1 == "macOS" ]; then
    if [ -d ${LEFT_TOP_RECT_IMAGE_PATH} ]; then
        PROCESSED_DIRS=$(($PROCESSED_DIRS+1))
        generateLeftTopRectImage
        if [ "${PROCESSED_DIRS}" == "${TOTAL_DIRS}" ]; then
            echo "" >> ${OUTPUT_FILE}
        else
            echo "," >> ${OUTPUT_FILE}
        fi
    else
        printRed "=> ${LEFT_TOP_RECT_IMAGE_PATH} not exist"
    fi
fi
### (3) - END

### (4) - START
###
###   "discovery_icon":{  //參考Remote Control功能定義
###        "ezcast":{  //ezcast產品，對應 family == ezcast 或是 family == NULL
###            "car": "iOS/discovery_icon/ezcast/car/car.png",
###            "lite": "iOS/discovery_icon/ezcast/lite/lite.png",
###            "music": "iOS/discovery_icon/ezcast/music/music.png",
###            "wire": "iOS/discovery_icon/ezcast/wire/wire.png",
###            "ezcast" : "iOS/discovery_icon/ezcast/ezcast/ezcast.png", // type == ezcast  or 其他ezcast ssid
###            "screen" : "iOS/discovery_icon/ezcast/screen/screen.png"
###        },
###
###        "ezcastpro":{   //ezcastpro產品，對應 family == ezcastpro, 此分類下有四種type
###            "box":"iOS/discovery_icon/ezcastpro/box/box.png",
###            "projector" : "iOS/discovery_icon/ezcastpro/projector/projector.png",
###            "tv" : "iOS/discovery_icon/ezcastpro/tv/tv.png",
###            "dongle" : "iOS/discovery_icon/ezcastpro/dongle/dongle.png",
###            "ezcastpro" : "iOS/discovery_icon/ezcastpro/ezcastpro/ezcastpro.png" // type == ezcastpro  or 其他ezcast ssid
###        }
###
###    },
###
DISCOVERY_ICON_STR=discovery_icon
DISCOVERY_ICON_PATH=${CURRENT_PATH}/${DIR_ARG1}/${DISCOVERY_ICON_STR}

if [ -d ${DISCOVERY_ICON_PATH} ]; then
    PROCESSED_DIRS=$(($PROCESSED_DIRS+1))
    generateDiscoveryIcon
    if [ "${PROCESSED_DIRS}" == "${TOTAL_DIRS}" ]; then
        echo "" >> ${OUTPUT_FILE}
    else
        echo "," >> ${OUTPUT_FILE}
    fi
else
    printRed "=> ${DISCOVERY_ICON_PATH} not exist"
fi

### (4) - END

### (5) - START
###     "app_functions": { //參考Remote Control功能定義
###        "ezchannel":{   //key ezchannel
###            "image":"iOS/app_functions/ezchannel/ezchannel.png",
###            "name":"custom name",//由小機傳回多國語言字串
###            "color" :"color hex" //6碼
###        },
###
###        "3g4g":{
###            "image":"iOS/app_functions/3g4g/3g4g.png",
###            "name":"custom name",
###            "color" :"color hex"
###        },
###        ...
###        ...
###        ...
###        ...
###        "web":{
###            "image":"iOS/app_functions/web/web.png",
###            "name":"custom name",
###            "color" :"color hex"
###        }
###
###        //視小機需要更新
###    }
###
APP_FUNCTIONS_STR=app_functions
APP_FUNCTIONS_PATH=${CURRENT_PATH}/${DIR_ARG1}/${APP_FUNCTIONS_STR}

if [ -d ${APP_FUNCTIONS_STR} ]; then
    PROCESSED_DIRS=$(($PROCESSED_DIRS+1))
    generateAppFunctions
    if [ "${PROCESSED_DIRS}" == "${TOTAL_DIRS}" ]; then
        echo "" >> ${OUTPUT_FILE}
    else
        echo "," >> ${OUTPUT_FILE}
    fi
else
    printRed "=> ${APP_FUNCTIONS_STR} not exist"
fi

### (5) - END

### end
echo "}" >> ${OUTPUT_FILE}

printGreen "====== apptheme_${DIR_ARG1}_${STRINGS_ARG2}.json file generate : Done ======"
