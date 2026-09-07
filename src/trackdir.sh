#!/bin/sh

devino() {
    stat -c%d:%i -- "${1}" 2>/dev/null
}

err() {
    echo "ERROR:" "$(basename "${0}"):" "${@}"
    exit 1
}

[ ${#} -eq 2 ] \
    || err "usage: <dir-to-track> <artifact-dir> env: HIDDEN"

[ -d "${1}" ] && [ -d "${2}" ] \
    || err "arguments must be valid directories"

[ "$(devino "${1}")" != "$(devino "${2}")" ] \
    || err "<dir-to-track> can not be <artifact-dir>"

DIR=${1} IDX=${2}/${HIDDEN+.}${DIR}.idx TMP=${IDX}.tmp

[ ! -e "${TMP}" ] \
    || err "file '${TMP}' exists, refusing to overwrite"

idx=$(find "${DIR}" -type f -exec stat -c%Y:%n {} + 2>/dev/null) \
    || err "failed to construct temporary index from '${DIR}'"

( printf "%s" "${idx}" | sort > "${TMP}" ) 2>/dev/null \
    || err "failed to write temporary index to '${TMP}'"

cmp -s "${TMP}" "${IDX}" || cp "${TMP}" "${IDX}" 2>/dev/null \
    || err "failed to move temporary index in '${TMP}' into '${IDX}'"

rm "${TMP}" 2>/dev/null \
    || err "failed to remove '${TMP}'"

echo "${IDX}"
