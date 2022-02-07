FROM ubuntu:20.04

ENV LANG C.UTF-8
ENV LC_ALL C.UTF-8
ENV LC_CTYPE C.UTF-8
ENV DEBIAN_FRONTEND=noninteractive
ENV LD_LIBRARY_PATH=/usr/local/lib

# ==================================================================================================

# Install the required system packages.
RUN apt-get -y update && apt-get install -y apt-transport-https build-essential cmake curl git gdb gnupg libboost-all-dev libfontconfig1-dev libfreetype6-dev libnss3-dev libopenjp2-7-dev libtiff5-dev python3-dev python3-pip valgrind wget

# Create python3 alias (as required by Bazel).
RUN ln -s /usr/bin/python3 /usr/bin/python

# Install the required Python packages.
RUN python -m pip install pip --upgrade
RUN python -m pip install numpy scikit-build

# Install Bazel as described on the offical documentation page of Bazel, see
# https://docs.bazel.build/versions/main/install-ubuntu.html.
RUN curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor > bazel.gpg && \
    mv bazel.gpg /etc/apt/trusted.gpg.d/ && \
    echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | tee /etc/apt/sources.list.d/bazel.list && \
    apt-get update && apt-get install -y bazel-3.1.0 && \
    ln -s /usr/bin/bazel-3.1.0 /usr/bin/bazel && \
    ldconfig

# Install Protocol buffers
RUN apt-get install autoconf automake libtool curl make g++ unzip -y && \
    mkdir /protocol_buffers && \
    cd /protocol_buffers && \
    wget -c https://github.com/protocolbuffers/protobuf/releases/download/v3.9.2/protobuf-all-3.9.2.tar.gz && \
    tar xvf protobuf-all-3.9.2.tar.gz --no-same-owner && cd protobuf-3.9.2/ && \
    ./configure && \
    make && \
    make install && \
    ldconfig

# Install Tensorflow.
ENV PYTHON_BIN_PATH=/usr/bin/python3
ENV PYTHON_LIB_PATH=/usr/lib/python3/dist-packages
# ENV CUDA_TOOLKIT_PATH=/usr/local/cuda-10.1
# ENV CUDNN_INSTALL_PATH=/usr/lib/x86_64-linux-gnu
ENV TF_NEED_GCP=0
# ENV TF_NEED_CUDA=1
# ENV TF_CUDA_VERSION=10.1
# ENV TF_CUDA_COMPUTE_CAPABILITIES=7.5
ENV TF_NEED_HDFS=0
ENV TF_NEED_OPENCL=0
ENV TF_NEED_JEMALLOC=1
ENV TF_ENABLE_XLA=0
ENV TF_NEED_VERBS=0
# ENV TF_CUDA_CLANG=0
# ENV TF_CUDNN_VERSION=7.6
ENV TF_NEED_MKL=0
ENV TF_DOWNLOAD_MKL=0
ENV TF_NEED_AWS=0
ENV TF_NEED_MPI=0
ENV TF_NEED_GDR=0
ENV TF_NEED_S3=0
ENV TF_NEED_OPENCL_SYCL=0
ENV TF_SET_ANDROID_WORKSPACE=0
ENV TF_NEED_COMPUTECPP=0
ENV GCC_HOST_COMPILER_PATH=/usr/bin/gcc
ENV CC_OPT_FLAGS="-march=native"
ENV TF_NEED_KAFKA=0
ENV TF_NEED_TENSORRT=0
RUN git clone https://github.com/tensorflow/tensorflow.git && \
    cd tensorflow && \
    git checkout v2.3.0 && \
    ./configure && \
    bazel build --jobs=8 \
            --config=v2 \
            --copt=-O3 \
            --copt=-m64 \
            --copt=-march=native \
            --config=opt \
            --verbose_failures \
            //tensorflow:tensorflow_cc \
            //tensorflow:install_headers \
            //tensorflow:tensorflow \
            //tensorflow:tensorflow_framework \
            //tensorflow/tools/lib_package:libtensorflow

# Copy and install the shared library files of Tensorflow.
RUN mkdir -p /opt/tensorflow && \
    cp -r /tensorflow/bazel-bin/tensorflow/* /opt/tensorflow/ && \
    ln -s /opt/tensorflow/libtensorflow_cc.so.2.3.0 /opt/tensorflow/libtensorflow_cc.so && \
    ln -s /opt/tensorflow/libtensorflow_cc.so.2.3.0 /opt/tensorflow/libtensorflow_cc.so.2

ENV LIBRARY_PATH=/opt/tensorflow:$LIBRARY_PATH
ENV LD_LIBRARY_PATH=/opt/tensorflow:$LD_LIBRARY_PATH
ENV CPATH=/opt/tensorflow/include:$CPATH

# Install gtest.
RUN wget https://github.com/google/googletest/archive/release-1.10.0.zip
RUN unzip release-1.10.0.zip && \
    cd googletest-release-1.10.0/googletest && \
    mkdir bld && \
    cd bld && \
    cmake .. && \
    make && \
    cp -a -r ../include/gtest /usr/local/include/. && \
    cp -a ./lib/*.a /usr/local/lib
RUN rm -rf release-1.10.0.zip googletest-release-1.10.0

RUN ldconfig

# Install the utf8proc library, needed for merging diacritic marks with their base characters.
WORKDIR /
RUN git clone --depth 1 --branch v2.6.1 https://github.com/JuliaStrings/utf8proc.git
WORKDIR /utf8proc/build
RUN cmake .. && make && make install

# Install poppler.
WORKDIR /
RUN git clone --depth 1 --branch poppler-22.01.0 https://github.com/freedesktop/poppler.git
WORKDIR /poppler/build
RUN cmake -DENABLE_QT5=OFF -DENABLE_QT6=OFF -DENABLE_LIBPNG=OFF .. && make && make install

COPY src src
COPY Makefile Makefile
RUN make compile POPPLER_DIR=/poppler LIB_DIR=lib

ENTRYPOINT ["./src/PdfToTextPlusPlusMain"]

# ==================================================================================================
# USEFUL COMMANDS

# BUILDING DOCKER IMAGE:
# docker build -t pdftotext-plus-plus .

# EXTRACTING TEXT FROM A SINGLE PDF:
# docker run --rm -v <path-to-pdf>:/file.pdf pdftotext-plus-plus /file.pdf -