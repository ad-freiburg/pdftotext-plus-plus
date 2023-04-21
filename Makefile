SRC_DIR = src
TEST_DIR = test
RESOURCES_DIR = resources

USR_DIR = usr
BUILD_DIR = build
PACKAGES_DIR = /local/data/pdftotext-plus-plus/packages

CONFIG_FILE = config.yml
VERSION_FILE = version.txt
VERSION = $(shell cat $(VERSION_FILE))

MAIN_CPP_FILE = src/pdftotext++.cpp
MAIN_BINARY = $(basename $(notdir $(MAIN_CPP_FILE)))
MAIN_OBJECT_FILE = $(MAIN_CPP_FILE:%.cpp=$(BUILD_DIR)/%.o)

SRC_HEADER_FILES = $(wildcard $(SRC_DIR)/*.h $(SRC_DIR)/**/*.h)
SRC_CPP_FILES = $(filter-out %$(MAIN_CPP_FILE), $(wildcard $(SRC_DIR)/*.cpp $(SRC_DIR)/**/*.cpp))
SRC_OBJECT_FILES = $(SRC_CPP_FILES:%.cpp=$(BUILD_DIR)/%.o)

TEST_CPP_FILES = $(wildcard $(TEST_DIR)/*Test.cpp $(TEST_DIR)/**/*Test.cpp)
TEST_BINARIES = $(basename $(TEST_CPP_FILES:%.cpp=$(BUILD_DIR)/%.o))

# Compiling.
CXX = g++ -g -Wall -std=c++17 -DCXX_PROJECT_VERSION=\"$(VERSION)\" -DCXX_PROJECT_RESOURCES_DIR=\"$(RESOURCES_DIR)\"
LIBS = -I$(USR_DIR)/include -L$(USR_DIR)/lib -ltensorflow_framework -ltensorflow -lpoppler -lutf8proc
LIBS_TEST = $(LIBS) -lgtest -lgtest_main -lpthread

# APT repository.
APT_REPO_DOCKERFILE = services/apt-repo/Dockerfile
APT_REPO_IMAGE_NAME = pdftotext-plus-plus.apt-repo
APT_REPO_SERVER_CONTAINER_NAME = pdftotext-plus-plus.apt-repo.server
APT_REPO_UPDATE_CONTAINER_NAME = pdftotext-plus-plus.apt-repo.update
APT_REPO_POOL_DIR = $(PACKAGES_DIR)
APT_REPO_DISTS_DIR = /local/data/pdftotext-plus-plus/apt-repo/dists
APT_REPO_GPG_DIR = /local/data/pdftotext-plus-plus/keyrings/apt-repo
APT_REPO_SERVER_PORT = 7192

