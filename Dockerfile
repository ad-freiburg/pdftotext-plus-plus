FROM ubuntu:20.04 as base

LABEL maintainer="Claudius Korzen <korzen@cs.uni-freiburg.de>"

ENV LANG C.UTF-8
ENV LC_ALL C.UTF-8
ENV LC_CTYPE C.UTF-8
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y curl git g++ cmake make wget

# ==================================================================================================
# Build Tensorflow.
#
# NOTE: Tensorflow provides precompiled C packages (see https://www.tensorflow.org/install/lang_c),
# but the C++ packages are required. Precompiled C++ packages are *not* provided officially, so we
# have to build them from source.

FROM base as tensorflow-builder

# Install protocol buffers (needed by Tensorflow for (de-)serializing the models).
ARG PB_VERSION=3.9.2
WORKDIR /protocol_buffers
RUN apt-get install -y autoconf automake libtool unzip
RUN wget -qc https://github.com/protocolbuffers/protobuf/releases/download/v${PB_VERSION}/protobuf-all-${PB_VERSION}.tar.gz
RUN tar xf protobuf-all-${PB_VERSION}.tar.gz --no-same-owner
WORKDIR /protocol_buffers/protobuf-${PB_VERSION}
RUN ./configure
RUN make
RUN make install
WORKDIR /opt/include
RUN cp -r /usr/local/include/google .
WORKDIR /opt/lib
RUN cp -r /usr/local/lib/libprotobuf.so.20.0.2 .
RUN ln -s /usr/local/lib/libprotobuf.so.20.0.2 libprotobuf.so.20
RUN ln -s /usr/local/lib/libprotobuf.so.20.0.2 libprotobuf.so
RUN ldconfig

# Install bazel (a build system required to build Tensorflow from source), as described on the
# official Bazel website, see https://docs.bazel.build/versions/main/install-ubuntu.html.
ARG BAZEL_VERSION=3.1.0
RUN apt-get install -y gnupg
RUN curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor > bazel.gpg
RUN mv bazel.gpg /etc/apt/trusted.gpg.d/
RUN echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | tee /etc/apt/sources.list.d/bazel.list
RUN apt-get update && apt-get install -y bazel-${BAZEL_VERSION}
RUN ln -s /usr/bin/bazel-${BAZEL_VERSION} /usr/bin/bazel
RUN ldconfig

