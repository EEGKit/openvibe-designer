#! /bin/bash
## Get needed dependencies from cache or remote server and unzip into
## dest folder.
#

DEPENDENCIES="./tests-data.txt"
CACHE_DIR="../dependencies/arch"
if [ ! -z $DEPENDENCY_CACHE ]; then
    CACHE_DIR=$DEPENDENCY_CACHE
fi
OUTPUT_DIR="../dependencies"

function usage () {
    cat <<EOF
 Usage: $0 --manifest dependencies-file --cache cache_folder [--out output-folder]
   [Options]
   --manifest: file listing dependencies
   --cache: dependency cache
   --out: output destination folder [default: ../dependencies]

    Example:
       $0 --manifest dependencies.txt --cache /media/dependencies
EOF
}

while test -n "$1"; do
    case "$1" in
        --help)
            usage
            exit 0
            ;;
        -h)
            usage
            exit 0
            ;;
        --manifest)
            DEPENDENCIES=$2
            shift
            ;;
        --cache)
            CACHE_DIR=$2
            shift
            ;;
        --out)
            OUTPUT_DIR=$2
            shift
            ;;
    esac
    shift
done
if [ -z ${DEPENDENCIES} ] || [ -z ${CACHE_DIR} ]; then
    usage
    exit 1
fi

if [ -z ${DEPENDENCY_SERVER} ]; then
    echo "If your dependencies are not up-to-date, you won't be able to download new files."
else
    echo "Download dependencies from server [$DEPENDENCY_SERVER]."
fi

if [ -z ${PROXYPASS} ]; then
    echo "Credentials were not provided. If your dependencies are not up-to-date, you won't be able to download new files."
else
    user=`echo ${PROXYPASS} | cut -d ':' -f 1`
    pass=`echo ${PROXYPASS} | cut -d ':' -f 2`
    echo "Credentials are provided. Try to download from server with username [$user]."
fi

if [ ! -d ${CACHE_DIR} ]; then
    echo "${CACHE_DIR} does not exist"
    exit 2
fi

function get_dependency() {
    local _dep=$1

    if [ ! -f ${CACHE_DIR}/${_dep} ]; then
        echo "Download zip archive: [${DEPENDENCY_SERVER}/${_dep}]"
        wget --user=$user --password=$pass -nv -P ${CACHE_DIR} ${DEPENDENCY_SERVER}/${_dep}
    fi
}

function install_dependency() {
    local _dep=$1 _dir=$2

    get_dependency ${_dep}

    echo "Unzip ${CACHE_DIR}/${_dep} to ${OUTPUT_DIR}/${_dir}"
    if [ ! -f ${CACHE_DIR}/${_dep} ]; then
        echo "Dependency ${_dep} does not exist in cache ${CACHE_DIR}"
        exit 3
    fi
    mkdir -p ${OUTPUT_DIR}/${_dir}
    unzip -q -o ${CACHE_DIR}/${_dep} -d ${OUTPUT_DIR}/${_dir}
}


mkdir -p ${OUTPUT_DIR}
echo "Get and unzip dependencies. Read dependencies from manifest [$DEPENDENCIES]"
sed 1d ${DEPENDENCIES} | while read d; do
    dep=`echo $d | cut -d ';' -f 1`
    dir=`echo $d | cut -d ';' -f 2`

    install_dependency ${dep} ${dir}
done
