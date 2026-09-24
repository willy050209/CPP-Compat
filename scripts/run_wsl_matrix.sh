#!/usr/bin/env bash
set -e

cd /mnt/d/program/C++/CPP-Compat

COMPILERS=("g++" "clang++")
STANDARDS=("11" "14" "17" "20" "23")

declare -A RESULTS

for COMPILER in "${COMPILERS[@]}"; do
    for STD in "${STANDARDS[@]}"; do
        NAME="${COMPILER}_cxx${STD}"
        echo "========================================"
        echo " WSL Testing: ${COMPILER} C++${STD}"
        echo "========================================"
        
        BDIR="build_wsl_${NAME}"
        rm -rf "${BDIR}"
        
        if cmake -B "${BDIR}" -DCMAKE_CXX_COMPILER="${COMPILER}" -DCMAKE_CXX_STANDARD="${STD}" -DCOMPAT_FORCE_SELF_IMPLEMENTATION=1 -DCOMPAT_BUILD_BENCHMARKS=OFF > /dev/null 2>&1; then
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

echo ""
echo "========================================"
echo " WSL Test Matrix Summary:"
echo "========================================"
for KEY in "${!RESULTS[@]}"; do
    echo "  ${KEY}: ${RESULTS[${KEY}]}"
done
