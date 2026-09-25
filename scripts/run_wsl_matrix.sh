#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
cd "${ROOT_DIR}"

COMPILERS=("g++" "clang++")
STANDARDS=("11" "14" "17" "20" "23")
MODES=("default" "fallback")

declare -A RESULTS

for COMPILER in "${COMPILERS[@]}"; do
    for STD in "${STANDARDS[@]}"; do
        for MODE in "${MODES[@]}"; do
            NAME="${COMPILER}_cxx${STD}_${MODE}"
            echo "========================================"
            echo " WSL Testing: ${COMPILER} C++${STD} (${MODE})"
            echo "========================================"
            
            BDIR="build_wsl_${NAME}"
            rm -rf "${BDIR}"
            
            EXTRA_FLAGS=""
            if [ "${MODE}" = "fallback" ]; then
                EXTRA_FLAGS="-DCOMPAT_FORCE_FALLBACK=ON -DCOMPAT_FORCE_SELF_IMPLEMENTATION=1"
            fi
            
            if cmake -B "${BDIR}" \
                     -DCMAKE_CXX_COMPILER="${COMPILER}" \
                     -DCMAKE_CXX_STANDARD="${STD}" \
                     -DCOMPAT_BUILD_BENCHMARKS=OFF \
                     ${EXTRA_FLAGS} > /dev/null 2>&1; then
                if cmake --build "${BDIR}" -j4 > /dev/null 2>&1; then
                    if "${BDIR}/compat_test" > /dev/null 2>&1; then
                        RESULTS["${NAME}"]="PASSED"
                        echo "--> ${NAME}: PASSED"
                    else
                        RESULTS["${NAME}"]="TEST FAILED"
                        echo "--> ${NAME}: TEST FAILED"
                    fi
                else
                    RESULTS["${NAME}"]="BUILD FAILED"
                    echo "--> ${NAME}: BUILD FAILED"
                fi
            else
                RESULTS["${NAME}"]="CONFIG FAILED"
                echo "--> ${NAME}: CONFIG FAILED"
            fi
            rm -rf "${BDIR}"
        done
    done
done

echo ""
echo "========================================"
echo " WSL Test Matrix Summary:"
echo "========================================"
for KEY in $(echo "${!RESULTS[@]}" | tr ' ' '\n' | sort); do
    echo "  ${KEY}: ${RESULTS[${KEY}]}"
done
