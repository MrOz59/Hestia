#!/usr/bin/env bash

set -euo pipefail

readonly SCRIPT_NAME="${0##*/}"
readonly SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
readonly PROJECT_ROOT="$(cd -- "${SCRIPT_DIR}/.." && pwd)"
readonly HESTIA_BINARY="${HESTIA_H2_BINARY:-${PROJECT_ROOT}/app/moonlight}"

usage() {
  cat <<EOF
Usage: ${SCRIPT_NAME} PROFILE OUTPUT_DIR [-- HESTIA_ARGUMENTS...]

Starts the locally built Hestia with terminal frame tracing enabled and writes
one process log to OUTPUT_DIR/hestia-PROFILE.log. Use one process/log for each
clean, lan, wifi, wan, burst-loss, bufferbloat, or reordering sample.
EOF
}

die() {
  printf '%s: %s\n' "${SCRIPT_NAME}" "$*" >&2
  exit 1
}

main() {
  if [[ "${1:-}" == "--help" || "${1:-}" == "-h" ]]; then
    usage
    return
  fi
  (($# >= 2)) || {
    usage >&2
    exit 2
  }

  local profile="$1"
  local output_dir="$2"
  shift 2
  if [[ "${1:-}" == "--" ]]; then
    shift
  fi

  case "${profile}" in
    setup | clean | lan | wifi | wan | burst-loss | bufferbloat | reordering)
      ;;
    *)
      die "unknown test profile '${profile}'"
      ;;
  esac

  [[ -x "${HESTIA_BINARY}" ]] ||
    die "Hestia test binary is missing: ${HESTIA_BINARY}"
  if pgrep -x moonlight >/dev/null 2>&1; then
    die "another Hestia/Moonlight process is active"
  fi

  mkdir -p -- "${output_dir}"
  local log_path="${output_dir}/hestia-${profile}.log"
  [[ ! -e "${log_path}" ]] ||
    die "refusing to overwrite ${log_path}"

  printf 'Hestia binary: %s\n' "${HESTIA_BINARY}"
  printf 'Trace log: %s\n' "${log_path}"
  printf 'Close Hestia normally after the sample completes.\n'

  HESTIA_FRAME_TRACE=1 \
    "${HESTIA_BINARY}" "$@" 2>&1 |
    tee -- "${log_path}"
}

main "$@"
