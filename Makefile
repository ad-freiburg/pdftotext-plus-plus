SRC_DIR = src
TEST_DIR = test
RESOURCES_DIR = resources

USR_DIR = usr
BUILD_DIR = build
PACKAGES_DIR = /local/data/pdftotext-plus-plus/packages

CONF_FILE = config.yml
VERSION_FILE = version.txt
PROJECT_NAME = $(shell yq ".project.name" "$(CONF_FILE)")
PROJECT_DESCRIPTION = $(shell yq ".project.description" "$(CONF_FILE)" | sed ':a;N;$$!ba;s/\n/\\n/g')  # the sed command replaces each newline by "\n".
PROJECT_USAGE = $(shell yq ".project.usage" "$(CONF_FILE)" | sed ':a;N;$$!ba;s/\n/\\n/g')
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
CXX_EXTRA = -O3 -Wall
CXX = g++ -std=c++17 $(CXX_EXTRA) -DCXX_PROGRAM_NAME="\"$(PROJECT_NAME)\"" -DCXX_PROGRAM_DESCRIPTION="\"$(PROJECT_DESCRIPTION)\"" -DCXX_PROGRAM_USAGE="\"$(PROJECT_USAGE)\"" -DCXX_PROGRAM_VERSION="\"$(VERSION)\"" -DCXX_PROJECT_RESOURCES_DIR="\"$(RESOURCES_DIR)\""
LIBS = -I$(USR_DIR)/include -L$(USR_DIR)/lib -ltensorflow_framework -ltensorflow -lpoppler -lutf8proc -lboost_program_options
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
	apt-repo/server/stop requirements/pre requirements/checkstyle requirements/compile \
	requirements/test requirements/install requirements/packages

# ==================================================================================================
# Help.

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

compile-debug: CXX_EXTRA = -g
compile-debug: compile

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

install: requirements/pre requirements/install install-without-deps

install-without-deps: clean compile
	@echo "$(INFO_STYLE)[$@] Installing pdftotext++ ...$(N)"
	mkdir -p "/usr/lib/pdftotext-plus-plus"
	cp -Pa "$(USR_DIR)/lib/." "/usr/lib/pdftotext-plus-plus"
	ldconfig "/usr/lib/pdftotext-plus-plus"
	mkdir -p "/usr/share/pdftotext-plus-plus/resources"
	cp -Pa "$(RESOURCES_DIR)/." "/usr/share/pdftotext-plus-plus/resources"
	cp "$(BUILD_DIR)/$(MAIN_BINARY)" "/usr/bin/$(MAIN_BINARY)"
	chmod +x "/usr/bin/$(MAIN_BINARY)"

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
# Releasing.

release: set-version packages apt-repo github-release

set-version:
	@if [ "$(VERSION)" = "$(shell cat "$(VERSION_FILE)")" ]; then \
		echo "$(ERROR_STYLE)The specified version is equal to the current version ($(VERSION))." ; \
		echo "$(ERROR_STYLE)Specify another version with: make release VERSION=\"<version>\"$(N)" ; \
		exit 1 ; \
	fi
	@echo "$(VERSION)" > "$(VERSION_FILE)"

packages:
	@echo "$(INFO_STYLE)[release] Building packages ...$(N)"
	./scripts/package.sh build_packages "$(CONF_FILE)" "$(VERSION)" "$(PACKAGES_DIR)"

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

github-release:
	@echo "$(INFO_STYLE)[github-release] Creating Github release ...$(N)"
	./services/github-release/github-release.sh create_release "$(VERSION)" "$(PACKAGES_DIR)"

# ==================================================================================================
# Installing requirements.

requirements/pre:
	apt-get update
	./scripts/install_requirements.sh install_yq

requirements/checkstyle:
	@echo "$(INFO_STYLE)[$@] Installing APT packages ...$(N)"
	apt-get install -y $(shell yq ".project.requirements.checkstyle.apt" "$(CONF_FILE)")

	@echo "$(INFO_STYLE)[$@] Installing other packages ...$(N)"
	@yq ".project.requirements.checkstyle.other | to_entries | .[] | [.value] | @tsv" "$(CONF_FILE)" | \
	while read -r CMD ; do \
		if [ -n "$$CMD" ]; then \
			echo $$CMD "$(USR_DIR)"; \
		fi \
	done

requirements/compile:
	@echo "$(INFO_STYLE)[$@] Installing APT packages ...$(N)"
	apt-get install -y $(shell yq ".project.requirements.compile.apt" "$(CONF_FILE)")

	@echo "$(INFO_STYLE)[$@] Installing other packages ...$(N)"
	@yq ".project.requirements.compile.other | to_entries | .[] | [.value] | @tsv" "$(CONF_FILE)" | \
	while read -r CMD ; do \
		if [ -n "$$CMD" ]; then \
			$$CMD "$(USR_DIR)"; \
		fi \
	done

requirements/test: requirements/compile
	@echo "$(INFO_STYLE)[$@] Installing APT packages ...$(N)"
	apt-get install -y $(shell yq ".project.requirements.test.apt" "$(CONF_FILE)")

	@echo "$(INFO_STYLE)[$@] Installing other packages ...$(N)"
	@yq ".project.requirements.test.other | to_entries | .[] | [.value] | @tsv" "$(CONF_FILE)" | \
	while read -r CMD ; do \
		if [ -n "$$CMD" ]; then \
			$$CMD "$(USR_DIR)"; \
		fi \
	done

requirements/install: requirements/compile
	@echo "$(INFO_STYLE)[$@] Installing APT packages ...$(N)"
	apt-get install -y $(shell yq ".project.requirements.install.apt" "$(CONF_FILE)")

	@echo "$(INFO_STYLE)[$@] Installing other packages ...$(N)"
	@yq ".project.requirements.install.other | to_entries | .[] | [.value] | @tsv" "$(CONF_FILE)" | \
	while read -r CMD ; do \
		if [ -n "$$CMD" ]; then \
			$$CMD "$(USR_DIR)"; \
		fi \
	done

requirements/packages: requirements/compile
	@echo "$(INFO_STYLE)[$@] Installing APT packages ...$(N)"
	apt-get install -y $(shell yq ".project.requirements.packages.apt" "$(CONF_FILE)")

	@echo "$(INFO_STYLE)[$@] Installing other packages ...$(N)"
	@yq ".project.requirements.packages.other | to_entries | .[] | [.value] | @tsv" "$(CONF_FILE)" | \
	while read -r CMD ; do \
		if [ -n "$$CMD" ]; then \
			$$CMD "$(USR_DIR)"; \
		fi \
	done
