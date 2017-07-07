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

if [ ! -d ${CACHE_DIR} ]; then
    echo "${CACHE_DIR} does not exists"
    exit 2
fi

function install_dependency() {
    local _dep=$1 _dir=$2
    echo "Unzip ${_dep} to ${_dir}"
    if [ ! -f ${_dep} ]; then
        echo "Dependency ${_dep} does not exist in cache ${CACHE_DIR}"
        exit 3
    fi
	mkdir -p ${_dir}
    unzip -o ${_dep} -d ${_dir}
}


mkdir -p ${OUTPUT_DIR}
echo "Get and unzip dependencies"
cat ${DEPENDENCIES} | while read d; do
    dep=`echo $d | cut -d ';' -f 1`
    rep=`echo $d | cut -d ';' -f 2`

    install_dependency ${CACHE_DIR}/${dep} ${OUTPUT_DIR}/${rep}
done
