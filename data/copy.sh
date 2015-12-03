#!/bin/sh

MCODE_PWD_DATA=`pwd`
MCODE_HOME_DIR="${HOME}/.mcode/"
MCODE_HOME_DB="${MCODE_HOME_DIR}/store.db"
MCODE_SOURCE_DB="${MCODE_PWD_DATA}/store.db"

mkdir -p "${MCODE_HOME_DIR}"
if [ ! -e "${MCODE_HOME_DB}" ]; then
  echo "Copy '${MCODE_SOURCE_DB}' to '${MCODE_HOME_DB}'"
  cp ${MCODE_SOURCE_DB} ${MCODE_HOME_DB}
  echo "Done."
fi
