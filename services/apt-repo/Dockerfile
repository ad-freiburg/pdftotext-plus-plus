FROM ubuntu:22.04

LABEL MAINTAINER "Claudius Korzen <korzen@cs.uni-freiburg.de>"

ENV LANG "C.UTF-8"
ENV LC_ALL "C.UTF-8"
ENV LC_CTYPE "C.UTF-8"
ENV DEBIAN_FRONTEND "noninteractive"

ENV REPO_NAME "pdftotext++"
ENV REPO_DESCRIPTION "The APT repository of pdftotext++."
ENV REPO_VERSION "1.0"
ENV REPO_MAINTAINER_NAME "Claudius Korzen"
ENV REPO_MAINTAINER_MAIL "korzen@cs.uni-freiburg.de"

ENV REPO_ROOT_DIR "/repo"
ENV REPO_POOL_DIR_REL "pool"
ENV REPO_DISTS_DIR_REL "dists"

ENV GPG_PUBLIC_KEY_FILE "/gpg/key.public"
ENV GPG_PRIVATE_KEY_FILE "/gpg/key.private"

ENV NGINX_CUSTOM_CONFIG_FILE "apt-repo.nginx.conf"

# ==================================================================================================

# Install the requirements.
RUN apt-get update && apt-get install -y gnupg gzip nginx
RUN useradd -s /bin/false nginx

# ==================================================================================================

# Create the parent directory for the public and private key file.
RUN mkdir -p "$(dirname $GPG_PUBLIC_KEY_FILE)"
RUN mkdir -p "$(dirname $GPG_PRIVATE_KEY_FILE)"

# Create the repository's pool directory and the repository's dists directory.
RUN mkdir -p "$REPO_ROOT_DIR/$REPO_POOL_DIR_REL"
RUN mkdir -p "$REPO_ROOT_DIR/$REPO_DISTS_DIR_REL"

COPY apt-repo.sh .
COPY apt-repo.nginx.conf $NGINX_CUSTOM_CONFIG_FILE

ENTRYPOINT [ "./apt-repo.sh" ]

# ==================================================================================================

# Build Docker image:
# docker build -f Dockerfile.apt-repo -t pdftotext-plus-plus.apt-repo .
#
# Run Docker container:
# docker run --rm -it -p <port>:80 --name pdftotext-plus-plus.apt-repo -v "<pool-dir>":/repo/pool -v "<dists-dir>":/repo/dists -v "<gpg-dir>":/gpg pdftotext-plus-plus.apt-repo