# Message styles.
INFO_STYLE = \033[34;1m
ERROR_STYLE = \033[31;1m
N = \033[0m

# ==================================================================================================

.PHONY: help checkstyle compile test release set-version packages clean apt-repo \
	apt-repo/build-docker-image apt-repo/update apt-repo/server/start apt-repo/server/start/force \
	apt-repo/server/stop requirements/pre requirements/run requirements/test requirements/pre/apt \
	requirements/run/apt requirements/test/apt requirements/pre/other requirements/run/other \
	requirements/test/other

# ==================================================================================================

help:
	@echo TODO

# ==================================================================================================
# Checkstyle.

checkstyle:
	@echo "$(INFO_STYLE)[$@] Checking style of header files ...$(N)"
	python3 cpplint.py --repository="$(SRC_DIR)" "$(SRC_DIR)"/*.h "$(SRC_DIR)"/**/*.h

	@echo "$(INFO_STYLE)[$@] Checking style of cpp files ...$(N)"
	python3 cpplint.py --repository="$(SRC_DIR)" "$(SRC_DIR)"/*.cpp "$(SRC_DIR)"/**/*.cpp

	@echo "$(INFO_STYLE)[$@] Checking style of test files ...$(N)"
	python3 cpplint.py --repository="$(TEST_DIR)" "$(TEST_DIR)"/*.cpp

# ==================================================================================================
# Compiling.

compile: $(MAIN_BINARY)

$(MAIN_BINARY): $(MAIN_OBJECT_FILE) $(SRC_OBJECT_FILES)
	@echo "$(INFO_STYLE)[compile] Creating main binary '$@' ...$(N)"
	@mkdir -p "$(dir $(BUILD_DIR)/$@)"
	$(CXX) -o "$(BUILD_DIR)/$@" $^ $(LIBS)

$(BUILD_DIR)/%.o: %.cpp $(SRC_HEADER_FILES)
	@echo "$(INFO_STYLE)[compile] Compiling '$<' ...$(N)"
	@mkdir -p "$(dir $@)"
	$(CXX) -c $< -o "$@" $(LIBS)

# ==================================================================================================
# Testing.

test: $(TEST_BINARIES)
	@for T in $(TEST_BINARIES); do \
		echo "$(INFO_STYLE)[$@] Running test '$$T' ...$(N)" ; \
    export TF_CPP_MIN_LOG_LEVEL=3 ; \
		./$$T || exit; \
	done

%Test: %Test.o $(SRC_OBJECT_FILES)
	@echo "$(INFO_STYLE)[test] Creating test '$@' ...$(N)"
	@mkdir -p "$(dir $@)"
	$(CXX) -o "$@" $^ $(LIBS_TEST)

$(BUILD_DIR)/%Test.o: %Test.cpp $(SRC_HEADER_FILES)
	@echo "$(INFO_STYLE)[test] Compiling '$<' ...$(N)"
	@mkdir -p "$(dir $@)"
	$(CXX) -c $< -o "$@" $(LIBS_TEST)

# ==================================================================================================
# Installing.

install:
	@echo "$(INFO_STYLE)[$@] Installing pdftotext++ ...$(N)"
	apt-get update && apt-get install -y build-essential cmake git tar wget
	wget https://github.com/mikefarah/yq/releases/latest/download/yq_linux_amd64 -O /usr/bin/yq && \
		chmod +x /usr/bin/yq
	make requirements/pre USR_DIR="$(USR_DIR)"
	make requirements/run USR_DIR="$(USR_DIR)"
	ldconfig "$(USR_DIR)"
	make clean compile USR_DIR="$(USR_DIR)" RESOURCES_DIR="$(RESOURCES_DIR)"
	mkdir -p "/usr/lib/pdftotext-plus-plus"
	cp -Pa "$(USR_DIR)/lib/." "/usr/lib/pdftotext-plus-plus"
	ldconfig "/usr/lib/pdftotext-plus-plus"
	mkdir -p "/usr/share/pdftotext-plus-plus/resources"
	cp -Pa "$(RESOURCES_DIR)/." "/usr/share/pdftotext-plus-plus/resources"
	cp "$(BUILD_DIR)/$(MAIN_BINARY)" "/usr/bin/$(MAIN_BINARY)"
	chmod +x "/usr/bin/$(MAIN_BINARY)"

# ==================================================================================================
# Releasing.

release: set-version packages apt-repo

set-version:
	@if [ "$(VERSION)" = "$(shell cat "$(VERSION_FILE)")" ]; then \
		echo "$(ERROR_STYLE)Usage: make release VERSION=\"<version>\"$(N)" ; \
		exit 1 ; \
	fi
	@echo "$(VERSION)" > "$(VERSION_FILE)"

packages:
	@echo "$(INFO_STYLE)[release] Building packages ...$(N)"
	./package.sh build_packages "$(CONFIG_FILE)" "$(VERSION)" "$(PACKAGES_DIR)"

# ==================================================================================================
# Cleaning.

clean:
	@echo "$(INFO_STYLE)[$@] Cleaning the project ...$(N)"
	rm -f core
	rm -rf $(BUILD_DIR)
	rm -f $(TEST_DIR)/*.aux
	rm -f $(TEST_DIR)/*.fdb_latexmk
	rm -f $(TEST_DIR)/*.fls
	rm -f $(TEST_DIR)/*.log
	rm -f $(TEST_DIR)/*.synctex.gz

# ==================================================================================================
# Services.

apt-repo: apt-repo/update apt-repo/server/start

apt-repo/build-docker-image:
	@echo "$(INFO_STYLE)[apt-repo] Building a Docker image from '$(APT_REPO_DOCKERFILE)' ...$(N)"
	cd "$(dir $(APT_REPO_DOCKERFILE))" && \
	docker build -f "$(notdir $(APT_REPO_DOCKERFILE))" -t "$(APT_REPO_IMAGE_NAME)" .

apt-repo/update: apt-repo/build-docker-image
	@echo "$(INFO_STYLE)[apt-repo] Updating the APT repository ...$(N)"
	docker run --rm -it \
		  --name "$(APT_REPO_UPDATE_CONTAINER_NAME)" \
		  -v "$(APT_REPO_POOL_DIR)":/repo/pool \
		  -v "$(APT_REPO_DISTS_DIR)":/repo/dists \
		  -v "$(APT_REPO_GPG_DIR)":/gpg \
		  "$(APT_REPO_IMAGE_NAME)" update

apt-repo/server/start: apt-repo/build-docker-image
	@echo "$(INFO_STYLE)[apt-repo] Starting the APT repository server ...$(N)"
	@if [ "$(shell docker ps -f 'name=$(APT_REPO_SERVER_CONTAINER_NAME)' --format '{{.Names}}')" = \
			"$(APT_REPO_SERVER_CONTAINER_NAME)" ]; then \
    echo "Server is already running."; \
		echo "To force restarting the server, type 'make apt-repo/server/start/force'"; \
	else \
		docker run --rm -it -d \
			--name "$(APT_REPO_SERVER_CONTAINER_NAME)" \
			-p "$(APT_REPO_SERVER_PORT)":80 \
			-v "$(APT_REPO_POOL_DIR)":/repo/pool \
			-v "$(APT_REPO_DISTS_DIR)":/repo/dists \
			-v "$(APT_REPO_GPG_DIR)":/gpg \
			"$(APT_REPO_IMAGE_NAME)" start_server ; \
	fi

apt-repo/server/start/force: apt-repo/server/stop apt-repo/server/start

apt-repo/server/stop:
	@echo "$(INFO_STYLE)[apt-repo] Stopping the APT repository server ...$(N)"

	docker stop "$(APT_REPO_SERVER_CONTAINER_NAME)"

# ==================================================================================================
# Installing requirements.

requirements/pre: requirements/pre/apt requirements/pre/other
requirements/run: requirements/run/apt requirements/run/other
requirements/test: requirements/test/apt requirements/test/other

requirements/pre/apt:
	@echo "$(INFO_STYLE)[requirements] Installing APT packages needed as a prerequisite ...$(N)"
	apt-get install -y $(shell yq ".project.requirements.pre.apt" "$(CONFIG_FILE)")

requirements/run/apt:
	@echo "$(INFO_STYLE)[requirements] Installing APT packages needed for running ...$(N)"
	apt-get install -y $(shell yq ".project.requirements.run.apt" "$(CONFIG_FILE)")

requirements/test/apt:
	@echo "$(INFO_STYLE)[requirements] Installing APT packages needed for testing ...$(N)"
	apt-get install -y $(shell yq ".project.requirements.test.apt" "$(CONFIG_FILE)")

requirements/pre/other:
	@echo "$(INFO_STYLE)[requirements] Installing other packages needed as a prerequisite ...$(N)"

	@yq ".project.requirements.pre.other | to_entries | .[] | [.value] | @tsv" "$(CONFIG_FILE)" | \
	while read -r CMD ; do \
		if [ -n "$$CMD" ]; then \
			$$CMD "$(USR_DIR)"; \
		fi \
	done

requirements/run/other:
	@echo "$(INFO_STYLE)[requirements] Installing other packages needed for running ...$(N)"

	@yq ".project.requirements.run.other | to_entries | .[] | [.value] | @tsv" "$(CONFIG_FILE)" | \
	while read -r CMD ; do \
		if [ -n "$$CMD" ]; then \
			$$CMD "$(USR_DIR)"; \
		fi \
	done

requirements/test/other:
	@echo "$(INFO_STYLE)[requirements] Installing other packages needed for testing ...$(N)"

	@yq ".project.requirements.test.other | to_entries | .[] | [.value] | @tsv" "$(CONFIG_FILE)" | \
	while read -r CMD ; do \
		if [ -n "$$CMD" ]; then \
			$$CMD "$(USR_DIR)"; \
		fi \
	done
