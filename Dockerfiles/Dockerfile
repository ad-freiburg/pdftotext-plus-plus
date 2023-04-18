FROM ubuntu:22.04

LABEL MAINTAINER "Claudius Korzen <korzen@cs.uni-freiburg.de>"

ENV LANG "C.UTF-8"
ENV LC_ALL "C.UTF-8"
ENV LC_CTYPE "C.UTF-8"
ENV DEBIAN_FRONTEND "noninteractive"

ENV USR_DIR "/usr/local/"
ENV TF_CPP_MIN_LOG_LEVEL "3"

# ==================================================================================================

RUN apt-get update && apt-get install -y build-essential cmake git tar wget

# Install yq, needed for reading config.yml.
RUN wget https://github.com/mikefarah/yq/releases/download/v4.32.2/yq_linux_amd64 \
    -O /usr/bin/yq && chmod +x /usr/bin/yq

# ==================================================================================================

WORKDIR /project

COPY log.sh .
COPY install_requirements.sh .
COPY Makefile .
COPY config.yml .

RUN make requirements/pre USR_DIR="$USR_DIR"
RUN make requirements/run USR_DIR="$USR_DIR"
RUN ldconfig

# Compile the project and build the package.
COPY src src
COPY resources resources
COPY version.txt .
RUN make clean compile USR_DIR="$USR_DIR"

ENTRYPOINT [ "./build/pdftotext++" ]

# ==================================================================================================

# Build Docker image:
# docker build -f Dockerfiles/Dockerfile -t pdftotext-plus-plus:`cat version.txt` .
#
# Run Docker container:
# docker run --rm -it --name pdftotext-plus-plus.`cat version.txt` pdftotext-plus-plus:`cat version.txt` --version