# Install Tensorflow.
ARG TF_VERSION=2.3.0
RUN apt-get install -y python3-dev python3-pip
RUN ln -s /usr/bin/python3 /usr/bin/python
RUN pip3 install numpy scikit-build
WORKDIR /tensorflow
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
RUN git config --global advice.detachedHead false
RUN git clone --depth 1 --branch v${TF_VERSION} https://github.com/tensorflow/tensorflow.git .
RUN ./configure
RUN bazel build \
    --jobs=8 \
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
WORKDIR /opt/include
RUN cp -r /tensorflow/bazel-bin/tensorflow/include/* .
WORKDIR /opt/lib
RUN cp -r /tensorflow/bazel-bin/tensorflow/libtensorflow.so.${TF_VERSION} .
RUN cp -r /tensorflow/bazel-bin/tensorflow/libtensorflow_cc.so.${TF_VERSION} .
RUN cp -r /tensorflow/bazel-bin/tensorflow/libtensorflow_framework.so.${TF_VERSION} .
RUN ln -s libtensorflow_cc.so.${TF_VERSION} libtensorflow_cc.so
RUN ln -s libtensorflow_cc.so.${TF_VERSION} libtensorflow_cc.so.2
RUN ln -s libtensorflow_framework.so.${TF_VERSION} libtensorflow_framework.so
RUN ln -s libtensorflow_framework.so.${TF_VERSION} libtensorflow_framework.so.2

# ==================================================================================================
# Build utf8proc (required for merging diacritic marks with their base characters).

FROM base as utf8proc-builder

ARG U8P_VERSION=2.6.1
RUN apt-get install -y cmake
WORKDIR /utf8proc
RUN git config --global advice.detachedHead false
RUN git clone --depth 1 --branch v${U8P_VERSION} https://github.com/JuliaStrings/utf8proc.git .
WORKDIR /utf8proc/build
RUN cmake ..
RUN make
RUN make install
WORKDIR /opt/include
RUN cp -r /usr/local/include/utf8proc.h .
WORKDIR /opt/lib
RUN cp -r /usr/local/lib/libutf8proc.a .

# ==================================================================================================
# Build poppler.

FROM base as poppler-builder

ARG POP_VERSION=22.01.0
RUN apt-get install -y libboost-all-dev libfontconfig1-dev libfreetype6-dev libnss3-dev libopenjp2-7-dev libtiff5-dev
WORKDIR /poppler
RUN git config --global advice.detachedHead false
RUN git clone --depth 1 --branch poppler-${POP_VERSION} https://github.com/freedesktop/poppler.git .
WORKDIR /poppler/build
RUN cmake -DENABLE_QT5=OFF -DENABLE_QT6=OFF -DENABLE_LIBPNG=OFF ..
RUN make
RUN make install
WORKDIR /opt/include
RUN cp -r /poppler/* /poppler/build/* /poppler/build/poppler/* .
WORKDIR /opt/lib
RUN cp /usr/local/lib/libpoppler-cpp.so.0.9.0 .
RUN ln -s libpoppler-cpp.so.0.9.0 libpoppler-cpp.so.0
RUN ln -s libpoppler-cpp.so.0 libpoppler-cpp.so
RUN cp /usr/local/lib/libpoppler.so.117.0.0 .
RUN ln -s libpoppler.so.117.0.0 libpoppler.so.117
RUN ln -s libpoppler.so.117 libpoppler.so
RUN cp /usr/lib/x86_64-linux-gnu/libfreetype.so.6 .
RUN cp /usr/lib/x86_64-linux-gnu/libfontconfig.so.1 .
RUN cp /usr/lib/x86_64-linux-gnu/libjpeg.so.8 .
RUN cp /usr/lib/x86_64-linux-gnu/libopenjp2.so.7 .
RUN cp /usr/lib/x86_64-linux-gnu/libpng16.so.16 .
RUN cp /usr/lib/x86_64-linux-gnu/libtiff.so.5 .
RUN cp /usr/lib/x86_64-linux-gnu/libnss3.so .
RUN cp /usr/lib/x86_64-linux-gnu/libsmime3.so .
RUN cp /usr/lib/x86_64-linux-gnu/libplc4.so .
RUN cp /usr/lib/x86_64-linux-gnu/libnspr4.so .
RUN cp /usr/lib/x86_64-linux-gnu/libwebp.so.6 .
RUN cp /usr/lib/x86_64-linux-gnu/libjbig.so.0 .
RUN cp /usr/lib/x86_64-linux-gnu/libnssutil3.so .
RUN cp /usr/lib/x86_64-linux-gnu/libplds4.so .

# ==================================================================================================
# Build gtest.

# ARG GT_VERSION=1.10.0
# RUN apt-get update -y && apt-get install -y cmake g++ unzip wget
# WORKDIR /gtest
# RUN wget -q https://github.com/google/googletest/archive/release-${GT_VERSION}.zip
# RUN unzip -qq release-${GT_VERSION}.zip
# WORKDIR /gtest/googletest-release-${GT_VERSION}/googletest/bld
# RUN cmake ..
# RUN make
# RUN cp -a -r ../include/gtest /usr/local/include/.
# RUN cp -a ./lib/*.a /usr/local/lib
# RUN ldconfig

# ==================================================================================================
# Build pdftotext++.

FROM base as runtime

WORKDIR /pdftotext
COPY --from=tensorflow-builder /opt/include/ /usr/local/include/
COPY --from=tensorflow-builder /opt/lib/ /usr/local/lib/
COPY --from=utf8proc-builder /opt/include/ /usr/local/include/
COPY --from=utf8proc-builder /opt/lib/ /usr/local/lib/
COPY --from=poppler-builder /opt/include/ /usr/local/include/
COPY --from=poppler-builder /opt/lib/ /usr/local/lib/
RUN ldconfig

COPY src src
COPY Makefile.Docker Makefile
RUN make install

ENTRYPOINT ["pdftotext++"]

# ==================================================================================================
# USEFUL COMMANDS

# BUILDING DOCKER IMAGE:
# docker build -t pdftotext-plus-plus .

# EXTRACTING TEXT FROM A SINGLE PDF:
# docker run --rm -v <path-to-pdf>:/file.pdf pdftotext-plus-plus /file.pdf -