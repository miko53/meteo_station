rm -rf build_debug_coverage
mkdir build_debug_coverage
cd build_debug_coverage
cmake -DCOVERAGE=ON -DUNIT_TEST=ON -DCMAKE_BUILD_TYPE="Debug" ../.
make all
ctest --verbose
lcov --directory . --capture --rc lcov_branch_coverage=1 --output-file coverage.info
genhtml --output-directory coverage_results --demangle-cpp --num-spaces 4 --sort --title "test coverage" --function-coverage --branch-coverage --legend coverage.info
cd ..
firefox build_debug_coverage/coverage_results/index.html
