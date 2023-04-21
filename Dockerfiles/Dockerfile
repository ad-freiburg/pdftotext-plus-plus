FROM ubuntu:22.04

LABEL MAINTAINER "Claudius Korzen <korzen@cs.uni-freiburg.de>"

ENV LANG "C.UTF-8"
ENV LC_ALL "C.UTF-8"
ENV LC_CTYPE "C.UTF-8"
ENV DEBIAN_FRONTEND "noninteractive"

ENV USR_DIR "/usr/local/"

# ==================================================================================================

WORKDIR /project

COPY log.sh .
COPY install_requirements.sh .
COPY Makefile .
COPY config.yml .

# Install the requirements.
RUN apt-get update && apt-get install -y make
RUN make requirements/pre USR_DIR="$USR_DIR"
RUN make requirements/compile USR_DIR="$USR_DIR"

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