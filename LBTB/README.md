I. User Guide:

1. Package management is done with conda (not required, but make sure to modify the relevant paths and parameters in the cmake file)
    To install conda:
        wget "http://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh" -O miniconda.sh
        export CONDA_PREFIX=xxx/miniconda3 (installation path)
        bash miniconda.sh -b -p $CONDA_PREFIX
        conda install pkg-config
        conda install -c conda-forge
        cmake ninja bison flex
        cppcheck valgrind doxygen
        xtensor-fftw
        xtensor-blas
        xtensor
        openblas boost
        catch2 benchmark
2. To use glpk:
    Download from the official website http://www.gnu.org/software/glpk/ which includes the document. When compiling with g++ separately, link the glpk library (-lglpk). When using cmake, specify the link of the library in the CMakeLists file.
3. To use leda:
    Download the free version from the official website http://www.algorithmic-solutions.com/. When compiling with g++ separately, use the compilation option (-I$LEDAROOT/incl -L$LEDAROOT -lleda -lX11 -lm). When using cmake, specify the link of the library in the CMakeLists file.
4.  To compile:
    mkdir build
    cmake ..
    make -j8
    ./main ../../rslt/run001diffeq.blif.out 1 100 (the three parameters represent the input file, the number of iterations, and the radius of the ellipse)

II. File Directory:
include header files, src source files